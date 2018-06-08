#include    "tvm.h"
#include    "tmain.h"

#define    TBL_USER_INFO            20

int   main(int argc, char *argv[])
{
    SATvm   *pstSavm = (SATvm *)pGetSATvm();
 
    /* 初始化TBL_USER_INFO表，每张表都需要初始化一次, 对于表重建后，需要重新初始化一次。*/          
    if(RC_SUCC != lInitSATvm(pstSavm, TBL_USER_INFO))
    {
        fprintf(stderr, "init failed, err:(%d)(%s)\n", pstSavm->m_lErrno, 
            sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    if(RC_SUCC != lTruncate(pstSavm, TBL_USER_INFO))
    {
        fprintf(stderr, "Truncate error:(%u)(%s)\n", pstSavm->m_lErrno,
            sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    return RC_SUCC;
}
