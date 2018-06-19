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
    dbUser  stUser, stUpd;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

    if(RC_SUCC != lTvmConnect(pstSavm, "127.0.0.1", 5050, 2))
    {
        fprintf(stderr, "connect failed, err:(%d)(%s)\n", pstSavm->m_lErrno, 
            sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    updateinit(pstSavm, stUpd);
    conditinit(pstSavm, stUser, TBL_USER_INFO);                // 绑定变量
    conditstr(pstSavm, stUser, user_no, "20180529");           // 查询条件赋值
    conditstr(pstSavm, stUser, user_type, "1");                // 查询条件赋值

    updatestr(pstSavm, stUser, user_addr, "china");            // 查询条件赋值
    updatestr(pstSavm, stUser, user_phone, "1869112XAZZ");
    updatestr(pstSavm, stUser, user_nm, "DeffPuzzL");          // 查询条件赋值
    updatestr(pstSavm, stUser, user_type, "1");                // 查询条件赋值
    updatestr(pstSavm, stUser, user_no, "20180529");           // 查询条件赋值
    if(RC_SUCC != lTvmReplace(pstSavm, (void *)&stUser))
//    if(RC_SUCC != lAsyReplace(pstSavm, (void *)&stUser))
    {
        fprintf(stderr, "Replace error: (%d) (%s), ep(%d)\n", pstSavm->m_lErrno, 
            sGetTError(pstSavm->m_lErrno), pstSavm->m_lEType);
        return RC_FAIL;
    }

    vTvmDisconnect(pstSavm);

    return RC_SUCC;
}
