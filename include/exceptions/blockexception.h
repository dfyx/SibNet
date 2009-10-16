#ifndef BLOCKEXCEPTION_H
#define BLOCKEXCEPTION_H

#include <networking.h>
#include <string>
#include <blocks/block.h>

class DLLDIR BlockException
{
protected:
	Block* m_pBlock;
	std::string m_strMessage;

public:
	BlockException(Block* p_pBlock, std::string p_strMessage = "");

	Block* GetBlock();
	std::string GetMessage();
};

#endif