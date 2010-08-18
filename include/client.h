#ifndef CLIENT_H
#define CLIENT_H


#include <blocks/block.h>
#include <networking.h>
#include <blocksocket.h>
#include <string>

#include <initializable.h>


class DLLDIR Client : public Initializable
{
protected:
	class BlockSocket* m_pinSocket;

	virtual void Init() {}

public:
	Client();
	Client(class BlockSocket *p_pinSocket);
	~Client();

	bool Connect(std::string m_strAddress, unsigned short p_sPort);
	void Disconnect();

	void SendBlock(Block* p_pinBlock);
	Block* ReceiveBlock(bool p_bWait = true);
};

#endif // CLIENT_H