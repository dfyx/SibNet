#include <server.h>
#include <listenersocket.h>
#include <blocksocket.h>

using namespace std;

Server::Server()
{
	m_pinSocket = new ListenerSocket();
	m_pinSocket->SetAddClientSocketCB(this);
}

Server::~Server()
{
	delete m_pinSocket;
}

bool Server::Listen(unsigned short p_sPort)
{
	return m_pinSocket->Listen(p_sPort);
}

vector<BlockSocket*> Server::GetBlockSockets()
{
	return m_pinSocket->GetBlockSockets();
}