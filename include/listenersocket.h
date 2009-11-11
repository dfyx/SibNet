#ifndef LISTENERSOCKET_H
#define LISTENERSOCKET_H

#include <networking.h>
#include <blocksocket.h>
#include <pstdint.h>
#include <nl.h>
#include <pthread.h>
#include <vector>

class ListenerSocket
{
private:
	bool m_bListening;

	NLsocket m_inSocket;
	pthread_t m_iListenThread;

	pthread_mutex_t m_iClientMutex;

	void (*m_pAddClientCallback)(BlockSocket*, void*);
	void *m_pAddClientParam;

	std::vector<BlockSocket*> m_vClients;
public:
	ListenerSocket();
	~ListenerSocket();

	bool Listen(uint16_t p_sPort);
	bool IsListening();

	void AddClient(BlockSocket *p_pinSocket);
	std::vector<BlockSocket*> GetClients();
	void SetAddClientCallback(void (*p_pAddClientCallback)(BlockSocket*, void*),
		void *p_pAddClientParam);

	static void *ListenLoop(void *p_pinSocket);
};

#endif // LISTENERSOCKET_H