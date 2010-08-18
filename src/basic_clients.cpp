#include <basic_clients.h>


Client_Events::Client_Events()
{
	m_pinSocket = new BlockSocket();
}

Client_Events::Client_Events(BlockSocket *p_pinSocket)
{
	m_pinSocket = p_pinSocket;

	bEnd = false;
	pthread_create(&m_iListenThread, NULL, Client_Events::ReadLoop, (void*) this);
}

Client_Events::~Client_Events()
{
	bEnd = true;
	pthread_join(m_iListenThread, NULL);

	delete m_pinSocket;
}


void Client_Events::Init()
{
	OnAccept();
}

bool Client_Events::Connect(std::string m_strAddress, unsigned short p_sPort)
{
	if(Client::Connect(m_strAddress, p_sPort))
	{
		bEnd = false;
		pthread_create(&m_iListenThread, NULL, Client_Events::ReadLoop, (void*) this);

		return true;
	}else
	{
		return false;
	}
}


void* Client_Events::ReadLoop(void* p_pClient)
{
	Client_Events* pinClient = static_cast<Client_Events*>(p_pClient);

	while(!pinClient->bEnd)
	{
		pinClient->OnReceive(pinClient->m_pinSocket->ReadBlock(true));
		
		// TODO: ....
		Sleep(100);
		//sched_yield();
	}

	return NULL;
}