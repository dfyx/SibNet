#include <blocksocket.h>
#include <sched.h>
#include <network_wrap.h>
#include <pthread.h>
#include <semaphore.h>

#ifdef _DEBUG
#include <iostream>
#include <windows.h>
#endif

using namespace std;

struct BlockSocketData
{
	Net::Socket m_inSocket;
	pthread_t m_iWriteThread, m_iReadThread;

	pthread_mutex_t m_iWriteMutex, m_iReadMutex;
	sem_t m_iWriteSemaphore, m_iReadSemaphore;
};

BlockSocket::BlockSocket()
{
	// Initialize HawkNL
	if(!Net::init())
	{
		DEBUG_ERROR("Could not init HawkNL.")
	}

	m_psData->m_inSocket = -1;
	m_bConnected = false;
	m_bDisconnect = false;

	pthread_mutex_init(&m_psData->m_iWriteMutex, NULL);
	pthread_mutex_init(&m_psData->m_iReadMutex, NULL);
	sem_init(&m_psData->m_iWriteSemaphore, 0, 0);
	sem_init(&m_psData->m_iReadSemaphore, 0, 0);
}

BlockSocket::BlockSocket(int p_inSocket)
{
	// Initialize network
	if(!Net::init())
	{
		DEBUG_ERROR("Could not init network.")
	}

	m_psData = new BlockSocketData;

	m_psData->m_inSocket = (Net::Socket) p_inSocket;
	m_bConnected = true;
	m_bDisconnect = false;

	pthread_mutex_init(&m_psData->m_iWriteMutex, NULL);
	pthread_mutex_init(&m_psData->m_iReadMutex, NULL);

	// Start worker threads
	pthread_create(&m_psData->m_iWriteThread, NULL, BlockSocket::WriteLoop, (void*) this);
	pthread_create(&m_psData->m_iReadThread, NULL, BlockSocket::ReadLoop, (void*) this);
	sem_init(&m_psData->m_iWriteSemaphore, 0, 0);
	sem_init(&m_psData->m_iReadSemaphore, 0, 0);
}

BlockSocket::~BlockSocket()
{
	// Close socket
	Net::close(m_psData->m_inSocket);

	if(IsConnected())
	{
		Disconnect();
		pthread_join(m_psData->m_iWriteThread, NULL);
	}

	pthread_mutex_destroy(&m_psData->m_iWriteMutex);
	pthread_mutex_destroy(&m_psData->m_iReadMutex);
	sem_destroy(&m_psData->m_iWriteSemaphore);
	sem_destroy(&m_psData->m_iReadSemaphore);

	delete m_psData;

	// This is okay, even for multiple sockets. There is an internal counter
	Net::shutdown();
}

bool BlockSocket::Connect(string p_strAddress, uint16_t p_sPort)
{
	// Close old socket. If there is no old socket, nothing happens
	Net::close(m_psData->m_inSocket);
	m_bConnected = false;

	// Open new socket
	m_psData->m_inSocket = Net::connect(p_strAddress, p_sPort, Net::TYPE_TCP);
	if(m_psData->m_inSocket == -1)
	{
		DEBUG_ERROR("Could not open socket.")
		return false;
	}

	// Stop existing worker threads
	m_bConnected = true;

	// Start worker threads
	pthread_create(&m_psData->m_iWriteThread, NULL, BlockSocket::WriteLoop, (void*) this);
	pthread_create(&m_psData->m_iReadThread, NULL, BlockSocket::ReadLoop, (void*) this);

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
	pthread_mutex_lock(&m_psData->m_iWriteMutex);
	m_inWriteQueue.push(p_pinBlock);
	pthread_mutex_unlock(&m_psData->m_iWriteMutex);
	sem_post(&m_psData->m_iWriteSemaphore);
}

Block* BlockSocket::ReadBlock(bool p_bWait)
{
	Block *pinResult = NULL;
	if(p_bWait)
	{
		sem_wait(&m_psData->m_iReadSemaphore);
		pthread_mutex_lock(&m_psData->m_iReadMutex);
		pinResult = m_inReadQueue.front();
		m_inReadQueue.pop();
		pthread_mutex_unlock(&m_psData->m_iReadMutex);
	}
	else if(sem_trywait(&m_psData->m_iReadSemaphore) == 0)
	{
		pthread_mutex_lock(&m_psData->m_iReadMutex);
		pinResult = m_inReadQueue.front();
		m_inReadQueue.pop();
		pthread_mutex_unlock(&m_psData->m_iReadMutex);
	}

	return pinResult;
}

