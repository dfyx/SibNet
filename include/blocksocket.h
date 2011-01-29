#ifndef BLOCKSOCKET_H
#define BLOCKSOCKET_H

#include <blocks/block.h>
#include <queue>
#include <string>

class BlockSocket
{
private:
	std::queue<Block*> m_inWriteQueue, m_inReadQueue;

	struct BlockSocketData *m_psData;

	bool m_bConnected, m_bDisconnect;
public:
	BlockSocket();
	BlockSocket(void *p_pinSocket);
	~BlockSocket();

	bool Connect(std::string p_strAddress, uint16_t p_sPort);
	void Disconnect();

	bool IsConnected();

	void WriteBlock(Block* p_pinBlock);
	Block* ReadBlock(bool p_bWait = true);

	static void *WriteLoop(void *p_pinSocket);
	static void *ReadLoop(void *p_pinSocket);
};

#endif // BLOCKSOCKET_H