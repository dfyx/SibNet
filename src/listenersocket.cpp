#include <listenersocket.h>
#include <network_wrap.h>
#include <pthread.h>

using namespace std;

struct ListenerSocketData
{
	Net::Socket m_inSocket;
	pthread_t m_iListenThread;

	pthread_mutex_t m_iClientMutex;
};

ListenerSocket::ListenerSocket()
{
	// Initialize network
	if(!Net::init())
	{
		DEBUG_ERROR("Could not init network.")
	}

	m_bListening = false;

	m_psData = new ListenerSocketData;

	m_psData->m_inSocket = -1;

	m_pAddClientSocketCB = NULL;

	pthread_mutex_init(&m_psData->m_iClientMutex, NULL);
}

ListenerSocket::~ListenerSocket()
{
	// Close socket
	Net::close(m_psData->m_inSocket);

	if(IsListening())
	{
		StopListening();
		pthread_join(m_psData->m_iListenThread, NULL);
	}

	pthread_mutex_destroy(&m_psData->m_iClientMutex);

	delete m_psData;

	// This is okay, even for multiple sockets. There is an internal counter
	Net::shutdown();
}

bool ListenerSocket::Listen(uint16_t p_sPort)
{
	DEBUG_NOTICE("Trying to listen")
	m_psData->m_inSocket = Net::listen(p_sPort, Net::TYPE_TCP);

	if(m_psData->m_inSocket == -1)
	{
		DEBUG_ERROR("Could not listen.")
		return false;
	}

	m_bListening = true;

	pthread_create(&m_psData->m_iListenThread, NULL, ListenerSocket::ListenLoop, (void*) this);

	return true;
}

bool ListenerSocket::IsListening()
{
	return m_bListening;
}

void ListenerSocket::StopListening()
{
	m_bListening = false;
}

void ListenerSocket::AddBlockSocket(BlockSocket *p_pinSocket)
{
	pthread_mutex_lock(&m_psData->m_iClientMutex);
	m_vClients.push_back(p_pinSocket);
	pthread_mutex_unlock(&m_psData->m_iClientMutex);
	
	if(m_pAddClientSocketCB != NULL)
	{
		m_pAddClientSocketCB->OnAddBlockSocket(p_pinSocket);
	}
}

vector<BlockSocket*> ListenerSocket::GetBlockSockets()
{
	pthread_mutex_lock(&m_psData->m_iClientMutex);
	vector<BlockSocket*> vTemp = m_vClients;
	pthread_mutex_unlock(&m_psData->m_iClientMutex);
	return vTemp;
}

void ListenerSocket::SetAddClientSocketCB(AddClientSocketCallback* p_pAddClientSocketCB)
{
	m_pAddClientSocketCB = p_pAddClientSocketCB;
}

void *ListenerSocket::ListenLoop(void *p_pinSocket)
{
	ListenerSocket *pinListener = (ListenerSocket*) p_pinSocket;

	while(pinListener->IsListening())
	{
		Net::Socket inSocket = Net::accept(pinListener->m_psData->m_inSocket);
		if(inSocket >= 0)
		{
			pinListener->AddBlockSocket(new BlockSocket((int) inSocket));
			DEBUG_NOTICE("Accepted new connection.")
		}
		else
		{
			DEBUG_ERROR("Failed to accept")
		}

		sched_yield();
	}

	return NULL;
}