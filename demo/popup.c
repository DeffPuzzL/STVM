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
    long    i, lRows = 0, lTime;
    dbUser  stUser, *pstUser = NULL;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();
 
    /* 初始化QUEUE_USER_INFO表，每张表都需要初始化一次, 对于表重建后，需要重新初始化一次。*/          
    if(RC_SUCC != lInitSATvm(pstSavm, QUEUE_USER_INFO))
    {
        fprintf(stderr, "init failed, err:(%d)(%s)\n", pstSavm->m_lErrno, 
            sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    lTime = time(NULL);
    queuenull(pstSavm, sizeof(dbUser), QUEUE_USER_INFO);
    if(RC_SUCC != lPopup(pstSavm, 3, 5, &lRows, (void **)&pstUser)) 
    {
        fprintf(stderr, "Pop error:(%u)(%s), time:%ld, effect:%d\n", pstSavm->m_lErrno, 
            sGetTError(pstSavm->m_lErrno), time(NULL) - lTime, pstSavm->m_lEffect);
    }

    if(0 != pstSavm->m_lErrno && 0 == pstSavm->m_lEffect)
        return RC_FAIL;

    for(i = 0; i < lRows; i ++)
    {
        fprintf(stdout, "row:%ld, acct_id:%ld, user_no:%s, user_type:%s, "
           "user_nm:%s, user_addr:%s, user_phone:%s\n", i, pstUser[i].acct_id, 
           pstUser[i].user_no, pstUser[i].user_type, pstUser[i].user_nm, 
           pstUser[i].user_addr, pstUser[i].user_phone);
    }

    TFree(pstUser);

    return RC_SUCC;
}
