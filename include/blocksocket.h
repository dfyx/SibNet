#ifndef BLOCKSOCKET_H
#define BLOCKSOCKET_H

#include <blocks/block.h>
#include <queue>
#include <string>

class BlockSocket
{
public:
	enum QueueBehavior
	{
		QUEUE_CLEAR,
		QUEUE_FLUSH_WAIT,
		QUEUE_FLUSH_ASYNC
	};
private:
	std::queue<Block*> m_inWriteQueue, m_inReadQueue;

	struct BlockSocketData *m_psData;

	enum State
	{
		STATE_DISCONNECTED,
		STATE_CONNECTING,
		STATE_CONNECTED,
		STATE_DISCONNECTING
	};
	
	State m_eState;
	QueueBehavior m_eWriteBehavior;
	
	static void *WriteLoop(void *p_pinSocket);
	static void *ReadLoop(void *p_pinSocket);
public:
	BlockSocket();
	BlockSocket(int p_inSocket);
	~BlockSocket();

	bool Connect(std::string p_strAddress, uint16_t p_sPort);
	void Disconnect(QueueBehavior p_eWriteBehavior);

	bool IsConnected();
	State GetState();

	void WriteBlock(Block* p_pinBlock);
	Block* ReadBlock(bool p_bWait = true);
	
	size_t GetWriteQueueSize();
	size_t GetReadQueueSize();
	
	void ClearReadQueue();
	void ClearWriteQueue();
};

#endif // BLOCKSOCKET_H