#include   "tmain.h"

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

typedef unsigned long long uint64;
extern uint64 get_tick_time();

typedef struct _ST_ARG_
{
    int threadIndex;
    uint64 rows;
    uint64 start;
}ARG;

void*    vPushUserInfo(void *arg)
{
    int     i = 0;
    dbUser  stUser;
    ARG     *pArgInfo = (ARG *)arg;
    SATvm   *pstSavm = (SATvm *)pCloneSATvm();

    queueinit(pstSavm, stUser, QUEUE_USER_INFO);          // 绑定变量
    for(i = 0; i < pArgInfo->rows; i++)
    {
        stUser.acct_id = pArgInfo->start + i;                // 对结构体赋值
        strcpy(stUser.user_no,    "20180223");               // 对结构体赋值
        strcpy(stUser.user_type,  "1");                      // 对结构体赋值
        strcpy(stUser.user_nm,    "Savens Liu");             // 对结构体赋值
        strcpy(stUser.user_addr,  "China");                  // 对结构体赋值
        strcpy(stUser.user_phone, "18672911111");            // 对结构体赋值
 
        if(RC_SUCC != lPush(pstSavm))      // 插入记录  
        {
            fprintf(stderr, "Insert error:(%d)(%s)\n", pstSavm->m_lErrno, 
                sGetTError(pstSavm->m_lErrno));
            return NULL;
        }
    }

    vCloneFree(pstSavm);
    return NULL;
}

int   main(int argc, char *argv[])
{
    uint64  uTime = 0;
    ARG     arg[100];
    pthread_t thread[10];
    int     i = 0, j = 0, rows = 0, num;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();
 
    if(1 != argc)
        num = strlen(argv[1])>0?atoi(argv[1]):1;
    else
        num = 1;

    /* 初始化QUEUE_USER_INFO表，每张表都需要初始化一次, 对于表重建后，需要重新初始化一次。*/          
    vHoldConnect(pstSavm);
    if(RC_SUCC != lAttchTable(pstSavm, QUEUE_USER_INFO))
    {
        fprintf(stderr, "attch failed, err:(%d)(%s)\n", pstSavm->m_lErrno, sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }
 
    rows = 4000000 / num;
    for(uTime = get_tick_time(); i < num; i++)
    {
        arg[i].threadIndex = i + 1;
        arg[i].start = rows * i;
        arg[i].rows = rows;
        pthread_create(&thread[i], NULL, vPushUserInfo, (void*)&arg[i]);
    }

    for(j = 0; j < num; j++)
         pthread_join(thread[j], NULL);

    fprintf(stdout, "cost_time:[%lld]\r\n", get_tick_time() - uTime);
    vTvmDisconnect(pstSavm);

    fprintf(stdout, "新增记录成功, completed successfully!!!\n");
    fflush(stderr);
    return RC_SUCC;
}
