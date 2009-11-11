#include <listenersocket.h>

using namespace std;

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

	m_inSocket = NL_INVALID;

	m_pAddClientCallback = NULL;
	m_pAddClientParam = NULL;

	pthread_mutex_init(&m_iClientMutex, NULL);
}

ListenerSocket::~ListenerSocket()
{
	// Close socket
	nlClose(m_inSocket);

	if(IsListening())
	{
		pthread_join(m_iListenThread, NULL);
	}

	pthread_mutex_destroy(&m_iClientMutex);

	// This is okay, even for multiple sockets. HawkNL has an internal counter
	nlShutdown();
}

bool ListenerSocket::Listen(uint16_t p_sPort)
{
	DEBUG_NOTICE("Trying to listen")
	m_inSocket = nlOpen(p_sPort, NL_RELIABLE);

	if(m_inSocket == NL_INVALID)
	{
		DEBUG_ERROR("Could not open socket.")
		return false;
	}

	if(!nlListen(m_inSocket))
	{
		nlClose(m_inSocket);
		DEBUG_ERROR("Could not listen.")
		return false;
	}

	m_bListening = true;

	pthread_create(&m_iListenThread, NULL, ListenerSocket::ListenLoop, (void*) this);

	return true;
}

bool ListenerSocket::IsListening()
{
	return m_bListening;
}

void ListenerSocket::AddClient(BlockSocket *p_pinSocket)
{
	pthread_mutex_lock(&m_iClientMutex);
	m_vClients.push_back(p_pinSocket);
	pthread_mutex_unlock(&m_iClientMutex);
	
	if(m_pAddClientCallback != NULL)
	{
		m_pAddClientCallback(p_pinSocket, m_pAddClientParam);
	}
}

vector<BlockSocket*> ListenerSocket::GetClients()
{
	pthread_mutex_lock(&m_iClientMutex);
	vector<BlockSocket*> vTemp = m_vClients;
	pthread_mutex_unlock(&m_iClientMutex);
	return vTemp;
}

void ListenerSocket::SetAddClientCallback(void (*p_pAddClientCallback)(BlockSocket*, void*),
										  void *p_pAddClientParam)
{
	m_pAddClientCallback = p_pAddClientCallback;
	m_pAddClientParam = p_pAddClientParam;
}

void *ListenerSocket::ListenLoop(void *p_pinSocket)
{
	ListenerSocket *pinListener = (ListenerSocket*) p_pinSocket;

	while(pinListener->IsListening())
	{
		NLsocket inSocket = nlAcceptConnection(pinListener->m_inSocket);
		if(inSocket != NL_INVALID)
		{
			pinListener->AddClient(new BlockSocket(inSocket));
			DEBUG_NOTICE("Accepted new connection.")
		}
		else if(nlGetError() != NL_NO_PENDING)
		{
			DEBUG_ERROR("Failed to accept")
		}
	}

	return NULL;
}