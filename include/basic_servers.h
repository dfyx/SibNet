#ifndef BASIC_SERVERS_H
#define BASIC_SERVERS_H


#include <server.h>
#include <client.h>
#include <vector>
#include <initializable.h>


template class DLLDIR std::allocator<Client*>;
template class DLLDIR std::vector<Client*>;


// A Server class with Automatic Client Creation & Storage
class DLLDIR Server_ACCS : public Server
{
private:
	std::vector<Client*> m_vpinClients;

public:
	Server_ACCS();
	~Server_ACCS();


	std::vector<Client*> GetClients() { return m_vpinClients; }

	/*final*/ virtual void OnAddBlockSocket(BlockSocket* p_pinSocket);

protected:
	// to be overloaded by Server_ACCS customisations to add event handling when new BlockSocket = Client is accepted
	virtual void OnAddClient(Client* p_pinClient) {};

	// to be overloaded by Server_ACCS customisations to create customised Client types
	virtual Client* CreateClient(BlockSocket* p_pinSocket);
};



// A Server class storing with Automatic Client Template Creation & Storage = const type creation
template<class _Client>
class Server_ACTCS : public Server_ACCS
{
protected:
	virtual Client* CreateClient(BlockSocket* p_pinSocket)
	{
		_Client* pinClient = new _Client(p_pinSocket);
		reinterpret_cast<Initializable*>(pinClient)->Init();

		return reinterpret_cast<Client*>(pinClient);
	}
};


#endif