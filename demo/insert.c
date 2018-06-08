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

int   main(int argc, char *argv[])
{
    dbUser  stUser;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();
 
    /* 初始化TBL_USER_INFO表，每张表都需要初始化一次, 对于表重建后，需要重新初始化一次。*/          
    if(RC_SUCC != lInitSATvm(pstSavm, TBL_USER_INFO))
    {
        fprintf(stderr, "init failed, err:(%d)(%s)\n", pstSavm->m_lErrno, 
            sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }
 
    conditinit(pstSavm, stUser, TBL_USER_INFO);          // 绑定变量
//  stUser.acct_id = time(NULL);                         // 对结构体赋值
    strcpy(stUser.user_no,    "20180223");               // 对结构体赋值
    strcpy(stUser.user_type,  "1");                      // 对结构体赋值
    strcpy(stUser.user_nm,    "Savens Liu");             // 对结构体赋值
    strcpy(stUser.user_addr,  "China");                  // 对结构体赋值
    strcpy(stUser.user_phone, "18672911111");            // 对结构体赋值
    decorate(pstSavm, dbUser, acct_id, FIELD_INCR);      // 自增llong型
 
    if(RC_SUCC != lInsert(pstSavm))      // 插入记录  
    {
        fprintf(stderr, "Insert error:(%d)(%s)\n", pstSavm->m_lErrno, 
            sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    fprintf(stdout, "insert success, effect:%d\n", pstSavm->m_lEffect);

    return RC_SUCC;
}
