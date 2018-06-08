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

CREATE  lQueueUserInfo()
{
   QUEUE(QUEUE_USER_INFO, "", dbUser)
   FIELD(dbUser,    acct_id,        FIELD_LONG)
   FIELD(dbUser,    user_no,        FIELD_CHAR)
   FIELD(dbUser,    user_type,      FIELD_CHAR)
   FIELD(dbUser,    user_nm,        FIELD_CHAR)
   FIELD(dbUser,    user_addr,      FIELD_CHAR)
   FIELD(dbUser,    user_phone,     FIELD_CHAR)
 
   FINISH
}

int   main(int argc, char *argv[])
{
    char    szMsg[1024];
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

//    if(RC_SUCC != lCreateQueue(pstSavm, QUEUE_USER_INFO, 1000000, sizeof(szMsg), ""))
    if(RC_SUCC != lTableQueue(pstSavm, QUEUE_USER_INFO, 1000000, lQueueUserInfo))
    {
	    fprintf(stderr, "create queue %d failed, err: %s\n", QUEUE_USER_INFO, sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    fprintf(stdout, "create table success\n");

    return RC_SUCC;
}
