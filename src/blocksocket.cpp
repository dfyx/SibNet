#include <blocksocket.h>
#include <sched.h>

#ifdef _DEBUG
#include <iostream>
#endif

using namespace std;

BlockSocket::BlockSocket()
{
	// Initialize HawkNL
	if(!nlInit())
	{
		DEBUG_ERROR("Could not init HawkNL.")
	}

	nlSelectNetwork(NL_IP);
	nlEnable(NL_BLOCKING_IO);

	m_inSocket = NL_INVALID;
	m_bConnected = false;
	m_bDisconnect = false;

	pthread_mutex_init(&m_iWriteMutex, NULL);
	pthread_mutex_init(&m_iReadMutex, NULL);
}

BlockSocket::BlockSocket(NLsocket p_inSocket)
{
	// Initialize HawkNL to make sure the destructor doesn't cause problems
	if(!nlInit())
	{
		DEBUG_ERROR("Could not init HawkNL.")
	}

	m_inSocket = p_inSocket;
	m_bConnected = true;
	m_bDisconnect = false;

	pthread_mutex_init(&m_iWriteMutex, NULL);
	pthread_mutex_init(&m_iReadMutex, NULL);

	// Start worker threads
	pthread_create(&m_iWriteThread, NULL, BlockSocket::WriteLoop, (void*) this);
	pthread_create(&m_iReadThread, NULL, BlockSocket::ReadLoop, (void*) this);
}

BlockSocket::~BlockSocket()
{
	// Close socket
	nlClose(m_inSocket);

	if(IsConnected())
	{
		pthread_join(m_iWriteThread, NULL);
	}

	pthread_mutex_destroy(&m_iWriteMutex);
	pthread_mutex_destroy(&m_iReadMutex);

	// This is okay, even for multiple sockets. HawkNL has an internal counter
	nlShutdown();
}

bool BlockSocket::Connect(string p_strAddress, uint16_t p_sPort)
{
	// Close old socket. If there is no old socket, nothing happens
	nlClose(m_inSocket);
	m_bConnected = false;

	// Resolve address
	NLaddress inAddress;
	if(!nlGetAddrFromName((const NLchar*) p_strAddress.c_str(), &inAddress))
	{
		DEBUG_ERROR("Could not resolve address.")
		return false;
	}

	nlSetAddrPort(&inAddress, p_sPort);

	// Open new socket
	m_inSocket = nlOpen(0, NL_RELIABLE);
	if(m_inSocket == NL_INVALID)
	{
		DEBUG_ERROR("Could not open socket.")
		return false;
	}

	// Connect
	if(!nlConnect(m_inSocket, &inAddress))
	{
		DEBUG_ERROR("Could not connect.")
		return false;
	}

	// Stop existing worker threads
	m_bConnected = true;

	// Start worker threads
	pthread_create(&m_iWriteThread, NULL, BlockSocket::WriteLoop, (void*) this);
	pthread_create(&m_iReadThread, NULL, BlockSocket::ReadLoop, (void*) this);

	// TODO: check protocol version
	return true;
}

void BlockSocket::Disconnect()
{
	m_bDisconnect = true;
}

bool BlockSocket::IsConnected()
{
	return m_bConnected;
}

void BlockSocket::WriteBlock(Block *p_pinBlock)
{
#ifdef _DEBUG
	cout << "- Adding block to write queue." << endl;
#endif
	pthread_mutex_lock(&m_iWriteMutex);
	m_inWriteQueue.push(p_pinBlock);
	pthread_mutex_unlock(&m_iWriteMutex);
}

Block* BlockSocket::ReadBlock(bool p_bWait)
{
	Block *pinResult = NULL;
	do
	{
		pthread_mutex_lock(&m_iReadMutex);
		if(!m_inReadQueue.empty())
		{
			pinResult = m_inReadQueue.front();
			m_inReadQueue.pop();
		}
		pthread_mutex_unlock(&m_iReadMutex);
	}
	while(pinResult == NULL && p_bWait == true);

	return pinResult;
}

