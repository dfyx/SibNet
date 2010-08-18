#include <basic_servers.h>


Server_ACCS::Server_ACCS()
: Server()
{}
Server_ACCS::~Server_ACCS()
{
	std::vector<Client*>::iterator i;
	for(i = m_vpinClients.begin(); i != m_vpinClients.end(); i++)
	{
		delete *i;
	}
}

void Server_ACCS::OnAddBlockSocket(BlockSocket *p_pinSocket)
{
	m_vpinClients.push_back(CreateClient(p_pinSocket));
}

Client* Server_ACCS::CreateClient(BlockSocket* p_pinSocket)
{
	Client* pinClient = new Client(p_pinSocket);
	static_cast<Initializable*>(pinClient)->Init();
	
	return pinClient;
}