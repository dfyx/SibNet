#ifndef NETWORKING_H
#define NETWORKING_H

#ifdef NETWORKING_EXPORTS
   #define DLLDIR  __declspec(dllexport)   // export DLL information
#else
   #define DLLDIR  __declspec(dllimport)   // import DLL information
#endif 

#ifdef _DEBUG
#include <iostream>
#define DEBUG_ERROR(x)						\
	std::cerr << "! " << x << std::endl;	\
	std::cerr << "  Error code was " << nlGetError() << ": " << (char*) nlGetErrorStr(nlGetError())	<< "." << std::endl; \
	if(nlGetError() == NL_SYSTEM_ERROR)		\
	std::cerr << "  System error code was " << nlGetSystemError() << ": " << (char*) nlGetSystemErrorStr(nlGetSystemError()) << "." << std::endl;

#define DEBUG_NOTICE(x)						\
	std::cout << "? " << x << std::endl;
#else
#define DEBUG_ERROR(x)
#define DEBUG_NOTICE(x)
#endif
#endif // NETWORKING_H