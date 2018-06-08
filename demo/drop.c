#include    "tvm.h"
#include    "tmain.h"

#define    TBL_USER_INFO            20

int   main(int argc, char *argv[])
{
    SATvm   *pstSavm = (SATvm *)pGetSATvm();
 
    if(RC_SUCC != lDropTable(pstSavm, TBL_USER_INFO))
    {
        fprintf(stderr, "drop table error:(%d)(%s)\n", pstSavm->m_lErrno, 
            sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    return RC_SUCC;
}
