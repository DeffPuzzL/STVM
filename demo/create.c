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

CREATE  lCreateUserInfo()
{
   DEFINE(TBL_USER_INFO, "TBL_USER_INFO", "", dbUser)
   FIELD(dbUser,    acct_id,         "acct_id",       FIELD_LONG)
   FIELD(dbUser,    user_no,         "user_no",       FIELD_CHAR)
   FIELD(dbUser,    user_type,       "user_type",     FIELD_CHAR)
   FIELD(dbUser,    user_nm,         "user_nm",       FIELD_CHAR)
   FIELD(dbUser,    user_addr,       "user_addr",     FIELD_CHAR)
   FIELD(dbUser,    user_phone,      "user_phone",    FIELD_CHAR)
 
   CREATE_IDX(NORMAL)        //    创建查询索引
   IDX_FIELD(dbUser, acct_id,       FIELD_LONG)
 
   CREATE_IDX(UNQIUE)        //    创建唯一索引
   IDX_FIELD(dbUser, user_no,       FIELD_CHAR)
   IDX_FIELD(dbUser, user_type,     FIELD_CHAR)
 
   FINISH
}

int   main(int argc, char *argv[])
{
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

    if(RC_SUCC != lCreateTable(pstSavm, TBL_USER_INFO, 1000, lCreateUserInfo))
    {
	    fprintf(stderr, "create table %d failed, err: %s\n", TBL_USER_INFO, sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

	fprintf(stdout, "初始化表成功, completed successfully!!!\n");
	fflush(stderr);

    return RC_SUCC;
}


