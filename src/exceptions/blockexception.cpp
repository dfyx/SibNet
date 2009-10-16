#include <exceptions/blockexception.h>

using namespace std;

BlockException::BlockException(Block* p_pBlock, string p_strMessage)
{
	m_pBlock = p_pBlock;
	m_strMessage = p_strMessage;
}

Block* BlockException::GetBlock()
{
	return m_pBlock;
}

string BlockException::GetMessage()
{
	return m_strMessage;
}