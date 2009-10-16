#ifndef BINARYBLOCK_H
#define BINARYBLOCK_H

#include <networking.h>
#include <blocks/block.h>

class DLLDIR BinaryBlock: public Block
{
// Associations
// Attributes
protected:
	char* m_pcData;
	int m_iSize;

// Operations
public:
	BinaryBlock();
	BinaryBlock(char* p_pcBuffer, int p_iSize);
	~BinaryBlock();

    int GetData(char** p_ppcData);
    void SetData(char* p_pcData, int p_iSize);
    int GetDataSize();

	virtual size_t Serialize(char** p_ppcBuffer);
    virtual void Deserialize(char* p_pcBuffer, size_t p_iSize);

	MAKE_BLOCKTYPE(BinaryBlock, BLOCKTYPE_BINARY);
};

#endif
