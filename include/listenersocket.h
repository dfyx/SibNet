#ifndef LISTENERSOCKET_H
#define LISTENERSOCKET_H

#include <networking.h>
#include <blocksocket.h>
#include <stdint_wrap.h>
#include <pthread.h>
#include <vector>



class ListenerSocket
{
public:
	// callback definition, as class
	class AddClientSocketCallback
	{
		friend class ListenerSocket;

	private:
		virtual void OnAddBlockSocket(BlockSocket*) = 0;
	};


private:
	bool m_bListening;

	struct ListenerSocketData *m_psData;

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