#include <client.h>
#include <vector>
#include <blocksocket.h>

using namespace std;

Client::Client()
{
	m_pinSocket = new BlockSocket();
}

Client::~Client()
{
	delete m_pinSocket;
}

bool Client::Connect(string p_strAddress, unsigned short p_sPort)
{
	return m_pinSocket->Connect(p_strAddress, p_sPort);
}

void Client::Disconnect()
{
	m_pinSocket->Disconnect();
}

void Client::SendBlock(Block *p_pinBlock)
{
	m_pinSocket->WriteBlock(p_pinBlock);
}