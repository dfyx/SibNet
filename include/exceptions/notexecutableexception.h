#ifndef NOTEXECUTABLEEXCEPTION_H
#define NOTEXECUTABLEEXCEPTION_H

#include <networking.h>
#include <exceptions/blockexception.h>

class DLLDIR NotExecutableException: public BlockException
{
public:
	NotExecutableException(Block* p_pBlock, std::string p_strMessage = "This block can't be executed.");
};

#endif