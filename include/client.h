#ifndef CLIENT_H
#define CLIENT_H

#include <blocks/block.h>
#include <networking.h>
#include <string>

class DLLDIR Client
{
private:
	class BlockSocket *m_pinSocket;

public:
	Client();
	Client(class BlockSocket *p_pinSocket);
	~Client();

	bool Connect(std::string m_strAddress, unsigned short p_sPort);
	void Disconnect();

	void SendBlock(Block* p_pinBlock);
};

#endif // CLIENT_H