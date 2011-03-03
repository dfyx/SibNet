#include <blocksocket.h>
#include <mutexhelper.h>
#include <sched.h>
#include <network_wrap.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

using namespace std;

struct BlockSocketData
{
	Net::Socket m_inSocket;
	pthread_t m_iWriteThread, m_iReadThread;

	pthread_mutex_t m_iWriteMutex, m_iReadMutex, m_iStateMutex;
	pthread_cond_t m_iWriteCond, m_iReadCond;
};

BlockSocket::BlockSocket()
{
	// Initialize network
	if(!Net::init())
	{
		DEBUG_ERROR("Could not init network.")
	}

    m_psData = new BlockSocketData;

	m_psData->m_inSocket = -1;
	m_eState = STATE_DISCONNECTED;

	pthread_mutex_init(&m_psData->m_iWriteMutex, NULL);
	pthread_mutex_init(&m_psData->m_iReadMutex, NULL);
	pthread_mutex_init(&m_psData->m_iStateMutex, NULL);
	pthread_cond_init(&m_psData->m_iWriteCond, NULL);
	pthread_cond_init(&m_psData->m_iReadCond, NULL);
}

BlockSocket::BlockSocket(int p_inSocket)
{
	// Initialize network. This is just to increase the internal counter
	if(!Net::init())
	{
		DEBUG_ERROR("Could not init network.")
	}

	m_psData = new BlockSocketData;

	m_psData->m_inSocket = (Net::Socket) p_inSocket;
	m_eState = STATE_CONNECTED;

	pthread_mutex_init(&m_psData->m_iWriteMutex, NULL);
	pthread_mutex_init(&m_psData->m_iReadMutex, NULL);
	pthread_mutex_init(&m_psData->m_iStateMutex, NULL);
	pthread_cond_init(&m_psData->m_iWriteCond, NULL);
	pthread_cond_init(&m_psData->m_iReadCond, NULL);

	// Start worker threads
	pthread_create(&m_psData->m_iWriteThread, NULL, BlockSocket::WriteLoop, (void*) this);
	pthread_create(&m_psData->m_iReadThread, NULL, BlockSocket::ReadLoop, (void*) this);
}

BlockSocket::~BlockSocket()
{
	Disconnect(QUEUE_CLEAR);
	ClearWriteQueue();
	ClearReadQueue();

	pthread_mutex_destroy(&m_psData->m_iWriteMutex);
	pthread_mutex_destroy(&m_psData->m_iReadMutex);
	pthread_mutex_destroy(&m_psData->m_iStateMutex);
	pthread_cond_destroy(&m_psData->m_iWriteCond);
	pthread_cond_destroy(&m_psData->m_iReadCond);

	delete m_psData;

	// This is okay, even for multiple sockets. There is an internal counter
	Net::shutdown();
}

bool BlockSocket::Connect(string p_strAddress, uint16_t p_sPort)
{
	MutexHelper inStateHelper(&m_psData->m_iStateMutex);
	
	Disconnect(QUEUE_CLEAR);
	ClearReadQueue();
	
	m_eState = STATE_CONNECTING;

	// Open new socket
	m_psData->m_inSocket = Net::connect(p_strAddress, p_sPort, Net::TYPE_TCP);
	if(m_psData->m_inSocket == -1)
	{
		DEBUG_ERROR("Could not open socket.");
		m_eState = STATE_DISCONNECTED;
		return false;
	}

	// Start worker threads
	pthread_create(&m_psData->m_iWriteThread, NULL, BlockSocket::WriteLoop, (void*) this);
	pthread_create(&m_psData->m_iReadThread, NULL, BlockSocket::ReadLoop, (void*) this);
	
	m_eState = STATE_CONNECTED;

	// TODO: check protocol version
	return true;
}

void BlockSocket::Disconnect(QueueBehavior p_eWriteBehavior)
{
	MutexHelper inStateHelper(&m_psData->m_iStateMutex);
	
	if(m_eState != STATE_DISCONNECTED)
	{
		m_eState = STATE_DISCONNECTING;
		m_eWriteBehavior = p_eWriteBehavior;
		
		pthread_cancel(m_psData->m_iReadThread);
		
		if(p_eWriteBehavior != QUEUE_FLUSH_ASYNC)
		{
			// Unlock semaphore so write thread may exit
			pthread_cond_signal(&m_psData->m_iWriteCond);
			pthread_join(m_psData->m_iWriteThread, NULL);
			ClearWriteQueue();
			
			Net::close(m_psData->m_inSocket);
			m_eState = STATE_DISCONNECTED;
		}
	}
}

