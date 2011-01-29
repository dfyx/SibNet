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
	blocksize_t m_iSize;

// Operations
public:
	BinaryBlock();
	BinaryBlock(char* p_pcBuffer, blocksize_t p_iSize);
	~BinaryBlock();

    blocksize_t GetData(char** p_ppcData);
    void SetData(char* p_pcData, blocksize_t p_iSize);
    blocksize_t GetDataSize();

	virtual blocksize_t Serialize(char** p_ppcBuffer);
    virtual void Deserialize(char* p_pcBuffer, blocksize_t p_iSize);

	MAKE_BLOCKTYPE(BinaryBlock, BLOCKTYPE_BINARY);
};

#endif
