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

size_t ComplexBlock::Serialize(char **p_ppcBuffer)
{
	int iNum = m_vSubblocks.size();
	char** apcData = new char*[iNum];
	int* aiSizes = new int[iNum];
	int iSize = 0;

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
		short iBlockType = htons(m_vSubblocks[i]->GetTypeID());
		short iBlockSize = htons(aiSizes[i]);

		memcpy(*p_ppcBuffer + iOffset, &iBlockType, 2);
		memcpy(*p_ppcBuffer + iOffset + 2, &aiSizes[i], 2);
		memcpy(*p_ppcBuffer + iOffset + 4, apcData[i], aiSizes[i]);
		iOffset += 4 + aiSizes[i];

		delete apcData[i];
	}

	delete apcData;
	delete aiSizes;

	return iSize;
}

void ComplexBlock::Deserialize(char *p_pcBuffer, size_t p_iSize)
{
	size_t iOffset = 0;

	while(iOffset < p_iSize)
	{
		// Read block type
		short iBlockType;
		memcpy(&iBlockType, p_pcBuffer + iOffset, 2);
		iBlockType = ntohs(iBlockType);

		// Read block size
		short iBlockSize;
		memcpy(&iBlockSize, p_pcBuffer + iOffset + 2, 2);
		iBlockSize = ntohs(iBlockSize);

		// Deserialize block
		Block* pBlock = Block::GetBlockFromTypeID(iBlockType);
		pBlock->Deserialize(p_pcBuffer + iOffset + 4, iBlockSize);

		// Add to block list
		m_vSubblocks.push_back(pBlock);

		iOffset += 4 + iBlockSize;
	}
}