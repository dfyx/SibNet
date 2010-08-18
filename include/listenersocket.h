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
public:
	// callback definition, as class
	class AddClientSocketCallback
	{
		friend ListenerSocket;

	private:
		virtual void OnAddBlockSocket(BlockSocket*) = 0;
	};


private:
	bool m_bListening;

	NLsocket m_inSocket;
	pthread_t m_iListenThread;

	pthread_mutex_t m_iClientMutex;

	AddClientSocketCallback* m_pAddClientSocketCB;

	std::vector<BlockSocket*> m_vClients;

	static void* ListenLoop(void* p_pinSocket);


public:
	ListenerSocket();
	~ListenerSocket();

	bool Listen(uint16_t p_sPort);
	bool IsListening();
	void StopListening();

	void AddBlockSocket(BlockSocket* p_pinSocket);
	std::vector<BlockSocket*> GetBlockSockets();
	void SetAddClientSocketCB(AddClientSocketCallback* p_pAddClientSocketCB);
};

#endif // LISTENERSOCKET_H