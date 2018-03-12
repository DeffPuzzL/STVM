#include    "tvm.h"
#include    "tmain.h"

#define    TBL_USER_INFO            20

typedef struct  __TBL_USER_INFO
{
   long    acct_id;
   char    user_no[21];
   char    user_type[2];
   char    user_nm[81];
   char    user_addr[161];
   char    user_phone[31];
}dbUser;

long    lCountUserInfo()
{
    dbUser  stUser;
    size_t  lRow = 0;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();
 
    /* 初始化TBL_USER_INFO表，每张表都需要初始化一次, 对于表重建后，需要重新初始化一次。*/          
    if(RC_SUCC != lInitSATvm(pstSavm, TBL_USER_INFO))
    {
        fprintf(stderr, "init failed, err:(%d)(%s)\n", pstSavm->m_lErrno, sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }
 
    conditinit(pstSavm, stUser, TBL_USER_INFO);                // 绑定变量
    stringset(pstSavm, stUser, user_type, "1");                // 查询条件赋值
    stringset(pstSavm, stUser, user_no, "20180224");           // 查询条件赋值
 
    if(RC_SUCC != lCount(pstSavm, (void *)&lRow))
    {
        fprintf(stderr, "Count error: (%d) (%s)\n", pstSavm->m_lErrno, sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    fprintf(stdout, "Count:%ld\n\n", lRow);
    return RC_SUCC;
}

int   main(int argc, char *argv[])
{
    if(RC_SUCC != lCountUserInfo())
        return RC_FAIL;

    return RC_SUCC;
}


