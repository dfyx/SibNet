#ifndef SERVER_H
#define SERVER_H

#include <blocks/block.h>
#include <networking.h>
#include <listenersocket.h>
#include <string>
#include <vector>



class DLLDIR Server : public ListenerSocket::AddClientSocketCallback
{
private:
	class ListenerSocket* m_pinSocket;

protected:
	virtual void OnAddBlockSocket(BlockSocket* p_pinSocket) {}

public:
	Server();
	~Server();

	std::vector<BlockSocket*> GetBlockSockets();

	bool Listen(unsigned short p_sPort);
};



#endif // SERVER_H