bool BlockSocket::IsConnected()
{
	return m_eState == STATE_CONNECTED;
}

void BlockSocket::WriteBlock(Block *p_pinBlock)
{
	if(p_pinBlock == NULL)
	{
		return;
	}
	
	DEBUG_NOTICE("Adding block to write queue.");
	
	pthread_mutex_lock(&m_psData->m_iWriteMutex);
	m_inWriteQueue.push(p_pinBlock);
	pthread_cond_signal(&m_psData->m_iWriteCond);
	pthread_mutex_unlock(&m_psData->m_iWriteMutex);
}

Block* BlockSocket::ReadBlock(bool p_bWait)
{
	Block *pinResult = NULL;
	
	pthread_mutex_lock(&m_psData->m_iReadMutex);
	if(!m_inReadQueue.empty())
	{
		// if there already is something in the queue, just return it
		pinResult = m_inReadQueue.front();
		m_inReadQueue.pop();
	}
	else if(p_bWait)
	{
		// Else wait if user wants to
		if(pthread_cond_wait(&m_psData->m_iReadCond, &m_psData->m_iReadMutex) == 0)
		{
		    pinResult = m_inReadQueue.front();
			m_inReadQueue.pop();
		}
	}	
	pthread_mutex_unlock(&m_psData->m_iReadMutex);

	return pinResult;
}

void BlockSocket::ClearReadQueue()
{
	MutexHelper inStateHelper(&m_psData->m_iStateMutex);
	
	// Don't allow this while there is a connection or you might get strange
	// results
	if(IsConnected())
	{
		return;
	}
	
	pthread_mutex_lock(&m_psData->m_iReadMutex);	
	// There should be a more elegant way but I couldn't find one in the header
	DEBUG_NOTICE("Removing " << m_inReadQueue.size() << " objects from read queue");
	while(!m_inReadQueue.empty())
	{
		m_inReadQueue.pop();
	}
	pthread_mutex_unlock(&m_psData->m_iReadMutex);
}

void BlockSocket::ClearWriteQueue()
{
	MutexHelper inStateHelper(&m_psData->m_iStateMutex);
	
	// Don't allow this while there is a connection or you might get strange
	// results
	if(IsConnected() || 
	   (m_eState == STATE_DISCONNECTING && m_eWriteBehavior != QUEUE_CLEAR))
	{
		return;
	}
	
	pthread_mutex_lock(&m_psData->m_iWriteMutex);	
	// There should be a more elegant way but I couldn't find one in the header
	DEBUG_NOTICE("Removing " << m_inWriteQueue.size() << " objects from write queue");
	while(!m_inWriteQueue.empty())
	{
		m_inWriteQueue.pop();
	}
	pthread_mutex_unlock(&m_psData->m_iWriteMutex);
}

