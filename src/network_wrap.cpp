#include <networking.h>
#include <network_wrap.h>

#ifndef CMAKE_HAVE_WINSOCK2

namespace Net
{
    bool init()
    {
        return true;
    }

    void shutdown()
    {
    }
    
    void close(Socket sock)
    {
        if(sock < 0)
        {
            return;
        }
        ::close(sock);
    }
};
#else

namespace Net
{
    static unsigned long initCount = 0;

    bool init()
    {	
        initCount++;
    
        if(initCount == 1)
    	{
            WORD wVersionRequested = MAKEWORD(2, 0);
            WSADATA wsaData;
    		return (WSAStartup(wVersionRequested, &wsaData) == 0);
    	}
	
        return true;
    }

    void shutdown()
    {
        initCount--;
    
        if(initCount == 0)
        {
            WSACleanup();
        }
    }
    
    void close(Socket sock)
    {
        if(sock < 0)
        {
            return;
        }
        ::closesocket(sock);
    }
};
#endif

// Common stuff
namespace Net
{    
    Socket connect(std::string address, unsigned short port, ConnectionType connectionType, int domain)
    {
        // Prepare type
        int type;
        if(connectionType == TYPE_TCP)
        {
            type = SOCK_STREAM;
        }
        else if(connectionType == TYPE_UDP)
        {
            type = SOCK_DGRAM;
        }
        else
        {
            return -1;
        }
        
        // Prepare address struct
        if(domain != AF_INET)
        {
            // Other domains are not supported yet
            return -1;
        }
        
        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));

		// Resolve address
		unsigned long ip_addr;
		if((ip_addr = inet_addr(address.c_str())) != INADDR_NONE)
		{
        	memcpy((char*) &addr.sin_addr, &ip_addr, sizeof(ip_addr));
		}
		else
		{
			// For localhost etc.
			hostent *host_info = gethostbyname(address.c_str());
			if(host_info == NULL)
			{
				DEBUG_ERROR("Unknown host: " + address);
				return -1;
			}
			
			memcpy(&addr.sin_addr, host_info->h_addr, host_info->h_length);
		}
        addr.sin_family = domain;
        addr.sin_port = htons(port);
        
        // Create socket
        int sock = socket(domain, type, 0);
        if(sock < 0)
        {
            return -1;
        }
        
        // Connect
		if(::connect(sock, (sockaddr*) &addr, sizeof(addr)) < 0)
        {
            close(sock);
            return -1;
        }
        else
        {
            return sock;
        }
    }
    
    int send(Socket sock, const void *data, size_t length)
    {
        return ::send(sock, data, length, 0);
    }
    
    int read(Socket sock, void *data, size_t length)
    {
        return ::read(sock, data, length);
    }
    
    Socket listen(unsigned int port, ConnectionType connectionType, int domain)
    {
        // Prepare type
        int type;
        if(connectionType == TYPE_TCP)
        {
            type = SOCK_STREAM;
        }
        else if(connectionType == TYPE_UDP)
        {
            type = SOCK_DGRAM;
        }
        else
        {
            return -1;
        }
        
        // Prepare address struct
        if(domain != AF_INET)
        {
            // Other domains are not supported yet
            return -1;
        }
        
        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = domain;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port);
        
        // Create socket
        int sock = socket(domain, type, 0);
        if(sock < 0)
        {
            return -1;
        }
        
        // Bind
        if(bind(sock, (sockaddr*) &addr, sizeof(addr)) < 0)
        {
            close(sock);
            return -1;
        }
        
        // Listen
        if(::listen(sock, 5) < 0)
        {
            close(sock);
            return -1;
        }
        
        return sock;
    }
    
    Socket accept(Socket sock)
    {
        sockaddr addr;
        socklen_t len = sizeof(addr);
        return ::accept(sock, &addr, &len);
    }
};