void *BlockSocket::WriteLoop(void *p_pinSocket)
{
#ifdef _DEBUG
	cout << "- Starting write loop." << endl;
#endif
	BlockSocket *pinSocket = (BlockSocket*) p_pinSocket;
	pthread_t iReadThread = pinSocket->m_iReadThread;

	while(pinSocket->IsConnected())
	{
		Block *pinBlock = NULL;
		pthread_mutex_lock(&pinSocket->m_iWriteMutex);
		if(!pinSocket->m_inWriteQueue.empty())
		{
			pinBlock = pinSocket->m_inWriteQueue.front();
			pinSocket->m_inWriteQueue.pop();
		}
		pthread_mutex_unlock(&pinSocket->m_iWriteMutex);

		if(pinBlock == NULL)
		{
			if(pinSocket->m_bDisconnect)
			{
				pinSocket->m_bConnected = false;
				nlClose(pinSocket->m_inSocket);

				// Stop read loop
				pthread_cancel(iReadThread);
				return NULL;
			}
			else
			{
				sched_yield();
			}
		}
		else
		{
#ifdef _DEBUG
			cout << "- Starting to send block." << endl;
#endif
			char* pcBuffer;
			size_t iSize;
			
			iSize = pinBlock->Serialize(&pcBuffer);

			// Write type id
			size_t iWritten = 0;
			uint16_t sType = nlSwaps(pinBlock->GetTypeID());
			while(iWritten < 2)
			{
				int iWriteResult = nlWrite(pinSocket->m_inSocket, &sType + iWritten, 2 - iWritten);
				if(iWriteResult >= 0)
				{
					iWritten += iWriteResult;
				}
				else
				{
					DEBUG_ERROR("Error writing block type.")
				}
			}

			// Write size
			iWritten = 0;
			uint16_t sSize = nlSwaps(iSize);
			while(iWritten < 2)
			{
				int iWriteResult = nlWrite(pinSocket->m_inSocket, &sSize + iWritten, 2 - iWritten);
				if(iWriteResult >= 0)
				{
					iWritten += iWriteResult;
				}
				else
				{
					DEBUG_ERROR("Error writing block size.")
				}
			}

			// Write data
			iWritten = 0;
			while(iWritten < iSize)
			{
				int iWriteResult = nlWrite(pinSocket->m_inSocket, pcBuffer + iWritten, iSize - iWritten);
				if(iWriteResult >= 0)
				{
					iWritten += iWriteResult;
				}
				else
				{
					DEBUG_ERROR("Error writing block data.")
				}
			}

			// Clean up
			delete pcBuffer;
			delete pinBlock;

			sched_yield();
		}
	}

	return NULL;
}

void *BlockSocket::ReadLoop(void *p_pinSocket)
{
	BlockSocket *pinSocket = new BlockSocket(); //(BlockSocket*) p_pinSocket;

	// TODO: stop thread if socket is disconnected
	while(pinSocket->IsConnected())
	{
		// Read type
		size_t iRead = 0;
		uint16_t sType;
		while(iRead < 2)
		{
			iRead += nlRead(pinSocket->m_inSocket, &sType + iRead, 2 - iRead);
		}
		sType = nlSwaps(sType);

		// Read size
		iRead = 0;
		uint16_t sSize;
		while(iRead < 2)
		{
			iRead += nlRead(pinSocket->m_inSocket, &sSize + iRead, 2 - iRead);
		}
		size_t iSize = nlSwaps(sSize);

		// Read data
		char *pcBuffer = new char[iSize];
		iRead = 0;
		while(iRead < iSize)
		{
			iRead += nlRead(pinSocket->m_inSocket, pcBuffer + iRead, iSize - iRead);
		}

		// Convert
		Block *pinBlock = Block::GetBlockFromTypeID(sType);
		pinBlock->Deserialize(pcBuffer, iSize);

		// Add to queue
		pthread_mutex_lock(&pinSocket->m_iReadMutex);
		pinSocket->m_inReadQueue.push(pinBlock);
		pthread_mutex_unlock(&pinSocket->m_iReadMutex);

		// Clean up
		delete pcBuffer;
	}

	return NULL;
}