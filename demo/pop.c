#include    "tvm.h"
#include    "tmain.h"

#define    QUEUE_USER_INFO            21

typedef struct  __QUEUE_USER_INFO
{
   long    acct_id;
   char    user_no[21];
   char    user_type[2];
   char    user_nm[81];
   char    user_addr[161];
   char    user_phone[31];
}dbUser;

int   main(int argc, char *argv[])
{
    dbUser  stUser;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();
 
    /* 初始化QUEUE_USER_INFO表，每张表都需要初始化一次, 对于表重建后，需要重新初始化一次。*/
    if(RC_SUCC != lInitSATvm(pstSavm, QUEUE_USER_INFO))
    {
        fprintf(stderr, "attch failed, err:(%d)(%s)\n", pstSavm->m_lErrno, 
            sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }
 
    queueinit(pstSavm, stUser, QUEUE_USER_INFO);               // 绑定变量
    if(RC_SUCC != lPop(pstSavm, (void *)&stUser, QUE_NORMAL))  // POP记录  
    {
        fprintf(stderr, "Pop error:(%d)(%s)\n", pstSavm->m_lErrno, sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    fprintf(stdout, "acct_id:%ld, user_no:%s, user_type:%s, user_nm:%s, user_addr:%s, "
        "user_phone:%s\n", stUser.acct_id, stUser.user_no, stUser.user_type, stUser.user_nm, 
        stUser.user_addr, stUser.user_phone);

    return RC_SUCC;
}