void *BlockSocket::WriteLoop(void *p_pinSocket)
{
    DEBUG_NOTICE("Starting write loop.");
	BlockSocket *pinSocket = (BlockSocket*) p_pinSocket;
	pthread_t iReadThread = pinSocket->m_psData->m_iReadThread;

	while(pinSocket->m_eState == STATE_CONNECTING)
	{
		sched_yield();
	}
	
	while(pinSocket->IsConnected() ||
		  (pinSocket->m_eState == STATE_DISCONNECTING && pinSocket->m_eWriteBehavior != QUEUE_CLEAR))
	{
		Block *pinBlock = NULL;

		pthread_mutex_lock(&pinSocket->m_psData->m_iWriteMutex);
		if(pinSocket->m_inWriteQueue.empty())
		{
			// Wait if queue is empty
			if(pthread_cond_wait(&pinSocket->m_psData->m_iWriteCond, &pinSocket->m_psData->m_iWriteMutex) != 0)
			{
				// Something went wrong, disconnect and return
				DEBUG_ERROR("Error waiting for write queue");
				pinSocket->Disconnect(QUEUE_CLEAR);
	            return NULL;
			}
		}
		
		// Check if still empty
		if(pinSocket->m_inWriteQueue.empty())
		{
			// empty queue usually means disconnect, just let the loop condition do its work
			pthread_mutex_unlock(&pinSocket->m_psData->m_iWriteMutex);
			continue;
		}
		else
		{
			pinBlock = pinSocket->m_inWriteQueue.front();
			pinSocket->m_inWriteQueue.pop();
		}
		pthread_mutex_unlock(&pinSocket->m_psData->m_iWriteMutex);
		
		if(pinBlock == NULL)
		{
			// Should never happen, but who knows...
			continue;
		}
		else
		{
			char* pcBuffer;
			blocksize_t iSize;
			
			iSize = pinBlock->Serialize(&pcBuffer);

			// Write type id
			size_t iWritten = 0;
			uint16_t sType = swap(pinBlock->GetTypeID());
			
			DEBUG_NOTICE("Starting to send block type.")
			while(iWritten < 2)
			{
				int iWriteResult = Net::send(pinSocket->m_psData->m_inSocket, (const char*) &sType + iWritten, 2 - iWritten);
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
			
			DEBUG_NOTICE("Starting to send block size.")
			while(iWritten < sizeof(blocksize_t))
			{
				int iWriteResult = Net::send(pinSocket->m_psData->m_inSocket, (const char*) &sSize + iWritten, 2 - iWritten);
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
			
			DEBUG_NOTICE("Starting to send block data.")
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
			
			DEBUG_NOTICE("Finished writing block.")

			// Clean up
			delete pcBuffer;
			delete pinBlock;
		}
	}

	return NULL;
}

void *BlockSocket::ReadLoop(void *p_pinSocket)
{
	DEBUG_NOTICE("Starting read loop.")
	BlockSocket *pinSocket = (BlockSocket*) p_pinSocket;

	while(pinSocket->m_eState == STATE_CONNECTING)
	{
		sched_yield();
	}
	
	// TODO: stop thread if socket is disconnected
	while(pinSocket->IsConnected())
	{
		// Read type
		size_t iRead = 0;
		uint16_t sType;
		
		DEBUG_NOTICE("Starting to read type.")
		while(iRead < 2)
		{
			int iReadResult = Net::recv(pinSocket->m_psData->m_inSocket, (char*) &sType + iRead, 2 - iRead);
			
			if(iReadResult > 0)
			{
                iRead += iReadResult;
			}
			else if(iReadResult == 0)
			{
				DEBUG_NOTICE("Connection closed by peer")
				pinSocket->Disconnect(QUEUE_CLEAR);
                return NULL;
			}
			else
			{
			    DEBUG_ERROR("Error reading block type.")
			}

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
		
		DEBUG_NOTICE("Starting to read size")
		while(iRead < sizeof(blocksize_t))
		{
			int iReadResult = Net::recv(pinSocket->m_psData->m_inSocket, (char*) &sSize + iRead, 2 - iRead);

            if(iReadResult > 0)
			{
                iRead += iReadResult;
			}
			else if(iReadResult == 0)
			{
				DEBUG_NOTICE("Connection closed by peer")
				pinSocket->Disconnect(QUEUE_CLEAR);
                return NULL;
			}
			else
			{
			    DEBUG_ERROR("Error reading block size.")
			}
			
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
		
		DEBUG_NOTICE("Starting to read data.")
		while(iRead < iSize)
		{
			int iReadResult = Net::recv(pinSocket->m_psData->m_inSocket, pcBuffer + iRead, iSize - iRead);

            if(iReadResult > 0)
			{
                iRead += iReadResult;
			}
			else if(iReadResult == 0)
			{
				DEBUG_NOTICE("Connection closed by peer")
				delete[] pcBuffer;
				pinSocket->Disconnect(QUEUE_CLEAR);
                return NULL;
			}
			else
			{
			    DEBUG_ERROR("Error reading block data.")
			}
			
			// Could not read everything: wait
			if(iRead < iSize)
			{
				sched_yield();
			}
		}

		// Convert
		Block *pinBlock = Block::GetBlockFromTypeID(sType);
		if(pinBlock)
		{
		    pinBlock->Deserialize(pcBuffer, iSize);

    		// Add to queue
    		pthread_mutex_lock(&pinSocket->m_psData->m_iReadMutex);
    		pinSocket->m_inReadQueue.push(pinBlock);
			pthread_cond_signal(&pinSocket->m_psData->m_iReadCond);
    		pthread_mutex_unlock(&pinSocket->m_psData->m_iReadMutex);
		}
		else
		{
            DEBUG_ERROR("Unknown block type: " << sType)
		}

		// Clean up
		delete[] pcBuffer;
	}

	return NULL;
}