#ifndef STRINGBLOCK_H
#define STRINGBLOCK_H

#include <networking.h>
#include <string>
#include <blocks/block.h>

class DLLDIR StringBlock: public Block
{
// Associations
// Attributes
protected:
	std::string m_strData;

// Operations
public:
	StringBlock();
	StringBlock(std::string p_strData);

	std::string GetData();
	void SetData(std::string p_strData);

	virtual size_t Serialize(char** p_ppcBuffer);
    virtual void Deserialize(char* p_pcBuffer, size_t p_iSize);

	MAKE_BLOCKTYPE(StringBlock, BLOCKTYPE_STRING);
};

#endif
