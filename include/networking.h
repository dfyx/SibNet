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
	std::cerr << "  Error code was " << nlGetError() << ": " << nlGetErrorStr(nlGetError())	<< "." << std::endl; \
	if(nlGetError() == NL_SYSTEM_ERROR)		\
	std::cerr << "  System error code was " << nlGetSystemError() << ": " << nlGetSystemErrorStr(nlGetSystemError()) << "." << std::endl;
#else
#define DEBUG_ERROR(x)
#endif
#endif // NETWORKING_H