#include <server.h>
#include <listenersocket.h>
#include <blocksocket.h>

using namespace std;

void AddClientCallback(BlockSocket *p_pinSocket, void *p_pServer)
{
	Server *pServer = (Server*) p_pServer;
	pServer->OnAccept(new Client(p_pinSocket));
}

Server::Server()
{
	m_pinSocket = new ListenerSocket();
	m_pinSocket->SetAddClientCallback(AddClientCallback, (void*) this);
}

Server::~Server()
{
	delete m_pinSocket;
}

bool Server::Listen(unsigned short p_sPort)
{
	return m_pinSocket->Listen(p_sPort);
}

void Server::OnAccept(Client *p_pinClient)
{
	DEBUG_NOTICE("OnAccept");
}