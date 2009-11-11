#ifndef SERVER_H
#define SERVER_H

#include <blocks/block.h>
#include <networking.h>
#include <client.h>
#include <string>

class DLLDIR Server
{
private:
	class ListenerSocket *m_pinSocket;

public:
	Server();
	~Server();

	bool Listen(unsigned short p_sPort);

	virtual void OnAccept(Client *p_pinClient);
};

#endif // SERVER_H