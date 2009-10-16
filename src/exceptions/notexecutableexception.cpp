#include <exceptions/notexecutableexception.h>

using namespace std;

NotExecutableException::NotExecutableException(Block* p_pBlock, string p_strMessage)
: BlockException(p_pBlock, p_strMessage)
{
}