#include <blocks/complexblock.h>

using namespace std;

ComplexBlock::~ComplexBlock()
{
	vector<Block*>::iterator iter;
	for(iter = m_vSubblocks.begin(); iter != m_vSubblocks.end(); iter++)
	{
		delete *iter;
	}
}

blocksize_t ComplexBlock::Serialize(char **p_ppcBuffer)
{
	int iNum = m_vSubblocks.size();
	char** apcData = new char*[iNum];
	blocksize_t* aiSizes = new blocksize_t[iNum];
	blocksize_t iSize = 0;

	// Serialize subblocks
	for(int i = 0; i < iNum; i++)
	{
		aiSizes[i] = m_vSubblocks[i]->Serialize(&apcData[i]);
		iSize += 4 + aiSizes[i];
	}

	// Merge serialized data
	*p_ppcBuffer = new char[iSize];
	int iOffset = 0;
	for(int i = 0; i < iNum; i++)
	{
		short iBlockType = swap(m_vSubblocks[i]->GetTypeID());
		blocksize_t sBlockSize = swap(aiSizes[i]);

		memcpy(*p_ppcBuffer + iOffset, &iBlockType, 2);
		iOffset += 2;

		memcpy(*p_ppcBuffer + iOffset, &sBlockSize, sizeof(blocksize_t));
		iOffset += sizeof(blocksize_t);

		memcpy(*p_ppcBuffer + iOffset, apcData[i], aiSizes[i]);
		iOffset += aiSizes[i];

		delete apcData[i];
	}

	delete apcData;
	delete aiSizes;

	return iSize;
}

void ComplexBlock::Deserialize(char *p_pcBuffer, blocksize_t p_iSize)
{
	blocksize_t iOffset = 0;

	while(iOffset < p_iSize)
	{
		// Read block type
		uint16_t iBlockType;
		memcpy(&iBlockType, p_pcBuffer + iOffset, 2);
		iOffset += 2;
		iBlockType = swap(iBlockType);

		// Read block size
		blocksize_t sBlockSize;
		memcpy(&sBlockSize, p_pcBuffer + iOffset, sizeof(blocksize_t));
		iOffset += sizeof(blocksize_t);
		blocksize_t iBlockSize = swap(sBlockSize);

		// Deserialize block
		Block* pBlock = Block::GetBlockFromTypeID(iBlockType);
		pBlock->Deserialize(p_pcBuffer + iOffset, iBlockSize);
		iOffset += iBlockSize;

		// Add to block list
		m_vSubblocks.push_back(pBlock);
	}
}