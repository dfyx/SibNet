#ifndef BLOCKINTERPRETER_H
#define BLOCKINTERPRETER_H

#include <networking.h>

// Include needed for byte order conversion
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

class DLLDIR BlockInterpreter
{
};

#endif