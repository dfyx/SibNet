#ifndef BASIC_CLIENS_H
#define BASIC_CLIENS_H


#include <client.h>
#include <pthread.h>


class DLLDIR Client_Events : protected Client
{
private:
	pthread_t m_iListenThread;
	bool bEnd;
	static void* ReadLoop(void* p_pClient);

protected:
	virtual void Init();

	virtual void OnAccept() = 0;
	virtual bool OnSend(Block* p_pinBlock) = 0;
	virtual void OnReceive(Block* p_pinBlock) = 0;

public:
	Client_Events();
	Client_Events(class BlockSocket *p_pinSocket);
	~Client_Events();

	using Client::SendBlock;
	using Client::Disconnect;

	bool Connect(std::string m_strAddress, unsigned short p_sPort);
};


#endif