void *BlockSocket::WriteLoop(void *p_pinSocket)
{
#ifdef _DEBUG
	cout << "- Starting write loop." << endl;

#endif
	BlockSocket *pinSocket = (BlockSocket*) p_pinSocket;
	pthread_t iReadThread = pinSocket->m_psData->m_iReadThread;

	while(pinSocket->IsConnected())

	{
		Block *pinBlock = NULL;

		// Wait until there is something in the queue
		sem_wait(&pinSocket->m_psData->m_iWriteSemaphore);
		pthread_mutex_lock(&pinSocket->m_psData->m_iWriteMutex);
		pinBlock = pinSocket->m_inWriteQueue.front();
		pinSocket->m_inWriteQueue.pop();
		pthread_mutex_unlock(&pinSocket->m_psData->m_iWriteMutex);
		

		if(pinBlock == NULL)
		{
			if(pinSocket->m_bDisconnect)
			{
				pinSocket->m_bConnected = false;
				Net::close(pinSocket->m_psData->m_inSocket);

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
			blocksize_t iSize;
			
			iSize = pinBlock->Serialize(&pcBuffer);

			// Write type id
			size_t iWritten = 0;
			uint16_t sType = swap(pinBlock->GetTypeID());
			while(iWritten < 2)
			{
				int iWriteResult = Net::send(pinSocket->m_psData->m_inSocket, &sType + iWritten, 2 - iWritten);
				if(iWriteResult >= 0)
				{
					iWritten += iWriteResult;
				}
				else
				{
					DEBUG_ERROR("Error writing block type.")
				}

				// Could not write everything: wait
				if(iWritten < 2)
				{
					sched_yield();
				}
			}

			// Write size
			iWritten = 0;
			blocksize_t sSize = swap(iSize);
			while(iWritten < sizeof(blocksize_t))
			{
				int iWriteResult = Net::send(pinSocket->m_psData->m_inSocket, &sSize + iWritten, 2 - iWritten);
				if(iWriteResult >= 0)
				{
					iWritten += iWriteResult;
				}
				else
				{
					DEBUG_ERROR("Error writing block size.")
				}

				// Could not write everything: wait
				if(iWritten < sizeof(blocksize_t))
				{
					sched_yield();
				}
			}

			// Write data
			iWritten = 0;
			while(iWritten < iSize)
			{
				int iWriteResult = Net::send(pinSocket->m_psData->m_inSocket, pcBuffer + iWritten, iSize - iWritten);
				if(iWriteResult >= 0)
				{
					iWritten += iWriteResult;
				}
				else
				{
					DEBUG_ERROR("Error writing block data.")
				}

				// Could not write everything: wait
				if(iWritten < iSize)
				{
					sched_yield();
				}
			}

			// Clean up
			delete pcBuffer;
			delete pinBlock;
		}
	}

	return NULL;
}

void *BlockSocket::ReadLoop(void *p_pinSocket)
{
#ifdef _DEBUG
	cout << "- Starting read loop." << endl;
#endif
	BlockSocket *pinSocket = (BlockSocket*) p_pinSocket;

	// TODO: stop thread if socket is disconnected
	while(pinSocket->IsConnected())
	{
		// Read type
		size_t iRead = 0;
		uint16_t sType;
		while(iRead < 2)
		{
#ifdef _DEBUG
			cout << "- Starting to read type." << endl;
#endif
			iRead += Net::read(pinSocket->m_psData->m_inSocket, &sType + iRead, 2 - iRead);

			// Could not read everything: wait
			if(iRead < 2)
			{
				sched_yield();
			}
		}
		sType = swap(sType);

		// Read size
		iRead = 0;
		blocksize_t sSize;
		while(iRead < sizeof(blocksize_t))
		{
#ifdef _DEBUG
			cout << "- Starting to read size." << endl;
#endif
			iRead += Net::read(pinSocket->m_psData->m_inSocket, &sSize + iRead, 2 - iRead);

			// Could not read everything: wait
			if(iRead < sizeof(blocksize_t))
			{
				sched_yield();
			}
		}
		blocksize_t iSize = swap(sSize);

		// Read data
		char *pcBuffer = new char[iSize];
		iRead = 0;
		while(iRead < iSize)
		{
#ifdef _DEBUG
			cout << "- Starting to read data." << endl;
#endif
			iRead += Net::read(pinSocket->m_psData->m_inSocket, pcBuffer + iRead, iSize - iRead);

			// Could not read everything: wait
			if(iRead < iSize)
			{
				sched_yield();
			}
		}

		// Convert
		Block *pinBlock = Block::GetBlockFromTypeID(sType);
		pinBlock->Deserialize(pcBuffer, iSize);

		// Add to queue
		pthread_mutex_lock(&pinSocket->m_psData->m_iReadMutex);
		pinSocket->m_inReadQueue.push(pinBlock);
		pthread_mutex_unlock(&pinSocket->m_psData->m_iReadMutex);
		sem_post(&pinSocket->m_psData->m_iReadSemaphore);

		// Clean up
		delete pcBuffer;
	}

	return NULL;
}