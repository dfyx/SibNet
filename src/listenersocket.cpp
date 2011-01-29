#include <listenersocket.h>
#include <nl.h>
#include <pthread.h>

using namespace std;

struct ListenerSocketData
{
	NLsocket m_inSocket;
	pthread_t m_iListenThread;

	pthread_mutex_t m_iClientMutex;
};

ListenerSocket::ListenerSocket()
{
	// Initialize HawkNL
	if(!nlInit())
	{
		DEBUG_ERROR("Could not init HawkNL.")
	}

	nlSelectNetwork(NL_IP);
	nlEnable(NL_BLOCKING_IO);

	m_bListening = false;

	m_psData = new ListenerSocketData;

	m_psData->m_inSocket = NL_INVALID;

	m_pAddClientSocketCB = NULL;

	pthread_mutex_init(&m_psData->m_iClientMutex, NULL);
}

ListenerSocket::~ListenerSocket()
{
	// Close socket
	nlClose(m_psData->m_inSocket);

	if(IsListening())
	{
		StopListening();
		pthread_join(m_psData->m_iListenThread, NULL);
	}

	pthread_mutex_destroy(&m_psData->m_iClientMutex);

	delete m_psData;

	// This is okay, even for multiple sockets. HawkNL has an internal counter
	nlShutdown();
}

bool ListenerSocket::Listen(uint16_t p_sPort)
{
	DEBUG_NOTICE("Trying to listen")
	m_psData->m_inSocket = nlOpen(p_sPort, NL_RELIABLE);

	if(m_psData->m_inSocket == NL_INVALID)
	{
		DEBUG_ERROR("Could not open socket.")
		return false;
	}

	if(!nlListen(m_psData->m_inSocket))
	{
		nlClose(m_psData->m_inSocket);
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
		NLsocket inSocket = nlAcceptConnection(pinListener->m_psData->m_inSocket);
		if(inSocket != NL_INVALID)
		{
			pinListener->AddBlockSocket(new BlockSocket((void*) &inSocket));
			DEBUG_NOTICE("Accepted new connection.")
		}
		else if(nlGetError() != NL_NO_PENDING)
		{
			DEBUG_ERROR("Failed to accept")
		}

		sched_yield();
	}

	return NULL;
}