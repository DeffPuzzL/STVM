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

    if(RC_SUCC != lTvmConnect(pstSavm, "127.0.0.1", 5050, 2))
    {
        fprintf(stderr, "init failed, err:(%d)(%s)\n", pstSavm->m_lErrno, sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    queueinit(pstSavm, stUser, QUEUE_USER_INFO);    // 绑定变量
    stUser.acct_id = time(NULL);                    // 对结构体赋值
    strcpy(stUser.user_no,    "20180223");          // 对结构体赋值
    strcpy(stUser.user_type,  "1");                 // 对结构体赋值
    strcpy(stUser.user_nm,    "Savens Liu");        // 对结构体赋值
    strcpy(stUser.user_addr,  "China");             // 对结构体赋值
    strcpy(stUser.user_phone, "18672911111");       // 对结构体赋值
    if(RC_SUCC != lAsyPush(pstSavm))                // 插入记录  
    {
        fprintf(stderr, "Push error:(%d)(%s)\n", pstSavm->m_lErrno, sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    vTvmDisconnect(pstSavm);

    fprintf(stdout, "asypush success\n");

    return RC_SUCC;
}
