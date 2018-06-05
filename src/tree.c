/*
*  Copyright (c) 2018 Savens Liu
*
*  The original has been patented, Open source is not equal to open rights. 
*  Anyone can clone, download, learn and discuss for free.  Without the permission 
*  of the copyright owner or author, it shall not be merged, published, licensed or sold. 
*  The copyright owner or author has the right to pursue his responsibility.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
*  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
*  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
*  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
*  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
*  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
*  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
*  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
*  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
*  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*/

#include    "tvm.h"
#include    "tmain.h"

/*************************************************************************************************
    globle
 *************************************************************************************************/
SATvm   g_stSavm = {0};
TblDef  g_stTblDef[TVM_MAX_TABLE] = {0};

extern long       _lInsertByRt(SATvm *pstSavm);
extern long       _lGroupByRt(SATvm *pstSavm, size_t *plOut, void **ppvOut);
extern long       _lSelectByRt(SATvm *pstSavm, void *psvOut);
extern long       _lCountByRt(SATvm *pstSavm, size_t *plCount);
extern long       _lDeleteByRt(SATvm *pstSavm);
extern long       _lUpdateByRt(SATvm *pstSavm, void *pvUpdate);
extern long       _lReplaceByRt(SATvm *pstSavm, void *pvReplace);
extern long       _lTruncateByRt(SATvm *pstSavm, TABLE t);
extern long       _lQueryByRt(SATvm *pstSavm, size_t *plOut, void **ppsvOut);
extern long       _lExtremeByRt(SATvm *pstSavm, void *psvOut);
extern void       _vDropTableByRt(SATvm *pstSavm, TABLE t);
extern long       _lRenameTableByRt(SATvm *pstSavm, TABLE to, TABLE tn);
/*************************************************************************************************
    macro
 *************************************************************************************************/

/*************************************************************************************************
    Error message definition
 *************************************************************************************************/
static char    tvmerr[128][MAX_INDEX_LEN] = {
    "completed successfully",
    "sever exception",
    "index field values is null",
    "condition is null",
    "no space for insert data",
    "generate shm key failure",
    "Invalid parameter or shm has disappeared",
    "shared memory already exists",
    "shared memory has been deleted",
    "Permission denied .shm",
    "Insufficient core memory .shm",
    "data truck version mismatch",
    "size is error to creat data block",
    "unique index definition overflow",
    "unique index length overflow",
    "normal index definition overflow",
    "normal index length overflow",
    "index type not define",
    "field define overflow",
    "index data mismatch",
    "field type not define",
    "memory has not been initialized",
    "unique index repeat",
    "no space for create index",
    "no data be found",
    "more then one records be selected",
    "malloc memory error",
    "cursor invalid",
    "table not define",
    "file not exist",
    "semget condition is null",
    "Invalid parameter or sem has disappeared",
    "semaphore already exists",
    "semaphore has been deleted",
    "Permission denied .sem",
    "Insufficient core memory .sem",
    "Semaphore value out of limit",
    "SQL syntax is error",
    "SQL syntax not be supported",
    "SQL no table name be inputted",
    "SQL field is not selected",
    "SQL conditional syntax error",
    "SQL field syntax error",
    "SQL where syntax error",
    "table not found",
    "SQL fields does not match the value",
    "set the read lock failure",
    "unlock read lock failure",
    "set the write lock failure",
    "unlock write lock failure",
    "socket connect failure",
    "socket connect timeout",
    "create socket failure",
    "socket recv failure",
    "socket send failure",
    "socket bind failure",
    "socket listen failure",
    "socket send timeout",
    "socket read timeout",
    "socket reset by peer",
    "socket communication anomaly",
    "epoll add fd error",
    "create epoll fd failure",
    "delete epoll fd failure",
    "socket accept failure",
    "SQL remote does not support",
    "file not found",
    "boot parameters error",
    "parameters table related error",
    "Incompatible version",
    "domain not register",
    "domain work does not support",
    "sequence does not exist",
    "file is not set",
    "record data too long",
    "Resource unavailable",
    "message queue already exists",
    "Permission denied .msg",
    "Insufficient(msg) core memory",
    "Invalid parameter or msg has disappeared",
    "message Invalid address",
    "message queue has been deleted",
    "message text length is greater than msgsz",
    "Interrupted by signal",
    "msgsnd queue overrun",
    "initial child process failed",
    "field not exist",
    "table already exist",
    "The transaction has not been opened yet",
    "The transaction has not been register",
    "domain not initail",
    "table field not define",
    "set field value error",
    "update field not set",
    "extreme set decorate error",
    "group set decorate error",
    "the table of field is missing",
    "queue waiting for timeout",
    "queue waiting for failure",
    "created queue is too big",
    "table does not support this operation",
    "",
};

/*************************************************************************************************
    description：error message customization
    parameters:
    return:
  *************************************************************************************************/
void    vRedeError(long err, char *s)
{
    if(100 <= err || !s)
        return ;
    strncpy(tvmerr[err], s, MAX_INDEX_LEN - 1);
}

/*************************************************************************************************
    description：output hexadecimal
    parameters:
    return:
  *************************************************************************************************/
void    vPrintHex(char *s, long lIdx, bool bf)
{
    int     i = 0;
    FILE    *fp = NULL;    

    if(!bf)
    {    
        for(i = 0; i < lIdx; i ++)
        {
            if((ushar)s[i] > 0x80 && (ushar)s[i + 1] >= 0x40)
            {
                fprintf(stdout, "\033[0;35m%c%c\033[0m", s[i], s[i + 1]);
                ++ i;
                continue;
            }
            
            if(s[i] > 40 && s[i] < 126)
                fprintf(stdout, "\033[0;35m%C\033[0m", s[i]);
            else
                fprintf(stdout, "/x%0X ", (ushar)s[i]);
        }
    }
    else
    {
        fp = fopen("log.txt", "a");
        fwrite(s, 1, lIdx,  fp);
        fclose(fp);
    }

    fprintf(stdout, "\n");
    fflush(stdout);
}

/*************************************************************************************************
    description：System table creation
    parameters:
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
  *************************************************************************************************/
CREATE  lCreateTvmIndex()
{   
    DEFINE(SYS_TVM_INDEX, "", TIndex)
    FIELD(TIndex,    m_table,        FIELD_LONG)
    FIELD(TIndex,    m_lType,        FIELD_LONG)
    FIELD(TIndex,    m_szTable,      FIELD_CHAR)
    FIELD(TIndex,    m_szPart,       FIELD_CHAR)
    FIELD(TIndex,    m_szOwner,      FIELD_CHAR)
    FIELD(TIndex,    m_yKey,         FIELD_LONG)
    FIELD(TIndex,    m_shmID,        FIELD_LONG)
    FIELD(TIndex,    m_semID,        FIELD_LONG)
    FIELD(TIndex,    m_lPid,         FIELD_LONG)
    FIELD(TIndex,    m_lValid,       FIELD_LONG)
    FIELD(TIndex,    m_lMaxRows,     FIELD_LONG)
    FIELD(TIndex,    m_lRowSize,     FIELD_LONG)
    FIELD(TIndex,    m_lLocal,       FIELD_LONG)
    FIELD(TIndex,    m_lState,       FIELD_LONG)
    FIELD(TIndex,    m_lPers,        FIELD_LONG)  
    FIELR(TIndex,    m_szTime,       FIELD_LONG)  

    CREATE_IDX(NORMAL)
    IDX_FIELD(TIndex, m_szTable,     FIELD_CHAR);
    IDX_FIELD(TIndex, m_szPart,      FIELD_CHAR);

    CREATE_IDX(UNQIUE)
    IDX_FIELD(TIndex, m_table,       FIELD_LONG);
    FINISH
}

/*************************************************************************************************
    description：field table creation
    parameters:
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
  *************************************************************************************************/
CREATE  lCreateTvmField()
{   
    DEFINE(SYS_TVM_FIELD, "", TField)
    FIELD(TField,    m_table,        FIELD_LONG)  
    FIELD(TField,    m_szOwner,      FIELD_CHAR)  
    FIELD(TField,    m_szTable,      FIELD_CHAR)  
    FIELD(TField,    m_szField,      FIELD_CHAR)  
    FIELD(TField,    m_lSeq,         FIELD_LONG)  
    FIELD(TField,    m_lAttr,        FIELD_LONG)  
    FIELD(TField,    m_lFrom,        FIELD_LONG)  
    FIELD(TField,    m_lLen,         FIELD_LONG)  
    FIELD(TField,    m_lIsPk,        FIELD_LONG)  

    CREATE_IDX(NORMAL)
    IDX_FIELD(TField, m_table,       FIELD_LONG);

    CREATE_IDX(UNQIUE)
    IDX_FIELD(TField, m_table,       FIELD_LONG);
    IDX_FIELD(TField, m_lSeq,        FIELD_LONG);
    FINISH
}

/*************************************************************************************************
    description：domain table creation
    parameters:
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
  *************************************************************************************************/
CREATE  lCreateTvmDomain()
{   
    DEFINE(SYS_TVM_DOMAIN, "", TDomain)
    FIELD(TDomain,    m_skSock,       FIELD_LONG)  
    FIELD(TDomain,    m_table,        FIELD_LONG)  
    FIELD(TDomain,    m_mtable,       FIELD_LONG)  
    FIELD(TDomain,    m_lLock,        FIELD_LONG)  
    FIELD(TDomain,    m_lGroup,       FIELD_LONG)  
    FIELD(TDomain,    m_lKeepLive,    FIELD_LONG)  
    FIELD(TDomain,    m_lLastTime,    FIELD_LONG)  
    FIELD(TDomain,    m_lTimeOut,     FIELD_LONG)  
    FIELD(TDomain,    m_lTryMax,      FIELD_LONG)  
    FIELD(TDomain,    m_lTryTimes,    FIELD_LONG)  
    FIELD(TDomain,    m_lRowSize,     FIELD_LONG)  
    FIELD(TDomain,    m_lStatus,      FIELD_LONG)  
    FIELD(TDomain,    m_lPers,        FIELD_LONG)  
    FIELD(TDomain,    m_lPort,        FIELD_LONG)
    FIELD(TDomain,    m_szIp,         FIELD_CHAR)
    FIELD(TDomain,    m_szTable,      FIELD_CHAR)  
    FIELD(TDomain,    m_szPart,       FIELD_CHAR)  
    FIELD(TDomain,    m_szOwner,      FIELD_CHAR)  

    CREATE_IDX(NORMAL)
    IDX_FIELD(TDomain, m_szTable,     FIELD_CHAR)
    IDX_FIELD(TDomain, m_szPart,      FIELD_CHAR)

    CREATE_IDX(UNQIUE)
    IDX_FIELD(TDomain, m_mtable,      FIELD_LONG)
    IDX_FIELD(TDomain, m_lPort,       FIELD_LONG)
    IDX_FIELD(TDomain, m_szIp,        FIELD_CHAR)

    FINISH
}

/*************************************************************************************************
    description：seque table creation 
    parameters:
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
  *************************************************************************************************/
CREATE  lCreateTvmSeque()
{   
    DEFINE(SYS_TVM_SEQUE, "", TSeque)
    FIELD(TSeque,    m_szSQName,     FIELD_CHAR) 
    FIELD(TSeque,    m_uIncrement,   FIELD_LONG) 

    CREATE_IDX(UNQIUE)
    IDX_FIELD(TSeque, m_szSQName,      FIELD_CHAR)

    FINISH
}

/*************************************************************************************************
    description：timestamp string
    parameters:
    return:
        char*
  *************************************************************************************************/
char*   sGetUpdTime()
{
    time_t  current;
    struct  tm *tm = NULL;
    static  char    szTime[20];

    tzset();
    time(&current);
    tm = localtime(&current);

    memset(szTime, 0, sizeof(szTime));
    snprintf(szTime, sizeof(szTime), "%4d/%02d/%02d %02d:%02d:%02d", tm->tm_year + 1900,
        tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    return szTime;
}

/*************************************************************************************************
    description：Conditional field setting
    parameters:
    return:
  *************************************************************************************************/
void    vSetCodField(FdCond *pstCond, uint ulen, uint uPos)
{
    register int i = 0;

    for(i; i < pstCond->uFldcmp; i ++)
    {
        if(pstCond->stFdKey[i].uFldpos == uPos)
            return ;
    }

    pstCond->stFdKey[pstCond->uFldcmp].uFldlen = ulen;
    pstCond->stFdKey[pstCond->uFldcmp].uFldpos = uPos;
    pstCond->uFldcmp ++;
    return ;
}

/*************************************************************************************************
    description：decorate field setting
    parameters:
    return:
  *************************************************************************************************/
void    vSetDecorate(FdCond *pstCond, uint ulen, uint uPos, Uenum em)
{
    register int i = 0;

    for(i; i < pstCond->uFldcmp; i ++)
    {
        if(pstCond->stFdKey[i].uFldpos == uPos)
            return ;
    }

    pstCond->stFdKey[pstCond->uFldcmp].uFldlen = ulen;
    pstCond->stFdKey[pstCond->uFldcmp].uFldpos = uPos;
    pstCond->stFdKey[pstCond->uFldcmp].uDecorate = em;
    pstCond->uFldcmp ++;
    
    return ;
}

/*************************************************************************************************
    description：pick index from data
    parameters:
    return:
       char*
  *************************************************************************************************/
char*    pPickIndex(long lIdx, TblKey *pstKey, void *psvData, char *pszIdx)
{
    register    int    i, j, m = 0;
    static char     szZore[MAX_INDEX_LEN] = {0};

    if(!psvData)        return NULL;

    for(i = 0, j = 0; i < lIdx; i ++)
    {
        if(FIELD_CHAR != pstKey[i].m_lAttr)
            m ++;    
        memcpy(pszIdx + j, psvData + pstKey[i].m_lFrom, pstKey[i].m_lLen);
        j += pstKey[i].m_lLen;
    }

    if(0 != m)    return pszIdx;

    if(!memcmp(pszIdx, szZore, j))
        return NULL;

    return pszIdx;
}

/*************************************************************************************************
    description：get index from data
    parameters:
    return:
       char*
  *************************************************************************************************/
char*    pGetIndex(FdCond *pstCond, long lIdx, TblKey *pstKey, void *psvData, char *pszIdx)
{
    register int i, j, m;

    if(!psvData || 0 == pstCond->uFldcmp)
        return NULL;

    for(i = 0, m = 0; i < lIdx; i ++)
    {
        for(j = 0; j < pstCond->uFldcmp; j ++)
        {
            if(pstKey[i].m_lFrom == pstCond->stFdKey[j].uFldpos)
            {
                memcpy(pszIdx + m, psvData + pstKey[i].m_lFrom, pstKey[i].m_lLen);
                m += pstKey[i].m_lLen;
                break;
            }
        }

        if(j == pstCond->uFldcmp)
            return NULL;
    }

    return pszIdx;
}

/*************************************************************************************************
    description：set number of current attaches
    parameters:
        pstSavm                    --stvm handle
        t                          --table
    return:
        void*                      --success
 *************************************************************************************************/
void    vSetQueueAttch(RunTime *pstRun, long lRevise)
{
    struct shmid_ds ds;

    if(TYPE_MQUEUE == pstRun->m_lType || pstRun->m_pvAddr) 
        return ;

    memset(&ds, 0, sizeof(struct shmid_ds));
    if(0 != shmctl(pstRun->m_shmID, IPC_STAT, &ds))
    {
        ds.shm_nattch = 1;
        return ;
    }

    ds.shm_nattch = (ds.shm_nattch + lRevise) < 0 ? 1 : (ds.shm_nattch + lRevise);
    ((TblDef *)pstRun->m_pvAddr)->m_lGroup = ds.shm_nattch;

    return ;
}

/*************************************************************************************************
    description：get stvm handle
    parameters:
    return:
       void*
  *************************************************************************************************/
void*   pGetSATvm()
{
    return &g_stSavm;
}

/*************************************************************************************************
    description：clone stvm handle by thread
    parameters:
    return:
       void*
  *************************************************************************************************/
void    vCloneQueue(SATvm *pstSovm, TABLE t)
{
    RunTime  *pstRun = NULL;

    if(!pstSovm || NULL == (pstRun = (RunTime *)pGetRunTime(pstSovm, t)))
        return ;

    vSetQueueAttch(pstRun, 1);
    return ;
}

/*************************************************************************************************
    description：clone stvm handle by thread
    parameters:
    return:
       void*
  *************************************************************************************************/
void*   pCloneSATvm()
{
    SATvm    *pstSovm = (SATvm *)malloc(sizeof(SATvm));

    if(!pstSovm)    return NULL;

    memcpy(pstSovm, (SATvm *)pGetSATvm(), sizeof(SATvm));
    return pstSovm;
}

/*************************************************************************************************
    description：free the clone handle by thread
    parameters:
    return:
       void*
  *************************************************************************************************/
void    vCloneFree(SATvm *pstSovm)
{
    TFree(pstSovm);
}

/*************************************************************************************************
    description：initial stvm handle
    parameters:
    return:
  *************************************************************************************************/
long    lAttchTable(SATvm *pstSovm, TABLE t)
{
    RunTime    *pstRun = (RunTime *)pGetRunTime(pGetSATvm(), t);

    if(!pstSovm) return RC_FAIL;

    if(pstRun->m_bAttch && pstRun->m_pvAddr)
    {
        memcpy((RunTime *)pGetRunTime(pstSovm, t), pstRun, sizeof(RunTime));
        return RC_SUCC;
    }

    if(RC_SUCC != lInitSATvm(pGetSATvm(), t))
        return RC_FAIL;

    if(NULL == (pstRun = (RunTime *)pInitHitTest(pGetSATvm(), t)))
       return RC_FAIL;

    memcpy((RunTime *)pGetRunTime(pstSovm, t), pstRun, sizeof(RunTime));
    return RC_SUCC;
}

/*************************************************************************************************
    description：initial stvm handle
    parameters:
    return:
  *************************************************************************************************/
void    vInitSATvm(SATvm *pstSavm)
{
    memset(pstSavm, 0, sizeof(SATvm));
}

/*************************************************************************************************
    description：User custom error message
    parameters:
    return:
 *************************************************************************************************/
void    vSetTvmMsg(SATvm *pstSamo, char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(pstSamo->m_szMsg, sizeof(pstSamo->m_szMsg), fmt, ap);
    va_end(ap);
}

/*************************************************************************************************
    description：get custom error message
    parameters:
    return:
       char*
  *************************************************************************************************/
char*    sGetTvmMsg(SATvm *pstSamo)
{
    return pstSamo->m_szMsg;
}

/*************************************************************************************************
    description：Gets the operation error code
    parameters:
    return:
       long                        --errno code
  *************************************************************************************************/
long    lGetTErrno()
{
    return g_stSavm.m_lErrno; 
}

/*************************************************************************************************
    description：set operation error code index from data
    parameters:
        error                      --errno
    return:
  *************************************************************************************************/
void    vSetTErrno(long err)
{
    g_stSavm.m_lErrno = err;    
}

/*************************************************************************************************
    description：Gets the operation error message
    parameters:
        error                      --errno
    return:
       char*
  *************************************************************************************************/
char*    sGetTError(long err)
{
    return tvmerr[(uint)err];
}

/*************************************************************************************************
    description：Operation STVM affects the number of records.
    parameters:
    return:
       size_t
  *************************************************************************************************/
size_t    lGetEffect()
{
    return  g_stSavm.m_lEffect;
}

/*************************************************************************************************
    description：Get the table run handle
    parameters:
    return:
       RunTime*
  *************************************************************************************************/
RunTime*   pGetRunTime(SATvm *pstSavm, TABLE t)
{
    return &pstSavm->stRunTime[t];
}

/*************************************************************************************************
    description：get table starting address
    parameters:
    return:
       void*
  *************************************************************************************************/
void*    pGetAddr(SATvm *pstSavm, TABLE t)
{
    return ((RunTime *)pGetRunTime(pstSavm, t))->m_pvAddr;
}

/*************************************************************************************************
    description：Gets the table structure information
    parameters:
        t                          --table
    return:
        TblDef*
  *************************************************************************************************/
TblDef* pGetTblDef(TABLE t)
{
    return &g_stTblDef[t];
}

/*************************************************************************************************
    description：Gets the table structure information
    parameters:
        pvAddr                     --address
    return:
        RWLock*
  *************************************************************************************************/
RWLock* pGetRWLock(char* pvAddr)
{
    return (RWLock *)&((TblDef *)pvAddr)->m_rwlock;
}

/*************************************************************************************************
    description：initial the table structure information
    parameters:
        pvAddr                     --address
    return:
        RWLock*
  *************************************************************************************************/
void    vInitTblDef(TABLE t)
{
    memset(&g_stTblDef[t], 0, sizeof(TblDef));
}

/*************************************************************************************************
    description：Get the number of query index groups
    parameters:
        t                          --table
    return:
        long
  *************************************************************************************************/
long    lGetTblGroup(TABLE t)
{
    return g_stTblDef[t].m_lGroup;
}

/*************************************************************************************************
    description：Get the maximum number of table support records
    parameters:
        t                          --table
    return:
        size_t
 *************************************************************************************************/
size_t   lGetTblRow(TABLE t)
{
    return g_stTblDef[t].m_lMaxRow;
}

/*************************************************************************************************
    description：Get the current record number of valid records
    parameters:
        t                          --table
    return:
        size_t
 *************************************************************************************************/
size_t   lGetTblValid(TABLE t)
{
    return g_stTblDef[t].m_lValid;
}

/*************************************************************************************************
    description：Get the table's unique index length
    parameters:
        t                          --table
    return:
        long
 *************************************************************************************************/
long    lGetIdxLen(TABLE t)
{
    return g_stTblDef[t].m_lIdxLen;
}

/*************************************************************************************************
    description：Get the location of the table index
    parameters:
        t                          --table
    return:
        size_t
 *************************************************************************************************/
size_t    lGetIdxPos(TABLE t)
{
    return g_stTblDef[t].m_lTreePos;
}

/*************************************************************************************************
    description：Gets the unique index of the table Root position
    parameters:
        t                          --table
    return:
        size_t
 *************************************************************************************************/
size_t    lGetIdxRoot(TABLE t)
{
    return g_stTblDef[t].m_lTreeRoot;
}

/*************************************************************************************************
    description：Get NIL position of the table
    parameters:
        t                          --table
    return:
        size_t
 *************************************************************************************************/
size_t   lGetNodeNil(TABLE t)
{
    return g_stTblDef[t].m_lNodeNil;
}

/*************************************************************************************************
    description：Get the index of the table query group length
    parameters:
        t                          --table
    return:
        long
 *************************************************************************************************/
long    lGetGrpLen(TABLE t)
{
    return g_stTblDef[t].m_lGrpLen;
}

/*************************************************************************************************
    description：Get the location of the index group for the table
    parameters:
        t                          --table
    return:
        size_t
 *************************************************************************************************/
size_t    lGetGrpPos(TABLE t)
{
    return g_stTblDef[t].m_lGroupPos;
}

/*************************************************************************************************
    description：Get the root position of the table
    parameters:
        t                          --table
    return:
        size_t
 *************************************************************************************************/
size_t    lGetGrpRoot(TABLE t)
{
    return g_stTblDef[t].m_lGroupRoot;
}

/*************************************************************************************************
    description：Gets the offset of the index list 
    parameters:
        t                          --table
    return:
        size_t
 *************************************************************************************************/
size_t    lGetListOfs(TABLE t)
{
    return g_stTblDef[t].m_lListOfs;
}

/*************************************************************************************************
    description：Gets the position of the index list 
    parameters:
        t                          --table
    return:
        size_t
 *************************************************************************************************/
size_t    lGetListPos(TABLE t)
{
    return g_stTblDef[t].m_lListPos;
}

/*************************************************************************************************
    description：Get table data area location
    parameters:
        t                          --table
    return:
        size_t
 *************************************************************************************************/
size_t    lGetTblData(TABLE t)
{
    return g_stTblDef[t].m_lData;
}

/*************************************************************************************************
    description：Get the number of unique index combinations
    parameters:
        t                          --table
    return:
        uint
 *************************************************************************************************/
uint    lGetIdxNum(TABLE t)
{
    return g_stTblDef[t].m_lIdxUp;
}

/*************************************************************************************************
    description：Get unique index field list
    parameters:
        t                          --table
    return:
        TblKey
        size_t
 *************************************************************************************************/
TblKey* pGetTblIdx(TABLE t)
{
    return  g_stTblDef[t].m_stIdxUp;
}

/*************************************************************************************************
    description：Get the number of index combinations
    parameters:
        t                          --table
    return:
        uint
 *************************************************************************************************/
uint    lGetGrpNum(TABLE t)
{
    return g_stTblDef[t].m_lGrpUp;
}

/*************************************************************************************************
    description：Get index field list
    parameters:
        t                          --table
    return:
        TblKey
 *************************************************************************************************/
TblKey* pGetTblGrp(TABLE t)
{
    return  g_stTblDef[t].m_stGrpUp;
}

/*************************************************************************************************
    description：Get the entire table size
    parameters:
        t                          --table
    return:
        size_t
 *************************************************************************************************/
size_t    lGetTableSize(TABLE t)    
{
    return g_stTblDef[t].m_lTable;
}

/*************************************************************************************************
    description：Get a table to define a single record size
    parameters:
        t                          --table
    return:
        long
 *************************************************************************************************/
long    lGetRowSize(TABLE t)
{
    return g_stTblDef[t].m_lReSize;
}

/*************************************************************************************************
    description：Get Table Defines a single data block size
    parameters:
        t                          --table
    return:
        size_t
 *************************************************************************************************/
size_t    lGetRowTruck(TABLE t)
{
    return g_stTblDef[t].m_lTruck;
}

/*************************************************************************************************
    description：Get the number of table fields
    parameters:
        t                          --table
    return:
        long
 *************************************************************************************************/
long    lGetFldNum(TABLE t)
{
    return g_stTblDef[t].m_lIdxNum;
}

/*************************************************************************************************
    description：Get the table fields
    parameters:
        t                          --table
    return:
        TblKey
 *************************************************************************************************/
TblKey* pGetTblKey(TABLE t)
{
    return  g_stTblDef[t].m_stKey;
}

/*************************************************************************************************
    description：Get the field by offset
    parameters:
        t                          --table
    return:
        TblKey
 *************************************************************************************************/
TblKey*    pGetFldKey(TABLE t, uint uFrom, uint uLen)
{
    long    i = 0;
    TblKey  *pstKey = (TblKey *)pGetTblKey(t);

    for(i = 0; i < lGetFldNum(t); i ++)
    {
        if(pstKey[i].m_lFrom == uFrom && pstKey[i].m_lLen == uLen)
            return &pstKey[i];
    }

    return NULL;
}

/*************************************************************************************************
    description：initial field condition
    parameters:
        t                          --table
    return:
 *************************************************************************************************/
void    vCondInsInit(FdCond *pstCond, TABLE t)
{
    long    i = 0;
    TblKey  *pstKey = (TblKey *)pGetTblKey(t);

    for(pstCond->uFldcmp = lGetFldNum(t); i < pstCond->uFldcmp; i ++)
    {
        pstCond->stFdKey[i].uFldpos = pstKey[i].m_lFrom;
        pstCond->stFdKey[i].uFldlen = pstKey[i].m_lLen;
    }

    return ;
}

/*************************************************************************************************
    description：Set the field properties
    parameters:
        pstCond                    --condion field
        t                          --table
    return:
        true
 *************************************************************************************************/
bool    bSetCondAttr(FdCond *pstCond, TABLE t, Uenum eCheck)
{
    FdKey   *pstFd;
    register int i, j;
    bool    bCheck = false;
    TblKey  *pstKey = (TblKey *)pGetTblKey(t);

    for(j = 0; j < pstCond->uFldcmp; j ++)
    {
        pstFd = &pstCond->stFdKey[j];
        for(i = 0; i < lGetFldNum(t); i ++)
        {
            if(pstKey[i].m_lFrom == pstFd->uFldpos)
            {
                pstFd->uDecorate |= pstKey[i].m_lAttr;
                break;
            }
        }

        if(pstFd->uDecorate & eCheck)
            bCheck = true;    
    }

    return bCheck;
}

/*************************************************************************************************
    description：Check to see if it is an index field.
    parameters:
        t                          --table
        pstRrp                     --field list
    return:
        true                       --yes
        false                      --no
 *************************************************************************************************/
bool    bIsGroupFld(TABLE t, FdCond *pstRrp)
{
    long    i, j;
    FdKey   *pstFd;
    TblKey  *pstKey = (TblKey *)pGetTblGrp(t);

    if(pstRrp->uFldcmp != lGetGrpNum(t))
        return false;

    for(i = 0; i < pstRrp->uFldcmp; i ++)
    {
           pstFd = &pstRrp->stFdKey[i]; 
        for(j = 0; j < lGetGrpNum(t); j ++)
        {
            if(pstKey[j].m_lFrom == pstFd->uFldpos)
                break;
        }
      
        if(j == lGetGrpNum(t))
            return false;
    }

    return true;
}

/*************************************************************************************************
    description：Get table name
    parameters:
        t                          --table
    return:
        char*
 *************************************************************************************************/
char* sGetTableName(TABLE t)
{
    return  g_stTblDef[t].m_szTable;
}

/*************************************************************************************************
    description：Gets the table partition
    parameters:
        t                          --table
    return:
        char*
 *************************************************************************************************/
char* sGetTablePart(TABLE t, char *pszNode)
{
    if(0x00 == g_stTblDef[t].m_szPart[0])
        strncpy(g_stTblDef[t].m_szPart, pszNode, MAX_FIELD_LEN);
    return  g_stTblDef[t].m_szPart;
}

/*************************************************************************************************
    description：Stand-alone mode, let stvm keep links improve efficiency
    parameters:
        pstSavm                    --stvm handle
    return:
 *************************************************************************************************/
void    vHoldConnect(SATvm *pstSavm)
{
    pstSavm->m_bHold = true;
}

/*************************************************************************************************
    description：Stand-alone mode, disconnect STVM
    parameters:
        pstSavm                    --stvm handle
    return:
 *************************************************************************************************/
void    vHoldRelease(SATvm *pstSavm)
{
    TABLE    t;
    RunTime  *pstRun = NULL;

    if(pstSavm != (SATvm *)pGetSATvm())
    {
        TFree(pstSavm);
        return;
    }

    pstSavm->m_bHold = false;
    for(t = 0; t < TVM_MAX_TABLE; t ++)
    {
        pstRun = (RunTime *)pGetRunTime(pstSavm, t);
        if(RES_REMOT_SID == pstRun->m_lLocal)
        {
            pstRun->m_lState = RESOURCE_INIT;
            continue;
        }

        pstRun->m_lCurType = 0;
        pstRun->m_lCurLine = 0;
        pstRun->m_pvCurAddr = NULL;
        TFree(pstRun->pstVoid);

        if(pstRun->m_pvAddr)
        {
            vSetQueueAttch(pstRun, -1);
            shmdt(pstRun->m_pvAddr);
        }
        pstRun->m_pvAddr = NULL;
        pstRun->m_bAttch = false;
    }

    return ;
}

/*************************************************************************************************
    description：Release the table resources
    parameters:
        pstSavm                    --stvm handle
        t                          --table
        bHold                      
    return:
 *************************************************************************************************/
void    _vTblRelease(SATvm *pstSavm, TABLE t, bool bHold)
{
    RunTime    *pstRun = (RunTime *)pGetRunTime(pstSavm, t);

    TFree(pstRun->pstVoid);
    if(bHold)    return ;

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        pstRun->m_lState = RESOURCE_INIT;
        return ;
    }

    pstRun->m_lCurType = 0;
    pstRun->m_lCurLine = 0;
    pstRun->m_pvCurAddr = NULL;

    if(pstRun->m_pvAddr)
    {
        vSetQueueAttch(pstRun, -1);
        shmdt(pstRun->m_pvAddr);
    }
    pstRun->m_pvAddr = NULL;
    pstRun->m_bAttch = false;
}

/*************************************************************************************************
    description：Close the table resources
    parameters:
        pstSavm                    --stvm handle
        t                          --table
    return:
 *************************************************************************************************/
void    vTblDisconnect(SATvm *pstSavm, TABLE t)
{
    _vTblRelease(pstSavm, t, pstSavm->m_bHold);
}

/*************************************************************************************************
    description：Forced to close the table resources
    parameters:
        pstSavm                    --stvm handle
        t                          --table
    return:
 *************************************************************************************************/
void    vForceDisconnect(SATvm *pstSavm, TABLE t)
{
    _vTblRelease(pstSavm, t, false);
}

/*************************************************************************************************
    description：API - begin work
    parameters:
        pstSavm                    --stvm handle
    return:
 *************************************************************************************************/
void    vBeginWork(SATvm *pstSavm)
{
    if(!pstSavm)    return ;

    pstSavm->m_lErrno = 0;
    pstSavm->m_lEffect = 0;
    pstSavm->m_bWork = true;
}

/*************************************************************************************************
    description：API - end work
    parameters:
        pstSavm                    --stvm handle
    return:
 *************************************************************************************************/
void    vEndWork(SATvm *pstSavm)
{
    if(!pstSavm)    return ;

    pstSavm->m_lErrno = 0;
    pstSavm->m_lEffect = 0;
    pstSavm->m_bWork = false;
    lCommitWork(pstSavm);
}

/*************************************************************************************************
    description：Save abnormal transaction data
    parameters:
        pstWork                    --work
    return:
 *************************************************************************************************/
void    vBackupWork(TWork *pstWork)
{
    FILE    *fp = NULL;
    char    szPath[512];

    memset(szPath, 0, sizeof(szPath));
    snprintf(szPath, sizeof(szPath), "%s/%s", getenv("TVMDBD"), WORK_ERROR_LOG);

    if(NULL == (fp = fopen(szPath, "ab")))
        return ;

    fwrite(pstWork, FPOS(TWork, m_lOperate) + FLEN(TWork, m_lOperate), 1, fp); 
    if(OPERATE_UPDATE == pstWork->m_lOperate)
        fwrite(pstWork->m_pvNew, pstWork->m_lRowSize, 1, fp); 
    fwrite(pstWork->m_pvData, pstWork->m_lRowSize, 1, fp); 
    fclose(fp);
}

/*************************************************************************************************
    description：API - rollback work
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lRollbackWork(SATvm *pstSavm)
{
    TWork   *pstWork = NULL;
    size_t  lRow = 0, lErrno = 0;
    CMList  *pstNode = NULL, *pstList = NULL;

    pstSavm->m_lEffect = 0;
    if(!pstSavm || !pstSavm->m_bWork)
    {
        pstSavm->m_lErrno = WORK_NOT_OPEN;
        return RC_FAIL;
    }

    //  In order to roll back from the tail to the front
    for(pstNode = pGetCMTail(pstSavm->m_pstWork); pstNode; )
    {
        if(NULL == (pstWork = (TWork *)pstNode->m_psvData))
        {
            pstNode = pstNode->pstLast;
            continue;
        }

        if(RC_SUCC != lInitSATvm(pstSavm, pstWork->m_table))
        {
            lErrno = pstSavm->m_lErrno;
            vBackupWork(pstWork);
            if(pstNode->pstLast)
                pstNode->pstLast->pstNext = pstNode->pstNext;
            else
                pstSavm->m_pstWork = pstNode->pstNext;

            if(pstNode->pstNext)
                pstNode->pstNext->pstLast = pstNode->pstLast;    
            pstList = pstNode;
            pstNode = pstNode->pstLast;
            if(pstWork)
            {
                TFree(pstWork->m_pvNew);    
                TFree(pstWork->m_pvData);    
            }
            TFree(pstList->m_psvData);
            TFree(pstList);
            continue;
        }

        pstSavm->m_bPrew = true;   // Transaction is turned on
        pstSavm->lSize   = pstWork->m_lRowSize;
        if(OPERATE_UPDATE == pstWork->m_lOperate)
        {
            pstSavm->lFind = FIRST_ROW;
            pstSavm->pstVoid = (void *)pstWork->m_pvNew;
            memcpy(&pstSavm->stCond, &pstWork->m_stUpdt, sizeof(FdCond));
            memcpy(&pstSavm->stUpdt, &pstWork->m_stCond, sizeof(FdCond));
            if(RC_SUCC != lUpdate(pstSavm, (void *)pstWork->m_pvData))
            {
                vBackupWork(pstWork);
                lErrno = pstSavm->m_lErrno;
            }
            else
                lRow += pstSavm->m_lEffect;
        }
        else if(OPERATE_DELETE == pstWork->m_lOperate)
        {
            pstSavm->lFind = FIRST_ROW;
            pstSavm->pstVoid = (void *)pstWork->m_pvData;
            memcpy(&pstSavm->stCond, &pstWork->m_stCond, sizeof(FdCond));
            if(RC_SUCC != lInsert(pstSavm))
            {
                vBackupWork(pstWork);
                lErrno = pstSavm->m_lErrno;
            }
            else
                lRow += pstSavm->m_lEffect;
        }
        else if(OPERATE_INSERT == pstWork->m_lOperate)
        {
            pstSavm->pstVoid = (void *)pstWork->m_pvData;
            memcpy(&pstSavm->stCond, &pstWork->m_stCond, sizeof(FdCond));
            if(RC_SUCC != lDelete(pstSavm))
            {
                vBackupWork(pstWork);
                lErrno = pstSavm->m_lErrno;
            }
            else
                lRow += pstSavm->m_lEffect;
        }

        //   delete this node
        if(pstNode->pstLast)
            pstNode->pstLast->pstNext = pstNode->pstNext;
        else
            pstSavm->m_pstWork = pstNode->pstNext;

        if(pstNode->pstNext)
            pstNode->pstNext->pstLast = pstNode->pstLast;    
        pstList = pstNode;
        pstNode = pstNode->pstLast;
        if(pstWork)
        {
            TFree(pstWork->m_pvNew);    
            TFree(pstWork->m_pvData);    
            pstSavm->pstVoid = NULL;
        }
        TFree(pstList->m_psvData);
        TFree(pstList);
    }

    pstSavm->lFind = 0;
    pstSavm->m_lEffect = lRow;
//  pstSavm->m_pstWork = NULL;
    pstSavm->m_bPrew = false;    // Transaction is turned off
    pstSavm->m_lErrno = lErrno;

    return RC_SUCC;
}

/*************************************************************************************************
    description：API - commit work
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lCommitWork(SATvm *pstSavm)
{
    TWork   *pstWork = NULL;
    CMList  *pstNode = NULL, *pstList = NULL;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    for(pstNode = pstSavm->m_pstWork; pstNode; )
    {
        if(NULL == (pstWork = (TWork *)pstNode->m_psvData))
        {
            pstNode = pstNode->pstNext;
            continue;
        }

        if(pstNode->pstLast)
            pstNode->pstLast->pstNext = pstNode->pstNext;
        else
            pstSavm->m_pstWork = pstNode->pstNext;

        if(pstNode->pstNext)
            pstNode->pstNext->pstLast = pstNode->pstLast;    
        pstList = pstNode;
        pstNode = pstNode->pstNext;
        if(pstWork)
        {
            TFree(pstWork->m_pvNew);    
            TFree(pstWork->m_pvData);    
        }
        TFree(pstList->m_psvData);
        TFree(pstList);
    }
//  pstSavm->m_bPrew = false;

    return RC_SUCC;
}

/*************************************************************************************************
    description：Record the transaction
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lRecordWork(SATvm *pstSavm, void *pvoData, long lOperate)
{
    TWork    stWork;

    // where transaction is on, do not record the transaction itself
    if(!pstSavm->m_bWork || pstSavm->m_bPrew) 
        return RC_SUCC;

    memset(&stWork, 0, sizeof(TWork));
    stWork.m_lOperate = lOperate;
    stWork.m_lRowSize = pstSavm->lSize;
    stWork.m_table    = pstSavm->tblName;
    if(NULL == (stWork.m_pvData = (void *)malloc(pstSavm->lSize)))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    if(OPERATE_UPDATE == lOperate)
    {
        if(NULL == (stWork.m_pvNew = (void *)malloc(pstSavm->lSize)))
        {
            pstSavm->m_lErrno = MALLC_MEM_ERR;
            return RC_FAIL;
        }

        memcpy(stWork.m_pvNew, pstSavm->pstVoid, pstSavm->lSize);
        vCondInsInit(&stWork.m_stUpdt, pstSavm->tblName);

        memcpy(stWork.m_pvData, pvoData, pstSavm->lSize);
        memcpy(&stWork.m_stCond, &pstSavm->stUpdt, sizeof(FdCond));
    }
    else if(OPERATE_INSERT == lOperate)
    {
        memcpy(stWork.m_pvData, pvoData, pstSavm->lSize);
        vCondInsInit(&stWork.m_stCond, pstSavm->tblName);
    }
    else
    {
        memcpy(stWork.m_pvData, pvoData, pstSavm->lSize);
        memcpy(&stWork.m_stCond, &pstSavm->stCond, sizeof(FdCond));
    }

    if(NULL == (pstSavm->m_pstWork = pInsertList(pstSavm->m_pstWork, &stWork, sizeof(TWork))))
        return RC_FAIL;

    return RC_SUCC;
}

/*************************************************************************************************
    description：Initialize the second level cache
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lInitSvCache(SATvm *pstSavm)
{
    size_t  lOut = 0, i;
    RunTime *pstRun = NULL;
    TIndex  *pstIndex = NULL;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    if(RC_SUCC != lExportTable(SYS_TVM_INDEX, &lOut, (void *)&pstIndex))
        return RC_FAIL;

    for(i = 0; i < lOut; i ++)
    {
        pstRun = (RunTime *)pGetRunTime(pstSavm, pstIndex[i].m_table);
        pstRun->m_lState = RESOURCE_ABLE;
        pstRun->m_shmID  = pstIndex[i].m_shmID;
        pstRun->m_semID  = pstIndex[i].m_semID;
        pstRun->m_lLocal = pstIndex[i].m_lLocal;
    }
    
    pstSavm->m_bCache = true;
    TFree(pstIndex);

    return RC_SUCC;
}

/*************************************************************************************************
    description：Gets the STVM handle by partition
    parameters:
        pstSavm                    --stvm handle
        pstTable                   --table name
        pstPart                    --table part
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
void*   pPartSATvm(SATvm *pstSavm, char *pszTable, char *pszPart)
{
    TABLE    i;
    TIndex  stIndex;
    RunTime *pstRun = NULL;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return NULL;
    }

    memset(&stIndex, 0, sizeof(TIndex));
    strncpy(stIndex.m_szPart, pszPart, sizeof(stIndex.m_szPart));
    strncpy(stIndex.m_szTable, pszTable, sizeof(stIndex.m_szTable));

    pstSavm->bSearch = TYPE_SYSTEM;
    conditinit(pstSavm, stIndex, SYS_TVM_INDEX)
    conditstr(pstSavm, stIndex, m_szPart, pszPart);
    conditstr(pstSavm, stIndex, m_szTable, pszTable);
    if(RC_SUCC != lSelect(pstSavm, (void *)&stIndex))
    {
        if(NO_DATA_FOUND == pstSavm->m_lErrno)
            pstSavm->m_lErrno = TBL_NOT_FOUND;
        return NULL;
    }

    pstSavm->pstVoid   = NULL;
    pstSavm->tblName = stIndex.m_table;

    pstRun = (RunTime *)pGetRunTime(pstSavm, pstSavm->tblName);
    pstRun->m_shmID  = stIndex.m_shmID;
    pstRun->m_semID  = stIndex.m_semID;
    pstRun->m_lLocal = stIndex.m_lLocal;
    return pstSavm;
}

/*************************************************************************************************
    description：Gets the table STVM handle.
    parameters:
        pstSavm                    --stvm handle
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lInitSATvm(SATvm *pstSavm, TABLE t)
{
    TIndex  stIndex;
    RunTime *pstRun = NULL;

    pstSavm->tblName = t;
    pstRun = (RunTime *)pGetRunTime(pstSavm, t);
    if(pstRun->m_bAttch && pstRun->m_pvAddr)
        return RC_SUCC;

    if(RESOURCE_ABLE == pstRun->m_lState)
    {
        pstSavm->pstVoid = NULL;
        pstSavm->bSearch = TYPE_CLIENT;
        return RC_SUCC;
    }

    pstSavm->bSearch = TYPE_SYSTEM;
    conditinit(pstSavm, stIndex, SYS_TVM_INDEX)
    conditnum(pstSavm, stIndex, m_table, t)
    if(RC_SUCC != lSelect(pstSavm, (void *)&stIndex))
    {
        if(NO_DATA_FOUND == pstSavm->m_lErrno)
            pstSavm->m_lErrno = TBL_NOT_FOUND;
        return RC_FAIL;
    }

    pstSavm->tblName   = t;
    pstSavm->pstVoid   = NULL;
    strcpy(pstSavm->m_szNode, stIndex.m_szOwner);

    pstRun->m_shmID    = stIndex.m_shmID;
    pstRun->m_semID    = stIndex.m_semID;
    pstRun->m_lLocal   = stIndex.m_lLocal;
    pstRun->m_lType    = stIndex.m_lType;
    pstRun->m_lRowSize = stIndex.m_lRowSize;

    return RC_SUCC;
}

/*************************************************************************************************
    description：Gets the table STVM handle.
    parameters:
        pstSavm                    --stvm handle
    return:
        void*                      --success
 *************************************************************************************************/
void*   pInitSATvm(TABLE t)
{
    SATvm     *pstSavm = (SATvm *)pGetSATvm();

    if(RC_SUCC != lInitSATvm(pstSavm, t))
        return NULL;

    return pstSavm;
}

/*************************************************************************************************
    description：Gets the KEY value for the connection to create the Shared memory
    parameters:
        pstSavm                    --stvm handle
    return:
        tKey 
 *************************************************************************************************/
key_t   yGetIPCPath(SATvm *pstSavm, Benum em)
{
    key_t    tKey = 0;
    char    szPath[512];

    memset(szPath, 0, sizeof(szPath));
    snprintf(szPath, sizeof(szPath), "%s", getenv("TVMDBD"));

    if(-1 == (tKey = ftok(szPath, em)))
    {
        pstSavm->m_lErrno = GENER_KEY_ERR;
        return RC_FAIL;
    }

    return tKey;
}

/*************************************************************************************************
    description：set alise
    parameters:
        pstSavm                    --stvm handle
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lSetAlias(SATvm *pstSavm, TABLE t, uint ulen, uint uPos, char *alias)
{
    register int  i;
    TblKey   *pstKey = (TblKey *)pGetTblKey(t);

    for(i = 0; i < lGetFldNum(t); i ++)
    {
        if(pstKey[i].m_lLen != ulen || pstKey[i].m_lFrom != uPos)
            continue;

        strncpy(pstKey[i].m_szAlias, alias, sizeof(pstKey[i].m_szAlias));
        return RC_SUCC;
    }

    return RC_FAIL;
}

/*************************************************************************************************
    description：map the member of struct value by alias
    parameters:
        pstSavm                    --stvm handle
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long   lSetTructByAlias(SATvm *pstSavm, TABLE t, void *pvData, const char *key, char *v)
{
    register int  i;
    TblKey    *pstKey = (TblKey *)pGetTblKey(t);

    for(i = 0; i < lGetFldNum(t); i ++)
    {
        if(strcmp(pstKey[i].m_szAlias, key))
            continue;

        switch(pstKey[i].m_lAttr)
        {
        case FIELD_CHAR:
            switch(pstKey[i].m_lLen)
            {
            case    1:
                memcpy(v + pstKey[i].m_lFrom, v, pstKey[i].m_lLen);
                return RC_SUCC;
            default:
                strncpy(v + pstKey[i].m_lFrom, v, pstKey[i].m_lLen);
                return RC_SUCC;
            }
            return RC_SUCC;
        case FIELD_DOUB:
            switch(pstKey[i].m_lLen)
            {
            case    4:
                *((float *)(v + pstKey[i].m_lFrom)) = atof(v);
                return RC_SUCC;
            case    8:
                *((double *)(v + pstKey[i].m_lFrom)) = atof(v);
                return RC_SUCC;
            default:
                return RC_SUCC;
            }
            return RC_SUCC;
        case FIELD_LONG:
            switch(pstKey[i].m_lLen)
            {
            case    2:
                *((sint *)(v + pstKey[i].m_lFrom)) = atoi(v);
                return RC_SUCC;
            case    4:
                *((int *)(v + pstKey[i].m_lFrom)) = atoi(v);
                return RC_SUCC;
            case    8:
                *((llong *)(v + pstKey[i].m_lFrom)) = atol(v);
                return RC_SUCC;
            default:
                return RC_SUCC;
            }
            return RC_SUCC;
        }
    }

    return RC_FAIL;
}

/*************************************************************************************************
    description：restore the table define from memory 
    parameters:
        pstSavm                    --stvm handle
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lResetDefine(SATvm *pstSavm, TABLE t)
{
    RunTime    *pstRun = (RunTime *)pGetRunTime(pstSavm, t);

    if(!pstRun->m_pvAddr)    return RC_FAIL;

    memcpy((void *)pGetTblDef(t), pstRun->m_pvAddr, sizeof(TblDef));
    return RC_SUCC;
}

/*************************************************************************************************
    description：Connect to Shared memory click test
    parameters:
        pstSavm                    --stvm handle
        t                          --table
    return:
        void*                      --success
 *************************************************************************************************/
void*    pInitHitTest(SATvm *pstSavm, TABLE t)
{
    RunTime    *pstRun = (RunTime *)pGetRunTime(pstSavm, t);

    if(RES_REMOT_SID == pstRun->m_lLocal || (pstRun->m_bAttch && pstRun->m_pvAddr))
        return pstRun;

    pstRun->m_pvAddr = (void* )shmat(pstRun->m_shmID, NULL, 0);
    if(NULL == pstRun->m_pvAddr || (void *)(-1) == (void *)pstRun->m_pvAddr)
    {
        if(EACCES == errno)    
            pstSavm->m_lErrno = SHM_ERR_ACCES;
        else if(ENOMEM == errno)
            pstSavm->m_lErrno = SHM_ERR_NOMEM;
        else 
            pstSavm->m_lErrno = SHM_ERR_INVAL;
        return NULL;
    }

    pstSavm->m_lErrno = 0;
    pstSavm->m_lEffect= 0;
    pstRun->m_bAttch  = true;
    pstSavm->lSize    = lGetRowSize(t);
    memcpy((void *)pGetTblDef(t), pstRun->m_pvAddr, sizeof(TblDef));

    return pstRun;
}

/*************************************************************************************************
    description：Connect to Shared memory
    parameters:
        pstSavm                    --stvm handle
        t                          --table
    return:
        void*                      --success
 *************************************************************************************************/
void*    pInitMemTable(SATvm *pstSavm, TABLE t)
{
    RunTime    *pstRun = (RunTime *)pGetRunTime(pstSavm, t);

    if(RES_REMOT_SID == pstRun->m_lLocal)    //  remote
        return pstRun;

    pstSavm->m_lErrno  = 0;
    pstSavm->m_lEffect = 0;
    if(pstRun->m_bAttch && pstRun->m_pvAddr)        
    {
        pstSavm->bSearch = TYPE_CLIENT;
        return pstRun;
    }

    if(TYPE_SYSTEM == pstSavm->bSearch) 
    {
        pstSavm->bSearch = TYPE_CLIENT;
        if(RC_FAIL == (pstSavm->m_yKey = yGetIPCPath(pstSavm, IPC_SHM)))
            return NULL;
        pstRun->m_shmID = shmget(pstSavm->m_yKey, 0, IPC_CREAT|0600);

        if(pstRun->m_shmID < 0)
        {
            switch(errno)
            {
            case   EEXIST:
                pstSavm->m_lErrno = SHM_ERR_EXIST;
                break;
            case   EIDRM:
                pstSavm->m_lErrno = SHM_ERR_EIDRM;
                break;
            case   EACCES:
                pstSavm->m_lErrno = SHM_ERR_ACCES;
                break;
            case   ENOMEM:
                pstSavm->m_lErrno = SHM_ERR_NOMEM;
                break;
            default:
                pstSavm->m_lErrno = SHM_ERR_INVAL;
                break;
            }
            return NULL;
        }
    }

    pstRun->m_pvAddr = (void* )shmat(pstRun->m_shmID, NULL, 0);
    if(NULL == pstRun->m_pvAddr || (void *)(-1) == (void *)pstRun->m_pvAddr)
    {
        if(EACCES == errno)    
            pstSavm->m_lErrno = SHM_ERR_ACCES;
        else if(ENOMEM == errno)
            pstSavm->m_lErrno = SHM_ERR_NOMEM;
        else 
            pstSavm->m_lErrno = SHM_ERR_INVAL;
        return NULL;
    }

    pstRun->m_bAttch = true;
    vSetQueueAttch(pstRun, 0);
    memcpy((void *)pGetTblDef(t), pstRun->m_pvAddr, sizeof(TblDef));

    if(pstSavm->lSize != lGetRowSize(t))
    {
        vTblDisconnect(pstSavm, t);
        pstSavm->m_lErrno = VER_NOT_MATCH;
        return NULL;
    }

    return pstRun;
}

/*************************************************************************************************
    description：Create a memory table space
    parameters:
        pstSavm                    --stvm handle
        t                          --table
        lSize                      --The block size
        bCreate 
    return:
        void*                      --success
 *************************************************************************************************/
void*    pCreateBlock(SATvm *pstSavm, TABLE t, size_t lSize, bool bCreate)
{
    RunTime    *pstRun = NULL;

    if(!pstSavm || lSize <= 0)
    {
        pstSavm->m_lErrno = BLCK_SIZE_ERR;
        return NULL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, t);
    memset(pstRun, 0, sizeof(RunTime));

    if(!bCreate)
    {
        pstSavm->m_yKey = IPC_PRIVATE;
        pstSavm->m_ySey = IPC_PRIVATE;
    }
    else
    {
        if(RC_FAIL == (pstSavm->m_yKey = yGetIPCPath(pstSavm, IPC_SHM)))
            return NULL;

        if(RC_FAIL == (pstSavm->m_ySey = yGetIPCPath(pstSavm, IPC_SEM)))
            return NULL;
    }

    pstRun->m_shmID = shmget(pstSavm->m_yKey, lSize, IPC_CREAT|IPC_EXCL|0600);
    if(pstRun->m_shmID < 0)
    {
        switch(errno)
        {
        case   EEXIST:
            pstSavm->m_lErrno = SHM_ERR_EXIST;
            break;
        case   EIDRM:
            pstSavm->m_lErrno = SHM_ERR_EIDRM;
            break;
        case   EACCES:
            pstSavm->m_lErrno = SHM_ERR_ACCES;
            break;
        case   ENOMEM:
            pstSavm->m_lErrno = SHM_ERR_NOMEM;
            break;
        default:
            pstSavm->m_lErrno = SHM_ERR_INVAL;
            break;
        }
        return NULL;
    }

    pstRun->m_pvAddr = (void* )shmat(pstRun->m_shmID, NULL, 0);
    if(NULL == pstRun->m_pvAddr || (void *)(-1) == (void *)pstRun->m_pvAddr)
    {
        if(EACCES == errno)    
            pstSavm->m_lErrno = SHM_ERR_ACCES;
        else if(ENOMEM == errno)
            pstSavm->m_lErrno = SHM_ERR_NOMEM;
        else 
            pstSavm->m_lErrno = SHM_ERR_INVAL;
        return NULL;
    }

//  memset(pstRun->m_pvAddr, 0, lSize);
    pstRun->m_bAttch = true;

    return pstRun;
}

/*************************************************************************************************
    description：add New index field
    parameters:
        t                          --table
        type                       --type
        from                       --from position
        len                        --length
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lAddIdxField(TABLE t, long type, long lFrom, long lLen, long lAttr, const char *pszDesc)
{
    long    lIdx = 0;
    TblKey  *pstKey = NULL;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

    if(UNQIUE == type)
    {
        for(lIdx = 0, pstKey = pGetTblIdx(t); lIdx < lGetIdxNum(t); lIdx++)
        {
            if((pstKey[lIdx].m_lFrom == lFrom) && pstKey[lIdx].m_lLen == lLen)
                return RC_SUCC;
        }

        if(MAX_FILED_IDX <= lGetIdxNum(t))
        {
            pstSavm->m_lErrno = IDX_DEF_SPILL;
            return RC_FAIL;
        }

        ((TblDef *)pGetTblDef(t))->m_lIdxLen += lLen;
        if(MAX_INDEX_LEN < (((TblDef *)pGetTblDef(t))->m_lIdxLen))
        {
            pstSavm->m_lErrno = IDX_LEN_SPILL;
            return RC_FAIL;
        }

        pstKey[lGetIdxNum(t)].m_lFrom = lFrom;
        pstKey[lGetIdxNum(t)].m_lLen  = lLen;
        pstKey[lGetIdxNum(t)].m_lAttr = lAttr;
        strncpy(pstKey[lGetIdxNum(t)].m_szField, pszDesc, sizeof(pstKey[lGetIdxNum(t)].m_szField));
        strcpy(pstKey[lGetIdxNum(t)].m_szAlias, pstKey[lGetIdxNum(t)].m_szField);

        ((TblDef *)pGetTblDef(t))->m_lIdxUp ++;
        ((TblDef *)pGetTblDef(t))->m_lIType |= type;

        return RC_SUCC;
    }
    else if(NORMAL == type || HASHID == type)
    {
        for(lIdx = 0, pstKey = pGetTblGrp(t); lIdx < lGetGrpNum(t); lIdx++)
        {
            if((pstKey[lIdx].m_lFrom == lFrom) && pstKey[lIdx].m_lLen == lLen)
                return RC_SUCC;
        }

        if(MAX_FILED_IDX <= lGetGrpNum(t))
        {
            pstSavm->m_lErrno = GRP_DEF_SPILL;
            return RC_FAIL;
        }

        ((TblDef *)pGetTblDef(t))->m_lGrpLen += lLen;
        if(MAX_INDEX_LEN < (((TblDef *)pGetTblDef(t))->m_lGrpLen))
        {
            pstSavm->m_lErrno = GRP_LEN_SPILL;
            return RC_FAIL;
        }

        pstKey[lGetGrpNum(t)].m_lFrom = lFrom;
        pstKey[lGetGrpNum(t)].m_lLen  = lLen;
        pstKey[lGetGrpNum(t)].m_lAttr = lAttr;
        strncpy(pstKey[lGetGrpNum(t)].m_szField, pszDesc, sizeof(pstKey[lGetGrpNum(t)].m_szField));
        strcpy(pstKey[lGetGrpNum(t)].m_szAlias, pstKey[lGetGrpNum(t)].m_szField);
        ((TblDef *)pGetTblDef(t))->m_lGrpUp ++;
        ((TblDef *)pGetTblDef(t))->m_lIType |= type;

        return RC_SUCC;
    }

    pstSavm->m_lErrno = IDX_TYP_NODEF;

    return RC_FAIL;
}

/*************************************************************************************************
    description：set index field
    parameters:
        t                          --table
        type                       --type
        from                       --from position
        len                        --length
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lSetTableIdx(TABLE t, long lFrom, long lLen, char *pszDesc, long lAttr, long lType)
{
    long    lIdx = 0;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();
    TblKey  *pstKey = (TblKey *)pGetTblKey(t);

    for(lIdx = 0; lIdx < lGetFldNum(t); lIdx ++)
    {
        if((pstKey[lIdx].m_lFrom == lFrom) && pstKey[lIdx].m_lLen == lLen)
           return RC_SUCC;
    }

    if(MAX_FILED_NUM <= lGetFldNum(t))
    {
        pstSavm->m_lErrno = FLD_DEF_SPILL;
        return RC_FAIL;
    }

    pstKey[lGetFldNum(t)].m_lFrom = lFrom;
    pstKey[lGetFldNum(t)].m_lLen  = lLen;
    pstKey[lGetFldNum(t)].m_lAttr = lAttr;
    pstKey[lGetFldNum(t)].m_lIsPk = lType;
    strncpy(pstKey[lGetFldNum(t)].m_szField, pszDesc, sizeof(pstKey[lGetFldNum(t)].m_szField));
    strcpy(pstKey[lGetFldNum(t)].m_szAlias, pstKey[lGetFldNum(t)].m_szField);
    ((TblDef *)pGetTblDef(t))->m_lIdxNum ++;

    return RC_SUCC;
}

/*************************************************************************************************
    description：find filed key
    parameters:
        pstIdx                     --field list
        lNum                       --number
        pszField                   --field name
    return:
        TblKey* 
 *************************************************************************************************/
TblKey*    pFindField(TblKey *pstIdx, long lNum, char *pszField)
{
    register int  i;

    for(i = 0; i < lNum; i ++)
    {
        if(!strcmp(pstIdx[i].m_szField, pszField))
            return &pstIdx[i];
    }

    return NULL;
}

/*************************************************************************************************
    description：insert table field
    parameters:
        pstSavm                    --stvm handle
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lInsertField(SATvm *pstSavm, TABLE t)
{
    TField    stField;
    long      i, lIdx = lGetFldNum(t);
    TblKey    *pstIdx = pGetTblKey(t);

    if(NULL == (pstSavm = (SATvm *)pInitSATvm(SYS_TVM_FIELD)))
        return RC_FAIL;

    conditbind(pstSavm, stField, SYS_TVM_FIELD)
    for(i = 0; i < lIdx; i ++)
    {
        memset(&stField, 0, sizeof(TField));
        stField.m_table = t;
        stField.m_lSeq  = i + 1;
        stField.m_lAttr = pstIdx[i].m_lAttr;
        stField.m_lFrom = pstIdx[i].m_lFrom;
        stField.m_lLen  = pstIdx[i].m_lLen;
        stField.m_lIsPk = pstIdx[i].m_lIsPk;
        strncpy(stField.m_szOwner, TVM_NODE_INFO, sizeof(stField.m_szOwner));
        strncpy(stField.m_szTable, sGetTableName(t), sizeof(stField.m_szTable));
        strncpy(stField.m_szField, pstIdx[i].m_szField, sizeof(stField.m_szField));

        if(RC_SUCC != lInsert(pstSavm))
            return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：Merge valid data to update fields
    parameters:
        pstSavm                    --stvm handle
        s
        p
        eType
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lMergeTruck(SATvm *pstSavm, FdCond *pstCond, char *p, char *pvData) 
{
    register int    i = 0;
    FdKey           *pFdKey;

    if(0 == pstCond->uFldcmp)
    {
        pstSavm->m_lErrno = UPDFD_NOT_SET;
        return RC_FAIL;
    }

    for(i = 0; i < pstCond->uFldcmp; i ++)
    {
        pFdKey = &pstCond->stFdKey[i]; 
        memcpy(pvData + pFdKey->uFldpos, p + pFdKey->uFldpos, pFdKey->uFldlen);
    }
}

/*************************************************************************************************
    description：Returns the record of the extremum field
    parameters:
        s                          --original date
        p                          --records to be compared
        pstKey                     --field key
        eType                      --MATCH_MAX or MATCH_MIN
    return:
        void*
 *************************************************************************************************/
void*    pvCompExtrem(void *s, void *p, TblKey *pstKey, Uenum eType)
{
    if(!p)    return s;

    if(eType & MATCH_MAX)
    {
        switch(pstKey->m_lAttr)
        {
        case FIELD_DOUB:
            switch(pstKey->m_lLen)
            {
            case    4:
                if(*((float *)(p + pstKey->m_lFrom)) < *((float *)(s + pstKey->m_lFrom)))
                    return s;
                return p;
            case    8:
                if(*((double *)(p + pstKey->m_lFrom)) < *((double *)(s + pstKey->m_lFrom)))
                    return s;
                return p;
            default:
                return p;
            }
            return p;
        case FIELD_LONG: 
            switch(pstKey->m_lLen)
            {
            case    2:
                if(*((sint *)(p + pstKey->m_lFrom)) < *((sint *)(s + pstKey->m_lFrom)))
                    return s;
                return p;
            case    4:
                if(*((int *)(p + pstKey->m_lFrom)) < *((int *)(s + pstKey->m_lFrom)))
                    return s;
                return p;
            case    8:
                if(*((llong *)(p + pstKey->m_lFrom)) < *((llong *)(s + pstKey->m_lFrom)))
                    return s;
                return p;
            default:
                return p;
            }
            return p;
        case FIELD_CHAR:
            if(0 < memcmp(s + pstKey->m_lFrom, p + pstKey->m_lFrom, pstKey->m_lLen))
                return s;
            return p;
        default:
            return p;
        }
    }
    else
    {
        switch(pstKey->m_lAttr)
        {
        case FIELD_DOUB:
            switch(pstKey->m_lLen)
            {
            case    4:
                if(*((float *)(p + pstKey->m_lFrom)) < *((float *)(s + pstKey->m_lFrom)))
                    return p;
                return s;
            case    8:
                if(*((double *)(p + pstKey->m_lFrom)) < *((double *)(s + pstKey->m_lFrom)))
                    return p;
                return s;
            default:
                return p;
            }
            return p;
        case FIELD_LONG: 
            switch(pstKey->m_lLen)
            {
            case    2:
                if(*((sint *)(p + pstKey->m_lFrom)) < *((sint *)(s + pstKey->m_lFrom)))
                    return p;
                return s;
            case    4:
                if(*((int *)(p + pstKey->m_lFrom)) < *((int *)(s + pstKey->m_lFrom)))
                    return p;
                return s;
            case    8:
                if(*((llong *)(p + pstKey->m_lFrom)) < *((llong *)(s + pstKey->m_lFrom)))
                    return p;
                return s;
            default:
                return p;
            }
            return p;
        case FIELD_CHAR:
            if(0 < memcmp(s + pstKey->m_lFrom, p + pstKey->m_lFrom, pstKey->m_lLen))
                return p;
            return s;
        default:
            return p;
        }
    }

    return p;
}

/*************************************************************************************************
    description：Match table field values
    parameters:
        pstCon                     --condit field
        s                          --original date
        p                          --records to be compared
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lFeildMatch(FdCond *pstCond, void *s, void *p)
{
    register    int   i;
    FdKey       *pFdKey;

    if(!p)    return RC_MATCH;

    for(i = 0; i < pstCond->uFldcmp; i ++)
    {
        pFdKey = &pstCond->stFdKey[i];
        if(memcmp(s + pFdKey->uFldpos, p + pFdKey->uFldpos, pFdKey->uFldlen))
            return RC_MISMA;
    }

    return RC_MATCH;
}

/*************************************************************************************************
    description：Gets the index group space size
    parameters:
        lRow                       --rows
    return:
        size_t
 *************************************************************************************************/
size_t    lGetGroupTruck(size_t lRow, size_t *plOfs)
{
    *plOfs = sizeof(SHTree) * lRow;
    return (sizeof(SHTree) + sizeof(SHList)) * lRow;
}

/*************************************************************************************************
    description：Gets the tree node space size
    parameters:
        rows
    return:
        size_t
 *************************************************************************************************/
size_t    lGetTreeTruck(size_t lRow)
{
    return sizeof(SHTree) * lRow;
}

/*************************************************************************************************
    description：insert table field
    parameters:
        pstSavm                    --stvm handle
        row                        --row
    return:
        size_t
 *************************************************************************************************/
size_t    lInitialTable(TABLE t, size_t lRow)
{
    size_t    lSize = sizeof(TblDef), lIdxOfs = 0;
    
    g_stTblDef[t].m_lMaxRow = lRow;

    // set NIL
    g_stTblDef[t].m_lNodeNil = FPOS(TblDef, m_stNil);
    memset(&g_stTblDef[t].m_stNil, 0, sizeof(SHTree));
    g_stTblDef[t].m_stNil.m_eColor = COLOR_BLK;
    g_stTblDef[t].m_stNil.m_lSePos = g_stTblDef[t].m_lNodeNil;

    if(HAVE_UNIQ_IDX(t)) 
    {
        g_stTblDef[t].m_lTreePos  = lSize;
        g_stTblDef[t].m_lTreeRoot = lSize;
        lSize += lGetTreeTruck(lRow);
    }
    else
    {
        g_stTblDef[t].m_lTreePos  = 0;
        g_stTblDef[t].m_lTreeRoot = 0;
    }

    if(HAVE_NORL_IDX(t) || HAVE_HASH_IDX(t)) 
    {
        g_stTblDef[t].m_lGroupPos  = lSize;
        g_stTblDef[t].m_lGroupRoot = lSize;
        lSize += lGetGroupTruck(lRow, &g_stTblDef[t].m_lListPos);
        g_stTblDef[t].m_lListOfs = g_stTblDef[t].m_lGroupPos + g_stTblDef[t].m_lListPos;
    }
    else
    {
        g_stTblDef[t].m_lListPos  = 0;
        g_stTblDef[t].m_lListOfs  = 0;
        g_stTblDef[t].m_lGroupPos = 0;
        g_stTblDef[t].m_lGroupRoot= 0;
    }

    g_stTblDef[t].m_lData = lSize;
    lSize += g_stTblDef[t].m_lTruck * lRow;

    return lSize;
}

/*************************************************************************************************
    description：count list 
    parameters:
        pvAddr                     --Adress
        pstList                    --SHList
    return:
        size_t
 *************************************************************************************************/
size_t    lGetListCount(void *pvAddr, SHList *pstList)
{
    size_t   lCount;

    if(!pstList || SELF_POS_UNUSE == pstList->m_lPos)
        return 0;

    for(lCount = 1; SELF_POS_UNUSE != pstList->m_lNext; 
        pstList = (SHList *)pGetNode(pvAddr, pstList->m_lNext))
        ++ lCount;

    return lCount;
}

/*************************************************************************************************
    description：Initialize the index group
    parameters:
        pvData                     --address
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lInitailGroup(SATvm *pstSavm, void *pvData, TABLE t)
{
    long    lRow = 0;
    void    *psData = NULL;
    SHTree  *pstTree = NULL;
    SHList  *pstList = NULL;

    if(!pvData)
    {
        pstSavm->m_lErrno = SHMT_NOT_INIT;
        return RC_FAIL;
    }

    if(lGetGrpNum(t) <= 0)        return RC_SUCC;

    for(psData = pvData + lGetListPos(t); lRow < lGetTblRow(t); lRow ++)
    {
        pstTree = (SHTree *)(pvData + lRow * sizeof(SHTree));
        memset(pstTree, 0, sizeof(SHTree));
        pstTree->m_lSePos = SELF_POS_UNUSE;
        pstTree->m_lData  = SELF_POS_UNUSE;

        pstList = (SHList *)(psData + lRow * sizeof(SHList));
        pstList->m_lPos  = SELF_POS_UNUSE;
        pstList->m_lNode = SELF_POS_UNUSE;
        pstList->m_lData = lGetTblData(t) + lGetRowTruck(t) * lRow;
        pstList->m_lNext = SELF_POS_UNUSE;
        pstList->m_lLast = SELF_POS_UNUSE;
    }
    ((TblDef *)pGetTblDef(t))->m_lGroupRoot = lGetGrpPos(t);

    return RC_SUCC;
}

/*************************************************************************************************
    description：Initialize the index tree
    parameters:
        pstSavm                    --stvm handle
        pvData                     --address
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lInitailTree(SATvm *pstSavm, void *pvData, TABLE t)
{
    long    lRow = 0;
    SHTree  *pstTree = NULL;

    if(!pvData)
    {
        pstSavm->m_lErrno = SHMT_NOT_INIT;
        return RC_FAIL;
    }

    if(lGetIdxNum(t) <= 0)        return RC_SUCC;

    for(lRow = 0; lRow < lGetTblRow(t); lRow ++)
    {
        pstTree = (SHTree *)(pvData + lRow * sizeof(SHTree));
        memset(pstTree, 0, sizeof(SHTree));
        pstTree->m_lSePos = SELF_POS_UNUSE;
        pstTree->m_lData  = lGetTblData(t) + lGetRowTruck(t) * lRow;
    }
    ((TblDef *)pGetTblDef(t))->m_lTreeRoot  = lGetIdxPos(t);

    return RC_SUCC;
}

/*************************************************************************************************
    description：Calculate the hash value
    parameters:
        s                          --key
        n                          --length
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
uint uGetHash(char *s, long n)
{
    uint    hash = 0;
    static  uint    i, seed = 131;

    for(i = 0; i < n; i ++)
        hash = hash * seed + (*s++);

    return (hash & 0x7FFFFFFF);
}

/*************************************************************************************************
    description：Gets the address of the offset
    parameters:
        lOffset                    --key
    return:
        void*
 *************************************************************************************************/
void*    pGetNode(void *pvData, size_t lOffset)
{
    return (void *)(pvData + lOffset);
}

/*************************************************************************************************
    description：Create node
    parameters:
        pvData                     --address
        t                          --length
        lOffset                    --offset address
    return:
        SHTree*  
 *************************************************************************************************/
SHTree* pCreateNode(void *pvData, TABLE t, size_t lOffset)
{
    return (SHTree *)(pvData + lOffset);
}

/*************************************************************************************************
    description：Create index node
    parameters:
        pvData                     --address memory
        t                          --table
        pstGroup                   --tree node
        lGroup                     --Group offset
        ppstTruck                  --Data truck 
    return:
        SHTree*
 *************************************************************************************************/
SHTree* pCreateGroup(void *pvData, TABLE t, SHTree *pstGroup, size_t lGroup, SHTruck **ppstTruck)
{
    size_t    lOffset = 0;
    SHList    *pstList = NULL, *pstNode = NULL;

    if(!pstGroup)    pstGroup = (SHTree *)(pvData + lGroup);
    lOffset = lGetListOfs(t) + sizeof(SHList) * ((TblDef *)pvData)->m_lValid;
    pstList = (SHList *)pGetNode(pvData, lOffset);
    pstList->m_lPos = lOffset;
    *ppstTruck = (PSHTruck)pGetNode(pvData, pstList->m_lData);    

    if(pstGroup->m_lData == SELF_POS_UNUSE)
    {
        pstList->m_lNode = lGroup;
        pstGroup->m_lData = pstList->m_lPos;
    }
    else
    {
        pstNode = (SHList *)pGetNode(pvData, pstGroup->m_lData);

#if        0
        pstList->m_lNext = pstNode->m_lNext;
        pstList->m_lLast = pstNode->m_lPos;
        if(SELF_POS_UNUSE != pstNode->m_lNext)
            ((SHList *)pGetNode(pvData, pstNode->m_lNext))->m_lLast = pstList->m_lPos;
        pstNode->m_lNext = pstList->m_lPos;
#else
        pstGroup->m_lData = pstList->m_lPos;
        pstList->m_lNode = pstGroup->m_lSePos;
        pstList->m_lNext = pstNode->m_lPos;
        pstNode->m_lLast  = pstList->m_lPos;
#endif
    }

    return pstGroup;
}

/*************************************************************************************************
    description：reset data list
    parameters:
        pvData                     --address memory
        t                          --table
        pstTree                    --tree node
        plOut
    return:
 *************************************************************************************************/
void    vResetList(void *pvData, TABLE t, SHTree *pstTree, long *plOut)
{
    size_t  lOffset, lNext;
    SHList  *pstList = NULL, *pstTail = NULL;


    return ;
#if        0    
    if(pstTree->m_lData <= SELF_POS_UNUSE)
        return ;

fprintf(stderr, "vResetList error \n");
exit(-1);

    lOffset = lGetListOfs(t) + lGetTblValid(t) * sizeof(SHList);
    //  将链表中的数据向前移动，将后面空间释放出来待下次使用, 同时 g_lTreeNode 个数减一
    for(pstList = (SHList *)pGetNode(pvData, pstTree->m_lData); SELF_POS_UNUSE != pstList->m_lPos; 
        (*plOut) ++, pstList = (SHList *)pGetNode(pvData, lNext))
    {
        lNext = pstList->m_lNext;
        
        lOffset -= sizeof(SHList);
        pstTail = (SHList *)pGetNode(pvData, lOffset);
        if(pstList == pstTail)
        {
            pstTail->m_lPos  = SELF_POS_UNUSE;
            pstTail->m_lNode = SELF_POS_UNUSE;
            pstTail->m_lNext = SELF_POS_UNUSE;
            pstTail->m_lLast = SELF_POS_UNUSE;
            if(SELF_POS_UNUSE == lNext)    break;
            continue;
        }

        if(SELF_POS_UNUSE != pstTail->m_lLast)
             ((SHList *)pGetNode(pvData, pstTail->m_lLast))->m_lNext = pstList->m_lPos;
        memcpy(pstList, pstTail, sizeof(SHList));
        pstTail->m_lPos  = SELF_POS_UNUSE;
        pstTail->m_lNode = SELF_POS_UNUSE;
        pstTail->m_lNext = SELF_POS_UNUSE;
        pstTail->m_lLast = SELF_POS_UNUSE;
        if(SELF_POS_UNUSE == lNext)    break;
    }
#endif

    pstTree->m_lData = SELF_POS_UNUSE;
}

/*************************************************************************************************
    description：Release the deleted node
    parameters:
        pstSavm                    --stvm handle
        pvData                     --address memory
        pstRoot                    --tree root 
        pstTree                    --tree node
        pstSon                     --child node
    return:
        SHTree*
 *************************************************************************************************/
SHTree*    pReleaseNode(void *pvData, TABLE t, SHTree *pstRoot, SHTree *pstTree, SHTree **ppstSon)
{
    size_t    lSePos = 0, lData, lVaild;
    SHTree    *pstTail = NULL, *pstNode = NULL;

    if(!pstTree || ((TblDef *)pvData)->m_lValid < 1)
        return pstRoot;

    lData  = pstTree->m_lData;    // backup lData
    lVaild = ((TblDef *)pvData)->m_lValid - 1;
    pstTail = (SHTree *)(pvData + lGetIdxPos(t) + lVaild * sizeof(SHTree));
    if(pstTail == pstTree)        // The tail node is exactly deleted
    {
        memset(pstTail, 0, sizeof(SHTree));
        pstTail->m_lData = lData;
        return pstRoot;
    }

    if(ppstSon)
    {
        if(*ppstSon == pstTail)        // move Son(tail) node
            *ppstSon = pstTree;
        else if(pstTree->m_lParent == pstTail->m_lSePos) 
            ((SHTree *)*ppstSon)->m_lParent = pstTree->m_lSePos;
    }

    lSePos = pstTail->m_lSePos;    //  backup lSePos
    pstTail->m_lSePos = pstTree->m_lSePos;
    memcpy((void *)pstTree, (void *)pstTail, sizeof(SHTree));
    memset(pstTail, 0, sizeof(SHTree));
    pstTail->m_lData = lData;

    // modify parent and child node
    if(NODE_NULL != pstTree->m_lParent)
    {
        pstNode = (SHTree *)pGetNode(pvData, pstTree->m_lParent);
        if(lSePos == pstNode->m_lLeft)
            pstNode->m_lLeft = pstTree->m_lSePos;
        else
            pstNode->m_lRight = pstTree->m_lSePos;
    }
    else   //  May be the tail of the root moved
    {
        pstRoot = pstTree;
    }

    if(NODE_NULL != pstTree->m_lLeft)
    {
        pstNode = (SHTree *)pGetNode(pvData, pstTree->m_lLeft);
        pstNode->m_lParent = pstTree->m_lSePos;
    }

    if(NODE_NULL != pstTree->m_lRight)
    {
        pstNode = (SHTree *)pGetNode(pvData, pstTree->m_lRight);
        pstNode->m_lParent = pstTree->m_lSePos;    
    }

    return pstRoot;
}

/*************************************************************************************************
    description：Release deleted data block space
    parameters:
        pvAddr                     --address memory
        t                          --table
        pstTruck                   --Data truck 
        bErase                     --erase
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lReleaseTruck(void *pvAddr, TABLE t, SHTruck *pstTruck, bool bErase)
{
    size_t  lOffset = 0;
    SHTruck *pstTail = NULL;

    if(bErase)
    {
        memset(pstTruck, 0, lGetRowTruck(t));
        return RC_SUCC;
    }

    if(((TblDef *)pvAddr)->m_lValid < 1) 
        return RC_SUCC;

    lOffset = lGetTblData(t) + lGetRowTruck(t) * (((TblDef *)pvAddr)->m_lValid - 1);
    pstTail = (PSHTruck)pGetNode(pvAddr, lOffset);
    if(pstTruck == pstTail)
    {
        memset(pstTail, 0, lGetRowTruck(t));
        return RC_SUCC;
    }

    memcpy(pstTruck, pstTail, lGetRowTruck(t));
    memset(pstTail, 0, lGetRowTruck(t));

    return RC_SUCC;
}

/*************************************************************************************************
    description：Release index node
    parameters:
        pvData                     --address memory
        t                          --table
        pstRoot                    --tree root 
        pstTree                    --tree node
        pstSon                     --child node
        plOut 
    return:
        SHTree*
 *************************************************************************************************/
SHTree*    pReleaseGroup(void *pvData, TABLE t, SHTree *pstRoot, SHTree *pstTree, SHTree **ppstSon, 
            long *plOut)
{
    size_t    lSePos = 0, lVaild;
    SHTree    *pstTail = NULL, *pstNode = NULL;

    if(!pstTree || ((TblDef *)pvData)->m_lGroup < 1) 
        return pstRoot;

//  vResetList(pvData, t, pstTree, plOut);

    lVaild = -- ((TblDef *)pvData)->m_lGroup;
    pstTail = (SHTree *)(pvData + lGetGrpPos(t) + lVaild * sizeof(SHTree));
    if(pstTail == pstTree)
    {
        memset(pstTail, 0, sizeof(SHTree));
        return pstRoot;
    }

    if(ppstSon)
    {
        if(*ppstSon == pstTail)     //  move Son(tail) node
            *ppstSon = pstTree;
        else if(pstTree->m_lParent == pstTail->m_lSePos)
            ((SHTree *)*ppstSon)->m_lParent = pstTree->m_lSePos;
    }

    lSePos = pstTail->m_lSePos; //  backup SePos
    pstTail->m_lSePos = pstTree->m_lSePos;
    memcpy((void *)pstTree, (void *)pstTail, sizeof(SHTree));
    memset(pstTail, 0, sizeof(SHTree));
    ((SHList *)pGetNode(pvData, pstTree->m_lData))->m_lNode = pstTree->m_lSePos;

    if(NODE_NULL != pstTree->m_lParent)
    {
        pstNode = (SHTree *)pGetNode(pvData, pstTree->m_lParent);
        if(lSePos == pstNode->m_lLeft)
            pstNode->m_lLeft = pstTree->m_lSePos;
        else
            pstNode->m_lRight = pstTree->m_lSePos;
    }
    else 
    {
        pstRoot = pstTree;
    }

    if(NODE_NULL != pstTree->m_lLeft)
    {
        pstNode = (SHTree *)pGetNode(pvData, pstTree->m_lLeft);
        pstNode->m_lParent = pstTree->m_lSePos;
    }

    if(NODE_NULL != pstTree->m_lRight)
    {
        pstNode = (SHTree *)pGetNode(pvData, pstTree->m_lRight);
        pstNode->m_lParent = pstTree->m_lSePos;    
    }

    return pstRoot;
}

/*************************************************************************************************
    description：Release list node
    parameters:
        pvData                     --address memory
        t                          --table
        pstTree                    --tree node
        pstList                    --data list
        lOffset                    --Data offset 
        plNext                     --lNext offset 
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lReleaseList(void *pvData, TABLE t, SHTree *pstTree, SHList *pstList, size_t lOffset, 
            size_t *plNext)
{
    size_t   lData;
    SHList  *pstTail = NULL;

    pstTail = (SHList *)pGetNode(pvData, lOffset);
    if(SELF_POS_UNUSE == pstTail->m_lPos)
        return RC_FAIL;

    if(pstTree->m_lData == pstList->m_lPos)
    {
        pstTree->m_lData = pstList->m_lNext;
        if(SELF_POS_UNUSE != pstList->m_lNext)
        {
            ((SHList *)pGetNode(pvData, pstList->m_lNext))->m_lLast = SELF_POS_UNUSE;
            ((SHList *)pGetNode(pvData, pstList->m_lNext))->m_lNode = pstTree->m_lSePos;
        }
    }
    else
    {
        ((SHList *)pGetNode(pvData, pstList->m_lLast))->m_lNext = pstList->m_lNext;
        if(SELF_POS_UNUSE != pstList->m_lNext)
            ((SHList *)pGetNode(pvData, pstList->m_lNext))->m_lLast = pstList->m_lLast;
    }

    if(pstTail == pstList)
    {
        //   save lData
        pstList->m_lPos  = SELF_POS_UNUSE;
        pstList->m_lNode = SELF_POS_UNUSE;
        pstList->m_lNext = SELF_POS_UNUSE;
        pstList->m_lLast = SELF_POS_UNUSE;
        return RC_SUCC;
    }

    //    If the pstTail node is the next scan node
    if(pstTail->m_lPos == pstList->m_lNext)    
        *plNext = pstList->m_lPos;

    lData = pstList->m_lData;
    if(pstTail->m_lPos == pstTree->m_lData)
        pstTree->m_lData = pstList->m_lPos;
    else
    {
        if(SELF_POS_UNUSE == pstTail->m_lLast)
            ((SHTree *)pGetNode(pvData, pstTail->m_lNode))->m_lData = pstList->m_lPos;
    }

    if(SELF_POS_UNUSE != pstTail->m_lLast)
        ((SHList *)pGetNode(pvData, pstTail->m_lLast))->m_lNext = pstList->m_lPos;
    if(SELF_POS_UNUSE != pstTail->m_lNext)
        ((SHList *)pGetNode(pvData, pstTail->m_lNext))->m_lLast = pstList->m_lPos;

    pstList->m_lNode = pstTail->m_lNode;
    pstList->m_lNext = pstTail->m_lNext;
    pstList->m_lData = pstTail->m_lData;
    pstList->m_lLast = pstTail->m_lLast;
    
    pstTail->m_lData = lData;
    pstTail->m_lPos  = SELF_POS_UNUSE;
    pstTail->m_lNode = SELF_POS_UNUSE;
    pstTail->m_lNext = SELF_POS_UNUSE;
    pstTail->m_lLast = SELF_POS_UNUSE;

    return RC_SUCC;
}

/*************************************************************************************************
    description：Left-Rotate the red-black tree node (pstTree)
    parameters:
        pvData                     --address memory
        pstRoot                    --tree root
        pstTree                    --tree node
    return:
        SHTree*

    Schematic:
       root                           root
       /                               /
     tree                            node                
     /  \      --(L--R)-->           /   \                                                      #
    lA  node                       tree  rC    
        /  \                       /  \
       lB   rC                    lA  lB  
 *************************************************************************************************/
SHTree*    _pLeftRotation(void *pvData, SHTree *pstRoot, SHTree *pstTree)
{
    SHTree    *pstCur = NULL, *pstNode = (SHTree *)pGetNode(pvData, pstTree->m_lRight);

    // set "pstNode's left child" as "pstTree's right child"
    // if the "pstNode's left is not null", set "pstTree" as father of the left child of pstNode
    pstTree->m_lRight = pstNode->m_lLeft;
    if(NODE_NULL != pstNode->m_lLeft)
    {
        pstCur = (SHTree *)pGetNode(pvData, pstNode->m_lLeft);
        pstCur->m_lParent = pstTree->m_lSePos;
    }

    // Set "pstTree's father" as "pstNode's father"
    pstNode->m_lParent = pstTree->m_lParent;

    if(NODE_NULL == pstTree->m_lParent)
        pstRoot = pstNode;    //  create new Root
    else
    {
        pstCur = (SHTree *)pGetNode(pvData, pstTree->m_lParent);
        if(pstCur->m_lLeft == pstTree->m_lSePos)
            pstCur->m_lLeft = pstNode->m_lSePos;
        else
            pstCur->m_lRight = pstNode->m_lSePos;
    }
    
    // set "pstTree" as "pstNode's left child"
    pstNode->m_lLeft = pstTree->m_lSePos;

    // set "pstTree's father" as "pstNode”
    pstTree->m_lParent = pstNode->m_lSePos;

    return pstRoot;
}

/*************************************************************************************************
    description：Right-Rotate the red-black tree node (pstTree)
    parameters:
        pvData                     --address memory
        pstRoot                    --tree root
        pstTree                    --tree node
    return:
        SHTree*

    Schematic:
        root                           root
        /                               /
       tree                           node                  
      /   \      --(R--R)-->          /  \                                                       #
     node  rC                        lA  tree  
     /  \                                /  \                                                    #
    lA  rB                              rB   rC
 *************************************************************************************************/
SHTree*   _pRightRotation(void *pvData, SHTree *pstRoot, SHTree *pstTree)
{
    SHTree    *pstCur = NULL, *pstNode = (SHTree *)pGetNode(pvData, pstTree->m_lLeft);

    // set "pstNode's right child" as "pstTree's left child"
    // if the "pstNode's right is not null, set "pstTree" as father of the right child of pstNode
    pstTree->m_lLeft = pstNode->m_lRight;
    if(NODE_NULL != pstNode->m_lRight)
    {
        pstCur = (SHTree *)pGetNode(pvData, pstNode->m_lRight);
        pstCur->m_lParent = pstTree->m_lSePos;
    }

    // set "pstTree's parent" as "pstNode's parent"
    pstNode->m_lParent = pstTree->m_lParent;
    if(NODE_NULL == pstTree->m_lParent)
        pstRoot = pstNode; 
    else
    {
        pstCur = (SHTree *)pGetNode(pvData, pstTree->m_lParent);
        if(pstCur->m_lLeft == pstTree->m_lSePos)
            pstCur->m_lLeft = pstNode->m_lSePos;
        else
            pstCur->m_lRight = pstNode->m_lSePos;
    }

    //  set "pstTree" as "pstNode's right child"
    pstNode->m_lRight = pstTree->m_lSePos;
    // set "pstTree's parent as "pstNode"
    pstTree->m_lParent = pstNode->m_lSePos;

    return pstRoot;
}

/*************************************************************************************************
    description：Get extreme values in RBTree traversal
    parameters:
        uEnum 
        pvData                    --addree memory
        pstTree                   --tree node
    return:
        SHTree*
 *************************************************************************************************/
SHTree* pExtremeTree(Uenum uEnum, void *pvData, SHTree *pstTree)
{
    if(SELF_POS_UNUSE == pstTree->m_lSePos || NODE_NULL == pstTree->m_lSePos)
        return NULL;

    if(uEnum & MATCH_MAX)
    {
        while(SELF_POS_UNUSE != pstTree->m_lRight && NODE_NULL != pstTree->m_lRight)
            pstTree = (SHTree *)pGetNode(pvData, pstTree->m_lRight);
    }
    else
    {
        while(SELF_POS_UNUSE != pstTree->m_lLeft && NODE_NULL != pstTree->m_lLeft)
            pstTree = (SHTree *)pGetNode(pvData, pstTree->m_lLeft);
    }

    if(SELF_POS_UNUSE == pstTree->m_lSePos && NODE_NULL == pstTree->m_lSePos)
        pstTree = NULL;

    return pstTree;
}

/*************************************************************************************************
    description：Specify the KEY value in the RBTree traversal
    parameters:
        pvData                    --addree memory
        psTree                    --tree node
        psvIdx                    --index value
        lIdx                      --value length
    return:
        SHTree*
 *************************************************************************************************/
SHTree* pSearchTree(void *pvData, SHTree *pstTree, void *psvIdx, long lIdx)
{
    if(SELF_POS_UNUSE == pstTree->m_lSePos || NODE_NULL == pstTree->m_lSePos)
        return NULL;

    if(!memcmp(pstTree->m_szIdx, psvIdx, lIdx))
        return pstTree;

    if(0 < memcmp(pstTree->m_szIdx, psvIdx, lIdx))
        return pSearchTree(pvData, (SHTree *)pGetNode(pvData, pstTree->m_lLeft), psvIdx, lIdx);
    else
        return pSearchTree(pvData, (SHTree *)pGetNode(pvData, pstTree->m_lRight), psvIdx, lIdx);
}

/*************************************************************************************************
    description：Gets a list of data sets
    parameters:
        pvData                    --addree memory
        psTree                    --tree node
        psvIdx                    --index value
        lIdx                      --value length
    return:
        SHList*
 *************************************************************************************************/
SHList    *pSearchGroup(void *pvData, SHTree *pstTree, void *psvIdx, long lIdx)
{
    if(NULL == (pstTree = pSearchTree(pvData, pstTree, psvIdx, lIdx)))
        return NULL;

    //  If the group information exists in an abnormal situation, 
    //  but there are numerous references, then the group should be deleted.
    if(SELF_POS_UNUSE == pstTree->m_lData)
        return NULL;
    
    return (SHList *)pGetNode(pvData, pstTree->m_lData);
}

/*************************************************************************************************
    description：Restore the new RBTree imbalance problem
    parameters:
        pvData                    --addree memory
        pstRoot                   --root node
        pstCur                    --tree node
    return:
        SHTree*
 *************************************************************************************************/
SHTree*    _pFixupInsert(void *pvData, SHTree *pstRoot, SHTree *pstCur)
{
    SHTree *pstParent = NULL, *pstUncle = NULL, *pstGrand = NULL;

    while((pstParent = (SHTree *)pGetNode(pvData, pstCur->m_lParent)) && IS_RED(pstParent))
    {
        pstGrand = (SHTree *)pGetNode(pvData, pstParent->m_lParent);
        if(pstParent->m_lSePos == pstGrand->m_lLeft)
        {
            pstUncle = (SHTree *)pGetNode(pvData, pstGrand->m_lRight);
            if((NODE_NULL != pstUncle->m_lSePos) && IS_RED(pstUncle))
            {
                pstUncle->m_eColor = COLOR_BLK;
                pstParent->m_eColor = COLOR_BLK;
                pstGrand->m_eColor = COLOR_RED;
                pstCur = pstGrand;
                continue;
            }

            if(pstCur->m_lSePos == pstParent->m_lRight)
            {
                pstRoot = _pLeftRotation(pvData, pstRoot, pstParent);
                pstParent = pstCur;
            }

            pstParent->m_eColor = COLOR_BLK;
            pstGrand->m_eColor = COLOR_RED; 
            pstRoot = _pRightRotation(pvData, pstRoot, pstGrand);
        }
        else
        {
            pstUncle = (SHTree *)pGetNode(pvData, pstGrand->m_lLeft);
            if((NODE_NULL != pstUncle->m_lSePos) && IS_RED(pstUncle))
            {
                pstUncle->m_eColor = COLOR_BLK;
                pstParent->m_eColor = COLOR_BLK;
                pstGrand->m_eColor = COLOR_RED;
                pstCur = pstGrand;
                continue;
            }

            if(pstCur->m_lSePos == pstParent->m_lLeft)
            {
                pstRoot = _pRightRotation(pvData, pstRoot, pstParent);
                pstParent = pstCur; 
            }

            pstParent->m_eColor = COLOR_BLK;
            pstGrand->m_eColor = COLOR_RED;
            pstRoot = _pLeftRotation(pvData, pstRoot, pstGrand);
        }
    }

    pstRoot->m_eColor = COLOR_BLK;    // set root is black

    return pstRoot;
}

/*************************************************************************************************
    description：Insert the new node into r-btree and return the root node
    parameters:
        pstSavm                    --stvm handle
        pstRoot                    --tree Root
        psvIdx                     --index value
        lIdx                       --value length
        ppstTruck                  --data truck
    return:
        SHTree*
 *************************************************************************************************/
SHTree*    pInsertTree(SATvm *pstSavm, SHTree *pstRoot, void *psvIdx, long lIdx, SHTruck **ppstTruck)
{
    int     nRet = 0;
    size_t  lOffset;
    SHTree  *pstNode = NULL, *pstCur = NULL, *pstTree = pstRoot;
    void    *pvData = NULL, *pvAddr = pGetAddr(pstSavm, pstSavm->tblName);

    while(SELF_POS_UNUSE != pstTree->m_lSePos && NODE_NULL != pstTree->m_lSePos)
    {
        pstNode = pstTree;
        nRet = memcmp(pstTree->m_szIdx, psvIdx, lIdx);
        if(0 < nRet)
        {
            if(NODE_NULL == pstTree->m_lLeft)
                break;
            pstTree = (SHTree *)pGetNode(pvAddr, pstTree->m_lLeft);
        }
        else if(0 > nRet)
        {
            if(NODE_NULL == pstTree->m_lRight)
                break;
            pstTree = (SHTree *)pGetNode(pvAddr, pstTree->m_lRight);
        }
        else  
        {
            pstSavm->m_lErrno = UNIQ_IDX_REPT;
            return NULL;
        }
    }
        
    lOffset = sizeof(SHTree) * ((TblDef *)pvAddr)->m_lValid + lGetIdxPos(pstSavm->tblName);
    if(NULL == (pstCur = pCreateNode(pvAddr, pstSavm->tblName, lOffset)))
    {       
        pstSavm->m_lErrno = IDX_SPC_SPILL;
        return NULL;
    }
    
    if(*ppstTruck && (*ppstTruck != (PSHTruck)pGetNode(pvAddr, pstCur->m_lData)))
    {
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return NULL;
    }
    else
    {
        *ppstTruck = (PSHTruck)pGetNode(pvAddr, pstCur->m_lData);
        if(!IS_TRUCK_NULL(*ppstTruck))
        {
            pstSavm->m_lErrno = SVR_EXCEPTION;
            return NULL;
        }
    }

    pstCur->m_lIdx = lIdx;
    pstCur->m_lSePos = lOffset;
    pstCur->m_lParent = NODE_NULL;
    memcpy(pstCur->m_szIdx, psvIdx, pstCur->m_lIdx);
    //  set parent node and left -right node is NIL
    pstCur->m_lLeft = pstCur->m_lRight = NODE_NULL;

    if(pstNode != NULL)
    {
        pstCur->m_eColor = COLOR_RED;
        pstCur->m_lParent = pstNode->m_lSePos;
        if(0 < memcmp(pstNode->m_szIdx, pstCur->m_szIdx, pstCur->m_lIdx))
            pstNode->m_lLeft = pstCur->m_lSePos;
        else
            pstNode->m_lRight = pstCur->m_lSePos;
    }
    else
    {
        pstRoot = pstCur;    
        pstCur->m_eColor = COLOR_BLK;
        return pstRoot;
    }

    return _pFixupInsert(pvAddr, pstRoot, pstCur);
}

/*************************************************************************************************
    description：Insert values and handle collisions according to the hash table
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --address memory
        t                          --table
        pstTree                    --tree node
        lGroup                     --group offset
        ppstTruck                  --data truck
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    __lInsertHash(SATvm *pstSavm, void *pvAddr, TABLE t, SHTree *pstTree, size_t lGroup, 
            SHTruck **ppstTruck)
{
    size_t  lOffset;
    SHTruck *pstTruck = NULL;
    SHList  *pstList = NULL, *pstNode = NULL;

    lOffset = lGetListOfs(t) + sizeof(SHList) * ((TblDef *)pvAddr)->m_lValid;
    pstList = (SHList *)pGetNode(pvAddr, lOffset);
    pstList->m_lPos = lOffset;
    pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);

    //  If the data space has been found in the previous only index creation, recheck it
    if(!IS_TRUCK_NULL(pstTruck) || ((*ppstTruck) && (*ppstTruck != pstTruck)))
    {
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    *ppstTruck = pstTruck; 
    if(SELF_POS_UNUSE == pstTree->m_lSePos || NODE_NULL == pstTree->m_lSePos)
    {
        pstTree->m_lSePos= lGroup;
        pstTree->m_lData = pstList->m_lPos;
        pstList->m_lNode = pstTree->m_lSePos;
    }
    else
    {
        pstNode = (SHList *)pGetNode(pvAddr, pstTree->m_lData);

        pstList->m_lNext = pstNode->m_lNext;
        pstList->m_lLast = pstNode->m_lPos;
        if(SELF_POS_UNUSE != pstNode->m_lNext)
            ((SHList *)pGetNode(pvAddr, pstNode->m_lNext))->m_lLast = pstList->m_lPos;
        pstNode->m_lNext = pstList->m_lPos;
    }

    return RC_SUCC;
}    

/*************************************************************************************************
    description：Insert the group node into r-btree and return the root node
    parameters:
        pstSavm                    --stvm handle
        pstRoot                    --tree Root
        psvIdx                     --index value
        lIdx                       --value length
        ppstTruck                  --data truck
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
SHTree* pInsertGroup(SATvm *pstSavm, SHTree *pstRoot, void *psvIdx, long lIdx, SHTruck **ppstTruck)
{
    int     nRet = 0;
    size_t  lOffset;
    SHTruck *pstTruck = NULL;
    void    *pvData = pGetAddr(pstSavm, pstSavm->tblName);
    SHTree  *pstNode = NULL, *pstCur = NULL, *pstTree = pstRoot;

    while(SELF_POS_UNUSE != pstTree->m_lSePos && NODE_NULL != pstTree->m_lSePos)
    {
        pstNode = pstTree;
        nRet = memcmp(pstTree->m_szIdx, psvIdx, lIdx);
        if(0 < nRet)
        {
            if(NODE_NULL == pstTree->m_lLeft)
                break;
            pstTree = (SHTree *)pGetNode(pvData, pstTree->m_lLeft);
        }
        else if(0 > nRet)
        {
            if(NODE_NULL == pstTree->m_lRight)
                break;
            pstTree = (SHTree *)pGetNode(pvData, pstTree->m_lRight);
        }
        else 
        {
            pCreateGroup(pvData, pstSavm->tblName, pstTree, 0, &pstTruck);
            if(!IS_TRUCK_NULL(pstTruck) || ((*ppstTruck) && (*ppstTruck != pstTruck)))
            {
                pstSavm->m_lErrno = SVR_EXCEPTION;
                return NULL;
            }

            return pstRoot;
        }
    }

    lOffset = sizeof(SHTree) * ((TblDef *)pvData)->m_lGroup + lGetGrpPos(pstSavm->tblName);
    if(NULL == (pstCur = pCreateGroup(pvData, pstSavm->tblName, NULL, lOffset, &pstTruck)))
    {
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return pstRoot;
    }

    //  If the data space has been found in the previous only index creation, recheck it
    if(!IS_TRUCK_NULL(pstTruck) || ((*ppstTruck) && (*ppstTruck != pstTruck)))
    {
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return NULL;
    }

    ((TblDef *)pvData)->m_lGroup ++;
    *ppstTruck = pstTruck;

    pstCur->m_lIdx = lIdx;
    pstCur->m_lSePos = lOffset;
    pstCur->m_lParent = NODE_NULL;
    memcpy(pstCur->m_szIdx, psvIdx, pstCur->m_lIdx);
    pstCur->m_lLeft = pstCur->m_lRight = NODE_NULL;
    if(pstNode != NULL)
    {
        pstCur->m_eColor = COLOR_RED;
        pstCur->m_lParent = pstNode->m_lSePos;
        if(0 < memcmp(pstNode->m_szIdx, pstCur->m_szIdx, pstCur->m_lIdx))
            pstNode->m_lLeft = pstCur->m_lSePos;
        else
            pstNode->m_lRight = pstCur->m_lSePos;
    }
    else
    {
        pstRoot = pstCur;
        pstCur->m_eColor = COLOR_BLK;
        return pstRoot;
    }

    return _pFixupInsert(pvData, pstRoot, pstCur);
}

/*************************************************************************************************
    description：Repair deletion causes RBTree imbalance
    parameters:
        pvData                     --stvm handle
        pstRoot                    --tree Root
        psTree                     --tree node
    return:
        SHTree*
 *************************************************************************************************/
SHTree*    _pFixupDelete(void *pvData, SHTree *pstRoot, SHTree *pstTree)
{
    SHTree    *pstParent = NULL, *pstBrother = NULL, *pstLeft = NULL, *pstRight = NULL;

    while(COLOR_BLK == pstTree->m_eColor && (pstRoot != pstTree))
    {   
        pstParent = (SHTree *)pGetNode(pvData, pstTree->m_lParent);
        
        if(pstTree->m_lSePos == pstParent->m_lLeft) 
        {
            pstBrother = (SHTree *)pGetNode(pvData, pstParent->m_lRight);

            if(COLOR_RED == pstBrother->m_eColor) 
            {
                pstParent->m_eColor = COLOR_RED;
                pstBrother->m_eColor = COLOR_BLK;
                pstRoot = _pLeftRotation(pvData, pstRoot, pstParent);

                // New sibling nodes. That's what it looks like
                pstBrother = (SHTree *)pGetNode(pvData, pstParent->m_lRight);
            }
            if(NODE_NULL == pstBrother->m_lSePos)
                return pstRoot;

            pstLeft = (SHTree *)pGetNode(pvData, pstBrother->m_lLeft);
            pstRight = (SHTree *)pGetNode(pvData, pstBrother->m_lRight);
            if(COLOR_BLK == pstLeft->m_eColor && COLOR_BLK == pstRight->m_eColor)
            {
                pstBrother->m_eColor = COLOR_RED;
                pstTree = pstParent;
            }
            else 
            {
                /* 3: Brothers node is black (default), the left node of the brother 
                    node is red, the right child node is black: to the brother as the 
                    fulcrum, right-Rotate*/
                if(COLOR_BLK == pstRight->m_eColor)
                {
                    pstLeft->m_eColor = COLOR_BLK;
                    pstBrother->m_eColor = COLOR_RED;
                    pstRoot = _pRightRotation(pvData, pstRoot, pstBrother);

                    /*  The changed right child of the parent node serves as a new 
                        brother node*/
                    pstBrother = (SHTree *)pGetNode(pvData, pstParent->m_lRight);
                    pstRight = (SHTree *)pGetNode(pvData, pstBrother->m_lRight);
                }
                
                /* 4: Brothers node is black (default), brother node right child 
                    node is red: parent as a fulcrum, left-Rotate */
                pstBrother->m_eColor = pstParent->m_eColor;
                pstParent->m_eColor = COLOR_BLK;
                pstRight->m_eColor  = COLOR_BLK;
                pstRoot = _pLeftRotation(pvData, pstRoot, pstParent);
                pstTree = pstRoot;
            }
        }
        else    // pstTree is pstParent right child
        {
            pstBrother = (SHTree *)pGetNode(pvData, pstParent->m_lLeft);
            if(COLOR_RED == pstBrother->m_eColor)
            {
                pstParent->m_eColor = COLOR_RED;
                pstBrother->m_eColor = COLOR_BLK;
                pstRoot = _pRightRotation(pvData, pstRoot, pstParent);

                // New Brothers node. In fact, the effect is left-handed
                pstBrother = (SHTree *)pGetNode(pvData, pstParent->m_lLeft);
            }
            if(NODE_NULL == pstBrother->m_lSePos)
                return pstRoot;

            /* Brothers node is black (default), and the two nodes of the 
               brother node are black */
            pstLeft = (SHTree *)pGetNode(pvData, pstBrother->m_lLeft);
            pstRight = (SHTree *)pGetNode(pvData, pstBrother->m_lRight);
            if(COLOR_BLK == pstLeft->m_eColor && COLOR_BLK == pstRight->m_eColor)
            {
                pstBrother->m_eColor = COLOR_RED;
                pstTree = pstParent;
            }
            else
            {
                /* Brothers node is black (default), the right node of the brother 
                   node is red, the left child node is black: 
                   the brother as a fulcrum, left-rotate*/
                if(COLOR_BLK == pstLeft->m_eColor)
                {
                    pstRight->m_eColor = COLOR_BLK;
                    pstBrother->m_eColor = COLOR_RED;
                    pstRoot = _pLeftRotation(pvData, pstRoot, pstBrother); 

                    // The changed right child of the parent node serves as a new brother node
                    pstBrother = (SHTree *)pGetNode(pvData, pstParent->m_lLeft);
                    pstLeft = (SHTree *)pGetNode(pvData, pstBrother->m_lLeft);
                }
            
                /* Brothers node is black (default), brother node left child node is red: 
                   parent as the fulcrum, right-handed processing */
                pstBrother->m_eColor = pstParent->m_eColor;
                pstParent->m_eColor = COLOR_BLK;
                pstLeft->m_eColor  = COLOR_BLK;
                pstRoot = _pRightRotation(pvData, pstRoot, pstParent);
                pstTree = pstRoot;
            }
        }
    }

    pstTree->m_eColor  = COLOR_BLK;
    
    return pstRoot;
}

/*************************************************************************************************
    description：Remove the execution index from RBTree
    parameters:
        pvData                      --addree memory
        t                           --table
        pstRoot                     --tree root
        pstTree                     --tree node
    return:
        SHTree*
 *************************************************************************************************/
SHTree*    _pDeleteTree(void *pvData, TABLE t, SHTree *pstRoot, SHTree *pstTree)
{
    size_t  lData;
    SHTree  *pstChild = NULL, *pstParent = NULL, *pstNext = NULL;

    // If there is a left node or a right node, notice that no node defaults to NIL 
    // for right node processing
    if(NODE_NULL == pstTree->m_lLeft || NODE_NULL == pstTree->m_lRight)
    {
        if(NODE_NULL == pstTree->m_lLeft)
            pstNext = (SHTree *)pGetNode(pvData, pstTree->m_lRight);
        else
            pstNext = (SHTree *)pGetNode(pvData, pstTree->m_lLeft);

        //  The child node points to the parent of the node to be deleted
        pstNext->m_lParent = pstTree->m_lParent;
        if(NODE_NULL == pstTree->m_lParent) 
            pstRoot = pstNext;
        else
        {
            pstParent = (SHTree *)pGetNode(pvData, pstTree->m_lParent);
            if(pstParent->m_lLeft == pstTree->m_lSePos)
                pstParent->m_lLeft = pstNext->m_lSePos;
            else
                pstParent->m_lRight = pstNext->m_lSePos;
        }  

        // If you delete the red node, it does not affect the tree structure
        if(pstTree->m_eColor == COLOR_RED)
            return pReleaseNode(pvData, t, pstRoot, pstTree, NULL);

        pstRoot = pReleaseNode(pvData, t, pstRoot, pstTree, &pstNext);

        return _pFixupDelete(pvData, pstRoot, pstNext);
    }

    pstNext = (SHTree *)pGetNode(pvData, pstTree->m_lRight);
    // Left and right children of deleted node D are not leaf nodes
    while(NODE_NULL != pstNext->m_lLeft)
        pstNext = (SHTree *)pGetNode(pvData, pstNext->m_lLeft);

    pstChild  = (SHTree *)pGetNode(pvData, pstNext->m_lRight);
    pstParent = (SHTree *)pGetNode(pvData, pstNext->m_lParent);

    pstChild->m_lParent = pstParent->m_lSePos;    
    if(pstNext->m_lSePos == pstParent->m_lLeft)
        pstParent->m_lLeft = pstChild->m_lSePos;
    else
        pstParent->m_lRight = pstChild->m_lSePos;

    lData = pstTree->m_lData;
    pstTree->m_lData = pstNext->m_lData;
    pstNext->m_lData = lData;
    memcpy(pstTree->m_szIdx, pstNext->m_szIdx, pstNext->m_lIdx);

    if(pstNext->m_eColor == COLOR_RED)
        return pReleaseNode(pvData, t, pstRoot, pstNext, NULL);

    pstRoot = pReleaseNode(pvData, t, pstRoot, pstNext, &pstChild);

    return _pFixupDelete(pvData, pstRoot, pstChild);
}

/*************************************************************************************************
    description：Remove the query index from RBTree
    parameters:
        pvData                     --address memory
        t                          --table
        pstRoot                    --tree Root
        pstTree                    --tree node
        lOut 
    return:
        SHTree*
 *************************************************************************************************/
SHTree*    _pDeleteGroup(void *pvData, TABLE t, SHTree *pstRoot, SHTree *pstTree, long *plOut)
{
    size_t  lData = 0;
    SHTree  *pstChild = NULL, *pstParent = NULL, *pstNext = NULL;

    // If there is a left node or a right node, notice that no node defaults to NIL 
    // for right node processing
    if(NODE_NULL == pstTree->m_lLeft || NODE_NULL == pstTree->m_lRight)
    {
        if(NODE_NULL == pstTree->m_lLeft)
            pstNext = (SHTree *)pGetNode(pvData, pstTree->m_lRight);
        else
            pstNext = (SHTree *)pGetNode(pvData, pstTree->m_lLeft);

        //  The child node points to the parent of the node to be deleted
        pstNext->m_lParent = pstTree->m_lParent;
        if(NODE_NULL == pstTree->m_lParent)
            pstRoot = pstNext;
        else
        {
            pstParent = (SHTree *)pGetNode(pvData, pstTree->m_lParent);
            if(pstParent->m_lLeft == pstTree->m_lSePos)
                pstParent->m_lLeft = pstNext->m_lSePos;
            else
                pstParent->m_lRight = pstNext->m_lSePos;
        }

        if(pstTree->m_eColor == COLOR_RED)
            return pReleaseGroup(pvData, t, pstRoot, pstTree, NULL, plOut);

        pstRoot = pReleaseGroup(pvData, t, pstRoot, pstTree, &pstNext, plOut);
        return _pFixupDelete(pvData, pstRoot, pstNext);
    }

    pstNext = (SHTree *)pGetNode(pvData, pstTree->m_lRight);
    while(NODE_NULL != pstNext->m_lLeft)
        pstNext = (SHTree *)pGetNode(pvData, pstNext->m_lLeft);

    pstChild  = (SHTree *)pGetNode(pvData, pstNext->m_lRight);
    pstParent = (SHTree *)pGetNode(pvData, pstNext->m_lParent);

    pstChild->m_lParent = pstParent->m_lSePos;
    if(pstNext->m_lSePos == pstParent->m_lLeft)
        pstParent->m_lLeft = pstChild->m_lSePos;
    else
        pstParent->m_lRight = pstChild->m_lSePos;

    lData = pstTree->m_lData;
    pstTree->m_lData = pstNext->m_lData;
    pstNext->m_lData = lData;    // set lData to the Release the node
    memcpy(pstTree->m_szIdx, pstNext->m_szIdx, pstNext->m_lIdx);
    ((SHList *)pGetNode(pvData, pstTree->m_lData))->m_lNode = pstTree->m_lSePos;

    if(pstNext->m_eColor == COLOR_RED)
        return pReleaseGroup(pvData, t, pstRoot, pstNext, NULL, plOut);

    pstRoot = pReleaseGroup(pvData, t, pstRoot, pstNext, &pstChild, plOut);

    return _pFixupDelete(pvData, pstRoot, pstChild);
}

/*************************************************************************************************
    description：Remove the execution index from RBTree
    parameters:
        pstSavm                    --stvm handle
        pstRoot                    --tree Root
        psvIdx                     --index value
        lIdx                       --value length
        *plData                    --data offset
    return:
        SHTree*
 *************************************************************************************************/
SHTree*    pDeleteTree(SATvm *pstSavm, SHTree *pstRoot, void *psvIdx, long lIdx, size_t *plData)
{
    SHTree  *pstTree = NULL;
    void    *pvData = pGetAddr(pstSavm, pstSavm->tblName);

    if(NULL == (pstTree = pSearchTree(pvData, pstRoot, psvIdx, lIdx)))
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return NULL;
    }

    *plData = pstTree->m_lData;

    return _pDeleteTree(pvData, pstSavm->tblName, pstRoot, pstTree);
}
   
/*************************************************************************************************
    description：Remove the index from the hash table
    parameters:
        pstSavm                     --stvm handle
        pvAddr                      --memory address
        pstTree                     --tree node
        lData                       --data offset
        pvData                      --data truck
    return:
        RC_SUCC                     --success
        RC_FAIL                     --failure
 *************************************************************************************************/
long    _lPurgedHash(SATvm *pstSavm, void *pvAddr, SHTree *pstTree, void *pvData)
{
    SHList  *pstList = NULL;
    SHTruck *pstTruck = NULL;
    size_t  lNext = 0, lOffset;

    pstList = (SHList *)pGetNode(pvAddr, pstTree->m_lData);
    lOffset = lGetListOfs(pstSavm->tblName) + ((TblDef *)pvAddr)->m_lValid * sizeof(SHList);
    for(;SELF_POS_UNUSE != pstList->m_lPos; pstList = (SHList *)pGetNode(pvAddr, lNext))
    {    
        lNext = pstList->m_lNext;
        pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
        if(memcmp(pstTruck->m_pvData, pvData, pstSavm->lSize))
        {    
            pstSavm->m_lErrno = SVR_EXCEPTION;
            return RC_FAIL;
        }

        lOffset -= sizeof(SHList);
        if(RC_SUCC != lReleaseList(pvAddr, pstSavm->tblName, pstTree, pstList, lOffset, &lNext))
        {    
            pstSavm->m_lErrno = SVR_EXCEPTION;
            return RC_FAIL;
        }    

        if(SELF_POS_UNUSE == pstTree->m_lData)
            pstTree->m_lSePos = SELF_POS_UNUSE;

        return RC_SUCC;
    }    

    return RC_FAIL;
}

/*************************************************************************************************
    description：Remove the execution index from RBTree
    parameters:
        pstSavm                    --stvm handle
        pstRoot                    --tree Root
        psvIdx                     --index value
        lIdx                       --value length
        pvData                     --data truck
        plOut                      --lOffset
        eType
    return:
        SHTree*
 *************************************************************************************************/
SHTree*    _pRemoveGroup(SATvm *pstSavm, SHTree *pstRoot, void *psvIdx, long lIdx, void *pvData, 
            long *plOut, long eType)
{
    SHList  *pstList = NULL;
    SHTree  *pstTree = NULL;
    SHTruck *pstTruck = NULL;
    size_t  lNext = 0, lOffset;
    void    *pvAddr = pGetAddr(pstSavm, pstSavm->tblName);

    if(NULL == (pstTree = pSearchTree(pvAddr, pstRoot, psvIdx, lIdx)))
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return NULL;
    }

    if(SELF_POS_UNUSE == pstTree->m_lData)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return _pDeleteGroup(pvAddr, pstSavm->tblName, pstRoot, pstTree, plOut);
    }

    pstList = (SHList *)pGetNode(pvAddr, pstTree->m_lData);

    lOffset = lGetListOfs(pstSavm->tblName) + ((TblDef *)pvAddr)->m_lValid * sizeof(SHList);
    for(*plOut = 0; SELF_POS_UNUSE != pstList->m_lPos; pstList = (SHList *)pGetNode(pvAddr, lNext))
    {
        lNext = pstList->m_lNext;
        pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
        if(0 == eType)
        {
            if(memcmp(pvData, (void *)&pstList->m_lData, sizeof(pstList->m_lData)))
            {
                if(SELF_POS_UNUSE == lNext)    break;
                continue;
            }
        }
        else
        {
            if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pvData))
            {
                if(SELF_POS_UNUSE == lNext)    break;
                continue;
            }
        }

        (*plOut) ++;
        lOffset -= sizeof(SHList);

        // lNext may be tail node, just moved, here need to update lNext
        if(RC_SUCC != lReleaseList(pvAddr, pstSavm->tblName, pstTree, pstList, lOffset, &lNext))
        {
            pstSavm->m_lErrno = SVR_EXCEPTION;
            return pstRoot;
        }

        if(SELF_POS_UNUSE == lNext || 0 == eType)    break;
    } 

    if(SELF_POS_UNUSE != pstTree->m_lData)
        return pstRoot;
    else
        return _pDeleteGroup(pvAddr, pstSavm->tblName, pstRoot, pstTree, plOut);
}

/*************************************************************************************************
    description：Remove the execution index from RBTree by Offset
    parameters:
        pstSavm                    --stvm handle
        pstRoot                    --tree Root
        psvIdx                     --index value
        lIdx                       --value length
        pvData                     --data truck
        plOut                      --lOffset
        eType
    return:
        SHTree*
 *************************************************************************************************/
SHTree*    pDeleteGroup(SATvm *pstSavm, SHTree *pstRoot, void *psvIdx, long lIdx, void *pvData, 
            long *plOut)
{
    return _pRemoveGroup(pstSavm, pstRoot, psvIdx, lIdx, pvData, plOut, 0);
}

/*************************************************************************************************
    description：Remove the index from RBTree
    parameters:
        pstSavm                    --stvm handle
        pstRoot                    --tree Root
        psvIdx                     --index value
        lIdx                       --value length
        pvData                     --data truck
        plOut                      --lOffset
        eType
    return:
        SHTree*
 *************************************************************************************************/
SHTree*    pRemoveGroup(SATvm *pstSavm, SHTree *pstRoot, void *psvIdx, long lIdx, void *pvData, 
            long *plOut)
{
    return _pRemoveGroup(pstSavm, pstRoot, psvIdx, lIdx, pvData, plOut, 1);
}

/*************************************************************************************************
    description：Remove the corresponding data from Hash
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        pstTree                    --tree node
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    __lDeleteHash(SATvm *pstSavm, void *pvAddr, SHTree *pstTree, TABLE t)
{
    SHTree  *pstRoot = NULL;
    SHList  *pstList = NULL;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN];
    size_t  lNext = 0, lOffset, lData = 0;
    RWLock  *prwLock = (RWLock *)pGetRWLock(pvAddr);

    pstSavm->m_lEType = EXE_PLAN_GRP;
    if(RC_SUCC != pthread_rwlock_wrlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DOWR_ERR;
        return RC_FAIL;
    }

    pstList  = (SHList *)pGetNode(pvAddr, pstTree->m_lData);
    lOffset  = lGetListOfs(t) + ((TblDef *)pvAddr)->m_lValid * sizeof(SHList);
    for(pstSavm->m_lEffect = 0; SELF_POS_UNUSE != pstList->m_lPos;
        pstList = (SHList *)pGetNode(pvAddr, lNext))
    {
        lNext = pstList->m_lNext;
        pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
        {
            if(SELF_POS_UNUSE == pstList->m_lNext)    break;
            continue;
        }

        if(HAVE_UNIQ_IDX(t)) 
        {
            memset(szIdx, 0, sizeof(szIdx));
            if(NULL == pPickIndex(lGetIdxNum(t), pGetTblIdx(t), pstTruck->m_pvData, szIdx))
            {
                pthread_rwlock_unlock(prwLock);
                pstSavm->m_lErrno = SVR_EXCEPTION;
                return RC_FAIL;
            }

            if(NULL == (pstRoot = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lTreeRoot)))
            {
                pthread_rwlock_unlock(prwLock);
                pstSavm->m_lErrno = SVR_EXCEPTION;
                return RC_FAIL;
            }

            pstRoot = pDeleteTree(pstSavm, pstRoot, szIdx, lGetIdxLen(t), &lData);
            if(!pstRoot || pstList->m_lData != lData)
            {
                pthread_rwlock_unlock(prwLock);
                pstSavm->m_lErrno = SVR_EXCEPTION;
                return RC_FAIL;
            }

            ((TblDef *)pvAddr)->m_lTreeRoot = pstRoot->m_lSePos;
        }

        pstSavm->m_lEffect ++;
        lOffset -= sizeof(SHList);
        if(RC_SUCC != lReleaseList(pvAddr, t, pstTree, pstList, lOffset, &lNext))
        {
            pthread_rwlock_unlock(prwLock);
            pstSavm->m_lErrno = SVR_EXCEPTION;
            return RC_FAIL;
        }

        if(SELF_POS_UNUSE == pstTree->m_lData)
            pstTree->m_lSePos = SELF_POS_UNUSE;

        if(RC_SUCC != lRecordWork(pstSavm, pstTruck->m_pvData, OPERATE_DELETE))
        {
            pthread_rwlock_unlock(prwLock);
            return RC_FAIL;
        }

        lReleaseTruck(pvAddr, t, pstTruck, true);
        ((TblDef *)pvAddr)->m_lValid --;

        if(SELF_POS_UNUSE == lNext)          break;
        if(FIRST_ROW & pstSavm->lFind)       break;
    }

    pthread_rwlock_unlock(prwLock);

    return RC_SUCC;
}

/*************************************************************************************************
    description：Delete Hash table points to the data
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lDeleteHash(SATvm *pstSavm, void *pvAddr, TABLE t)
{
    size_t  lIdx;
    SHTree  *pstTree = NULL;
    char    szIdx[MAX_INDEX_LEN];

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetGrpNum(t), pGetTblGrp(t), pstSavm->pstVoid, szIdx))
        return RC_CONTU;

    lIdx = uGetHash(szIdx, lGetGrpLen(t)) % ((TblDef *)pvAddr)->m_lMaxRow;
    pstTree = pvAddr + ((TblDef *)pvAddr)->m_lGroupRoot + lIdx * sizeof(SHTree);
    if(NULL == pstTree || SELF_POS_UNUSE == pstTree->m_lData)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    return __lDeleteHash(pstSavm, pvAddr, pstTree, t);
}

/*************************************************************************************************
    description：For the unique index traversal of data deleted
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        psvIdx                     --index value
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    __lDeleteIndex(SATvm *pstSavm, void *pvAddr, TABLE t, void *psvIdx)
{
    long    lRow = 0;
    size_t  lData, lIdx;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN];
    SHTree  *pstRoot = NULL, *pstTree = NULL;
    RWLock  *prwLock = (RWLock *)pGetRWLock(pvAddr);

    if(RC_SUCC != pthread_rwlock_wrlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DOWR_ERR;
        return RC_FAIL;
    }

    pstSavm->m_lEType = EXE_PLAN_IDX;
    if(NULL == (pstRoot = pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lTreeRoot)))
    {
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    if(NULL == (pstTree = pSearchTree(pvAddr, pstRoot, psvIdx, lGetIdxLen(t))))
    {
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    lData = pstTree->m_lData;
    pstTruck = (PSHTruck)pGetNode(pvAddr, pstTree->m_lData);
    if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
    {
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    if(NULL == (pstRoot = _pDeleteTree(pvAddr, pstSavm->tblName, pstRoot, pstTree)))
    {
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    ((TblDef *)pvAddr)->m_lTreeRoot = pstRoot->m_lSePos;
    
    if(HAVE_NORL_IDX(t)) 
    {
        memset(szIdx, 0, sizeof(szIdx));
        if(NULL == pPickIndex(lGetGrpNum(t), pGetTblGrp(t), pstTruck->m_pvData, szIdx))
        {
            pthread_rwlock_unlock(prwLock);
            pstSavm->m_lErrno = SVR_EXCEPTION;
            return RC_FAIL;
        }

        if(NULL == (pstRoot = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lGroupRoot)))
        {   
            pthread_rwlock_unlock(prwLock);
            pstSavm->m_lErrno = SVR_EXCEPTION;
            return RC_FAIL;
        }    

        if(NULL == (pstRoot = pDeleteGroup(pstSavm, pstRoot, szIdx, lGetGrpLen(t), &lData, &lRow)))
        {    
            pthread_rwlock_unlock(prwLock);
            return RC_FAIL;
        }

        if(1 != lRow)
        {
            pthread_rwlock_unlock(prwLock);
            pstSavm->m_lErrno = SVR_EXCEPTION;
            return RC_FAIL;
        }

        ((TblDef *)pvAddr)->m_lGroupRoot = pstRoot->m_lSePos;    
    }
    else if(HAVE_HASH_IDX(t))
    {
        memset(szIdx, 0, sizeof(szIdx));
        if(NULL == pPickIndex(lGetGrpNum(t), pGetTblGrp(t), pstTruck->m_pvData, szIdx))
        {
            pthread_rwlock_unlock(prwLock);
            pstSavm->m_lErrno = SVR_EXCEPTION;
            return RC_FAIL;
        }

        lIdx = uGetHash(szIdx, lGetGrpLen(t)) % ((TblDef *)pvAddr)->m_lMaxRow;
        pstRoot = pvAddr + ((TblDef *)pvAddr)->m_lGroupRoot + lIdx * sizeof(SHTree);
        if(NULL == pstRoot || SELF_POS_UNUSE == pstRoot->m_lData)
        {
            pthread_rwlock_unlock(prwLock);
            pstSavm->m_lErrno = SVR_EXCEPTION;
            return RC_FAIL;
        }

        if(RC_SUCC != _lPurgedHash(pstSavm, pvAddr, pstRoot, pstTruck->m_pvData))
        {
            pthread_rwlock_unlock(prwLock);
            return RC_FAIL;
        }
    }

    if(RC_SUCC != lRecordWork(pstSavm, pstTruck->m_pvData, OPERATE_DELETE))
    {
        pthread_rwlock_unlock(prwLock);
        return RC_FAIL;
    }

    lReleaseTruck(pvAddr, t, pstTruck, true);
    pthread_rwlock_unlock(prwLock);

    pstSavm->m_lEffect = 1;
    ((TblDef *)pvAddr)->m_lValid --;

    return RC_SUCC;
}

/*************************************************************************************************
    description：delete The unique index truck data
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lDeleteIndex(SATvm *pstSavm, void *pvAddr, TABLE t)
{
    char    szIdx[MAX_INDEX_LEN];

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetIdxNum(t), pGetTblIdx(t), pstSavm->pstVoid, szIdx))
        return RC_CONTU;

    return __lDeleteIndex(pstSavm, pvAddr, t, (void *)szIdx);
}

/*************************************************************************************************
    description：Delete the data that matches conditions
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        pstTree                    --tree node
        t                          --table
        psvIdx                     --index value
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    __lDeleteGroup(SATvm *pstSavm, void *pvAddr, TABLE t, void *psvIdx)
{
    SHList  *pstList = NULL;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN];
    size_t  lNext = 0, lOffset, lData = 0;
    RWLock  *prwLock = (RWLock *)pGetRWLock(pvAddr);
    SHTree  *pstRoot = NULL, *pstTree = NULL, *pstIRoot = NULL;

    pstSavm->m_lEType = EXE_PLAN_GRP;
    if(RC_SUCC != pthread_rwlock_wrlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DOWR_ERR;
        return RC_FAIL;
    }

    if(NULL == (pstRoot = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lGroupRoot)))
    {
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    if(NULL == (pstTree = pSearchTree(pvAddr, pstRoot, psvIdx, lGetGrpLen(t))))
    {
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    if(SELF_POS_UNUSE == pstTree->m_lData)
    {
        pstRoot = _pDeleteGroup(pvAddr, t, pstRoot, pstTree, &pstSavm->m_lEffect);
        ((TblDef *)pvAddr)->m_lGroupRoot = pstRoot->m_lSePos;    
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_SUCC;
    }
    
    pstList  = (SHList *)pGetNode(pvAddr, pstTree->m_lData);
    lOffset  = lGetListOfs(t) + ((TblDef *)pvAddr)->m_lValid * sizeof(SHList);
    for(pstSavm->m_lEffect = 0; SELF_POS_UNUSE != pstList->m_lPos; 
        pstList = (SHList *)pGetNode(pvAddr, lNext))
    {
        lNext = pstList->m_lNext;
        pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
        {
            if(SELF_POS_UNUSE == pstList->m_lNext) break;
            continue;
        }

        if(HAVE_UNIQ_IDX(t)) 
        {
            memset(szIdx, 0, sizeof(szIdx));
            if(NULL == pPickIndex(lGetIdxNum(t), pGetTblIdx(t), pstTruck->m_pvData, szIdx))
            {
                pthread_rwlock_unlock(prwLock);
                pstSavm->m_lErrno = SVR_EXCEPTION;
                return RC_FAIL;
            }

            if(NULL == (pstIRoot = pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lTreeRoot)))
            {   
                pthread_rwlock_unlock(prwLock);
                pstSavm->m_lErrno = SVR_EXCEPTION;
                return RC_FAIL;
            }

            pstIRoot = pDeleteTree(pstSavm, pstIRoot, szIdx, lGetIdxLen(t), &lData);
            if(!pstIRoot || pstList->m_lData != lData)
            {
                pthread_rwlock_unlock(prwLock);
                pstSavm->m_lErrno = SVR_EXCEPTION;
                return RC_FAIL;
            }

            ((TblDef *)pvAddr)->m_lTreeRoot = pstIRoot->m_lSePos;
        }

        pstSavm->m_lEffect ++;
        lOffset -= sizeof(SHList);
        if(RC_SUCC != lReleaseList(pvAddr, t, pstTree, pstList, lOffset, &lNext))
        {
            pthread_rwlock_unlock(prwLock);
            pstSavm->m_lErrno = SVR_EXCEPTION;
            return RC_FAIL;
        }

        if(RC_SUCC != lRecordWork(pstSavm, pstTruck->m_pvData, OPERATE_DELETE))
        {
            pthread_rwlock_unlock(prwLock);
            return RC_FAIL;
        }

        lReleaseTruck(pvAddr, t, pstTruck, true);
        ((TblDef *)pvAddr)->m_lValid --;

        if(SELF_POS_UNUSE == lNext)       break;
        if(FIRST_ROW & pstSavm->lFind)    break;
    }

    if(SELF_POS_UNUSE == pstTree->m_lData)
    {
        pstRoot = _pDeleteGroup(pvAddr, pstSavm->tblName, pstRoot, pstTree, &pstSavm->m_lEffect);
        ((TblDef *)pvAddr)->m_lGroupRoot = pstRoot->m_lSePos;    
    }

    pthread_rwlock_unlock(prwLock);

    return RC_SUCC;
}

/*************************************************************************************************
    description：Delete the data that matches conditions
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lDeleteGroup(SATvm *pstSavm, void *pvAddr, TABLE t)
{
    char    szIdx[MAX_INDEX_LEN];

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetGrpNum(t), pGetTblGrp(t), pstSavm->pstVoid, szIdx))
        return RC_CONTU;

    return __lDeleteGroup(pstSavm, pvAddr, t, (void *)szIdx);
}

/*************************************************************************************************
    description：Delete the truck that matches conditions
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lDeleteTruck(SATvm *pstSavm, void *pvAddr, TABLE t)
{
    bool    bIsIdx = false;
    SHTree  *pstRoot = NULL;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN];
    RWLock  *prwLock = (RWLock *)pGetRWLock(pvAddr);
    size_t  lData = 0, lOffset = lGetTblData(t), lIdx;
    long    lRow, lValid = ((TblDef *)pvAddr)->m_lValid;

    if(HAVE_INDEX(t))    bIsIdx = true;

    if(RC_SUCC != pthread_rwlock_wrlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DOWR_ERR;
        return RC_FAIL;
    }

    pstSavm->m_lEType = EXE_PLAN_ALL;
    for(lRow = 0, pstSavm->m_lEffect = 0, pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset); 
        (lRow < lValid) && (lOffset < lGetTableSize(t)); pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset))
    {
        if(IS_TRUCK_NULL(pstTruck))
        {
            lOffset += lGetRowTruck(t);
            continue;
        }

        ++ lRow; 
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
        {
            lOffset += lGetRowTruck(t);
            continue;
        }

        pstSavm->m_lEffect ++;
        if(HAVE_UNIQ_IDX(t)) 
        {
            memset(szIdx, 0, sizeof(szIdx));
            if(NULL == pPickIndex(lGetIdxNum(t), pGetTblIdx(t), pstTruck->m_pvData, szIdx))
            {
                pthread_rwlock_unlock(prwLock);
                pstSavm->m_lErrno = SVR_EXCEPTION;
                return RC_FAIL;
            }

            if(NULL == (pstRoot = pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lTreeRoot)))
            {   
                pthread_rwlock_unlock(prwLock);
                pstSavm->m_lErrno = SVR_EXCEPTION;
                return RC_FAIL;
            }

            pstRoot = pDeleteTree(pstSavm, pstRoot, szIdx, lGetIdxLen(t), &lData);
            if(!pstRoot || lOffset != lData)
            {
                pthread_rwlock_unlock(prwLock);
                pstSavm->m_lErrno = SVR_EXCEPTION;
                return RC_FAIL;
            }

            ((TblDef *)pvAddr)->m_lTreeRoot = pstRoot->m_lSePos;
        }

        if(HAVE_NORL_IDX(t))
        {
            memset(szIdx, 0, sizeof(szIdx));
            if(NULL == pPickIndex(lGetGrpNum(t), pGetTblGrp(t), pstTruck->m_pvData, szIdx))
            {
                pthread_rwlock_unlock(prwLock);
                pstSavm->m_lErrno = SVR_EXCEPTION;
                return RC_FAIL;
            }
        
            if(NULL == (pstRoot = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lGroupRoot)))
            {   
                pthread_rwlock_unlock(prwLock);
                pstSavm->m_lErrno = SVR_EXCEPTION;
                return RC_FAIL;
            }

            pstRoot = pDeleteGroup(pstSavm, pstRoot, szIdx, lGetGrpLen(t), &lOffset, (long *)&lData);
            if(!pstRoot)    return RC_FAIL;
            if(1 != lData)
            {
                pthread_rwlock_unlock(prwLock);
                pstSavm->m_lErrno = SVR_EXCEPTION;
                return RC_FAIL;
            }
        
            ((TblDef *)pvAddr)->m_lGroupRoot = pstRoot->m_lSePos;    
        }
        else if(HAVE_HASH_IDX(t))
        {
            memset(szIdx, 0, sizeof(szIdx));
            if(NULL == pPickIndex(lGetGrpNum(t), pGetTblGrp(t), pstTruck->m_pvData, szIdx))
            {
                pthread_rwlock_unlock(prwLock);
                pstSavm->m_lErrno = SVR_EXCEPTION;
                return RC_FAIL;
            }

            lIdx = uGetHash(szIdx, lGetGrpLen(t)) % ((TblDef *)pvAddr)->m_lMaxRow;
            pstRoot = pvAddr + ((TblDef *)pvAddr)->m_lGroupRoot + lIdx * sizeof(SHTree);
            if(NULL == pstRoot || SELF_POS_UNUSE == pstRoot->m_lData)
            {
                pthread_rwlock_unlock(prwLock);
                pstSavm->m_lErrno = SVR_EXCEPTION;
                return RC_FAIL;
            }

            if(RC_SUCC != _lPurgedHash(pstSavm, pvAddr, pstRoot, pstTruck->m_pvData))
            {
                pthread_rwlock_unlock(prwLock);
                return RC_FAIL;
            }
        }

        if(bIsIdx)    lOffset += lGetRowTruck(t);

        if(RC_SUCC != lRecordWork(pstSavm, pstTruck->m_pvData, OPERATE_DELETE))
        {
            pthread_rwlock_unlock(prwLock);
            return RC_FAIL;
        }

        lReleaseTruck(pvAddr, t, pstTruck, bIsIdx);
        ((TblDef *)pvAddr)->m_lValid --;
        if(FIRST_ROW & pstSavm->lFind)    break;
    }
    pthread_rwlock_unlock(prwLock);

    if(0 == pstSavm->m_lEffect)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：API - delete
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lDelete(SATvm *pstSavm)
{
    long    lRet;
    RunTime *pstRun = NULL;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    if(NULL == (pstRun = (RunTime *)pInitMemTable(pstSavm, pstSavm->tblName)))
        return RC_FAIL;

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        Tremohold(pstSavm, pstRun);
        return _lDeleteByRt(pstSavm);
    }

    if(((TblDef *)pstRun->m_pvAddr)->m_lValid <= 0)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        vTblDisconnect(pstSavm, pstSavm->tblName);
        return RC_FAIL;
    }

    if(!pstSavm->pstVoid)
    {
        lRet = _lDeleteTruck(pstSavm, pstRun->m_pvAddr, pstSavm->tblName);
        vTblDisconnect(pstSavm, pstSavm->tblName);
        return lRet;
    }

    if(HAVE_UNIQ_IDX(pstSavm->tblName))
    {
        lRet = _lDeleteIndex(pstSavm, pstRun->m_pvAddr, pstSavm->tblName);
        if(RC_CONTU != lRet)
        {
            vTblDisconnect(pstSavm, pstSavm->tblName);
            return lRet;
        }
    }

    if(HAVE_NORL_IDX(pstSavm->tblName))
    {
        lRet = _lDeleteGroup(pstSavm, pstRun->m_pvAddr, pstSavm->tblName);
        if(RC_CONTU != lRet)
        {
            vTblDisconnect(pstSavm, pstSavm->tblName);
            return lRet;
        }
    }
    else if(HAVE_HASH_IDX(pstSavm->tblName))
    {
        lRet = _lDeleteHash(pstSavm, pstRun->m_pvAddr, pstSavm->tblName);
        if(RC_CONTU != lRet)
        {
            vTblDisconnect(pstSavm, pstSavm->tblName);
            return lRet;
        }
    }

    lRet = _lDeleteTruck(pstSavm, pstRun->m_pvAddr, pstSavm->tblName);
    vTblDisconnect(pstSavm, pstSavm->tblName);
    return lRet;
}

/*************************************************************************************************
    description：Statistical compliance record
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        plCount                    --index value
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lCountIndex(SATvm *pstSavm, void *pvAddr, TABLE t, size_t *plCount)
{
    SHTree  *pstTree = NULL;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN];

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetIdxNum(t), pGetTblIdx(t), pstSavm->pstVoid, szIdx))
        return RC_CONTU;

    pstSavm->m_lEType = EXE_PLAN_IDX;
    if(NULL == (pstTree = pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lTreeRoot)))
    {
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    if(NULL == (pstTree = pSearchTree(pvAddr, pstTree, szIdx, lGetIdxLen(t))))
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    pstTruck = (PSHTruck)pGetNode(pvAddr, pstTree->m_lData);
    if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    pstSavm->m_lEffect = *plCount = 1;
    return RC_SUCC;
}

/*************************************************************************************************
    description：The statistics matches the index record
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        plCount
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lCountGroup(SATvm *pstSavm, void *pvAddr, TABLE t, size_t *plCount)
{
    size_t  lNext;
    SHList  *pstList = NULL;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN];
    SHTree  *pstRoot = NULL, *pstTree = NULL;

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetGrpNum(t), pGetTblGrp(t), pstSavm->pstVoid, szIdx))
        return RC_CONTU;

    *plCount = 0;
    if(NULL == (pstRoot = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lGroupRoot)))
    {
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    if(NULL == (pstTree = pSearchTree(pvAddr, pstRoot, szIdx, lGetGrpLen(t))))
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_SUCC;
    }

    pstList = (SHList *)pGetNode(pvAddr, pstTree->m_lData);
    if(lGetGrpNum(t) == pstSavm->stCond.uFldcmp)
    {
        *plCount = lGetListCount(pvAddr, pstList);
        pstSavm->m_lEffect = *plCount;
        return RC_SUCC;
    }

    for(; SELF_POS_UNUSE != pstList->m_lPos; pstList = (SHList *)pGetNode(pvAddr, lNext))
    {
        lNext = pstList->m_lNext;
        pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
        {
            if(SELF_POS_UNUSE == pstList->m_lNext) break;
            continue;
        }

        (*plCount) ++;
        if(SELF_POS_UNUSE == lNext)     break;
    }

    if(0 == (pstSavm->m_lEffect = *plCount))
        pstSavm->m_lErrno = NO_DATA_FOUND;

    return RC_SUCC;
}

/*************************************************************************************************
    description：The statistics matches the hash record
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        pstTree                    --tree node
        t                          --table
        psvIdx                     --index value
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lCountHash(SATvm *pstSavm, void *pvAddr, TABLE t, size_t *plCount)
{
    size_t  lIdx, lNext;
    SHList  *pstList = NULL;
    SHTruck *pstTruck = NULL;
    SHTree  *pstTree = NULL;
    char    szIdx[MAX_INDEX_LEN];

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetGrpNum(t), pGetTblGrp(t), pstSavm->pstVoid, szIdx))
        return RC_CONTU;

    *plCount = 0;
    lIdx = uGetHash(szIdx, lGetGrpLen(t)) % ((TblDef *)pvAddr)->m_lMaxRow;
    pstTree = pvAddr + ((TblDef *)pvAddr)->m_lGroupRoot + lIdx * sizeof(SHTree);
    if(NULL == pstTree || SELF_POS_UNUSE == pstTree->m_lData)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_SUCC;
    }

    for(pstList  = (SHList *)pGetNode(pvAddr, pstTree->m_lData);
        SELF_POS_UNUSE != pstList->m_lPos; pstList = (SHList *)pGetNode(pvAddr, lNext))
    {
        lNext = pstList->m_lNext;
        pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
        {
            if(SELF_POS_UNUSE == pstList->m_lNext) break;
            continue;
        }

        (*plCount) ++;
        if(SELF_POS_UNUSE == lNext)     break;
    }

    if(0 == (pstSavm->m_lEffect = *plCount))
        pstSavm->m_lErrno = NO_DATA_FOUND;

    return RC_SUCC;
}

/*************************************************************************************************
    description：The statistics matches the truck record
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        plCount                    --count
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lCountTruck(SATvm *pstSavm, void *pvAddr, TABLE t, size_t *plCount)
{
    SHTruck *pstTruck = NULL;
    FdCond  *pstCond  = &pstSavm->stCond;
    size_t  lRow = 0, lOffset = lGetTblData(t);

    for(*plCount = 0; (lRow < ((TblDef *)pvAddr)->m_lValid) && (lOffset < lGetTableSize(t)); 
        lOffset += lGetRowTruck(t))
    {
        pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset);
        if(IS_TRUCK_NULL(pstTruck))
            continue;

        ++ lRow; 
        if(RC_MISMA == lFeildMatch(pstCond, pstTruck->m_pvData, pstSavm->pstVoid))
            continue;

        (*plCount) ++;
    }

    if(0 == (pstSavm->m_lEffect = *plCount))
        pstSavm->m_lErrno = NO_DATA_FOUND;

    return RC_SUCC;
}

/*************************************************************************************************
    description：API - lCount
    parameters:
        pstSavm                    --stvm handle
        plCount                    --count
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lCount(SATvm *pstSavm, size_t *plCount)
{
    long    lRet;
    RunTime *pstRun = NULL;
    RWLock  *prwLock = NULL;

    if(!pstSavm || !plCount)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    if(NULL == (pstRun = (RunTime *)pInitMemTable(pstSavm, pstSavm->tblName)))
        return RC_FAIL;

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        Tremohold(pstSavm, pstRun);
        return _lCountByRt(pstSavm, plCount);
    }

    if(((TblDef *)pstRun->m_pvAddr)->m_lValid <= 0)
    {
        *plCount = 0;
        vTblDisconnect(pstSavm, pstSavm->tblName);
        return RC_SUCC;
    }

    if(!pstSavm->pstVoid || pstSavm->stCond.uFldcmp == 0)
    {
        *plCount = ((TblDef *)pstRun->m_pvAddr)->m_lValid;
        pstSavm->m_lEffect = *plCount;
        vTblDisconnect(pstSavm, pstSavm->tblName);
        return RC_SUCC;
    }

    if(HAVE_UNIQ_IDX(pstSavm->tblName))
    {
        lRet = _lCountIndex(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, plCount);
        if(RC_CONTU != lRet)
        {
            pstSavm->m_lEType = EXE_PLAN_IDX;
            vTblDisconnect(pstSavm, pstSavm->tblName);
            return lRet;
        }
    }

    if(HAVE_NORL_IDX(pstSavm->tblName))
    {
        lRet = _lCountGroup(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, plCount);
        if(RC_CONTU != lRet)
        {
            pstSavm->m_lEType = EXE_PLAN_GRP;
            vTblDisconnect(pstSavm, pstSavm->tblName);
            return lRet;
        }
    }
    else if(HAVE_HASH_IDX(pstSavm->tblName))
    {
        lRet = _lCountHash(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, plCount);
        if(RC_CONTU != lRet)
        {
            pstSavm->m_lEType = EXE_PLAN_GRP;
            vTblDisconnect(pstSavm, pstSavm->tblName);
            return lRet;
        }
    }

    pstSavm->m_lEType = EXE_PLAN_ALL;
    lRet = _lCountTruck(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, plCount);
    vTblDisconnect(pstSavm, pstSavm->tblName);
    return lRet;
}

/*************************************************************************************************
    description：Rebuild unique index
    parameters:
        pstSavm                    --stvm handle
        pstRoot                    --tree root
        psvIdx                     --index value
        lIdx                       --index length
        lRow                       --rows
        lData                      --data offset
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
SHTree*    pRebuildTree(SATvm *pstSavm, SHTree *pstRoot, void *psvIdx, long lIdx, long lRow, size_t lData)
{
    int     nRet = 0;
    size_t  lOffset, lPos = 0, lNext = 0;
    void    *pvData = NULL, *pvAddr = pGetAddr(pstSavm, pstSavm->tblName);
    SHTree  *pstNode = NULL, *pstCur = NULL, *pstNext = NULL, *pstTree = pstRoot;

    while(SELF_POS_UNUSE != pstTree->m_lSePos && NODE_NULL != pstTree->m_lSePos)
    {
        pstNode = pstTree;
        nRet = memcmp(pstTree->m_szIdx, psvIdx, lIdx);
        if(0 < nRet)
        {
            if(NODE_NULL == pstTree->m_lLeft)
                break;
            pstTree = (SHTree *)pGetNode(pvAddr, pstTree->m_lLeft);
        }
        else if(0 > nRet)
        {
            if(NODE_NULL == pstTree->m_lRight)
                break;
            pstTree = (SHTree *)pGetNode(pvAddr, pstTree->m_lRight);
        }
        else
        {
            pstSavm->m_lErrno = UNIQ_IDX_REPT;
            return NULL;
        }
    }

    lOffset = sizeof(SHTree) * lRow + lGetIdxPos(pstSavm->tblName);
    if(NULL == (pstCur = pCreateNode(pvAddr, pstSavm->tblName, lOffset)))
    {
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return NULL;
    }

    lPos = pstCur->m_lData;
    pstCur->m_lIdx    = lIdx;
    pstCur->m_lSePos  = lOffset;
    pstCur->m_lData   = lData;
    pstCur->m_lParent = NODE_NULL;
    memcpy(pstCur->m_szIdx, psvIdx, pstCur->m_lIdx);
    pstCur->m_lLeft = pstCur->m_lRight = NODE_NULL;

    if(lData != lPos)  
    {
        lNext = (lData - lGetTblData(pstSavm->tblName)) / lGetRowTruck(pstSavm->tblName);
        pstNext = (SHTree *)pGetNode(pvAddr, lNext * sizeof(SHTree) + lGetIdxPos(pstSavm->tblName));
        pstNext->m_lData = lPos;
    }

    if(pstNode != NULL)
    {
        pstCur->m_eColor = COLOR_RED;
        pstCur->m_lParent = pstNode->m_lSePos;
        if(0 < memcmp(pstNode->m_szIdx, pstCur->m_szIdx, pstCur->m_lIdx))
            pstNode->m_lLeft = pstCur->m_lSePos;
        else
            pstNode->m_lRight = pstCur->m_lSePos;
    }
    else
    {
        pstRoot = pstCur;
        pstCur->m_eColor = COLOR_BLK;
        return pstRoot; 
    }

    return _pFixupInsert(pvAddr, pstRoot, pstCur);
}

/*************************************************************************************************
    description：Rebuild the index node
    parameters:
        pvAddr                     --memory address
        t                          --table
        pstGroup                   --group node
        lGroup                     --group offset 
        lRow                       --rows
        lData                      --data offset
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
SHTree*    _pRebuildGroup(void *pvAddr, TABLE t, SHTree *pstGroup, long lGroup, long lRow, size_t lData)
{
    size_t  lOffset = 0, lPos = 0, lNext = 0;
    SHList  *pstList = NULL, *pstNode = NULL, *pstNext = NULL;

    if(!pstGroup)   pstGroup = (SHTree *)(pvAddr + lGroup);
    lOffset = lGetListOfs(t) + sizeof(SHList) * lRow;
    pstList = (SHList *)pGetNode(pvAddr, lOffset);
    pstList->m_lPos = lOffset;
    lPos = pstList->m_lData;
    pstList->m_lData= lData;

    if(lData != lPos)
    {
        lNext = (lData - lGetTblData(t)) / lGetRowTruck(t);
        pstNext = (SHList *)pGetNode(pvAddr, lNext * sizeof(SHList) + lGetListOfs(t));
        pstNext->m_lData = lPos;
    }  

    if(pstGroup->m_lData == SELF_POS_UNUSE)
    {
        pstList->m_lNode = lGroup;
        pstGroup->m_lData = pstList->m_lPos;
    }
    else
    {
        pstNode = (SHList *)pGetNode(pvAddr, pstGroup->m_lData);

        pstList->m_lNext = pstNode->m_lNext;
        pstList->m_lLast = pstNode->m_lPos;
        if(SELF_POS_UNUSE != pstNode->m_lNext)
            ((SHList *)pGetNode(pvAddr, pstNode->m_lNext))->m_lLast = pstList->m_lPos;
        pstNode->m_lNext = pstList->m_lPos;
    }

    return pstGroup;
}

/*************************************************************************************************
    description：Rebuild the hash index node
    parameters:
        pstSavm                    --stvm handle
        t                          --table
        pvAddr                     --Memory address
        pstTree                    --tree node
        lRow                       --rows
        lData                      --data offset
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lRebuildHash(SATvm *pstSavm, TABLE t, void *pvAddr, SHTree *pstTree, size_t lRow, size_t lData)
{
    size_t  lOffset = 0, lPos, lNext;
    SHList  *pstList = NULL, *pstNode = NULL, *pstNext = NULL;

    lOffset = lGetListOfs(t) + sizeof(SHList) * lRow;
    pstList = (SHList *)pGetNode(pvAddr, lOffset);
    pstList->m_lPos = lOffset;
    lPos = pstList->m_lData;
    pstList->m_lData= lData;

    if(lData != lPos) 
    {
        lNext = (lData - lGetTblData(t)) / lGetRowTruck(t);
        pstNext = (SHList *)pGetNode(pvAddr, lNext * sizeof(SHList) + lGetListOfs(t));
        pstNext->m_lData = lPos;
    }

    if(SELF_POS_UNUSE == pstTree->m_lSePos || NODE_NULL == pstTree->m_lSePos)
    {
        pstTree->m_lSePos= (void *)pstTree - pvAddr;
        pstList->m_lNode = pstTree->m_lSePos;
        pstTree->m_lData = pstList->m_lPos;
    }
    else
    {
        pstNode = (SHList *)pGetNode(pvAddr, pstTree->m_lData);

        pstList->m_lNext = pstNode->m_lNext;
        pstList->m_lLast = pstNode->m_lPos;
        if(SELF_POS_UNUSE != pstNode->m_lNext)
            ((SHList *)pGetNode(pvAddr, pstNode->m_lNext))->m_lLast = pstList->m_lPos;
        pstNode->m_lNext = pstList->m_lPos;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：Rebuild the index
    parameters:
        pstSavm                    --stvm handle
        pstRoot                    --root node
        psvIdx                     --index values
        lRow                       --rows
        lData                      --data offset
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
SHTree*    pRebuildGroup(SATvm *pstSavm, SHTree *pstRoot, char *psvIdx, long lIdx, long lRow, size_t lData)
{
    int     nRet;
    size_t  lOffset;
    SHTruck *pstTruck = NULL;
    void    *pvAddr = pGetAddr(pstSavm, pstSavm->tblName);
    SHTree  *pstNode = NULL, *pstCur = NULL, *pstTree = pstRoot;

    while(SELF_POS_UNUSE != pstTree->m_lSePos && NODE_NULL != pstTree->m_lSePos)
    {
        pstNode = pstTree;
        nRet = memcmp(pstTree->m_szIdx, psvIdx, lIdx);
        if(0 < nRet)
        {
            if(NODE_NULL == pstTree->m_lLeft)
                break;
            pstTree = (SHTree *)pGetNode(pvAddr, pstTree->m_lLeft);
        }
        else if(0 > nRet)
        {
            if(NODE_NULL == pstTree->m_lRight)
                break;
            pstTree = (SHTree *)pGetNode(pvAddr, pstTree->m_lRight);
        }
        else
        {
            _pRebuildGroup(pvAddr, pstSavm->tblName, pstTree, 0, lRow, lData);
            return pstRoot;
        }
    }

    lOffset = sizeof(SHTree) * ((TblDef *)pvAddr)->m_lGroup + lGetGrpPos(pstSavm->tblName);
    if(NULL == (pstCur = _pRebuildGroup(pvAddr, pstSavm->tblName, NULL, lOffset, lRow, lData)))
    {
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return pstRoot;
    }

    pstCur->m_lIdx = lIdx;
    pstCur->m_lSePos = lOffset;
    pstCur->m_lParent = NODE_NULL;
    memcpy(pstCur->m_szIdx, psvIdx, pstCur->m_lIdx);
    pstCur->m_lLeft = pstCur->m_lRight = NODE_NULL;
    ((TblDef *)pvAddr)->m_lGroup ++;

    if(pstNode != NULL)
    {
        pstCur->m_eColor = COLOR_RED;
        pstCur->m_lParent = pstNode->m_lSePos;
        if(0 < memcmp(pstNode->m_szIdx, pstCur->m_szIdx, pstCur->m_lIdx))
            pstNode->m_lLeft = pstCur->m_lSePos;
        else
            pstNode->m_lRight = pstCur->m_lSePos;
    }
    else
    {
        pstRoot = pstCur;
        pstCur->m_eColor = COLOR_BLK;
        return pstRoot; 
    }

    return _pFixupInsert(pvAddr, pstRoot, pstCur);
}

/*************************************************************************************************
    description：API - truncate (without transactions)
    parameters:
        pstSavm                    --stvm handle
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTruncate(SATvm *pstSavm, TABLE t)
{
    void    *pvData = NULL;
    RunTime *pstRun = NULL;
    RWLock  *prwLock = NULL;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    if(NULL == (pstRun = (RunTime *)pInitHitTest(pstSavm, t)))
        return RC_FAIL;

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        Tremohold(pstSavm, pstRun);
        return _lTruncateByRt(pstSavm, t);
    }

    prwLock = (RWLock *)pGetRWLock(pstRun->m_pvAddr);
    if(RC_SUCC != pthread_rwlock_wrlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DOWR_ERR;
        vTblDisconnect(pstSavm, pstSavm->tblName);
        return RC_FAIL;
    }

    if(RC_SUCC != lInitailTree(pstSavm, (void *)pGetNode(pstRun->m_pvAddr, lGetIdxPos(t)), t))
    {
        pthread_rwlock_unlock(prwLock);
        vTblDisconnect(pstSavm, t);
        return RC_FAIL;
    }

    if(RC_SUCC != lInitailGroup(pstSavm, (void *)pGetNode(pstRun->m_pvAddr, lGetGrpPos(t)), t))
    {
        pthread_rwlock_unlock(prwLock);
        vTblDisconnect(pstSavm, t);
        return RC_FAIL;
    }

    pstSavm->m_lEffect = ((TblDef *)pstRun->m_pvAddr)->m_lValid;
    pvData = (void *)pGetNode(pstRun->m_pvAddr, lGetTblData(t));
    memset(pvData, 0, lGetTableSize(t) - lGetTblData(t));
    ((TblDef *)pstRun->m_pvAddr)->m_lGroup = 0;
    ((TblDef *)pstRun->m_pvAddr)->m_lValid = 0;

    pthread_rwlock_unlock(prwLock);
    vTblDisconnect(pstSavm, t);

    return RC_SUCC;
}

/*************************************************************************************************
    description：Index reconstruction based on data area data
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lRebuildIndex(SATvm *pstSavm, void *pvAddr)
{
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN];
    TABLE   t = pstSavm->tblName;
    SHTree  *pstIRoot = NULL, *pstGRoot = NULL;
    size_t  lRow = 0, lOffset = lGetTblData(t), lIdx;

    if(RC_SUCC != lInitailTree(pstSavm, (void *)pGetNode(pvAddr, lGetIdxPos(t)), t))
        return RC_FAIL;

    if(RC_SUCC != lInitailGroup(pstSavm, (void *)pGetNode(pvAddr, lGetGrpPos(t)), t))
        return RC_FAIL;

    ((TblDef *)pvAddr)->m_lGroup = 0;
    ((TblDef *)pvAddr)->m_lGroupRoot = ((TblDef *)pvAddr)->m_lGroupPos;
    pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset);
    for(lRow = 0; (lRow < ((TblDef *)pvAddr)->m_lValid) && (lOffset < lGetTableSize(t));
        lOffset += lGetRowTruck(t), pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset))
    {
        if(IS_TRUCK_NULL(pstTruck))
            continue;

        if(HAVE_UNIQ_IDX(t))
        {
            memset(szIdx, 0, sizeof(szIdx));
            if(NULL == pPickIndex(lGetIdxNum(t), pGetTblIdx(t), pstTruck->m_pvData, szIdx))
            {
                pstSavm->m_lErrno = SVR_EXCEPTION;
                return RC_FAIL;
            }

            pstIRoot = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lTreeRoot);
            pstIRoot = pRebuildTree(pstSavm, pstIRoot, szIdx, lGetIdxLen(t), lRow, lOffset);
            if(!pstIRoot)    return RC_FAIL;

            ((TblDef *)pvAddr)->m_lTreeRoot = pstIRoot->m_lSePos;
        }

        if(HAVE_NORL_IDX(t))
        {
            memset(szIdx, 0, sizeof(szIdx));
            if(NULL == pPickIndex(lGetGrpNum(t), pGetTblGrp(t), pstTruck->m_pvData, szIdx))
            {
                pstSavm->m_lErrno = SVR_EXCEPTION;
                return RC_FAIL;
            }

            pstGRoot = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lGroupRoot);
            pstGRoot = pRebuildGroup(pstSavm, pstGRoot, szIdx, lGetGrpLen(t), lRow, lOffset);
            if(!pstGRoot)    return RC_FAIL;

            ((TblDef *)pvAddr)->m_lGroupRoot = pstGRoot->m_lSePos;
        }
        else if(HAVE_HASH_IDX(t))
        {
            memset(szIdx, 0, sizeof(szIdx));
            if(NULL == pPickIndex(lGetGrpNum(t), pGetTblGrp(t), pstTruck->m_pvData, szIdx))
            {
                pstSavm->m_lErrno = SVR_EXCEPTION;
                return RC_FAIL;
            }

            lIdx = uGetHash(szIdx, lGetGrpLen(t)) % ((TblDef *)pvAddr)->m_lMaxRow;
            pstGRoot = pvAddr + ((TblDef *)pvAddr)->m_lGroupPos + lIdx * sizeof(SHTree);
            if(NULL == pstGRoot)    return RC_FAIL;

            if(RC_SUCC != lRebuildHash(pstSavm, t, pvAddr, pstGRoot, lRow, lOffset))
                return RC_FAIL;
        }

        lRow ++;
    }

    ((TblDef *)pvAddr)->m_lValid = lRow;

    return RC_SUCC;
}

/*************************************************************************************************
    description：The index rebuild
    parameters:
        pstSavm                    --stvm handle
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lRebuildIndex(SATvm *pstSavm, TABLE t)
{
    long    lRet;
    RunTime *pstRun = NULL;
    RWLock  *prwLock = NULL;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    if(RC_SUCC != lInitSATvm(pstSavm, t))
        return RC_FAIL;

    if(NULL == (pstRun = (RunTime *)pInitHitTest(pstSavm, pstSavm->tblName)))
        return RC_FAIL;

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        pstSavm->m_lErrno = RMT_NOT_SUPPT;
        return RC_FAIL;
    }

    if(!HAVE_INDEX(pstSavm->tblName))
    {
        vTblDisconnect(pstSavm, pstSavm->tblName);
        return RC_SUCC;
    }

    prwLock = (RWLock *)pGetRWLock(pstRun->m_pvAddr);
    if(RC_SUCC != pthread_rwlock_wrlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DOWR_ERR;
        vTblDisconnect(pstSavm, pstSavm->tblName);
        return RC_FAIL;
    }

    pstSavm->lSize = lGetRowSize(t);
    lRet = _lRebuildIndex(pstSavm, pstRun->m_pvAddr);
    pthread_rwlock_unlock(prwLock);
    vTblDisconnect(pstSavm, pstSavm->tblName);

    return lRet;
}

/*************************************************************************************************
    description：Select the table data according to the hash
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        psvOut                     --result data
        plData                     --offset 
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lSelectHash(SATvm *pstSavm, void *pvAddr, TABLE t, void *psvOut, size_t *plData)
{
    size_t  lData, lIdx;
    SHTree  *pstTree = NULL;
    SHList  *pstList = NULL;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN], *pvData = NULL;
    RWLock  *prwLock = (RWLock *)pGetRWLock(pvAddr);

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetGrpNum(t), pGetTblGrp(t), pstSavm->pstVoid, szIdx))
        return RC_CONTU;

    lIdx = uGetHash(szIdx, lGetGrpLen(t)) % ((TblDef *)pvAddr)->m_lMaxRow;
    pstTree = pvAddr + ((TblDef *)pvAddr)->m_lGroupRoot + lIdx * sizeof(SHTree);
    if(NULL == pstTree || SELF_POS_UNUSE == pstTree->m_lData)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    if(RC_SUCC != pthread_rwlock_rdlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DORD_ERR;
        return RC_FAIL;
    }

    pstList = (SHList *)pGetNode(pvAddr, pstTree->m_lData);
    for(pstSavm->m_lEType = EXE_PLAN_GRP; SELF_POS_UNUSE != pstList->m_lPos; 
        pstList = (SHList *)pGetNode(pvAddr, pstList->m_lNext))
    {
        pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
        if(RC_MATCH == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
        {
            if(1 < (++ pstSavm->m_lEffect))
            {
                pthread_rwlock_unlock(prwLock);
                pstSavm->m_lErrno = MORE_ROWS_SEL;
                return RC_FAIL;
            }

            pstTruck->m_lTimes ++;
            *plData = pstList->m_lData;
            pvData  = pstTruck->m_pvData;
            if(FIRST_ROW & pstSavm->lFind)    break;
        }

        if(SELF_POS_UNUSE == pstList->m_lNext)   break;
    }

    if(0 == pstSavm->m_lEffect)
    {
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    memcpy(psvOut, pvData, lGetRowSize(pstSavm->tblName));
    pthread_rwlock_unlock(prwLock);
    return RC_SUCC;
}

/*************************************************************************************************
    description：Select the table data according to the index
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        psvOut                     --result data
        plData                     --offset 
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lSelectGroup(SATvm *pstSavm, void *pvAddr, TABLE t, void *psvOut, size_t *plData)
{
    void    *pvData = NULL;
    SHList  *pstList = NULL;
    SHTree  *pstTree = NULL;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN];
    RWLock  *prwLock = (RWLock *)pGetRWLock(pvAddr);

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetGrpNum(t), pGetTblGrp(t), pstSavm->pstVoid, szIdx))
        return RC_CONTU;

    if(RC_SUCC != pthread_rwlock_rdlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DORD_ERR;
        return RC_FAIL;
    }

    if(NULL == (pstTree = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lGroupRoot)))
    {   
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    if(NULL == (pstList = pSearchGroup(pvAddr, pstTree, szIdx, lGetGrpLen(t))))
    {
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_CONTU;
    }

    for(pstSavm->m_lEType = EXE_PLAN_GRP; SELF_POS_UNUSE != pstList->m_lPos; 
        pstList = (SHList *)pGetNode(pvAddr, pstList->m_lNext))
    {
        pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
        if(RC_MATCH == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
        {
            if(1 < (++ pstSavm->m_lEffect))
            {
                pthread_rwlock_unlock(prwLock);
                pstSavm->m_lErrno = MORE_ROWS_SEL;
                return RC_FAIL;
            }

            pstTruck->m_lTimes ++;
            *plData = pstList->m_lData;
            pvData = pstTruck->m_pvData;
            if(FIRST_ROW & pstSavm->lFind)    break;
        }

        if(SELF_POS_UNUSE == pstList->m_lNext)   break;
    }

    if(0 == pstSavm->m_lEffect)
    {
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    memcpy(psvOut, pvData, lGetRowSize(pstSavm->tblName));
    pthread_rwlock_unlock(prwLock);

    return RC_SUCC;
}

/*************************************************************************************************
    description：Select the table data according to the truck
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        psvOut                     --result data
        plData                     --offset 
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lSelectTruck(SATvm *pstSavm, void *pvAddr, TABLE t, void *psvOut, size_t *plData)
{
    void    *pvData = NULL;
    SHTruck *pstTruck = NULL;
    size_t  lRow = 0, lOffset = lGetTblData(t);
    RWLock  *prwLock = (RWLock *)pGetRWLock(pvAddr);

    if(RC_SUCC != pthread_rwlock_rdlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DORD_ERR;
        return RC_FAIL;
    }

    pstSavm->m_lEType = EXE_PLAN_ALL;
    pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset);
    for(lRow = 0; (lRow < ((TblDef *)pvAddr)->m_lValid) && (lOffset < lGetTableSize(t)); 
        pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset))
    {        
        if(IS_TRUCK_NULL(pstTruck))
        {
            lOffset += lGetRowTruck(t);
            continue;
        }

        lRow ++;
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
        {
            lOffset += lGetRowTruck(t);
            continue;
        }
        
        if(1 < (++ pstSavm->m_lEffect))
        {
            pthread_rwlock_unlock(prwLock);
            pstSavm->m_lErrno = MORE_ROWS_SEL;
            return RC_FAIL;
        }

        *plData = lOffset;
        lOffset+= lGetRowTruck(t);
        pstTruck->m_lTimes ++;
        pvData  = pstTruck->m_pvData;
        if(FIRST_ROW & pstSavm->lFind)   break;
    }

    if(0 == pstSavm->m_lEffect)
    {
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    memcpy(psvOut, pvData, lGetRowSize(pstSavm->tblName));
    pthread_rwlock_unlock(prwLock);

    return RC_SUCC;
}

/*************************************************************************************************
    description：Select the table data according to the Unique index
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        psvOut                     --result data
        plData                     --offset 
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lSelectIndex(SATvm *pstSavm, void *pvAddr, TABLE t, void *psvOut, size_t *plData)
{
    SHTree  *pstTree = NULL;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN];
    RWLock  *prwLock = (RWLock *)pGetRWLock(pvAddr);

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetIdxNum(t), pGetTblIdx(t), pstSavm->pstVoid, szIdx))
        return RC_CONTU;

    if(RC_SUCC != pthread_rwlock_rdlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DORD_ERR;
        return RC_FAIL;
    }

    pstSavm->m_lEType = EXE_PLAN_IDX;
    if(NULL == (pstTree = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lTreeRoot)))
    {   
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    pstTree = (SHTree *)pSearchTree(pvAddr, pstTree, szIdx, lGetIdxLen(pstSavm->tblName));
    if(!pstTree)
    {
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    pstTruck = (PSHTruck)pGetNode(pvAddr, pstTree->m_lData);
    if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
    {
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    memcpy(psvOut, pstTruck->m_pvData, lGetRowSize(pstSavm->tblName));
    pthread_rwlock_unlock(prwLock);
    pstTruck->m_lTimes ++;
    pstSavm->m_lEffect = 1;
    *plData = pstTree->m_lData;

    return RC_SUCC;
}

/*************************************************************************************************
    description：Group the table data according to the index
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        plData                     --offset 
        ppsvOut                    --result data list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lGroupIndex(SATvm *pstSavm, void *pvAddr, TABLE t, size_t *plOut, void **ppsvOut)
{
    SHTree  *pstTree = NULL;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN];

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetIdxNum(t), pGetTblIdx(t), pstSavm->pstVoid, szIdx))
        return RC_CONTU;

    if(NULL == (pstTree = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lTreeRoot)))
    {   
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    pstTree = (SHTree *)pSearchTree(pvAddr, pstTree, szIdx, lGetIdxLen(pstSavm->tblName));
    if(!pstTree)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    pstTruck = (PSHTruck)pGetNode(pvAddr, pstTree->m_lData);
    if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    *plOut = 0;
    return lInsertLstgrp(pstSavm, &pstSavm->stUpdt, pstTruck->m_pvData, t, plOut, ppsvOut);
}

/*************************************************************************************************
    description：Group the group tree
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        pstree                     --tree node
        plData                     --offset 
        ppsvOut                    --result data list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lGroupTree(SATvm *pstSavm, void *pvAddr, SHTree *pstTree, FdCond *pstRrp, size_t *plOut, 
            void **ppsvOut)
{
    size_t  lCount;
    SHList  *pstList = NULL;
    SHTruck *pstTruck = NULL;

    if(SELF_POS_UNUSE == pstTree->m_lSePos || NODE_NULL == pstTree->m_lSePos)
        return RC_SUCC;

    pstList  = (SHList *)pGetNode(pvAddr, pstTree->m_lData);    
    pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
    lCount = lGetListCount(pvAddr, pstList);

    if(!bRepeatLstgrp(pstRrp, pstTruck->m_pvData, pstSavm->tblName, *plOut, *ppsvOut))
    {
        if(RC_SUCC != lInsertLstgrp(pstSavm, &pstSavm->stUpdt, pstTruck->m_pvData, 
            pstSavm->tblName, plOut, ppsvOut))
            return RC_FAIL;
    }

    if(RC_SUCC != _lGroupTree(pstSavm, pvAddr, (SHTree *)pGetNode(pvAddr, pstTree->m_lLeft), 
        pstRrp, plOut, ppsvOut))
        return RC_FAIL;

    return _lGroupTree(pstSavm, pvAddr, (SHTree *)pGetNode(pvAddr, pstTree->m_lRight), pstRrp, 
        plOut, ppsvOut);
}

/*************************************************************************************************
    description：Group the table data according to the group tree
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        plData                     --offset 
        ppsvOut                    --result data list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lGroupGroup(SATvm *pstSavm, void *pvAddr, TABLE t, size_t *plOut, void **ppsvOut)
{
    SHList  *pstList = NULL;
    SHTree  *pstTree = NULL;
    SHTruck *pstTruck = NULL;
    FdCond  *pstRrp = &pstSavm->stUpdt;
    char    *pszIdx, szIdx[MAX_INDEX_LEN];

    if(NULL == (pstTree = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lGroupRoot)))
    {
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    if(bIsGroupFld(t, pstRrp) && NULL == pstSavm->pstVoid)
        return _lGroupTree(pstSavm, pvAddr, pstTree, pstRrp, plOut, ppsvOut);

    memset(szIdx, 0, sizeof(szIdx));
    pszIdx = pGetIndex(&pstSavm->stCond, lGetGrpNum(t), pGetTblGrp(t), pstSavm->pstVoid, szIdx);
    if(!pszIdx)    return RC_CONTU;

    if(NULL == (pstList = pSearchGroup(pvAddr, pstTree, szIdx, lGetGrpLen(t))))
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_CONTU;
    }

    for(*plOut = 0; SELF_POS_UNUSE != pstList->m_lPos; 
        pstList = (SHList *)pGetNode(pvAddr, pstList->m_lNext))
    {
        pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
        {
            if(SELF_POS_UNUSE == pstList->m_lNext)    break;
            continue;
        }

        if(bRepeatLstgrp(pstRrp, pstTruck->m_pvData, t, *plOut, *ppsvOut))
        {
            if(SELF_POS_UNUSE == pstList->m_lNext)    break;
            continue;
        }

        if(RC_SUCC != lInsertLstgrp(pstSavm, &pstSavm->stUpdt, pstTruck->m_pvData, t, 
            plOut, ppsvOut))
            return RC_FAIL;

        pstSavm->m_lEffect ++;
        if(SELF_POS_UNUSE == pstList->m_lNext)    break;
    }

    if(0 == pstSavm->m_lEffect)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：Group the table data according to the hash
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        plData                     --offset 
        ppsvOut                    --result data list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lGroupHash(SATvm *pstSavm, void *pvAddr, TABLE t, size_t *plOut, void **ppsvOut)
{
    size_t  lIdx;
    SHTree  *pstTree = NULL;
    SHList  *pstList = NULL;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN];
    FdCond  *pstRrp = &pstSavm->stUpdt;

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetGrpNum(t), pGetTblGrp(t), pstSavm->pstVoid, szIdx))
        return RC_CONTU;

    lIdx = uGetHash(szIdx, lGetGrpLen(t)) % ((TblDef *)pvAddr)->m_lMaxRow;
    pstTree = pvAddr + ((TblDef *)pvAddr)->m_lGroupRoot + lIdx * sizeof(SHTree);
    if(NULL == pstTree || SELF_POS_UNUSE == pstTree->m_lData)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    for(*plOut = 0, pstList = (SHList *)pGetNode(pvAddr, pstTree->m_lData); 
        SELF_POS_UNUSE != pstList->m_lPos; pstList = (SHList *)pGetNode(pvAddr, pstList->m_lNext))
    {
        pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
        {
            if(SELF_POS_UNUSE == pstList->m_lNext)    break;
            continue;
        }

        if(bRepeatLstgrp(pstRrp, pstTruck->m_pvData, t, *plOut, *ppsvOut))
        {
            if(SELF_POS_UNUSE == pstList->m_lNext)    break;
            continue;
        }

        if(RC_SUCC != lInsertLstgrp(pstSavm, &pstSavm->stUpdt, pstTruck->m_pvData, t, 
            plOut, ppsvOut))
            return RC_FAIL;

        pstSavm->m_lEffect ++;
        if(SELF_POS_UNUSE == pstList->m_lNext)    break;
    }

    if(0 == pstSavm->m_lEffect)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：Group the table data according to the truck
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        plData                     --offset 
        ppsvOut                    --result data list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lGroupTruck(SATvm *pstSavm, void *pvAddr, TABLE t, size_t *plOut, void **ppsvOut)
{
    SHTruck *pstTruck = NULL;
    FdCond  *pstRrp = &pstSavm->stUpdt;
    size_t  lRow = 0, lOffset = lGetTblData(t);

    pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset);
    for(*plOut = 0; (lRow < ((TblDef *)pvAddr)->m_lValid) && (lOffset < lGetTableSize(t)); 
        pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset))
    {        
        if(IS_TRUCK_NULL(pstTruck))
        {
            lOffset += lGetRowTruck(t);
            continue;
        }

        lRow ++;
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
        {
            lOffset += lGetRowTruck(t);
            continue;
        }

        if(bRepeatLstgrp(pstRrp, pstTruck->m_pvData, t, *plOut, *ppsvOut))
        {
            lOffset += lGetRowTruck(t);
            continue;
        }

        if(RC_FAIL == lInsertLstgrp(pstSavm, pstRrp, pstTruck->m_pvData, t, plOut, ppsvOut))
            return RC_FAIL;

        lOffset += lGetRowTruck(t);
    }

    if(0 == (pstSavm->m_lEffect = *plOut))
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：Group the table data according to the index
    parameters:
        pstSavm                    --stvm handle
        t                          --table
        plData                     --offset 
        ppsvOut                    --result data list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lGroup(SATvm *pstSavm, RunTime *pstRun, size_t *plOut, void **ppsvOut)
{
    long    lRet;
    RWLock  *prwLock = (RWLock *)pGetRWLock(pstRun->m_pvAddr);

    if(RC_SUCC != pthread_rwlock_rdlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DORD_ERR;
        return RC_FAIL;
    }

    if(HAVE_UNIQ_IDX(pstSavm->tblName))
    {
        lRet = _lGroupIndex(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, plOut, ppsvOut);
        if(RC_CONTU != lRet) 
        {
            pthread_rwlock_unlock(prwLock);
            pstSavm->m_lEType = EXE_PLAN_IDX;
            return lRet;
        }
    }

    if(HAVE_NORL_IDX(pstSavm->tblName))
    {
        lRet = _lGroupGroup(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, plOut, ppsvOut);
        if(RC_CONTU != lRet)
        {
            pthread_rwlock_unlock(prwLock);
            pstSavm->m_lEType = EXE_PLAN_GRP;
            return lRet;
        }
    }
    else if(HAVE_HASH_IDX(pstSavm->tblName))
    {
        lRet = _lGroupHash(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, plOut, ppsvOut);
        if(RC_CONTU != lRet)
        {
            pthread_rwlock_unlock(prwLock);
            pstSavm->m_lEType = EXE_PLAN_GRP;
            return lRet;
        }
    }

    pstSavm->m_lEType = EXE_PLAN_ALL;
    lRet = _lGroupTruck(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, plOut, ppsvOut);
    pthread_rwlock_unlock(prwLock);
    return lRet;
}

/*************************************************************************************************
    description：Group the table data according to the index
    parameters:
        pstSavm                    --stvm handle
        plOut                      --number 
        ppsvOut                    --result data list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lGroup(SATvm *pstSavm, size_t *plOut, void **ppsvOut)
{
    RunTime *pstRun = NULL;

    if(!pstSavm) 
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    if(NULL == (pstRun = (RunTime *)pInitMemTable(pstSavm, pstSavm->tblName)))
        return RC_FAIL;

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        Tremohold(pstSavm, pstRun);
        return _lGroupByRt(pstSavm, plOut, ppsvOut);
    }

    if(RC_FAIL == _lGroup(pstSavm, pstRun, plOut, ppsvOut))
    {
        vTblDisconnect(pstSavm, pstSavm->tblName);
        return RC_FAIL;
    }

    vTblDisconnect(pstSavm, pstSavm->tblName);
    return lSortRowList(pstSavm, *plOut, *ppsvOut, lGetRowSize(pstSavm->tblName));
}

/*************************************************************************************************
    description：Query the table data according to the Unique index
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        plData                     --offset 
        ppsvOut                    --result data list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lQueryIndex(SATvm *pstSavm, void *pvAddr, TABLE t, size_t *plOut, void **ppsvOut)
{
    SHTree  *pstTree = NULL;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN];

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetIdxNum(t), pGetTblIdx(t), pstSavm->pstVoid, szIdx))
        return RC_CONTU;

    if(NULL == (pstTree = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lTreeRoot)))
    {   
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    pstTree = (SHTree *)pSearchTree(pvAddr, pstTree, szIdx, lGetIdxLen(pstSavm->tblName));
    if(!pstTree)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    pstTruck = (PSHTruck)pGetNode(pvAddr, pstTree->m_lData);
    if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    if(NULL == (*ppsvOut = (char *)realloc(*ppsvOut, lGetRowSize(pstSavm->tblName))))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    pstTruck->m_lTimes ++;
    *plOut = pstSavm->m_lEffect = 1;
    memcpy(*ppsvOut, pstTruck->m_pvData, lGetRowSize(pstSavm->tblName));

    return RC_SUCC;
}

/*************************************************************************************************
    description：Query the table data according to the hash
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        plOut                      --number
        ppsvOut                    --result data list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lQueryHash(SATvm *pstSavm, void *pvAddr, TABLE t, size_t *plOut, void **ppsvOut)
{
    SHTree  *pstTree = NULL;
    SHList  *pstList = NULL;
    SHTruck *pstTruck = NULL;
    size_t  lData, lIdx, lPos;
    char    szIdx[MAX_INDEX_LEN];

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetGrpNum(t), pGetTblGrp(t), pstSavm->pstVoid, szIdx))
        return RC_CONTU;

    lIdx = uGetHash(szIdx, lGetGrpLen(t)) % ((TblDef *)pvAddr)->m_lMaxRow;
    pstTree = pvAddr + ((TblDef *)pvAddr)->m_lGroupRoot + lIdx * sizeof(SHTree);
    if(NULL == pstTree || SELF_POS_UNUSE == pstTree->m_lData)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    pstList = (SHList *)pGetNode(pvAddr, pstTree->m_lData);
    for(*plOut = 0, lPos = 0; SELF_POS_UNUSE != pstList->m_lPos; 
        pstList = (SHList *)pGetNode(pvAddr, pstList->m_lNext))
    {
        pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
        {
            if(SELF_POS_UNUSE == pstList->m_lNext)    break;
            continue;
        }

        lPos = (++ (*plOut)) * lGetRowSize(t);
        if(NULL == (*ppsvOut = (char *)realloc(*ppsvOut, lPos)))
        {
            pstSavm->m_lErrno = MALLC_MEM_ERR;
            return RC_FAIL;
        }

        pstTruck->m_lTimes ++;
        memcpy(*ppsvOut + (lPos - lGetRowSize(t)), pstTruck->m_pvData, lGetRowSize(t));
        if(SELF_POS_UNUSE == pstList->m_lNext)    break;
    }

    if(0 == (pstSavm->m_lEffect = *plOut))
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：Query the table data according to the group
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        plOut                      --number
        ppsvOut                    --result data list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lQueryGroup(SATvm *pstSavm, void *pvAddr, TABLE t, size_t *plOut, void **ppsvOut)
{
    size_t  lPos;
    TblKey  *pstIdx = NULL;
    SHList  *pstList = NULL;
    SHTree  *pstTree = NULL;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN];

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetGrpNum(t), pGetTblGrp(t), pstSavm->pstVoid, szIdx))
        return RC_CONTU;

    if(NULL == (pstTree = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lGroupRoot)))
    {   
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    if(NULL == (pstList = pSearchGroup(pvAddr, pstTree, szIdx, lGetGrpLen(t))))
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_CONTU;
    }

    for(*plOut = 0, lPos = 0; SELF_POS_UNUSE != pstList->m_lPos; 
        pstList = (SHList *)pGetNode(pvAddr, pstList->m_lNext))
    {
        pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
        {
            if(SELF_POS_UNUSE == pstList->m_lNext)    break;
            continue;
        }    
    
        lPos = (++ (*plOut)) * lGetRowSize(t);
        if(NULL == (*ppsvOut = (char *)realloc(*ppsvOut, lPos)))
        {
            pstSavm->m_lErrno = MALLC_MEM_ERR;
            return RC_FAIL;
        }
    
        pstTruck->m_lTimes ++;
        memcpy(*ppsvOut + (lPos - lGetRowSize(t)), pstTruck->m_pvData, lGetRowSize(t));
        if(SELF_POS_UNUSE == pstList->m_lNext)    break;
    }

    if(0 == (pstSavm->m_lEffect = *plOut))
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：Query the table data according to the truck
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        plOut                      --number
        ppsvOut                    --result data list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lQueryTruck(SATvm *pstSavm, void *pvAddr, TABLE t, size_t *plOut, void **ppsvOut)
{
    SHTruck *pstTruck = NULL;
    size_t  lRow = 0, lOffset = lGetTblData(t), lPos = 0;

    pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset);
    for(lRow = 0, *plOut = 0; (lRow < ((TblDef *)pvAddr)->m_lValid) && (lOffset < lGetTableSize(t)); 
        pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset))
    {        
        if(IS_TRUCK_NULL(pstTruck))
        {
            lOffset += lGetRowTruck(t);
            continue;
        }

        lRow ++;
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
        {
            lOffset += lGetRowTruck(t);
            continue;
        }
        
        lPos = (++ (*plOut)) * lGetRowSize(t);
        if(NULL == (*ppsvOut = (char *)realloc(*ppsvOut, lPos)))
        {
            pstSavm->m_lErrno = MALLC_MEM_ERR;
            return RC_FAIL;
        }

        pstTruck->m_lTimes ++;
        memcpy(*ppsvOut + (lPos - lGetRowSize(t)), pstTruck->m_pvData, lGetRowSize(t));
        lOffset += lGetRowTruck(t);
    }

    if(0 == (pstSavm->m_lEffect = *plOut))
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：Query the table data 
    parameters:
        pstSavm                    --stvm handle
        pstRun                     --table handle
        plOut                      --number
        ppsvOut                    --result data list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lQuery(SATvm *pstSavm, RunTime *pstRun, size_t *plOut, void **ppsvOut)
{
    long    lRet;
    RWLock  *prwLock = (RWLock *)pGetRWLock(pstRun->m_pvAddr);

    if(RC_SUCC != pthread_rwlock_rdlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DORD_ERR;
        return RC_FAIL;
    }

    if(HAVE_UNIQ_IDX(pstSavm->tblName))
    {
        lRet = _lQueryIndex(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, plOut, ppsvOut);
        if(RC_CONTU != lRet) 
        {
            pthread_rwlock_unlock(prwLock);
            pstSavm->m_lEType = EXE_PLAN_IDX;
            return lRet;
        }
    }

    if(HAVE_NORL_IDX(pstSavm->tblName))
    {
        lRet = _lQueryGroup(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, plOut, ppsvOut);
        if(RC_CONTU != lRet)
        {
            pthread_rwlock_unlock(prwLock);
            pstSavm->m_lEType = EXE_PLAN_GRP;
            return lRet;
        }
    }
    else if(HAVE_HASH_IDX(pstSavm->tblName))
    {
        lRet = _lQueryHash(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, plOut, ppsvOut);
        if(RC_CONTU != lRet)
        {
            pthread_rwlock_unlock(prwLock);
            pstSavm->m_lEType = EXE_PLAN_GRP;
            return lRet;
        }
    }

    pstSavm->m_lEType = EXE_PLAN_ALL;
    lRet = _lQueryTruck(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, plOut, ppsvOut);
    pthread_rwlock_unlock(prwLock);

    return lRet;
}

/*************************************************************************************************
    description：API - Query 
    parameters:
        pstSavm                    --stvm handle
        plOut                      --number
        ppsvOut                    --result data list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lQuery(SATvm *pstSavm, size_t *plOut, void **ppsvOut)
{
    long    lRet;
    RunTime *pstRun = NULL;
    TblKey  *pstIdx = NULL;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    if(NULL == (pstRun = (RunTime *)pInitMemTable(pstSavm, pstSavm->tblName)))
        return RC_FAIL;

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        Tremohold(pstSavm, pstRun);
        return _lQueryByRt(pstSavm, plOut, ppsvOut);
    }

    if(RC_SUCC != _lQuery(pstSavm, pstRun, plOut, ppsvOut))
    {
        vTblDisconnect(pstSavm, pstSavm->tblName);
        return RC_FAIL;
    }

    vTblDisconnect(pstSavm, pstSavm->tblName);
    return lSortRowList(pstSavm, *plOut, *ppsvOut, lGetRowSize(pstSavm->tblName));
}

/*************************************************************************************************
    description：Select the table data
    parameters:
        pstSavm                    --stvm handle
        pstRun                     --table handle
        t                          --table
        ppsvOut                    --result data list
        plData                     --Offset
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lSelect(SATvm *pstSavm, RunTime *pstRun, TABLE t, void *psvOut, size_t *plData)
{
    long    lRet;

    if(HAVE_UNIQ_IDX(pstSavm->tblName))
    {
        lRet = _lSelectIndex(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, psvOut, plData);
        if(RC_CONTU != lRet)    return lRet;
    }

    if(HAVE_NORL_IDX(pstSavm->tblName))
    {
        lRet = _lSelectGroup(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, psvOut, plData);
        if(RC_CONTU != lRet)    return lRet;
    }
    else if(HAVE_HASH_IDX(t))
    {
        lRet = _lSelectHash(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, psvOut, plData);
        if(RC_CONTU != lRet)    return lRet;
    }

    return _lSelectTruck(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, psvOut, plData);
}

/*************************************************************************************************
    description：API - Select
    parameters:
        pstSavm                    --stvm handle
        ppsvOut                    --result data list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lSelect(SATvm *pstSavm, void *psvOut)
{
    long    lRet;
    size_t  lData = 0;
    RunTime *pstRun  = NULL;

    if(!pstSavm || !pstSavm->pstVoid)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    if(NULL == (pstRun = (RunTime *)pInitMemTable(pstSavm, pstSavm->tblName)))
        return RC_FAIL;

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        Tremohold(pstSavm, pstRun);
        return _lSelectByRt(pstSavm, psvOut);
    }

    lRet = _lSelect(pstSavm, pstRun, pstSavm->tblName, psvOut, &lData);
    vTblDisconnect(pstSavm, pstSavm->tblName);
    return lRet;
}

/*************************************************************************************************
    description：Add a unique index
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        ppstTruck                  --data truck
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lInsertIndex(SATvm *pstSavm, void *pvAddr, TABLE t, SHTruck **ppstTruck)
{
    SHTree  *pstRoot = NULL;
    char    szIdx[MAX_INDEX_LEN];

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == (pstRoot = pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lTreeRoot)))
    {   
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    if(NULL == pPickIndex(lGetIdxNum(t), pGetTblIdx(t), pstSavm->pstVoid, szIdx))
    {
        pstSavm->m_lErrno = IDX_FIELD_NIL;
        return RC_FAIL;
    }

    pstRoot = pInsertTree(pstSavm, pstRoot, szIdx, lGetIdxLen(t), ppstTruck);
    if(!pstRoot)    return RC_FAIL;

    ((TblDef *)pvAddr)->m_lTreeRoot = pstRoot->m_lSePos;

    return RC_SUCC;
}

/*************************************************************************************************
    description：Add a hash index
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        ppstTruck                  --data truck
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lInsertHash(SATvm *pstSavm, void *pvAddr, TABLE t, SHTruck **ppstTruck)
{
    size_t  lIdx, lOffset;
    SHTree  *pstTree = NULL;
    char    szIdx[MAX_INDEX_LEN];

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pPickIndex(lGetGrpNum(t), pGetTblGrp(t), pstSavm->pstVoid, szIdx))
    {
        pstSavm->m_lErrno = IDX_FIELD_NIL;
        return RC_FAIL;
    }

    lIdx = uGetHash(szIdx, lGetGrpLen(t)) % ((TblDef *)pvAddr)->m_lMaxRow;
    lOffset = ((TblDef *)pvAddr)->m_lGroupPos + lIdx * sizeof(SHTree);
    if(NULL == (pstTree = (SHTree *)(pvAddr + lOffset)))
    {
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    return __lInsertHash(pstSavm, pvAddr, t, pstTree, lOffset, ppstTruck);
}

/*************************************************************************************************
    description：Add a index
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        ppstTruck                  --data truck
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lInsertGroup(SATvm *pstSavm, void *pvAddr, TABLE t, SHTruck **ppstTruck)
{
    SHTree  *pstRoot = NULL;
    char    szIdx[MAX_INDEX_LEN];

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == (pstRoot = pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lGroupRoot)))
    {   
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    if(NULL == pPickIndex(lGetGrpNum(t), pGetTblGrp(t), pstSavm->pstVoid, szIdx))
    {
        pstSavm->m_lErrno = IDX_FIELD_NIL;
        return RC_FAIL;
    }

    pstRoot = pInsertGroup(pstSavm, pstRoot, szIdx, lGetGrpLen(t), ppstTruck);
    if(!pstRoot)    return RC_FAIL;

    ((TblDef *)pvAddr)->m_lGroupRoot = pstRoot->m_lSePos;    

    return RC_SUCC;
}

/*************************************************************************************************
    description：Add one data
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        ppstTruck                  --data truck
        uTimes                     --Click volume
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lInsertTruck(SATvm *pstSavm, void *pvAddr, TABLE t, SHTruck *pstTruck, ulong uTimes)
{
    SHTruck    *pvTruck = pstTruck;

    if(!pvTruck)    //    说明无索引
        pvTruck = pGetNode(pvAddr, lGetTblData(t) + lGetRowTruck(t) * ((TblDef *)pvAddr)->m_lValid);

    SET_DATA_TRUCK(pvTruck, DATA_TRUCK_NRML);
    pvTruck->m_lTimes = uTimes;
    memcpy(pvTruck->m_pvData, pstSavm->pstVoid, lGetRowSize(t));

    ((TblDef *)pvAddr)->m_lValid ++;
    pstSavm->m_lEffect = 1;

    return RC_SUCC;
}

/*************************************************************************************************
    description：increate field
    parameters:
        pstCond                    --decoreate condit
        pvData                     --memory address
        pe                         --Table struct define
    return:
 *************************************************************************************************/
void    vIncrease(FdCond *pstCond, char *pvData, TblDef *pe)
{
    register int    i = 0;
    FdKey           *pFdKey;

    if(0 == pstCond->uFldcmp)
        return ;

    for(i = 0; i < pstCond->uFldcmp; i ++)
    {
        pFdKey = &pstCond->stFdKey[i]; 
        if(!(pFdKey->uDecorate & FIELD_INCR))
            continue;

        if(pFdKey->uFldlen < sizeof(llong))
            continue;
   
        pe->m_lExSeQ ++;
        memcpy(pvData + pFdKey->uFldpos, &pe->m_lExSeQ, sizeof(llong));
    }
}

/*************************************************************************************************
    description：insert data to table
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        uTimes                     --Click volume
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    __lInsert(SATvm *pstSavm, void *pvAddr, TABLE t, ulong uTimes)
{
    SHTruck *pstTruck = NULL;

    if(lGetTblRow(pstSavm->tblName) <= ((TblDef *)pvAddr)->m_lValid)
    {
        pstSavm->m_lErrno = DATA_SPC_FULL;
        return RC_FAIL;
    }

    if(FIELD_INCR & pstSavm->lFind)
        vIncrease(&pstSavm->stUpdt, (char *)pstSavm->pstVoid, (TblDef *)pvAddr);

    if(HAVE_UNIQ_IDX(t))
    {
        if(RC_SUCC != _lInsertIndex(pstSavm, pvAddr, t, &pstTruck))
            return RC_FAIL;
    }

    if(HAVE_NORL_IDX(t))
    {
        if(RC_SUCC != _lInsertGroup(pstSavm, pvAddr, t, &pstTruck))
            return RC_FAIL;
    }
    else if(HAVE_HASH_IDX(t))
    {
        if(RC_SUCC != _lInsertHash(pstSavm, pvAddr, t, &pstTruck))
            return RC_FAIL;
    }

    return _lInsertTruck(pstSavm, pvAddr, t, pstTruck, uTimes);
}

/*************************************************************************************************
    description：Reset the table lock
    parameters:
        pstSavm                    --stvm handle
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lResetLock(SATvm *pstSavm, TABLE t)
{
    RWAttr  attr;
    RunTime *pstRun = NULL;
    RWLock  *prwLock = NULL;

    if(RC_SUCC != lInitSATvm(pstSavm, t))
        return RC_FAIL;

    if(NULL == (pstRun = (RunTime *)pInitHitTest(pstSavm, t)))
        return RC_FAIL;

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        pstSavm->m_lErrno = RMT_NOT_SUPPT;
        return RC_FAIL;
    }

    prwLock = (RWLock *)pGetRWLock(pstRun->m_pvAddr);
    pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_rwlock_init(prwLock, &attr);
    vTblDisconnect(pstSavm, t);

    return RC_SUCC;
}

/*************************************************************************************************
    description：API - Insert
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lInsert(SATvm *pstSavm)
{
    long    lRet;
    RunTime *pstRun = NULL;
    RWLock  *prwLock = NULL;

    if(!pstSavm || !pstSavm->pstVoid)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    if(NULL == (pstRun = (RunTime *)pInitMemTable(pstSavm, pstSavm->tblName)))
        return RC_FAIL;

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        Tremohold(pstSavm, pstRun);
        return _lInsertByRt(pstSavm);
    }

    prwLock = (RWLock *)pGetRWLock(pstRun->m_pvAddr);
    if(RC_SUCC != pthread_rwlock_wrlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DOWR_ERR;
        vTblDisconnect(pstSavm, pstSavm->tblName);
        return RC_FAIL;
    }

    lRet = __lInsert(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, 0);
    pthread_rwlock_unlock(prwLock);
    vTblDisconnect(pstSavm, pstSavm->tblName);
    if(RC_SUCC != lRet)    return RC_FAIL;

    return lRecordWork(pstSavm, pstSavm->pstVoid, OPERATE_INSERT);
}

/*************************************************************************************************
    description：Transaction Flow Add record
    parameters:
        pstSavm                    --stvm handle
        plOffset                   --memory address
        pllSeq                     --seqno
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lInsertTrans(SATvm *pstSavm, size_t *plOffset, llSEQ *pllSeq)
{
    size_t    lOffset;
    RunTime *pstRun = NULL;
    RWLock  *prwLock = NULL;
    SHTruck *pstTruck = NULL;

    if(!pstSavm || !pstSavm->pstVoid)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    if(NULL == (pstRun = (RunTime *)pInitMemTable(pstSavm, pstSavm->tblName)))
        return RC_FAIL;

    if(lGetTblRow(pstSavm->tblName) <= ((TblDef *)pstRun->m_pvAddr)->m_lValid)
    {
        pstSavm->m_lErrno = DATA_SPC_FULL;
        vTblDisconnect(pstSavm, pstSavm->tblName);
        return RC_FAIL;
    }

    prwLock = (RWLock *)pGetRWLock(pstRun->m_pvAddr);
    if(RC_SUCC != pthread_rwlock_wrlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DOWR_ERR;
        vTblDisconnect(pstSavm, pstSavm->tblName);
        return RC_FAIL;
    }

    if(HAVE_UNIQ_IDX(pstSavm->tblName))
    {
        if(RC_SUCC != _lInsertIndex(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, &pstTruck))
            goto ErrInsert;
    }

    if(HAVE_NORL_IDX(pstSavm->tblName))
    {
        if(RC_SUCC != _lInsertGroup(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, &pstTruck))
            goto ErrInsert;
    }
    else if(HAVE_HASH_IDX(pstSavm->tblName))
    {
        if(RC_SUCC != _lInsertHash(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, &pstTruck))
            goto ErrInsert;
    }

    if(!pstTruck)
    {
        lOffset = lGetTblData(pstSavm->tblName) + 
            lGetRowTruck(pstSavm->tblName) * ((TblDef *)pstRun->m_pvAddr)->m_lValid;
        pstTruck = pGetNode(pstRun->m_pvAddr, lOffset);
    }

    lOffset = (void *)pstTruck->m_pvData - (void *)pstRun->m_pvAddr;
    SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
    pstTruck->m_lTimes = 0;
    memcpy(pstTruck->m_pvData, pstSavm->pstVoid, lGetRowSize(pstSavm->tblName));
    ((TblDef *)pstRun->m_pvAddr)->m_lValid ++;
    ((TblDef *)pstRun->m_pvAddr)->m_lExSeQ ++;
    if(pllSeq)    *pllSeq = ((TblDef *)pstRun->m_pvAddr)->m_lExSeQ;

    pthread_rwlock_unlock(prwLock);
    vTblDisconnect(pstSavm, pstSavm->tblName);
    pstSavm->m_lEffect = 1;
    if(plOffset)    *plOffset = lOffset;

    return RC_SUCC;

ErrInsert:
    pthread_rwlock_unlock(prwLock);
      vTblDisconnect(pstSavm, pstSavm->tblName);
    return RC_FAIL;
}

/*************************************************************************************************
    description：Cursor-fetch the data as unique index
    parameters:
        pstSavm                    --stvm handle
        pstRun                     --table handle
        t                          --table 
        psvout                     --result data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _llFetchIndex(SATvm *pstSavm, RunTime *pstRun, TABLE t, void *psvOut)
{
    SHTree  *pstTree = NULL;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN];
    void    *pvAddr = pGetAddr(pstSavm, t);

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pPickIndex(lGetIdxNum(t), pGetTblIdx(t), pstRun->pstVoid, szIdx))
    {
        pstSavm->m_lErrno = IDX_FIELD_NIL;
        return RC_FAIL;
    }

    if(NULL == (pstTree = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lTreeRoot)))
    {   
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    pstTree = (SHTree *)pSearchTree(pvAddr, pstTree, szIdx, lGetIdxLen(t));
    if(!pstTree)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_NOTFOUND;
    }

    pstTruck = (PSHTruck)pGetNode(pvAddr, pstTree->m_lData);
    if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstRun->pstVoid))
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    pstRun->m_lCurLine ++;
    pstRun->m_pvCurAddr = NULL;
    memcpy(psvOut, pstTruck->m_pvData, lGetRowSize(t));

    return RC_SUCC;
}

/*************************************************************************************************
    description：Cursor-fetch the data as index
    parameters:
        pstSavm                    --stvm handle
        pstRun                     --table handle
        t                          --table 
        psvout                     --result data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _llFetchGroup(SATvm *pstSavm, RunTime *pstRun, TABLE t, void *psvOut)
{
    SHList  *pstList = NULL;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN];
    void    *pvAddr = pGetAddr(pstSavm, t);

    for(pstList = (SHList *)pstRun->m_pvCurAddr; pstList && (SELF_POS_UNUSE != pstList->m_lPos); 
        pstList = (SHList *)pGetNode(pvAddr, pstList->m_lNext))
    {
        pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
        if(IS_TRUCK_NULL(pstTruck))
        {
            if(SELF_POS_UNUSE == pstList->m_lNext)    break;
            continue;
        }

        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstRun->pstVoid))
        {
            if(SELF_POS_UNUSE == pstList->m_lNext)    break;
            continue;
        }

        pstRun->m_lCurLine ++;
        memcpy(psvOut, pstTruck->m_pvData, lGetRowSize(t));
        if(SELF_POS_UNUSE == pstList->m_lNext)
            pstRun->m_pvCurAddr = NULL;
        else
            pstRun->m_pvCurAddr = (void *)pGetNode(pvAddr, pstList->m_lNext);

        return RC_SUCC;
    }

    return RC_NOTFOUND;
}

/*************************************************************************************************
    description：Cursor-fetch the data as truck
    parameters:
        pstSavm                    --stvm handle
        pstRun                     --table handle
        t                          --table 
        psvout                     --result data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _llFetchTruck(SATvm *pstSavm, RunTime *pstRun, TABLE t, void *psvOut)
{
    SHTruck *pstTruck = NULL;
    long    lRow = 0, lOffset;
    void    *pvAddr = pGetAddr(pstSavm, t);

    if(1 == pstRun->m_lCurLine)
    {
        lOffset = lGetTblData(t);
        pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset);
    }
    else
    {
        lOffset = pstRun->m_pvCurAddr - pvAddr;
        pstTruck = (PSHTruck)pstRun->m_pvCurAddr;
    }

    for(lRow = 0; (lRow < ((TblDef *)pvAddr)->m_lValid) && (lOffset < lGetTableSize(t));
        pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset))
    {
        if(IS_TRUCK_NULL(pstTruck))
        {
            lOffset += lGetRowTruck(t);
            continue;
        }

        lRow ++;
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstRun->pstVoid))
        {
            lOffset += lGetRowTruck(t);
            continue;
        }

        pstRun->m_lCurLine ++;
        lOffset += lGetRowTruck(t);
        pstRun->m_pvCurAddr = pGetNode(pvAddr, lOffset);
        memcpy(psvOut, pstTruck->m_pvData, lGetRowSize(t));
        return RC_SUCC;
    }

    return RC_NOTFOUND;
}

/*************************************************************************************************
    description：Define the cursor
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTableDeclare(SATvm *pstSavm)
{
    long    lIdx;
    SHTree  *pstTree = NULL;
    RunTime *pstRun = NULL;
    char    szIdx[MAX_INDEX_LEN];

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    if(NULL == (pstRun = (RunTime *)pInitMemTable(pstSavm, pstSavm->tblName)))
        return RC_FAIL;

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        pstSavm->m_lErrno = RMT_NOT_SUPPT;
        return RC_FAIL;
    }

    pstRun->m_lCurLine = 1;
    pstRun->m_lCurType = EXE_PLAN_ALL;
    pstRun->m_lRowSize = pstSavm->lSize;
    pstRun->m_pvCurAddr= pstRun->m_pvAddr;
    pstSavm->m_lEType  = pstRun->m_lCurType;

    if(!pstSavm->pstVoid)    return RC_SUCC;

    pstRun->pstVoid = (char *)malloc(pstSavm->lSize);
    memcpy(pstRun->pstVoid, pstSavm->pstVoid, pstSavm->lSize);
    if(HAVE_UNIQ_IDX(pstSavm->tblName))
    {
        memset(szIdx, 0, sizeof(szIdx));
        if(NULL != pGetIndex(&pstSavm->stCond, lGetIdxNum(pstSavm->tblName), 
            pGetTblIdx(pstSavm->tblName), pstSavm->pstVoid, szIdx))
        {
            pstRun->m_lCurType = EXE_PLAN_IDX;
            pstSavm->m_lEType  = pstRun->m_lCurType;
            return RC_SUCC;
        }
    }

    if(HAVE_NORL_IDX(pstSavm->tblName))
    {
        memset(szIdx, 0, sizeof(szIdx));
        if(NULL == pGetIndex(&pstSavm->stCond, lGetGrpNum(pstSavm->tblName), 
            pGetTblGrp(pstSavm->tblName), pstSavm->pstVoid, szIdx))
            return RC_SUCC;
    
        pstRun->m_lCurType = EXE_PLAN_GRP;
        pstSavm->m_lEType  = pstRun->m_lCurType;
        pstRun->m_pvCurAddr = pSearchGroup(pstRun->m_pvAddr, (SHTree *)pGetNode(pstRun->m_pvAddr,
            ((TblDef *)pstRun->m_pvAddr)->m_lGroupRoot), szIdx, lGetGrpLen(pstSavm->tblName));

        return RC_SUCC;
    }
    else if(HAVE_HASH_IDX(pstSavm->tblName))
    {
        memset(szIdx, 0, sizeof(szIdx));
        if(NULL == pGetIndex(&pstSavm->stCond, lGetGrpNum(pstSavm->tblName), 
            pGetTblGrp(pstSavm->tblName), pstSavm->pstVoid, szIdx))
            return RC_SUCC;

        pstRun->m_lCurType = EXE_PLAN_GRP;
        pstSavm->m_lEType  = pstRun->m_lCurType;
        lIdx = uGetHash(szIdx, lGetGrpLen(pstSavm->tblName)) % ((TblDef *)pstRun->m_pvAddr)->m_lMaxRow;
        pstTree = pstRun->m_pvAddr + ((TblDef *)pstRun->m_pvAddr)->m_lGroupRoot + lIdx * sizeof(SHTree);
        if(NULL == pstTree || SELF_POS_UNUSE == pstTree->m_lData)
        {
            pstRun->m_pvCurAddr = NULL;
            return RC_SUCC;
        }

        pstRun->m_pvCurAddr = (SHList *)pGetNode(pstRun->m_pvAddr, pstTree->m_lData);
        return RC_SUCC;
    }    

    return RC_SUCC;
}

/*************************************************************************************************
    description：Cursor-fetch the data
    parameters:
        pstSavm                    --stvm handle
        psvout                     --result data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTableFetch(SATvm *pstSavm, void *psvOut)
{
    long    lRet;
    RunTime    *pstRun = NULL;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CURS_IS_INVAL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, pstSavm->tblName);
    if(NULL == pstRun->m_pvCurAddr)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_NOTFOUND;
    }

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        pstSavm->m_lErrno = RMT_NOT_SUPPT;
        return RC_FAIL;
    }

    if(EXE_PLAN_IDX == pstRun->m_lCurType)
        lRet = _llFetchIndex(pstSavm, pstRun, pstSavm->tblName, psvOut);
    else if(EXE_PLAN_GRP == pstRun->m_lCurType)
        lRet = _llFetchGroup(pstSavm, pstRun, pstSavm->tblName, psvOut);
    else
        lRet = _llFetchTruck(pstSavm, pstRun, pstSavm->tblName, psvOut);
    if(RC_NOTFOUND == lRet)
    {
        pstRun->m_pvCurAddr = NULL;
        pstSavm->m_lErrno = NO_DATA_FOUND;
    }

    return lRet;    
}

/*************************************************************************************************
    description：Cursor-next truck data
    parameters:
        pstSavm                    --stvm handle
        pstRun                     --table handle
        t                          --table 
        psvout                     --result data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lNextTruck(SATvm *pstSavm, RunTime *pstRun, TABLE t, void **ppvOAddr)
{
    SHTruck *pstTruck = NULL;
    long    lRow = 0, lOffset;
    void    *pvAddr = pGetAddr(pstSavm, t);

    if(1 == pstRun->m_lCurLine)
    {
        lOffset = lGetTblData(t);
        pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset);
    }
    else
    {
        lOffset = pstRun->m_pvCurAddr - pvAddr;
        pstTruck = (PSHTruck)pstRun->m_pvCurAddr;
    }

    for(lRow = 0; (lRow < ((TblDef *)pvAddr)->m_lValid) && (lOffset < lGetTableSize(t));
        pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset))
    {
        if(IS_TRUCK_NULL(pstTruck))
        {
            lOffset += lGetRowTruck(t);
            continue;
        }

        lRow ++;
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstRun->pstVoid))
        {
            lOffset += lGetRowTruck(t);
            continue;
        }

        pstRun->m_lCurLine ++;
        *ppvOAddr = pstTruck->m_pvData;
        lOffset += lGetRowTruck(t);
        pstRun->m_pvCurAddr = pGetNode(pvAddr, lOffset);
        return RC_SUCC;
    }

    return RC_NOTFOUND;
}

/*************************************************************************************************
    description：Cursor-next index data
    parameters:
        pstSavm                    --stvm handle
        pstRun                     --table handle
        t                          --table 
        psvout                     --result data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lNextGroup(SATvm *pstSavm, RunTime *pstRun, TABLE t, void **ppvOAddr)
{
    SHList  *pstList = NULL;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN];
    void    *pvAddr = pGetAddr(pstSavm, t);

    for(pstList = (SHList *)pstRun->m_pvCurAddr; pstList && (SELF_POS_UNUSE != pstList->m_lPos); 
        pstList = (SHList *)pGetNode(pvAddr, pstList->m_lNext))
    {
        pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstRun->pstVoid))
        {
            if(SELF_POS_UNUSE == pstList->m_lNext)    break;
            continue;
        }

        pstRun->m_lCurLine ++;
        *ppvOAddr = pstTruck->m_pvData;
        if(SELF_POS_UNUSE == pstList->m_lNext)
            pstRun->m_pvCurAddr = NULL;
        else
            pstRun->m_pvCurAddr = (void *)pGetNode(pvAddr, pstList->m_lNext);

        return RC_SUCC;
    }

    return RC_NOTFOUND;
}

/*************************************************************************************************
    description：Cursor-next unique index data
    parameters:
        pstSavm                    --stvm handle
        pstRun                     --table handle
        t                          --table 
        psvout                     --result data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lNextIndex(SATvm *pstSavm, RunTime *pstRun, TABLE t, void **ppvOAddr)
{
    SHTree  *pstTree = NULL;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN];
    void    *pvAddr = pGetAddr(pstSavm, t);

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pPickIndex(lGetIdxNum(t), pGetTblIdx(t), pstSavm->pstVoid, szIdx))
    {
        pstSavm->m_lErrno = IDX_FIELD_NIL;
        return RC_FAIL;
    }

    if(NULL == (pstTree = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lTreeRoot)))
    {   
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    pstTree = (SHTree *)pSearchTree(pvAddr, pstTree, szIdx, lGetIdxLen(t));
    if(!pstTree)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_NOTFOUND;
    }

    pstTruck = (PSHTruck)pGetNode(pvAddr, pstTree->m_lData);
    if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstRun->pstVoid))
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    pstRun->m_lCurLine ++;
    pstRun->m_pvCurAddr = NULL;
    *ppvOAddr = pstTruck->m_pvData;
    return RC_SUCC;
}

/*************************************************************************************************
    description：Cursor-next fetch
    parameters:
        pstSavm                    --stvm handle
        ppvOAddr                   --result data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lNextFetch(SATvm *pstSavm, void **ppvOAddr)
{
    long       lRet;
    RunTime    *pstRun = NULL;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CURS_IS_INVAL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, pstSavm->tblName);
    if(NULL == pstRun->m_pvCurAddr)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_NOTFOUND;
    }

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        pstSavm->m_lErrno = RMT_NOT_SUPPT;
        return RC_FAIL;
    }

    if(EXE_PLAN_IDX == pstRun->m_lCurType)
        lRet = _lNextIndex(pstSavm, pstRun, pstSavm->tblName, ppvOAddr);
    else if(EXE_PLAN_GRP == pstRun->m_lCurType)
        lRet = _lNextGroup(pstSavm, pstRun, pstSavm->tblName, ppvOAddr);
    else
        lRet = _lNextTruck(pstSavm, pstRun, pstSavm->tblName, ppvOAddr);
    if(RC_NOTFOUND == lRet)
    {
        pstRun->m_pvCurAddr = NULL;
        pstSavm->m_lErrno = NO_DATA_FOUND;
    }

    return lRet;    
}

/*************************************************************************************************
    description：close Cursor
    parameters:
        pstSavm                    --stvm handle
    return:
 *************************************************************************************************/
void    vTableClose(SATvm *pstSavm)
{
    _vTblRelease(pstSavm, pstSavm->tblName, pstSavm->m_bHold);
}

/*************************************************************************************************
    description：Update record hash index
    parameters:
        pstSavm                    --stvm handle
        pstAddr                    --table handle
        t                          --table 
        pstTruck                   --data truck
        lData                      --offset
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    __lUpdateHash(SATvm *pstSavm, void *pvAddr, TABLE t, SHTruck *pstTruck, size_t lData)
{
    SHList  *pstList = NULL;
    SHTree  *pstTree = NULL;
    size_t  lIdx, lOld, lOffset, lNext;
    char    szOld[MAX_INDEX_LEN], szIdx[MAX_INDEX_LEN];

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pPickIndex(lGetGrpNum(t), pGetTblGrp(t), pstSavm->pstVoid, szIdx))
    {
        pstSavm->m_lErrno = IDX_FIELD_NIL;
        return RC_FAIL;
    }

    memset(szOld, 0, sizeof(szOld));
    if(NULL == pPickIndex(lGetGrpNum(t), pGetTblGrp(t), pstTruck->m_pvData, szOld))
    {
        pstSavm->m_lErrno = IDX_DATA_MISM;
        return RC_FAIL;
    }

    lOld = uGetHash(szOld, lGetGrpLen(t)) % ((TblDef *)pvAddr)->m_lMaxRow;
    lIdx = uGetHash(szIdx, lGetGrpLen(t)) % ((TblDef *)pvAddr)->m_lMaxRow;
    if(lOld == lIdx)    return RC_SUCC;

    lOffset = ((TblDef *)pvAddr)->m_lGroupPos + lOld * sizeof(SHTree);
    if(NULL == (pstTree = (SHTree *)(pvAddr + lOffset)))
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    lOffset = lGetListOfs(t) + ((TblDef *)pvAddr)->m_lValid * sizeof(SHList);
    for(pstList = (SHList *)pGetNode(pvAddr, pstTree->m_lData); SELF_POS_UNUSE != pstList->m_lPos;
        pstList = (SHList *)pGetNode(pvAddr, lNext))
    {
        lNext = pstList->m_lNext;
        if(lData != pstList->m_lData)
        {
            if(SELF_POS_UNUSE == pstList->m_lNext) break;
            continue;
        }

        lOffset -= sizeof(SHList);
        if(RC_SUCC != lReleaseList(pvAddr, t, pstTree, pstList, lOffset, &lNext))
        {
            pstSavm->m_lErrno = SVR_EXCEPTION;
            return RC_FAIL;
        }

        ((TblDef *)pvAddr)->m_lValid --;
        lOffset = RC_CLOSE;
        break;
    }

    if(SELF_POS_UNUSE == pstTree->m_lData)
        pstTree->m_lSePos = SELF_POS_UNUSE;

    if(RC_CLOSE != lOffset)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    lOffset = ((TblDef *)pvAddr)->m_lGroupPos + lIdx * sizeof(SHTree);
    if(NULL == (pstTree = (SHTree *)(pvAddr + lOffset)))
    {
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    if(RC_SUCC !=  __lInsertHash(pstSavm, pvAddr, t, pstTree, lOffset, &pstTruck))
        return RC_FAIL;

    ((TblDef *)pvAddr)->m_lValid ++;
    return RC_SUCC;
}

/*************************************************************************************************
    description：Update record index
    parameters:
        pstSavm                    --stvm handle
        pstAddr                    --table handle
        t                          --table 
        pstTruck                   --data truck
        lData                      --offset
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    __lUpdateGroup(SATvm *pstSavm, void *pvAddr, TABLE t, SHTruck *pstTruck, size_t lData)
{
    SHList  *pstList = NULL;
    size_t  lNext = 0, lOffset;
    SHTree  *pstRoot, *pstTree = NULL;
    char    szOld[MAX_INDEX_LEN], szIdx[MAX_INDEX_LEN];

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pPickIndex(lGetGrpNum(t), pGetTblGrp(t), pstSavm->pstVoid, szIdx))
    {
        pstSavm->m_lErrno = IDX_FIELD_NIL;
        return RC_FAIL;
    }

    memset(szOld, 0, sizeof(szOld));
    if(NULL == pPickIndex(lGetGrpNum(t), pGetTblGrp(t), pstTruck->m_pvData, szOld))
    {
        pstSavm->m_lErrno = IDX_DATA_MISM;
        return RC_FAIL;
    }

    if(!memcmp(szIdx, szOld, MAX_INDEX_LEN))  // Index has not changed
        return RC_SUCC;

    if(NULL == (pstRoot = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lGroupRoot)))
    {
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    if(NULL == (pstTree = pSearchTree(pvAddr, pstRoot, szOld, lGetGrpLen(t))))
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    lOffset  = lGetListOfs(t) + ((TblDef *)pvAddr)->m_lValid * sizeof(SHList);
    for(pstList = (SHList *)pGetNode(pvAddr, pstTree->m_lData); SELF_POS_UNUSE != pstList->m_lPos;
        pstList = (SHList *)pGetNode(pvAddr, lNext))
    {
        lNext = pstList->m_lNext;
        if(lData != pstList->m_lData)
        {
            if(SELF_POS_UNUSE == pstList->m_lNext) break;
            continue;
        }

        lOffset -= sizeof(SHList);
        if(RC_SUCC != lReleaseList(pvAddr, t, pstTree, pstList, lOffset, &lNext))
        {
            pstSavm->m_lErrno = SVR_EXCEPTION;
            return RC_FAIL;
        }

        ((TblDef *)pvAddr)->m_lValid --;
        lOffset = RC_CLOSE;
        break;
    }

    if(SELF_POS_UNUSE == pstTree->m_lData)
    {
        pstRoot = _pDeleteGroup(pvAddr, pstSavm->tblName, pstRoot, pstTree, &pstSavm->m_lEffect);
        ((TblDef *)pvAddr)->m_lGroupRoot = pstRoot->m_lSePos;
    }

    if(RC_CLOSE != lOffset)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    if(NULL == (pstRoot = pInsertGroup(pstSavm, pstRoot, szIdx, lGetGrpLen(t), &pstTruck)))
        return RC_FAIL;

    ((TblDef *)pvAddr)->m_lValid ++;
    ((TblDef *)pvAddr)->m_lGroupRoot = pstRoot->m_lSePos;

    return RC_SUCC;
}

/*************************************************************************************************
    description：Update record unique index
    parameters:
        pstSavm                    --stvm handle
        pstAddr                    --table handle
        t                          --table 
        pstTruck                   --data truck
        lData                      --offset
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long   __lIndexUpdate(SATvm *pstSavm, void *pvAddr, TABLE t, SHTruck *pstTruck, size_t lData)
{
    SHTree  *pstRoot, *pstTree = NULL;
    char    szOld[MAX_INDEX_LEN], szIdx[MAX_INDEX_LEN];

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pPickIndex(lGetIdxNum(t), pGetTblIdx(t), pstSavm->pstVoid, szIdx))
    {
        pstSavm->m_lErrno = IDX_FIELD_NIL;
        return RC_FAIL;
    }

    memset(szOld, 0, sizeof(szOld));
    if(NULL == pPickIndex(lGetIdxNum(t), pGetTblIdx(t), pstTruck->m_pvData, szOld))
    {
        pstSavm->m_lErrno = IDX_DATA_MISM;
        return RC_FAIL;
    }

    if(!memcmp(szIdx, szOld, MAX_INDEX_LEN)) // Index has not changed
        return RC_SUCC;

    if(NULL == (pstRoot = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lTreeRoot)))
    {
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    // frist select, then add The unique index, avoiding the uniqueness error after deletion.
    if(NULL != (SHTree *)pSearchTree(pvAddr, pstRoot, szIdx, lGetIdxLen(t)))
    {
        pstSavm->m_lErrno = UNIQ_IDX_REPT;
        return RC_FAIL;
    }

    if(NULL == (pstTree = (SHTree *)pSearchTree(pvAddr, pstRoot, szOld, lGetIdxLen(t))))
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    if(lData != pstTree->m_lData)
    {
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    if(NULL == (pstRoot = _pDeleteTree(pvAddr, pstSavm->tblName, pstRoot, pstTree)))
    {
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }
    ((TblDef *)pvAddr)->m_lValid --;

    if(NULL == (pstRoot = pInsertTree(pstSavm, pstRoot, szIdx, lGetIdxLen(t), &pstTruck)))
        return RC_FAIL;
    ((TblDef *)pvAddr)->m_lValid ++;
    ((TblDef *)pvAddr)->m_lTreeRoot = pstRoot->m_lSePos;
    return RC_SUCC;
}

/*************************************************************************************************
    description：Update record unique index
    parameters:
        pstSavm                    --stvm handle
        pstAddr                    --table handle
        t                          --table 
        pvUpdate                   --update data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lUpdateIndex(SATvm *pstSavm, void *pvAddr, TABLE t, void *pvUpdate)
{
    size_t  lData;
    long    lRet = RC_SUCC;
    void    *pvData = NULL;
    SHTruck *pstTruck = NULL;
    SHTree  *pstRoot, *pstTree = NULL;
    RWLock  *prwLock = (RWLock *)pGetRWLock(pvAddr);
    char    szOld[MAX_INDEX_LEN], szIdx[MAX_INDEX_LEN];

    memset(szOld, 0, sizeof(szOld));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetIdxNum(t), pGetTblIdx(t), pstSavm->pstVoid, szOld))
        return RC_CONTU;
    
    if(NULL == (pvData = (void *)malloc(lGetRowSize(pstSavm->tblName))))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    if(RC_SUCC != pthread_rwlock_wrlock(prwLock)) 
    {
        pstSavm->m_lErrno = LOCK_DOWR_ERR;
        TFree(pvData);
        return RC_FAIL;
    }

    pstSavm->m_lEType = EXE_PLAN_IDX;
    if(NULL == (pstRoot = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lTreeRoot)))
    {
        pthread_rwlock_unlock(prwLock);    
        pstSavm->m_lErrno = SVR_EXCEPTION;
        TFree(pvData);
        return RC_FAIL;
    }

    if(NULL == (pstTree = (SHTree *)pSearchTree(pvAddr, pstRoot, szOld, lGetIdxLen(t))))
    {
        pthread_rwlock_unlock(prwLock);    
        pstSavm->m_lErrno = NO_DATA_FOUND;
        TFree(pvData);
        return RC_FAIL;
    }

    lData = pstTree->m_lData;
    pstTruck = (PSHTruck)pGetNode(pvAddr, pstTree->m_lData);
    if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
    {
        pthread_rwlock_unlock(prwLock);    
        pstSavm->m_lErrno = NO_DATA_FOUND;
        TFree(pvData);
        return RC_FAIL;
    }

    memcpy(pvData, pstTruck->m_pvData, lGetRowSize(pstSavm->tblName));
    if(RC_FAIL == lMergeTruck(pstSavm, &pstSavm->stUpdt, pvUpdate, pvData))
    {
        pthread_rwlock_unlock(prwLock);    
        TFree(pvData);
        return RC_FAIL;
    }

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pPickIndex(lGetIdxNum(t), pGetTblIdx(t), pvData, szIdx))
    {
        pthread_rwlock_unlock(prwLock);    
        pstSavm->m_lErrno = IDX_FIELD_NIL;
        TFree(pvData);
        return RC_FAIL;
    }

    pstSavm->pstVoid = pvData;
    pstTruck->m_lTimes ++;
    SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NULL);
    if(!memcmp(szIdx, szOld, MAX_INDEX_LEN))
    {
        if(HAVE_NORL_IDX(t))
            lRet = __lUpdateGroup(pstSavm, pvAddr, t, pstTruck, lData);
        else if(HAVE_HASH_IDX(t))            
            lRet = __lUpdateHash(pstSavm, pvAddr, t, pstTruck, lData);
    }
    else
    {
        if(NULL != (SHTree *)pSearchTree(pvAddr, pstRoot, szIdx, lGetIdxLen(t)))
        {
            SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
            pthread_rwlock_unlock(prwLock);    
            pstSavm->m_lErrno = UNIQ_IDX_REPT;
            TFree(pvData);
            return RC_FAIL;
        }

        if(NULL == (pstRoot = _pDeleteTree(pvAddr, pstSavm->tblName, pstRoot, pstTree)))
        {
            SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
            pthread_rwlock_unlock(prwLock);    
            pstSavm->m_lErrno = SVR_EXCEPTION;
            TFree(pvData);
            return RC_FAIL;
        }
        
        ((TblDef *)pvAddr)->m_lValid --;
        if(NULL == (pstRoot = pInsertTree(pstSavm, pstRoot, szIdx, lGetIdxLen(t), &pstTruck)))
        {
            SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
            pthread_rwlock_unlock(prwLock);    
            TFree(pvData);
            return RC_FAIL;
        }

        ((TblDef *)pvAddr)->m_lValid ++;
        ((TblDef *)pvAddr)->m_lTreeRoot = pstRoot->m_lSePos;
        if(HAVE_NORL_IDX(t))
            lRet = __lUpdateGroup(pstSavm, pvAddr, t, pstTruck, lData);
        else if(HAVE_HASH_IDX(t))
            lRet = __lUpdateHash(pstSavm, pvAddr, t, pstTruck, lData);
    }
    SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
    if(RC_SUCC != lRet)
    {
        pthread_rwlock_unlock(prwLock);    
        TFree(pvData);
        return RC_FAIL;
    }

    if(RC_SUCC != lRecordWork(pstSavm, pstTruck->m_pvData, OPERATE_UPDATE))
    {
        pthread_rwlock_unlock(prwLock);    
        TFree(pvData);
        return RC_FAIL;
    }

    memcpy(pstTruck->m_pvData, pvData, lGetRowSize(t));
    pthread_rwlock_unlock(prwLock);    
    pstSavm->m_lEffect = 1;

    TFree(pvData);
    return RC_SUCC;
}

/*************************************************************************************************
    description：Update record index
    parameters:
        pstSavm                    --stvm handle
        pstAddr                    --table handle
        t                          --table 
        pvUpdate                   --update data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lUpdateGroup(SATvm *pstSavm, void *pvAddr, TABLE t, void *pvUpdate)
{
    SHList  *pstList = NULL;
    SHTruck *pstTruck = NULL;
    size_t  lOffset, lNext;
    SHTree  *pstTree, *pstRoot;
    void    *pvData,  *pvCond = NULL;
    RWLock  *prwLock = (RWLock *)pGetRWLock(pvAddr);
    char    szOld[MAX_INDEX_LEN], szIdx[MAX_INDEX_LEN];

    memset(szOld, 0, sizeof(szOld));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetGrpNum(t), pGetTblGrp(t), pstSavm->pstVoid, szOld))
        return RC_CONTU;

    if(NULL == (pvData = (void *)malloc(lGetRowSize(t))))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    if(RC_SUCC != pthread_rwlock_wrlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DOWR_ERR;
        TFree(pvData);
        return RC_FAIL;
    }

    if(NULL == (pstRoot = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lGroupRoot)))
    {
        pstSavm->m_lErrno = SVR_EXCEPTION;
        goto REPGROUP_ERROR;
    }

    if(NULL == (pstTree = pSearchTree(pvAddr, pstRoot, szOld, lGetGrpLen(t))))
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        goto REPGROUP_ERROR;
    }

    pstSavm->m_lEffect = 0;
    pvCond = pstSavm->pstVoid;
    pstSavm->m_lEType = EXE_PLAN_GRP;
    for(pstList = (SHList *)pGetNode(pvAddr, pstTree->m_lData); SELF_POS_UNUSE != pstList->m_lPos;
        pstList = (SHList *)pGetNode(pvAddr, lNext))
    {
        lNext = pstList->m_lNext;
        pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pvCond))
        {
            if(SELF_POS_UNUSE == pstList->m_lNext)    break;
            continue;
        }

        memcpy(pvData, pstTruck->m_pvData, lGetRowSize(t)); 
        if(RC_FAIL == lMergeTruck(pstSavm, &pstSavm->stUpdt, pvUpdate, pvData))
            goto REPGROUP_ERROR;

        memset(szIdx, 0, sizeof(szIdx));
        if(NULL == pPickIndex(lGetGrpNum(t), pGetTblGrp(t), pvData, szIdx))
        {
            pstSavm->m_lErrno = IDX_FIELD_NIL;
            goto REPGROUP_ERROR;
        }

        pstSavm->m_lEffect ++;
        pstSavm->pstVoid = pvData;
        SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NULL);
        if(HAVE_UNIQ_IDX(t))
        {
            if(RC_SUCC != __lIndexUpdate(pstSavm, pvAddr, t, pstTruck, pstList->m_lData))
            {
                SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
                goto REPGROUP_ERROR;
            }
        }

        if(memcmp(szIdx, szOld, MAX_INDEX_LEN))
        {
            -- ((TblDef *)pvAddr)->m_lValid;
            lOffset = lGetListOfs(t) + ((TblDef *)pvAddr)->m_lValid * sizeof(SHList);
            if(RC_SUCC != lReleaseList(pvAddr, t, pstTree, pstList, lOffset, &lNext))
            {
                ((TblDef *)pvAddr)->m_lValid ++;
                pstSavm->m_lErrno = SVR_EXCEPTION;
                SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
                goto REPGROUP_ERROR;
            }

            if(SELF_POS_UNUSE == pstTree->m_lData)
            {
                pstRoot = _pDeleteGroup(pvAddr, t, pstRoot, pstTree, &pstSavm->m_lEffect);
                ((TblDef *)pvAddr)->m_lGroupRoot = pstRoot->m_lSePos;
            }

            if(NULL == (pstRoot = pInsertGroup(pstSavm, pstRoot, szIdx, lGetGrpLen(t), &pstTruck)))
            {
                SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
                goto REPGROUP_ERROR;
            }

            ((TblDef *)pvAddr)->m_lValid ++;
            ((TblDef *)pvAddr)->m_lGroupRoot = pstRoot->m_lSePos;
        }

        if(RC_SUCC != lRecordWork(pstSavm, pstTruck->m_pvData, OPERATE_UPDATE)) 
            goto REPGROUP_ERROR;

        memcpy(pstTruck->m_pvData, pvData, lGetRowSize(t));
        SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);

        if(FIRST_ROW & pstSavm->lFind)    break;
    }
    pthread_rwlock_unlock(prwLock);
    TFree(pvData);

    if(0 == pstSavm->m_lEffect)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    return RC_SUCC;

REPGROUP_ERROR:
    pthread_rwlock_unlock(prwLock);
    TFree(pvData);
    return RC_FAIL;
}

/*************************************************************************************************
    description：Update record hash index
    parameters:
        pstSavm                    --stvm handle
        pstAddr                    --table handle
        t                          --table 
        pvUpdate                   --update data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lUpdateHash(SATvm *pstSavm, void *pvAddr, TABLE t, void *pvUpdate)
{
    SHList  *pstList = NULL;
    SHTruck *pstTruck = NULL;
    SHTree  *pstTree, *pstHash;
    void    *pvData = NULL, *pvCond;
    size_t  lOld, lIdx, lOffset, lNext;
    RWLock  *prwLock = (RWLock *)pGetRWLock(pvAddr);
    char    szOld[MAX_INDEX_LEN], szIdx[MAX_INDEX_LEN];

    memset(szOld, 0, sizeof(szOld));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetGrpNum(t), pGetTblGrp(t), pstSavm->pstVoid, szOld))
        return RC_CONTU;

    if(NULL == (pvData = (void *)malloc(lGetRowSize(t))))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    lOld = uGetHash(szOld, lGetGrpLen(t)) % ((TblDef *)pvAddr)->m_lMaxRow;
    lOffset = ((TblDef *)pvAddr)->m_lGroupPos + lOld * sizeof(SHTree);
    if(NULL == (pstHash = (SHTree *)(pvAddr + lOffset)))
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        TFree(pvData);
        return RC_FAIL;
    }

    if(RC_SUCC != pthread_rwlock_wrlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DOWR_ERR;
        TFree(pvData);
        return RC_FAIL;
    }

    pstSavm->m_lEffect = 0;
    pvCond = pstSavm->pstVoid;
    pstSavm->m_lEType = EXE_PLAN_GRP;
    for(pstList = (SHList *)pGetNode(pvAddr, pstHash->m_lData); SELF_POS_UNUSE != pstList->m_lPos; 
        pstList = (SHList *)pGetNode(pvAddr, lNext))
    {
        lNext = pstList->m_lNext;
        pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pvCond))
        {
            if(SELF_POS_UNUSE == pstList->m_lNext)    break;
            continue;
        }

        memcpy(pvData, pstTruck->m_pvData, lGetRowSize(t)); 
        if(RC_FAIL == lMergeTruck(pstSavm, &pstSavm->stUpdt, pvUpdate, pvData))
            goto REPHASH_ERROR;

        memset(szIdx, 0, sizeof(szIdx));
        if(NULL == pPickIndex(lGetGrpNum(t), pGetTblGrp(t), pvData, szIdx))
        {
            pstSavm->m_lErrno = IDX_FIELD_NIL;
            goto REPHASH_ERROR;
        }

        pstSavm->m_lEffect ++;
        pstSavm->pstVoid = pvData;
        lIdx = uGetHash(szIdx, lGetGrpLen(t)) % ((TblDef *)pvAddr)->m_lMaxRow;
        SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NULL);
        if(HAVE_UNIQ_IDX(t))
        {
            if(RC_SUCC != __lIndexUpdate(pstSavm, pvAddr, t, pstTruck, pstList->m_lData))
            {
                SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NULL);
                goto REPHASH_ERROR;
            }
        }

        // Index does not match, rebuild inde
        if(lOld != lIdx)
        {
            -- ((TblDef *)pvAddr)->m_lValid;
            lOffset = lGetListOfs(t) + ((TblDef *)pvAddr)->m_lValid * sizeof(SHList);
            if(RC_SUCC != lReleaseList(pvAddr, t, pstHash, pstList, lOffset, &lNext))
            {
                ((TblDef *)pvAddr)->m_lValid ++;
                pstSavm->m_lErrno = SVR_EXCEPTION;
                SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
                goto REPHASH_ERROR;
            }

            lOffset = ((TblDef *)pvAddr)->m_lGroupPos + lIdx * sizeof(SHTree);
            if(NULL == (pstTree = (SHTree *)(pvAddr + lOffset)))
            {
                pstSavm->m_lErrno = SVR_EXCEPTION;
                SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
                goto REPHASH_ERROR;
            }

            if(RC_SUCC != __lInsertHash(pstSavm, pvAddr, t, pstTree, lOffset, &pstTruck))
            {
                pstSavm->m_lErrno = SVR_EXCEPTION;
                SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
                goto REPHASH_ERROR;
            }

            ((TblDef *)pvAddr)->m_lValid ++;
        }
        if(RC_SUCC != lRecordWork(pstSavm, pstTruck->m_pvData, OPERATE_UPDATE)) 
            goto REPHASH_ERROR;

        memcpy(pstTruck->m_pvData, pvData, lGetRowSize(t));
        SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);

        if(FIRST_ROW & pstSavm->lFind)    break;
    }
    pthread_rwlock_unlock(prwLock);
    TFree(pvData);

    if(0 == pstSavm->m_lEffect)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    return RC_SUCC;

REPHASH_ERROR:
    pthread_rwlock_unlock(prwLock);
    TFree(pvData);
    return RC_FAIL;
}

/*************************************************************************************************
    description：Update record truck 
    parameters:
        pstSavm                    --stvm handle
        pstAddr                    --table handle
        t                          --table 
        pvUpdate                   --update data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lTruckUpdate(SATvm *pstSavm, void *pvAddr, TABLE t, void *pvUpdate)
{
    long    lRet = RC_SUCC;
    SHTruck *pstTruck = NULL;
    void    *pvData = NULL, *pvCond;
    size_t  lRow, lOffset = lGetTblData(t);
    RWLock  *prwLock = (RWLock *)pGetRWLock(pvAddr);

    if(NULL == (pvData = (void *)malloc(lGetRowSize(t))))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    if(RC_SUCC != pthread_rwlock_wrlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DOWR_ERR;
        TFree(pvData);
        return RC_FAIL;
    }

    pstSavm->m_lEffect = 0;
    pvCond = pstSavm->pstVoid;
    pstSavm->m_lEType = EXE_PLAN_ALL;
    pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset);
    for(lRow = 0; (lRow < ((TblDef *)pvAddr)->m_lValid) && (lOffset < lGetTableSize(t));
        pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset))
    {
        if(IS_TRUCK_NULL(pstTruck))
        {
            lOffset += lGetRowTruck(t);
            continue;
        }

        lRow ++;
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pvCond))
        {
            lOffset += lGetRowTruck(t);
            continue;
        }

        memcpy(pvData, pstTruck->m_pvData, lGetRowSize(t)); 
        if(RC_FAIL == lMergeTruck(pstSavm, &pstSavm->stUpdt, pvUpdate, pvData))
            goto TRUCK_ERROR;

        pstSavm->m_lEffect ++;
        pstSavm->pstVoid = pvData;
        SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NULL);
        if(HAVE_UNIQ_IDX(t))
        {
            if(RC_SUCC != __lIndexUpdate(pstSavm, pvAddr, t, pstTruck, lOffset))
            {
                SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
                goto TRUCK_ERROR;
            }
        }
 
        if(HAVE_NORL_IDX(t))
            lRet = __lUpdateGroup(pstSavm, pvAddr, t, pstTruck, lOffset);
        else if(HAVE_HASH_IDX(t))
            lRet = __lUpdateHash(pstSavm, pvAddr, t, pstTruck, lOffset);
        SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
        if(RC_SUCC != lRet)
            goto TRUCK_ERROR;

        if(RC_SUCC != lRecordWork(pstSavm, pstTruck->m_pvData, OPERATE_UPDATE)) 
            goto TRUCK_ERROR;

        memcpy(pstTruck->m_pvData, pvData, lGetRowSize(t));
        if(FIRST_ROW & pstSavm->lFind)    break;
        lOffset += lGetRowTruck(t);
    }

    pthread_rwlock_unlock(prwLock);
    TFree(pvData);
    if(0 == pstSavm->m_lEffect)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    return RC_SUCC;

TRUCK_ERROR:
    pthread_rwlock_unlock(prwLock);
    TFree(pvData);
    return RC_FAIL;
}

/*************************************************************************************************
    description：API - Update
    parameters:
        pstSavm                    --stvm handle
        pvUpdate                   --update data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lUpdate(SATvm *pstSavm, void *pvUpdate)
{
    long    lRet;
    void    *pvData;
    RunTime *pstRun = NULL;

    if(!pstSavm || !pvUpdate)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pvData = pstSavm->pstVoid;
    if(NULL == (pstRun = (RunTime *)pInitMemTable(pstSavm, pstSavm->tblName)))
        return RC_FAIL;

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        Tremohold(pstSavm, pstRun);
        return _lUpdateByRt(pstSavm, pvUpdate);
    }

    if(HAVE_UNIQ_IDX(pstSavm->tblName))
    {
        lRet = _lUpdateIndex(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, pvUpdate);
        if(RC_CONTU != lRet)
        {
            vTblDisconnect(pstSavm, pstSavm->tblName);
            pstSavm->pstVoid = pvData;
            return lRet;
        }
    }

    if(HAVE_NORL_IDX(pstSavm->tblName))
    {
        lRet = _lUpdateGroup(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, pvUpdate);
        if(RC_CONTU != lRet)
        {
            vTblDisconnect(pstSavm, pstSavm->tblName);
            pstSavm->pstVoid = pvData;
            return lRet;
        }
    }
    else if(HAVE_HASH_IDX(pstSavm->tblName))
    {
        lRet = _lUpdateHash(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, pvUpdate);
        if(RC_CONTU != lRet)
        {
            vTblDisconnect(pstSavm, pstSavm->tblName);
            pstSavm->pstVoid = pvData;
            return lRet;
        }
    }

    lRet = _lTruckUpdate(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, pvUpdate);
    vTblDisconnect(pstSavm, pstSavm->tblName);
    pstSavm->pstVoid = pvData;
    return lRet;
}

/*************************************************************************************************
    description：Registry and table fields
    parameters:
        pstSavm                    --stvm handle
        pstRun                     --table handle
        t                          --table 
        lType   
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lRegisterTable(SATvm *pstSavm, RunTime *pstRun, TABLE t, long lType)
{
    TIndex  stIndex;
    TBoot   *pstBoot = (TBoot *)pBootInitial();

    if(TYPE_SYSTEM == lType || TYPE_INCORE == lType)
        return RC_SUCC;

    if(RC_SUCC != lInitSATvm(pstSavm, SYS_TVM_INDEX))
        return RC_FAIL;

    if(RC_FAIL == (pstSavm->m_ySey = yGetIPCPath(pstSavm, IPC_SEM)))
          return RC_FAIL;

    pstRun->m_semID = semget(pstSavm->m_ySey, pstBoot->m_lMaxTable, IPC_CREAT|0660);

    conditinit(pstSavm, stIndex, SYS_TVM_INDEX);
    stIndex.m_lValid   = 0;
    stIndex.m_yKey     = 0;
    stIndex.m_table    = t;
    stIndex.m_lType    = lType;
    stIndex.m_lMaxRows = lGetTblRow(t);
    stIndex.m_lRowSize = lGetRowSize(t);
    stIndex.m_shmID    = pstRun->m_shmID;
    stIndex.m_semID    = pstRun->m_semID;
    stIndex.m_lLocal   = RES_LOCAL_SID;
    stIndex.m_lPers    = lGetPermit(t);

    strncpy(stIndex.m_szOwner, pstSavm->m_szNode, sizeof(stIndex.m_szOwner));
    strncpy(stIndex.m_szTime, sGetUpdTime(), sizeof(stIndex.m_szTime));
    strncpy(stIndex.m_szTable, sGetTableName(t), sizeof(stIndex.m_szTable));
    strncpy(stIndex.m_szPart, sGetTablePart(t, pstSavm->m_szNode), sizeof(stIndex.m_szPart));

    if(RC_SUCC != lInsert(pstSavm))
    {
        if(UNIQ_IDX_REPT == pstSavm->m_lErrno)
            pstSavm->m_lErrno = TBL_ARD_EXIST;
        return RC_FAIL;
    }

    if(RC_SUCC != lInsertField(pstSavm, t))
        return RC_FAIL;

    if(TVM_BOOT_SIMPLE != pstBoot->m_lBootType)
        lRefreshNotify(pstSavm, pstBoot->m_lBootPort);
    return RC_SUCC;
}

/*************************************************************************************************
    description：Define the customer table
    parameters:
        pstSavm                    --stvm handle
        t                          --table 
        lRow                       --table maxrows
        bCreate                    --create type
        lType                      --table type
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lCustomTable(SATvm *pstSavm, TABLE t, size_t lRow, bool bCreate, long lType)
{
    RWAttr  attr;
    RunTime *pstRun = NULL;
    RWLock  *prwLock = NULL;

    pstSavm->tblName = t;
    ((TblDef *)pGetTblDef(t))->m_lTable = lInitialTable(t, lRow);
    if(NULL == (pstRun = (RunTime *)pCreateBlock(pstSavm, t, ((TblDef *)pGetTblDef(t))->m_lTable, 
        bCreate)))
        return RC_FAIL;

    //  After the memory is created, it will initialize the index information
    if(RC_SUCC != lInitailTree(pstSavm, (void *)pGetNode(pstRun->m_pvAddr, lGetIdxPos(t)), t))
        return RC_FAIL;

    if(RC_SUCC != lInitailGroup(pstSavm, (void *)pGetNode(pstRun->m_pvAddr, lGetGrpPos(t)), t))
        return RC_FAIL;

    memcpy(pstRun->m_pvAddr, (void *)pGetTblDef(t), sizeof(TblDef));
    prwLock = (RWLock *)pGetRWLock(pstRun->m_pvAddr);
    pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_rwlock_init(prwLock, &attr);

    memset(pstRun->m_pvAddr + lGetTblData(t), 0, lGetTableSize(t) - lGetTblData(t));
    vTblDisconnect(pstSavm, t);

    if(RC_SUCC != lRegisterTable(pstSavm, pstRun, t, lType))
    {
        shmctl(pstRun->m_shmID, IPC_RMID, NULL);
        return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：create queue 
    parameters:
        pstSavm                    --stvm handle
        t                          --table 
        lRow                       --table maxrows
        bCreate                    --create type
        lType                      --table type
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lCreateQueue(SATvm *pstSavm, TABLE t, size_t lRow, size_t lSize, char *pszTable, 
            char *pszNode, bool bCover)
{
    RWAttr  attr;
    RunTime *pstRun = NULL;
    RWLock  *prwLock = NULL;

    if(!pstSavm || lRow <= 0)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }
   
    if((lRow >> (sizeof(int) * 8 - 1)) > 0)
    {
        pstSavm->m_lErrno = MQUE_CRTE_BIG;
        return RC_FAIL;
    } 

    vInitTblDef(t);
    pstSavm->tblName = t;
    ((TblDef *)pGetTblDef(t))->m_lIType = bCover;
    ((TblDef *)pGetTblDef(t))->m_table = t; 
    ((TblDef *)pGetTblDef(t))->m_lReSize = lSize;
    ((TblDef *)pGetTblDef(t))->m_lTruck = lSize + sizeof(SHTruck);
    strncpy(((TblDef *)pGetTblDef(t))->m_szPart, pszNode, MAX_FIELD_LEN); 
    strncpy(((TblDef *)pGetTblDef(t))->m_szTable, pszTable, MAX_FIELD_LEN); 
    ((TblDef *)pGetTblDef(t))->m_lTable = lInitialTable(t, lRow);
    if(NULL == (pstRun = (RunTime *)pCreateBlock(pstSavm, t, ((TblDef *)pGetTblDef(t))->m_lTable, 
        false)))
        return RC_FAIL;

    memcpy(pstRun->m_pvAddr, (void *)pGetTblDef(t), sizeof(TblDef));
    prwLock = (RWLock *)pGetRWLock(pstRun->m_pvAddr);
    pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_rwlock_init(prwLock, &attr);

    memset(pstRun->m_pvAddr + lGetTblData(t), 0, lGetTableSize(t) - lGetTblData(t));
    vTblDisconnect(pstSavm, t);

    if(RC_SUCC != lRegisterTable(pstSavm, pstRun, t, TYPE_MQUEUE))
    {
        shmctl(pstRun->m_shmID, IPC_RMID, NULL);
        return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：create table 
    parameters:
        pstSavm                    --stvm handle
        t                          --table 
        lRow                       --table maxrows
        bCreate                    --create type
        lType                      --table type
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lCreateTable(SATvm *pstSavm, TABLE t, size_t lRow, bool bCreate, long lType, 
            TCREATE pfCreateFunc)
{
    long    lRet;

    if(!pstSavm || lRow <= 0)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    vInitTblDef(t);
    pstSavm->tblName = t;
    switch(t)
    {
    case SYS_TVM_INDEX:
        lRet = lCreateTvmIndex();
        break;
    case SYS_TVM_FIELD:
        lRet = lCreateTvmField();
        break;
    case SYS_TVM_DOMAIN:
        lRet = lCreateTvmDomain();
        break;
    case SYS_TVM_SEQUE:
        lRet = lCreateTvmSeque();
        break;
    default:
        if(!pfCreateFunc)
            return RC_SUCC;

        lRet = pfCreateFunc(t);
        break;
    }    
    if(RC_SUCC != lRet)        return lRet;

    return _lCustomTable(pstSavm, t, lRow, bCreate, lType);
}

/*************************************************************************************************
    description：Create a sequence
    parameters:
        pstSavm                    --stvm handle
        pszSQName                  --SQL name
        uIncre                     --increment
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lCreateSeque(SATvm *pstSavm, char *pszSQName, uint uIncre)
{
    TSeque    stSeque;

    if(RC_SUCC != lInitSATvm(pstSavm, SYS_TVM_SEQUE))
        return RC_FAIL;

    conditinit(pstSavm, stSeque, SYS_TVM_SEQUE);
    strncpy(stSeque.m_szSQName, pszSQName, sizeof(stSeque.m_szSQName));
    stSeque.m_uIncrement = uIncre > 0 ? uIncre : 1;
    return lInsert(pstSavm);
}

/*************************************************************************************************
    description：Set the starting value of the sequence
    parameters:
        pstSavm                    --stvm handle
        pszSQName                  --SQL name
        uIncre                     --increment
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lSetSequence(SATvm *pstSavm, char *pszSQName, ulong uStart)
{
    TSeque  stSeque;
    SHTree  *pstTree = NULL;
    RunTime *pstRun  = NULL;
    RWLock  *prwLock = NULL;
    SHTruck *pstTruck = NULL;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    memset(&stSeque, 0, sizeof(stSeque));
    strncpy(stSeque.m_szSQName, pszSQName, sizeof(stSeque.m_szSQName));
    if(RC_SUCC != lInitSATvm(pstSavm, SYS_TVM_SEQUE))
        return RC_FAIL;

    pstSavm->lSize   = sizeof(TSeque);
    if(NULL == (pstRun = (RunTime *)pInitMemTable(pstSavm, SYS_TVM_SEQUE)))
        return RC_FAIL;

    prwLock = (RWLock *)pGetRWLock(pstRun->m_pvAddr);
    if(RC_SUCC != pthread_rwlock_wrlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DORD_ERR;
        vTblDisconnect(pstSavm, SYS_TVM_SEQUE);
        return RC_FAIL;
    }

    if(NULL == (pstTree = (SHTree *)pGetNode(pstRun->m_pvAddr, 
        ((TblDef *)pstRun->m_pvAddr)->m_lTreeRoot)))
    {
        pstSavm->m_lErrno = SVR_EXCEPTION;
        pthread_rwlock_unlock(prwLock);
        vTblDisconnect(pstSavm, SYS_TVM_SEQUE);
        return RC_FAIL;
    }

    pstTree = (SHTree *)pSearchTree(pstRun->m_pvAddr, pstTree, stSeque.m_szSQName, 
        lGetIdxLen(SYS_TVM_SEQUE));
    if(!pstTree)
    {
        pthread_rwlock_unlock(prwLock);
        vTblDisconnect(pstSavm, SYS_TVM_SEQUE);
        pstSavm->m_lErrno = SEQ_NOT_FOUND;
        return RC_FAIL;
    }

    pstTruck = (PSHTruck)pGetNode(pstRun->m_pvAddr, pstTree->m_lData);
    pstTruck->m_lTimes = uStart;
    pthread_rwlock_unlock(prwLock);
    vTblDisconnect(pstSavm, SYS_TVM_SEQUE);

    return RC_SUCC;
}

/*************************************************************************************************
    description：select seque
    parameters:
        pstSavm                    --stvm handle
        pszSQName                  --SQL name
        pulNumber                  --number
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lSelectSeque(SATvm *pstSavm, char *pszSQName, ulong *pulNumber)
{
    TSeque  stSeque;
    SHTree  *pstTree = NULL;
    RunTime *pstRun  = NULL;
    RWLock  *prwLock = NULL;
    SHTruck *pstTruck = NULL;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    memset(&stSeque, 0, sizeof(stSeque));
    strncpy(stSeque.m_szSQName, pszSQName, sizeof(stSeque.m_szSQName));
    if(RC_SUCC != lInitSATvm(pstSavm, SYS_TVM_SEQUE))
        return RC_FAIL;

    pstSavm->lSize   = sizeof(TSeque);
    if(NULL == (pstRun = (RunTime *)pInitMemTable(pstSavm, SYS_TVM_SEQUE)))
        return RC_FAIL;

    prwLock = (RWLock *)pGetRWLock(pstRun->m_pvAddr);
    if(RC_SUCC != pthread_rwlock_wrlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DORD_ERR;
        vTblDisconnect(pstSavm, SYS_TVM_SEQUE);
        return RC_FAIL;
    }

    if(NULL == (pstTree = (SHTree *)pGetNode(pstRun->m_pvAddr, 
        ((TblDef *)pstRun->m_pvAddr)->m_lTreeRoot)))
    {
        pstSavm->m_lErrno = SVR_EXCEPTION;
        pthread_rwlock_unlock(prwLock);
        vTblDisconnect(pstSavm, SYS_TVM_SEQUE);
        return RC_FAIL;
    }

    pstTree = (SHTree *)pSearchTree(pstRun->m_pvAddr, pstTree, stSeque.m_szSQName, 
        lGetIdxLen(SYS_TVM_SEQUE));
    if(!pstTree)
    {
        pthread_rwlock_unlock(prwLock);
        vTblDisconnect(pstSavm, SYS_TVM_SEQUE);
        pstSavm->m_lErrno = SEQ_NOT_FOUND;
        return RC_FAIL;
    }

    pstTruck = (PSHTruck)pGetNode(pstRun->m_pvAddr, pstTree->m_lData);
    pstTruck->m_lTimes += ((TSeque *)pstTruck->m_pvData)->m_uIncrement;
    *pulNumber = pstTruck->m_lTimes;
    pthread_rwlock_unlock(prwLock);
    vTblDisconnect(pstSavm, SYS_TVM_SEQUE);
    pstSavm->m_lEffect = 1;

    return RC_SUCC;
}

/*************************************************************************************************
    description：shut down STVM
    parameters:
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lShutdownTvm()
{
    size_t  lOut = 0;
    TIndex  *pstIndex = NULL;
    RunTime *pstRun = NULL;
    RWLock  *prwLock = NULL;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

    if(RC_SUCC != lExportTable(SYS_TVM_INDEX, &lOut, (void *)&pstIndex))
        return RC_FAIL;

    if(0 == lOut)    return RC_SUCC;

    // Remove from the back forward, root index table last deleted
    for(lOut; 0 < lOut; -- lOut)
    {
        if(RES_REMOT_SID == pstIndex[lOut - 1].m_lLocal)
            continue;

        vForceDisconnect(pstSavm, pstIndex[lOut - 1].m_table);
        pstRun = (RunTime *)pGetRunTime(pstSavm, pstIndex[lOut - 1].m_table);
        pstRun->m_shmID = pstIndex[lOut - 1].m_shmID;
        if(NULL == (pstRun = (RunTime *)pInitHitTest(pstSavm, pstIndex[lOut - 1].m_table)))
        {
            TFree(pstIndex);
            return RC_FAIL;
        }
 
        if(RES_REMOT_SID == pstRun->m_lLocal)
            continue;

        prwLock = (RWLock *)pGetRWLock(pstRun->m_pvAddr);
        pthread_rwlock_destroy(prwLock);
        vForceDisconnect(pstSavm, pstIndex[lOut - 1].m_table);

        // The table is deleted, whether successful or not
        shmctl(pstIndex[lOut - 1].m_shmID, IPC_RMID, NULL);
    }
    semctl(pstIndex[0].m_semID, 0, IPC_RMID, NULL);
    TFree(pstIndex);

    return RC_SUCC;
}

/*************************************************************************************************
    description：initial domain
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lInitDomain(SATvm *pstSavm)
{
    long    i, lCount = 0;
    TDomain *pstDomain = NULL;
    TIndex  stIndex, *pstIndex = NULL;

    if(RC_SUCC != lGetDomainIndex(pstSavm, &lCount, &pstIndex))
        return RC_FAIL;

    conditbind(pstSavm, pstIndex[i], SYS_TVM_INDEX)
    for(i = 0; i < lCount; i ++)
    {
        pstIndex[i].m_lValid = 0;
        pstIndex[i].m_lType  = TYPE_CLIENT;
        pstIndex[i].m_lLocal = RES_REMOT_SID;
        pstIndex[i].m_lState = RESOURCE_STOP; 
        pstIndex[i].m_lPers  = OPERATE_NULL;   
        strncpy(pstIndex[i].m_szTime, sGetUpdTime(), sizeof(pstIndex[i].m_szTime));

        pstSavm->pstVoid = (void *)&pstIndex[i];
        if(RC_SUCC != lInsert(pstSavm))
        {
            TFree(pstIndex);
            return RC_FAIL;
        }
    }
    lCount = 0;
    TFree(pstIndex);

    if(RC_SUCC != lGetDomainTable(pstSavm, &lCount, &pstDomain))
        return RC_FAIL;

    for(i = 0; i < lCount; i ++)
    {
        pstDomain[i].m_lStatus = RESOURCE_STOP;

        conditbind(pstSavm, pstDomain[i], SYS_TVM_DOMAIN)
        if(RC_SUCC != lInsert(pstSavm))
        {
            TFree(pstDomain);
            return RC_FAIL;
        }
    }

    lCount = 0;
    TFree(pstDomain);

    return RC_SUCC;
}

/*************************************************************************************************
    description：Get table permissions
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lGetPermit(TABLE t)
{
    TIndex  *pstIndex = NULL;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();
    long    i, lCount = 0, lPers = OPERATE_DEFAULT;

    if(RC_SUCC != lGetLocalIndex(pstSavm, &lCount, &pstIndex))
        return lPers;

    for(i = 0; i < lCount; i ++)
    {
        if(t == pstIndex[i].m_table)
        {
            lPers = pstIndex[i].m_lPers;
            break;    
        }
    }
    TFree(pstIndex);

    return lPers;
}

/*************************************************************************************************
    description：startup STVM 
    parameters:
        pstBoot                    --boot paramter
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lStartupTvm(TBoot *pstBoot)
{
    long      semID;
    TIndex    stIndex;
    RunTime   *pstRun = NULL;
    SATvm     *pstSavm = (SATvm *)pGetSATvm();

    memset(&stIndex, 0, sizeof(TIndex));
    if(RC_SUCC != _lCreateTable(pstSavm, SYS_TVM_INDEX, pstBoot->m_lMaxTable, true, 
        TYPE_SYSTEM, NULL))
        return RC_FAIL;

    pstRun = (RunTime *)pGetRunTime(pstSavm, SYS_TVM_INDEX);
    if(RC_SUCC != lCreateSems(pstSavm, pstRun, pstBoot->m_lMaxTable, SEM_INIT))
    {
        shmctl(pstRun->m_shmID, IPC_RMID, NULL);
        return RC_FAIL;
    }

    semID = pstRun->m_semID;
    stIndex.m_lValid   = 0;
    stIndex.m_semID    = semID;
    stIndex.m_lType    = TYPE_SYSTEM;
    stIndex.m_table    = SYS_TVM_INDEX;
    stIndex.m_yKey     = pstSavm->m_yKey;
    stIndex.m_shmID    = pstRun->m_shmID;
    stIndex.m_lMaxRows = pstBoot->m_lMaxTable;
    stIndex.m_lLocal   = RES_LOCAL_SID;
    stIndex.m_lState   = RESOURCE_STOP; 
    stIndex.m_lPers    = OPERATE_NULL; 
    stIndex.m_lRowSize = lGetRowSize(SYS_TVM_INDEX);
    strncpy(stIndex.m_szTime, sGetUpdTime(), sizeof(stIndex.m_szTime));
    strncpy(stIndex.m_szOwner, pstBoot->m_szNode, sizeof(stIndex.m_szOwner));
    strncpy(stIndex.m_szPart, pstBoot->m_szNode, sizeof(stIndex.m_szPart));
    strncpy(stIndex.m_szTable, sGetTableName(SYS_TVM_INDEX), sizeof(stIndex.m_szTable));
    
    /*   The table was initialized when it was created
    if(NULL == (pstSavm = (SATvm *)pInitSATvm(SYS_TVM_INDEX)))
        return RC_FAIL;
    */

    conditbind(pstSavm, stIndex, SYS_TVM_INDEX)
    if(RC_SUCC != lInsert(pstSavm))
        return RC_FAIL;

    memset(&stIndex, 0, sizeof(TIndex));
    if(RC_SUCC != _lCreateTable(pstSavm, SYS_TVM_FIELD, pstBoot->m_lMaxField, false, 
        TYPE_INCORE, NULL))
        return RC_FAIL;

    pstRun = (RunTime *)pGetRunTime(pstSavm, SYS_TVM_FIELD);
    stIndex.m_lValid   = 0;
    stIndex.m_semID    = semID;
    stIndex.m_lType    = TYPE_INCORE;
    stIndex.m_lLocal   = RES_LOCAL_SID;
    stIndex.m_table    = SYS_TVM_FIELD;
    stIndex.m_yKey     = pstSavm->m_yKey;
    stIndex.m_shmID    = pstRun->m_shmID;
    stIndex.m_lState   = RESOURCE_STOP;
    stIndex.m_lPers    = OPERATE_NULL; 
    stIndex.m_lRowSize = lGetRowSize(SYS_TVM_FIELD);
    strncpy(stIndex.m_szTime, sGetUpdTime(), sizeof(stIndex.m_szTime));
    strncpy(stIndex.m_szOwner, pstBoot->m_szNode, sizeof(stIndex.m_szOwner));
    strncpy(stIndex.m_szPart, pstBoot->m_szNode, sizeof(stIndex.m_szPart));
    strncpy(stIndex.m_szTable, sGetTableName(SYS_TVM_FIELD), sizeof(stIndex.m_szTable));
    
    conditbind(pstSavm, stIndex, SYS_TVM_INDEX)
    if(RC_SUCC != lInsert(pstSavm))
        return RC_FAIL;

    if(RC_SUCC != lInsertField(pstSavm, SYS_TVM_FIELD))
        return RC_FAIL;

    memset(&stIndex, 0, sizeof(TIndex));
    if(RC_SUCC != _lCreateTable(pstSavm, SYS_TVM_DOMAIN, pstBoot->m_lMaxDomain, false, 
        TYPE_INCORE, NULL))
        return RC_FAIL;

    pstRun = (RunTime *)pGetRunTime(pstSavm, SYS_TVM_DOMAIN);
    stIndex.m_lValid   = 0;
    stIndex.m_semID    = semID;
    stIndex.m_lType    = TYPE_INCORE;
    stIndex.m_lLocal   = RES_LOCAL_SID;
    stIndex.m_table    = SYS_TVM_DOMAIN;
    stIndex.m_yKey     = pstSavm->m_yKey;
    stIndex.m_shmID    = pstRun->m_shmID;
    stIndex.m_lState   = RESOURCE_STOP;
    stIndex.m_lPers    = OPERATE_NULL;
    stIndex.m_lRowSize = lGetRowSize(SYS_TVM_DOMAIN);
    strncpy(stIndex.m_szTime, sGetUpdTime(), sizeof(stIndex.m_szTime));
    strncpy(stIndex.m_szOwner, pstBoot->m_szNode, sizeof(stIndex.m_szOwner));
    strncpy(stIndex.m_szPart, pstBoot->m_szNode, sizeof(stIndex.m_szPart));
    strncpy(stIndex.m_szTable, sGetTableName(SYS_TVM_DOMAIN), sizeof(stIndex.m_szTable));
    
    conditbind(pstSavm, stIndex, SYS_TVM_INDEX)
    if(RC_SUCC != lInsert(pstSavm))
        return RC_FAIL;

    if(RC_SUCC != lInsertField(pstSavm, SYS_TVM_DOMAIN))
        return RC_FAIL;

    memset(&stIndex, 0, sizeof(TIndex));
    if(RC_SUCC != _lCreateTable(pstSavm, SYS_TVM_SEQUE, pstBoot->m_lMaxSeque, false, 
        TYPE_INCORE, NULL))
        return RC_FAIL;

    pstRun = (RunTime *)pGetRunTime(pstSavm, SYS_TVM_SEQUE);
    stIndex.m_lValid   = 0;
    stIndex.m_semID    = semID;
    stIndex.m_lType    = TYPE_INCORE;
    stIndex.m_lLocal   = RES_LOCAL_SID;
    stIndex.m_table    = SYS_TVM_SEQUE;
    stIndex.m_yKey     = pstSavm->m_yKey;
    stIndex.m_shmID    = pstRun->m_shmID;
    stIndex.m_lState   = RESOURCE_STOP; 
    stIndex.m_lPers    = OPERATE_NULL;   
    stIndex.m_lRowSize = lGetRowSize(SYS_TVM_SEQUE);
    strncpy(stIndex.m_szTime, sGetUpdTime(), sizeof(stIndex.m_szTime));
    strncpy(stIndex.m_szOwner, pstBoot->m_szNode, sizeof(stIndex.m_szOwner));
    strncpy(stIndex.m_szPart, pstBoot->m_szNode, sizeof(stIndex.m_szPart));
    strncpy(stIndex.m_szTable, sGetTableName(SYS_TVM_SEQUE), sizeof(stIndex.m_szTable));
    
    conditbind(pstSavm, stIndex, SYS_TVM_INDEX)
    if(RC_SUCC != lInsert(pstSavm))
        return RC_FAIL;

    if(RC_SUCC != lInsertField(pstSavm, SYS_TVM_SEQUE))
        return RC_FAIL;

    return lInitDomain(pstSavm);
}

/*************************************************************************************************
    description：API - CreateTable
    parameters:
        pstSavm                    --stvm handle
        t                          --table 
        lRow                       --table maxrows
        pfCreateFunc               --table field define
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lCreateTable(SATvm *pstSavm, TABLE t, size_t lRow, TCREATE pfCreateFunc)
{
    return _lCreateTable(pstSavm, t, lRow, false, TYPE_CLIENT, pfCreateFunc);
}

/*************************************************************************************************
    description：Custom template creation
    parameters:
        pstSavm                    --stvm handle
        t                          --table 
        lRow                       --table maxrows
        pstDef                     --table struck infor
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lCustomTable(SATvm *pstSavm, TABLE t, size_t lRow, TblDef *pstDef)
{

    memcpy((void *)pGetTblDef(t), (void *)pstDef, sizeof(TblDef));    

    return _lCustomTable(pstSavm, t, lRow, false, TYPE_CLIENT);
}

/*************************************************************************************************
    description：API - CreateQueue
    parameters:
        pstSavm                    --stvm handle
        t                          --table 
        lRow                       --table maxrows
        pfCreateFunc               --table field define
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lCircleQueue(SATvm *pstSavm, TABLE t, size_t lRow, size_t lSize, char *pszTable, char *node)
{
    return _lCreateQueue(pstSavm, t, lRow, lSize, pszTable, node, false);
}

/*************************************************************************************************
    description：API - lDropTable
    parameters:
        pstSavm                    --stvm handle
        t                          --table 
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lDropTable(SATvm *pstSavm, TABLE t)
{
    TIndex    stIndex;
    TField    stField;
    RunTime   *pstRun = NULL;
    RWLock    *prwLock = NULL;

    pstSavm->bSearch = TYPE_SYSTEM;
    conditinit(pstSavm, stIndex, SYS_TVM_INDEX)
    conditnum(pstSavm, stIndex, m_table, t)
    if(RC_SUCC != lSelect(pstSavm, (void *)&stIndex))
        return RC_FAIL;

    pstRun = (RunTime *)pGetRunTime(pstSavm, t);
    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        conditinit(pstSavm, stIndex, SYS_TVM_INDEX)
        conditnum(pstSavm, stIndex, m_table, t)
        if(RC_SUCC != lDelete(pstSavm))
            return RC_FAIL;

        memset(pstRun, 0, sizeof(RunTime));
        _vDropTableByRt(pstSavm, t);
        pstRun->m_lState = RESOURCE_INIT;
        return RC_SUCC;
    }

    vForceDisconnect(pstSavm, t);
    pstRun->m_shmID = stIndex.m_shmID;
    if(NULL == (pstRun = (RunTime *)pInitHitTest(pstSavm, t)))
        return RC_FAIL;

    prwLock = (RWLock *)pGetRWLock(pstRun->m_pvAddr);
    pthread_rwlock_destroy(prwLock);
    vForceDisconnect(pstSavm, t);

    // The table is deleted, whether successful or not
    shmctl(stIndex.m_shmID, IPC_RMID, NULL);
    semctl(stIndex.m_semID, 0, IPC_RMID, 0);

    conditinit(pstSavm, stIndex, SYS_TVM_INDEX)
    conditnum(pstSavm, stIndex, m_table, t)
    if(RC_SUCC != lDelete(pstSavm))    return RC_FAIL;

    if(TYPE_MQUEUE == pstRun->m_lType)
    {
        memset(pstRun, 0, sizeof(RunTime));
        pstSavm->m_lEffect = 1;
        return RC_SUCC;
    }

    // Delete the field table
    if(RC_SUCC != lInitSATvm(pstSavm, SYS_TVM_FIELD))
        return RC_FAIL;

    conditinit(pstSavm, stField, SYS_TVM_FIELD)
    conditnum(pstSavm, stField, m_table, t)
    if(RC_SUCC != lDelete(pstSavm))    return RC_FAIL;

    memset(pstRun, 0, sizeof(RunTime));
    pstSavm->m_lEffect = 1;
    return RC_SUCC;
}

/*************************************************************************************************
    description：Format the data content for export
    parameters:
        fp                         --File descriptor
        s                          --data record 
        lIdx                       --field number
        pstIdx                     --field key
        f                          --file descriptor
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lExportContext(FILE *fp, char *s, long lIdx, TblKey *pstIdx, char *f)
{
    long    i = 0;

    if(!fp || !s || !pstIdx)    return RC_FAIL;

    for(i = 0; i < lIdx; i ++)
    {
        switch(pstIdx[i].m_lAttr)
        {
        case FIELD_DOUB:
            switch(pstIdx[i].m_lLen)
            {
            case    4:
                fprintf(fp, "%.6f", *((float *)(s + pstIdx[i].m_lFrom)));
                break;
            case    8:
                fprintf(fp, "%.6f", *((double *)(s + pstIdx[i].m_lFrom)));
                break;
            default:
                break;
            }
            break;
        case FIELD_LONG:
            switch(pstIdx[i].m_lLen)
            {
            case    2:
                fprintf(fp, "%d", *((sint *)(s + pstIdx[i].m_lFrom)));
                break;
            case    4:
                fprintf(fp, "%d", *((int *)(s + pstIdx[i].m_lFrom)));
                break;
            case    8:
                fprintf(fp, "%lld", *((llong *)(s + pstIdx[i].m_lFrom)));
                break;
            default:
                break;
            }
            break;
        case FIELD_CHAR:
            fprintf(fp, "%.*s", (int)pstIdx[i].m_lLen, s + pstIdx[i].m_lFrom);
            break;
        default:
            fprintf(stderr, "Export field attribute exception\n");
            break;
        }
        fprintf(fp, "%s", f);
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：Format the data content into the import
    parameters:
        s                          --data line 
        lIdx                       --field number
        pstIdx                     --field key
        pvOut                      --format to row record
        f                          --file descriptor
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lImportContext(char *s, long lIdx, TblKey *pstIdx, char *pvOut, char *f)
{
    void    *p = NULL;
    register long i;

    for(i = 0; i < lIdx; i ++) 
    {
        p = sgetvalue(s, f, i + 1);

        switch(pstIdx[i].m_lAttr)
        {
        case FIELD_DOUB:
            switch(pstIdx[i].m_lLen)
            {
            case    4:
                *((float *)(pvOut + pstIdx[i].m_lFrom)) = atof(p);
                break;
            case    8:
                *((double *)(pvOut + pstIdx[i].m_lFrom)) = atof(p);
                break;
            default:
                break;
            }
            break;
        case FIELD_LONG:
            switch(pstIdx[i].m_lLen)
            {
            case    2:
                *((sint *)(pvOut + pstIdx[i].m_lFrom)) = atoi(p);
                break;
            case    4:
                *((int *)(pvOut + pstIdx[i].m_lFrom)) = atoi(p);
                break;
            case    8:
                *((llong *)(pvOut + pstIdx[i].m_lFrom)) = atol(p);
                break;
            default:
                break;
            }
            break;
        case FIELD_CHAR:
            memcpy(pvOut + pstIdx[i].m_lFrom, p, pstIdx[i].m_lLen);
            break;
        default:
            break;
        }
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：API - CreateTable
    parameters:
        pstSavm                    --stvm handle
        pszFile                    --file name
        pszFlag                    --separator
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lImportFile(TABLE t, char *pszFile, char *pszFlag)
{
    FILE    *fp = NULL;
    char    szLine[1024];
    RunTime *pstRun = NULL;    
    void    *pvData = NULL;
    RWLock  *prwLock = NULL;
    long    lEffect = 0, lRet = RC_SUCC;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

    if(!pszFile || !pszFlag || !strlen(pszFlag))
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    memset(szLine, 0, sizeof(szLine));
    if(NULL == (pstSavm = (SATvm *)pInitSATvm(t)))
        return RC_FAIL;

    if(NULL == (pstRun = (RunTime *)pInitHitTest(pstSavm, pstSavm->tblName)))
        return RC_FAIL;

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        pstSavm->m_lErrno = RMT_NOT_SUPPT;
        return RC_FAIL;
    }

    if(NULL == (fp = fopen(pszFile, "rb")))
    {
        pstSavm->m_lErrno = FIL_NOT_EXIST;
        return RC_FAIL;
    }

    if(NULL == (pvData = (void *)malloc(lGetRowSize(t))))
    {
        fclose(fp);
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    pstSavm->pstVoid = pvData;
    pstSavm->lSize   = lGetRowSize(t);
    prwLock = (RWLock *)pGetRWLock(pstRun->m_pvAddr);
    if(RC_SUCC != pthread_rwlock_wrlock(prwLock))
    {
        fclose(fp);
        TFree(pvData);
        pstSavm->m_lErrno = LOCK_DOWR_ERR;
        return RC_FAIL;
    }

    while(fgets(szLine, sizeof(szLine), fp))
    {
        strimcrlf(szLine);
        if(!strlen(szLine))
            continue;

        if(lGetTblRow(t) <= ((TblDef *)pstRun->m_pvAddr)->m_lValid)
        {
            lRet = RC_FAIL;
            pstSavm->m_lErrno = DATA_SPC_FULL;
            break;
        }

        memset(pvData, 0, lGetRowSize(t));
        _lImportContext(szLine, lGetFldNum(t), pGetTblKey(t), pvData, pszFlag);
        if(RC_SUCC != (lRet = __lInsert(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, 0)))
            break;

        lEffect ++;
        memset(szLine, 0, sizeof(szLine));
        if(RC_SUCC != lRecordWork(pstSavm, pstSavm->pstVoid, OPERATE_INSERT))
            break;
    }
    fclose(fp);
    pthread_rwlock_unlock(prwLock);
    pstSavm->m_lEffect = lEffect;
    vTblDisconnect(pstSavm, pstSavm->tblName);

    TFree(pvData);
    return lRet;
}

/*************************************************************************************************
    description：Export the data from the memory table
    parameters:
        t                          --table 
        plOut                      --plOut
        ppsvOut                    --result data list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lExportTable(TABLE t, size_t *plOut, void **ppsvOut)
{
    long    lRet;
    RunTime *pstRun = NULL;
    SATvm   *pstSavm = NULL;

    if(NULL == (pstSavm = (SATvm *)pInitSATvm(t)))
        return RC_FAIL;

    if(NULL == (pstRun = (RunTime *)pInitHitTest(pstSavm, t)))
        return RC_FAIL;

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        pstSavm->m_lErrno = RMT_NOT_SUPPT;
        return RC_FAIL;
    }

    pstSavm->pstVoid = NULL;
    lRet = _lQueryTruck(pstSavm, pstRun->m_pvAddr, t, plOut, ppsvOut);
    vTblDisconnect(pstSavm, pstSavm->tblName);
    if(NO_DATA_FOUND == pstSavm->m_lErrno)
        return RC_SUCC;

    return lRet;
}

/*************************************************************************************************
    description：Import the data into the memory table
    parameters:
        t                          --table 
        lCount                     --count
        psvData                    --record list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lImportTable(TABLE t, size_t lCount, void *psvData)
{
    RunTime *pstRun = NULL;
    SATvm   *pstSavm = NULL;
    RWLock  *prwLock = NULL;
    long    i, lRet = RC_SUCC;

    if(!psvData || lCount < 0)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    if(NULL == (pstSavm = (SATvm *)pInitSATvm(t)))
        return RC_FAIL;

    if(NULL == (pstRun = (RunTime *)pInitHitTest(pstSavm, t)))
        return RC_FAIL;

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        pstSavm->m_lErrno = RMT_NOT_SUPPT;
        return RC_FAIL;
    }

    prwLock = (RWLock *)pGetRWLock(pstRun->m_pvAddr);
    if(RC_SUCC != pthread_rwlock_wrlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DOWR_ERR;
        return RC_FAIL;
    }

    for(i = 0; i < lCount; i ++)
    {
        pstSavm->pstVoid = psvData + lGetRowSize(t) * i;
        if(RC_SUCC != (lRet = __lInsert(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, 0)))
            break;
    }

    pthread_rwlock_unlock(prwLock);
    vTblDisconnect(pstSavm, pstSavm->tblName);

    return lRet;
}

/*************************************************************************************************
    description：Export the memory table to a file
    parameters:
        t                          --table 
        pszFile                    --file name
        pszFlag                    --separator
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lExportFile(TABLE t, char *pszFile, char *pszFlag)
{
    FILE    *fp = NULL;
    char    szLine[1024];
    void    *psvOut = NULL;
    RunTime *pstRun = NULL;
    SATvm   *pstSavm = NULL;

    if(!pszFile || !pszFlag || !strlen(pszFlag))
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    memset(szLine, 0, sizeof(szLine));
    if(NULL == (pstSavm = (SATvm *)pInitSATvm(t)))
        return RC_FAIL;

    if(NULL == (fp = fopen(pszFile, "wb")))
    {
        pstSavm->m_lErrno = FIL_NOT_EXIST;
        return RC_FAIL;
    }

    pstSavm->pstVoid   = NULL;
    if(NULL == (pstRun = (RunTime *)pInitHitTest(pstSavm, t)))
    {
        fclose(fp);
        return RC_FAIL;
    }

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        pstSavm->m_lErrno = RMT_NOT_SUPPT;
        return RC_FAIL;
    }

    if(NULL == (psvOut = (char *)malloc(lGetRowSize(t))))
    {
        fclose(fp);
        vTblDisconnect(pstSavm, pstSavm->tblName);
        return RC_FAIL;
    }

    pstRun->m_lCurLine = 1;
    pstRun->m_lCurType = EXE_PLAN_ALL;
    pstRun->m_pvCurAddr = pstRun->m_pvAddr;    
    while(RC_NOTFOUND != _llFetchTruck(pstSavm, pstRun, t, psvOut))
    {
        _lExportContext(fp, psvOut, lGetFldNum(t), pGetTblKey(t), pszFlag);
        fprintf(fp, "\n");
    }

    fclose(fp);
    TFree(psvOut);
    vTblDisconnect(pstSavm, pstSavm->tblName);
    return RC_SUCC;
}

/*************************************************************************************************
    description：Get the table field
    parameters:
        t                          --table 
        plOut                      --number
        ppstField                  --out field list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lGetTblField(TABLE t, size_t *plOut, TField **ppstField)
{   
    TField  stField; 
    SATvm   *pstSavm = NULL; 
    
    if(NULL == (pstSavm = (SATvm *)pInitSATvm(SYS_TVM_FIELD)))
        return RC_FAIL;
    
    conditinit(pstSavm, stField, SYS_TVM_FIELD)
    conditnum(pstSavm, stField, m_table, t)
    if(RC_SUCC != lQuery(pstSavm, plOut, (void **)ppstField))
    {
        if(NO_DATA_FOUND == pstSavm->m_lErrno)
            pstSavm->m_lErrno = FIELD_NOT_DEF;
        return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：Get the table properties
    parameters:
        pstSavm                    --table 
        pstTable                   --table name
        pszPart                    --table part
        pszIndex                   --result index
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lGetTblIndex(SATvm *pstSavm, char *pszTable, char *pszPart, TIndex *pstIndex)
{
    TIndex    stIndex;

    if(!pszTable || !pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstSavm->bSearch = TYPE_SYSTEM;
    conditinit(pstSavm, stIndex, SYS_TVM_INDEX);
    conditstr(pstSavm, stIndex, m_szPart, pszPart);
    conditstr(pstSavm, stIndex, m_szTable, pszTable);
    conditnum(pstSavm, stIndex, m_lLocal, RES_LOCAL_SID);
    if(RC_SUCC != lSelect(pstSavm, (void *)pstIndex))
    {
        if(NO_DATA_FOUND == pstSavm->m_lErrno)
            pstSavm->m_lErrno = TBL_NOT_FOUND;
        return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：Update the table partition 
    parameters:
        pstSavm                    --stvm handle 
        t                          --table
        pszPart                    --table part
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lUpdIndexPart(SATvm *pstSavm, TABLE t, char *pszPart)
{
    TIndex    stIndex, stUpdate;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstSavm->bSearch = TYPE_SYSTEM;
    updateinit(pstSavm, stUpdate);
    conditinit(pstSavm, stIndex, SYS_TVM_INDEX);
    conditnum(pstSavm, stIndex, m_table, t)
    conditnum(pstSavm, stIndex, m_lLocal, RES_LOCAL_SID);

    updatestr(pstSavm, stUpdate, m_szPart, pszPart);
    return lUpdate(pstSavm, &stUpdate);
}

/*************************************************************************************************
    description：Does the table exist
    parameters:
        t                          --table
    return:
        true                       --success
        false                      --failure
 *************************************************************************************************/
bool    bTableIsExist(TABLE t)
{
    TIndex  stIndex;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

    pstSavm->bSearch = TYPE_SYSTEM;
    conditinit(pstSavm, stIndex, SYS_TVM_INDEX)
    conditnum(pstSavm, stIndex, m_table, t)
    if(RC_SUCC != lSelect(pstSavm, (void *)&stIndex))
        return false;

    return true;
}

/*************************************************************************************************
    description：API - renametable
    parameters:
        pstSavm                    --stvm handle 
        to                          --old table
        tn                          --new table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lRenameTable(SATvm *pstSavm, TABLE to, TABLE tn)
{
    RunTime *pstRun = NULL;
    TIndex  stIndex, stNIdx;
    TField  stField, stNFld;

    pstSavm->bSearch = TYPE_SYSTEM;
    updateinit(pstSavm, stNIdx);
    conditinit(pstSavm, stIndex, SYS_TVM_INDEX);
    conditnum(pstSavm, stIndex, m_table, to);
    updatenum(pstSavm, stNIdx, m_table, tn);

    if(RC_SUCC != lUpdate(pstSavm, &stNIdx))
        return RC_FAIL;

    _lRenameTableByRt(pstSavm, to, tn);

    memcpy((void *)pGetRunTime(pstSavm, tn), (void *)pGetRunTime(pstSavm, to), sizeof(RunTime));
    if(RC_SUCC != lInitSATvm(pstSavm, tn))
        return RC_FAIL;

    if(NULL == (pstRun = (RunTime *)pInitHitTest(pstSavm, pstSavm->tblName)))
        return RC_FAIL;

    if(RES_REMOT_SID == pstRun->m_lLocal)
        return RC_SUCC; 

    ((TblDef *)pstRun->m_pvAddr)->m_table = tn;
    vTblDisconnect(pstSavm, pstSavm->tblName);
    memset((void *)pGetRunTime(pstSavm, to), 0, sizeof(RunTime)); 
    if(RC_SUCC != lInitSATvm(pstSavm, SYS_TVM_FIELD))
        return RC_FAIL;

    updateinit(pstSavm, stNFld);
    conditinit(pstSavm, stField, SYS_TVM_FIELD);
    conditnum(pstSavm, stField, m_table, to);
    updatenum(pstSavm, stNFld, m_table, tn);
    return lUpdate(pstSavm, &stNFld);
}

/*************************************************************************************************
    description：Does the table part exist
    parameters:
        t                          --table
    return:
        true                       --success
        false                      --failure
 *************************************************************************************************/
bool    bPartIsExist(char *pszTable, char *pszPart)
{
    TIndex  stIndex;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

    pstSavm->bSearch = TYPE_SYSTEM;
    conditinit(pstSavm, stIndex, SYS_TVM_INDEX)
    conditstr(pstSavm, stIndex, m_szPart, pszPart)
    conditstr(pstSavm, stIndex, m_szTable, pszTable)
    conditnum(pstSavm, stIndex, m_lLocal, RES_LOCAL_SID)
    if(RC_SUCC != lSelect(pstSavm, (void *)&stIndex))
    {
        if(MORE_ROWS_SEL == pstSavm->m_lErrno)
            return true;

        return false;
    }

    return true;
}

/*************************************************************************************************
    description：Gets the table maxrows
    parameters:
        pstSavm                    --stvm handle 
        t                          --table
    return:
        true                       --success
        false                      --failure
 *************************************************************************************************/
long    lTableMaxRow(SATvm *pstSavm, TABLE t)
{
    TIndex  stIndex;
    
    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstSavm->bSearch = TYPE_SYSTEM;
    conditinit(pstSavm, stIndex, SYS_TVM_INDEX)
    conditnum(pstSavm, stIndex, m_table, t)
    if(RC_SUCC != lSelect(pstSavm, (void *)&stIndex))
        return RC_FAIL;

    return stIndex.m_lMaxRows;
}

/*************************************************************************************************
    description：Whether or not the STVM is started
    parameters:
        t                          --table
    return:
        true                       --yes
        false                      --failed
 *************************************************************************************************/
bool    bIsTvmBoot()
{
    RunTime *pstRun = NULL;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

    pstSavm->bSearch = TYPE_SYSTEM;
    pstSavm->tblName = SYS_TVM_INDEX;
    pstSavm->lSize   = sizeof(TIndex);

    pstRun = (RunTime *)pGetRunTime(pstSavm, SYS_TVM_INDEX);
    pstRun->m_lLocal = RES_LOCAL_SID;

    if(NULL == (pstRun = (RunTime *)pInitMemTable(pstSavm, pstSavm->tblName)))
        return false;

    vTblDisconnect(pstSavm, pstSavm->tblName);
    return true;
}

/*************************************************************************************************
    description：Returns the row record of the field's extreme value by index
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        pFdKey                     --field key
        psvOut                     --result data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lExtremGroup(SATvm *pstSavm, void *pvAddr, TABLE t, FdKey *pFdKey, void *psvOut)
{
    TblKey  *pstIdx;
    void    *pvData = NULL;
    SHList  *pstList = NULL;
    SHTree  *pstTree = NULL;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN];
    TblKey  *pstKey = pGetTblGrp(t);

    if(NULL == (pstIdx = pGetFldKey(t, pFdKey->uFldpos, pFdKey->uFldlen)))
    {
        pstSavm->m_lErrno = FLD_NOT_EXIST;
        return RC_FAIL;
    }

    if(NULL == (pstTree = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lGroupRoot)))
    {
         pstSavm->m_lErrno = SVR_EXCEPTION;
         return RC_FAIL;
    }

    if(NULL == pstSavm->pstVoid && 1 == lGetGrpNum(t) && pstKey[0].m_lFrom == pFdKey->uFldpos
        && pstKey[0].m_lAttr == FIELD_CHAR)
    {
        pstSavm->m_lEType = EXE_PLAN_GRP;
        if(NULL == (pstTree = (SHTree *)pExtremeTree(pFdKey->uDecorate, pvAddr, pstTree)))
        {
            pstSavm->m_lErrno = NO_DATA_FOUND;
            return RC_FAIL;
        }

        if(NULL == (pstList = (SHList *)pGetNode(pvData, pstTree->m_lData)))
        {
            pstSavm->m_lErrno = NO_DATA_FOUND;
            return RC_FAIL;
        }

        memcpy(psvOut + pFdKey->uFldpos, pstTree->m_szIdx, pFdKey->uFldlen);
        return RC_SUCC;
    }

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetGrpNum(t), pGetTblGrp(t), 
        pstSavm->pstVoid, szIdx))
        return RC_CONTU;
    
    if(NULL == (pstList = pSearchGroup(pvAddr, pstTree, szIdx, lGetGrpLen(t))))
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_CONTU;
    }

    for(pstSavm->m_lEType = EXE_PLAN_GRP; SELF_POS_UNUSE != pstList->m_lPos; 
        pstList = (SHList *)pGetNode(pvAddr, pstList->m_lNext))
    {
        pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
        if(RC_MATCH == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
            pvData = pvCompExtrem(pstTruck->m_pvData, pvData, pstIdx, pFdKey->uDecorate);
    
        if(SELF_POS_UNUSE == pstList->m_lNext)    break;
    }

    if(!pvData)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    pstSavm->m_lEffect = 1;
    memcpy(psvOut + pFdKey->uFldpos, pvData + pFdKey->uFldpos, pFdKey->uFldlen);
    return RC_SUCC;
}   

/*************************************************************************************************
    description：Returns the row record of the field's extreme value by hash
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        pFdKey                     --field key
        psvOut                     --result data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lExtremeHash(SATvm *pstSavm, void *pvAddr, TABLE t, FdKey *pFdKey, void *psvOut)
{
    TblKey  *pstIdx;
    size_t  lData, lIdx;
    SHTree  *pstTree = NULL;
    SHList  *pstList = NULL;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN], *pvData = NULL;

    if(NULL == (pstIdx = pGetFldKey(t, pFdKey->uFldpos, pFdKey->uFldlen)))
    {
        pstSavm->m_lErrno = FLD_NOT_EXIST;
        return RC_FAIL;
    }

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetGrpNum(t), pGetTblGrp(t), 
        pstSavm->pstVoid, szIdx))
        return RC_CONTU;

    lIdx = uGetHash(szIdx, lGetGrpLen(t)) % ((TblDef *)pvAddr)->m_lMaxRow;
    pstTree = pvAddr + ((TblDef *)pvAddr)->m_lGroupRoot + lIdx * sizeof(SHTree);
    if(NULL == pstTree || SELF_POS_UNUSE == pstTree->m_lData)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    pstList = (SHList *)pGetNode(pvAddr, pstTree->m_lData);
    for(pstSavm->m_lEType = EXE_PLAN_GRP; SELF_POS_UNUSE != pstList->m_lPos; 
        pstList = (SHList *)pGetNode(pvAddr, pstList->m_lNext))
    {
        pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
        if(RC_MATCH == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
            pvData = pvCompExtrem(pstTruck->m_pvData, pvData, pstIdx, pFdKey->uDecorate);

        if(SELF_POS_UNUSE == pstList->m_lNext)    break;
    }

    if(!pvData)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    pstSavm->m_lEffect = 1;
    memcpy(psvOut + pFdKey->uFldpos, pvData + pFdKey->uFldpos, pFdKey->uFldlen);
    return RC_SUCC;
}

/*************************************************************************************************
    description：Returns the row record of the field's extreme value by truck
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        pFdKey                     --field key
        psvOut                     --result data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lExtremeTruck(SATvm *pstSavm, void *pvAddr, TABLE t, FdKey *pFdKey, void *psvOut)
{
    TblKey  *pstIdx;
    void    *pvData = NULL;
    SHTruck *pstTruck = NULL;
    size_t  lRow = 0, lOffset = lGetTblData(t);

    if(NULL == (pstIdx = pGetFldKey(t, pFdKey->uFldpos, pFdKey->uFldlen)))
    {
        pstSavm->m_lErrno = FLD_NOT_EXIST;
        return RC_FAIL;
    }

    pstSavm->m_lEffect = 0;
    pstSavm->m_lEType = EXE_PLAN_ALL;
    pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset);
    for(lRow = 0; (lRow < ((TblDef *)pvAddr)->m_lValid) && (lOffset < lGetTableSize(t)); 
        pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset))
    {        
        if(IS_TRUCK_NULL(pstTruck))
        {
            lOffset += lGetRowTruck(t);
            continue;
        }

        lRow ++;
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
        {
            lOffset += lGetRowTruck(t);
            continue;
        }
        
        lOffset += lGetRowTruck(t);
        pvData = pvCompExtrem(pstTruck->m_pvData, pvData, pstIdx, pFdKey->uDecorate);
    }

    if(!pvData)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    pstSavm->m_lEffect = 1;
    memcpy(psvOut + pFdKey->uFldpos, pvData + pFdKey->uFldpos, pFdKey->uFldlen);
    return RC_SUCC;
}

/*************************************************************************************************
    description：Returns the row record of the field's extreme value
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        pFdKey                     --field key
        psvOut                     --result data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lExtremIndex(SATvm *pstSavm, void *pvAddr, TABLE t, FdKey *pFdKey, void *psvOut)
{
    SHTree  *pstTree = NULL;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN];
    TblKey  *pstKey = pGetTblIdx(t);

    if(NULL == (pstTree = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lTreeRoot)))
    {
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    if(NULL == pstSavm->pstVoid && 1 == lGetIdxNum(t) && pstKey[0].m_lFrom == pFdKey->uFldpos
        && pstKey[0].m_lAttr == FIELD_CHAR)
    {
        pstSavm->m_lEType = EXE_PLAN_IDX;
        pstTree = (SHTree *)pExtremeTree(pFdKey->uDecorate, pvAddr, pstTree);
    }    
    else
    {
        memset(szIdx, 0, sizeof(szIdx));
        if(NULL == pGetIndex(&pstSavm->stCond, lGetIdxNum(t), pGetTblIdx(t), 
            pstSavm->pstVoid, szIdx))
            return RC_CONTU;
        
        pstSavm->m_lEType = EXE_PLAN_IDX;
        pstTree = (SHTree *)pSearchTree(pvAddr, pstTree, szIdx, lGetIdxLen(t));
    }
    if(!pstTree)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    pstTruck = (PSHTruck)pGetNode(pvAddr, pstTree->m_lData);
    memcpy(psvOut + pFdKey->uFldpos, pstTruck->m_pvData + pFdKey->uFldpos, pFdKey->uFldlen);
    pstSavm->m_lEffect = 1;
    return RC_SUCC;
}

/*************************************************************************************************
    description：Returns extreme value 
    parameters:
        pstSavm                    --stvm handle
        pstRun                     --table handle
        pFdKey                     --field key
        psvOut                     --result data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lExtreme(SATvm *pstSavm, RunTime *pstRun, FdKey *pFdKey, void *psvOut)
{
    long    lRet;
    RWLock  *prwLock = (RWLock *)pGetRWLock(pstRun->m_pvAddr);

    if(MATCH_MAX != pFdKey->uDecorate && MATCH_MIN != pFdKey->uDecorate)
    {
        pstSavm->m_lErrno = EXTRE_SET_ERR;
        return RC_FAIL;
    }

    if(RC_SUCC != pthread_rwlock_rdlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DORD_ERR;
        return RC_FAIL;
    }

    if(HAVE_UNIQ_IDX(pstSavm->tblName))
    {
        lRet = _lExtremIndex(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, pFdKey, psvOut);
        if(RC_CONTU != lRet)
        {
            pthread_rwlock_unlock(prwLock);
            return lRet;
        }
    }

    if(HAVE_NORL_IDX(pstSavm->tblName))
    {
        lRet = _lExtremGroup(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, pFdKey, psvOut);
        if(RC_CONTU != lRet)
        {
            pthread_rwlock_unlock(prwLock);
            return lRet;
        }
    }
    else if(HAVE_HASH_IDX(pstSavm->tblName))
    {
        lRet = _lExtremeHash(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, pFdKey, psvOut);
        if(RC_CONTU != lRet)
        {
            pthread_rwlock_unlock(prwLock);
            return lRet;
        }
    }

    lRet = _lExtremeTruck(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, pFdKey, psvOut);
    pthread_rwlock_unlock(prwLock);
    return lRet;
}

/*************************************************************************************************
    description：API - extreme
    parameters:
        pstSavm                    --stvm handle
        psvOut                     --result data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lExtreme(SATvm *pstSavm, void *psvOut)
{
    uint    i;
    FdCond  *pstExm;
    RunTime *pstRun = NULL;

    if(!pstSavm || NULL == (pstExm = &pstSavm->stUpdt))
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    if(0 == pstExm->uFldcmp)
    {
        pstSavm->m_lErrno = FIELD_NOT_SET;
        return RC_FAIL;
    }

    if(NULL == (pstRun = (RunTime *)pInitMemTable(pstSavm, pstSavm->tblName)))
        return RC_FAIL;

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        Tremohold(pstSavm, pstRun);
        return _lExtremeByRt(pstSavm, psvOut);
    }

    for(i = 0; i < pstExm->uFldcmp; i ++)
    {
        if(RC_FAIL == _lExtreme(pstSavm, pstRun, &pstExm->stFdKey[i], psvOut))
        {
            vTblDisconnect(pstSavm, pstSavm->tblName);
            return RC_FAIL;
        }
    }

    vTblDisconnect(pstSavm, pstSavm->tblName);
    return RC_SUCC;
}

/*************************************************************************************************
    description：Select the table hits according to the Unique index
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        puHits                     --offset 
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lClickIndex(SATvm *pstSavm, void *pvAddr, TABLE t, ulong *puHits)
{
    SHTree  *pstTree = NULL;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN];
    RWLock  *prwLock = (RWLock *)pGetRWLock(pvAddr);

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetIdxNum(t), pGetTblIdx(t), pstSavm->pstVoid, szIdx))
        return RC_CONTU;

    if(RC_SUCC != pthread_rwlock_rdlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DORD_ERR;
        return RC_FAIL;
    }

    pstSavm->m_lEType = EXE_PLAN_IDX;
    if(NULL == (pstTree = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lTreeRoot)))
    {   
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    pstTree = (SHTree *)pSearchTree(pvAddr, pstTree, szIdx, lGetIdxLen(pstSavm->tblName));
    if(!pstTree)
    {
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    pstTruck = (PSHTruck)pGetNode(pvAddr, pstTree->m_lData);
    if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
    {
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    pthread_rwlock_unlock(prwLock);
    *puHits = pstTruck->m_lTimes;
    pstSavm->m_lEffect = 1;

    return RC_SUCC;
}

/*************************************************************************************************
    description：Select the table hits according to the index
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        puHits                     --offset 
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lClickGroup(SATvm *pstSavm, void *pvAddr, TABLE t, ulong *puHits)
{
    void    *pvData = NULL;
    SHList  *pstList = NULL;
    SHTree  *pstTree = NULL;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN];
    RWLock  *prwLock = (RWLock *)pGetRWLock(pvAddr);

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetGrpNum(t), pGetTblGrp(t), pstSavm->pstVoid, szIdx))
        return RC_CONTU;

    if(RC_SUCC != pthread_rwlock_rdlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DORD_ERR;
        return RC_FAIL;
    }

    if(NULL == (pstTree = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lGroupRoot)))
    {   
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    if(NULL == (pstList = pSearchGroup(pvAddr, pstTree, szIdx, lGetGrpLen(t))))
    {
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_CONTU;
    }

    for(pstSavm->m_lEType = EXE_PLAN_GRP; SELF_POS_UNUSE != pstList->m_lPos; 
        pstList = (SHList *)pGetNode(pvAddr, pstList->m_lNext))
    {
        pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
        if(RC_MATCH == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
        {
            if(1 < (++ pstSavm->m_lEffect))
            {
                pthread_rwlock_unlock(prwLock);
                pstSavm->m_lErrno = MORE_ROWS_SEL;
                return RC_FAIL;
            }

            pvData  = pstTruck->m_pvData;
            *puHits = pstTruck->m_lTimes;
            if(FIRST_ROW & pstSavm->lFind)    break;
        }

        if(SELF_POS_UNUSE == pstList->m_lNext)   break;
    }

    if(0 == pstSavm->m_lEffect)
    {
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    pthread_rwlock_unlock(prwLock);

    return RC_SUCC;
}

/*************************************************************************************************
    description：Select the table hits according to the hash
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        puHits                     --offset 
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lClickHash(SATvm *pstSavm, void *pvAddr, TABLE t, ulong *puHits)
{
    size_t  lData, lIdx;
    SHTree  *pstTree = NULL;
    SHList  *pstList = NULL;
    SHTruck *pstTruck = NULL;
    char    szIdx[MAX_INDEX_LEN], *pvData = NULL;
    RWLock  *prwLock = (RWLock *)pGetRWLock(pvAddr);

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetGrpNum(t), pGetTblGrp(t), pstSavm->pstVoid, szIdx))
        return RC_CONTU;

    lIdx = uGetHash(szIdx, lGetGrpLen(t)) % ((TblDef *)pvAddr)->m_lMaxRow;
    pstTree = pvAddr + ((TblDef *)pvAddr)->m_lGroupRoot + lIdx * sizeof(SHTree);
    if(NULL == pstTree || SELF_POS_UNUSE == pstTree->m_lData)
    {
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    if(RC_SUCC != pthread_rwlock_rdlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DORD_ERR;
        return RC_FAIL;
    }

    pstList = (SHList *)pGetNode(pvAddr, pstTree->m_lData);
    for(pstSavm->m_lEType = EXE_PLAN_GRP; SELF_POS_UNUSE != pstList->m_lPos; 
        pstList = (SHList *)pGetNode(pvAddr, pstList->m_lNext))
    {
        pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
        if(RC_MATCH == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
        {
            if(1 < (++ pstSavm->m_lEffect))
            {
                pthread_rwlock_unlock(prwLock);
                pstSavm->m_lErrno = MORE_ROWS_SEL;
                return RC_FAIL;
            }

            pvData  = pstTruck->m_pvData;
            *puHits = pstTruck->m_lTimes;
            if(FIRST_ROW & pstSavm->lFind)    break;
        }

        if(SELF_POS_UNUSE == pstList->m_lNext)   break;
    }

    if(0 == pstSavm->m_lEffect)
    {
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    pthread_rwlock_unlock(prwLock);
    return RC_SUCC;
}

/*************************************************************************************************
    description：Select the table hits according to the truck
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        puHits                     --offset 
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lClickTruck(SATvm *pstSavm, void *pvAddr, TABLE t, ulong *puHits)
{
    void    *pvData = NULL;
    SHTruck *pstTruck = NULL;
    size_t  lRow = 0, lOffset = lGetTblData(t);
    RWLock  *prwLock = (RWLock *)pGetRWLock(pvAddr);

    if(RC_SUCC != pthread_rwlock_rdlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DORD_ERR;
        return RC_FAIL;
    }

    pstSavm->m_lEType = EXE_PLAN_ALL;
    pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset);
    for(lRow = 0; (lRow < ((TblDef *)pvAddr)->m_lValid) && (lOffset < lGetTableSize(t)); 
        pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset))
    {        
        if(IS_TRUCK_NULL(pstTruck))
        {
            lOffset += lGetRowTruck(t);
            continue;
        }

        lRow ++;
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
        {
            lOffset += lGetRowTruck(t);
            continue;
        }
        
        if(1 < (++ pstSavm->m_lEffect))
        {
            pthread_rwlock_unlock(prwLock);
            pstSavm->m_lErrno = MORE_ROWS_SEL;
            return RC_FAIL;
        }

        lOffset+= lGetRowTruck(t);
        *puHits = pstTruck->m_lTimes;
        pvData  = pstTruck->m_pvData;
        if(FIRST_ROW & pstSavm->lFind)   break;
    }

    if(0 == pstSavm->m_lEffect)
    {
        pthread_rwlock_unlock(prwLock);
        pstSavm->m_lErrno = NO_DATA_FOUND;
        return RC_FAIL;
    }

    pthread_rwlock_unlock(prwLock);
    return RC_SUCC;
}

/*************************************************************************************************
    description：API - lClick
    parameters:
        pstSavm                    --stvm handle
        puHits                     --hits
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lClick(SATvm *pstSavm, ulong *puHits)
{
    long    lRet;
    size_t  lData = 0;
    RunTime *pstRun  = NULL;

    if(!pstSavm || !pstSavm->pstVoid)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    if(NULL == (pstRun = (RunTime *)pInitMemTable(pstSavm, pstSavm->tblName)))
        return RC_FAIL;

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        pstSavm->m_lErrno = RMT_NOT_SUPPT;
        return RC_FAIL;
//        Tremohold(pstSavm, pstRun);
//        return _lSelectByRt(pstSavm, psvOut);
    }

    if(HAVE_UNIQ_IDX(pstSavm->tblName))
    {
        lRet = _lClickIndex(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, puHits);
        if(RC_CONTU != lRet)    return lRet;
    }

    if(HAVE_NORL_IDX(pstSavm->tblName))
    {
        lRet = _lClickGroup(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, puHits);
        if(RC_CONTU != lRet)    return lRet;
    }
    else if(HAVE_HASH_IDX(pstSavm->tblName))
    {
        lRet = _lClickHash(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, puHits);
        if(RC_CONTU != lRet)    return lRet;
    }

    lRet = _lClickTruck(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, puHits);
    vTblDisconnect(pstSavm, pstSavm->tblName);
    return lRet;
}

/*************************************************************************************************
    description：Select the table data according to the Unique index
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --memory address
        t                          --table
        psvOut                     --result data
        plData                     --offset 
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lReplaceIndex(SATvm *pstSavm, void *pvAddr, TABLE t, void *pvUpdate)
{
    size_t  lData;
    long    lRet = RC_SUCC;
    void    *pvData = NULL;
    SHTruck *pstTruck = NULL;
    SHTree  *pstRoot, *pstTree = NULL;
    RWLock  *prwLock = (RWLock *)pGetRWLock(pvAddr);
    char    szOld[MAX_INDEX_LEN], szIdx[MAX_INDEX_LEN];

    memset(szOld, 0, sizeof(szOld));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetIdxNum(t), pGetTblIdx(t), pstSavm->pstVoid, szOld))
        return RC_CONTU;

    if(NULL == (pvData = (void *)malloc(lGetRowSize(pstSavm->tblName))))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    if(RC_SUCC != pthread_rwlock_wrlock(prwLock)) 
    {
        pstSavm->m_lErrno = LOCK_DOWR_ERR;
        TFree(pvData);
        return RC_FAIL;
    }

    pstSavm->m_lEType = EXE_PLAN_IDX;
    if(NULL == (pstRoot = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lTreeRoot)))
    {
        pthread_rwlock_unlock(prwLock);    
        pstSavm->m_lErrno = SVR_EXCEPTION;
        TFree(pvData);
        return RC_FAIL;
    }

    if(NULL == (pstTree = (SHTree *)pSearchTree(pvAddr, pstRoot, szOld, lGetIdxLen(t))))
    {
        // If don't find it, so insert
        pstSavm->pstVoid = pvUpdate;
        lRet = __lInsert(pstSavm, pvAddr, t, 0);
        pthread_rwlock_unlock(prwLock);    
        TFree(pvData);
        return lRet;
    }

    lData = pstTree->m_lData;
    pstTruck = (PSHTruck)pGetNode(pvAddr, pstTree->m_lData);
    if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pstSavm->pstVoid))
    {
        pthread_rwlock_unlock(prwLock);    
        //  The uniq index is same, and the new data must trigger the index repetition.
        pstSavm->m_lErrno = UNIQ_IDX_REPT;
        TFree(pvData);
        return RC_FAIL;
    }

    memcpy(pvData, pstTruck->m_pvData, lGetRowSize(pstSavm->tblName));
    if(RC_FAIL == lMergeTruck(pstSavm, &pstSavm->stUpdt, pvUpdate, pvData))
    {
        pthread_rwlock_unlock(prwLock);    
        TFree(pvData);
        return RC_FAIL;
    }

    memset(szIdx, 0, sizeof(szIdx));
    if(NULL == pPickIndex(lGetIdxNum(t), pGetTblIdx(t), pvData, szIdx))
    {
        pthread_rwlock_unlock(prwLock);    
        pstSavm->m_lErrno = IDX_FIELD_NIL;
        TFree(pvData);
        return RC_FAIL;
    }

    pstSavm->pstVoid = pvData;
    pstTruck->m_lTimes ++;
    SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NULL);
    if(!memcmp(szIdx, szOld, MAX_INDEX_LEN))
    {
        if(HAVE_NORL_IDX(t))
            lRet = __lUpdateGroup(pstSavm, pvAddr, t, pstTruck, lData);
        else if(HAVE_HASH_IDX(t))            
            lRet = __lUpdateHash(pstSavm, pvAddr, t, pstTruck, lData);
    }
    else
    {
        if(NULL != (SHTree *)pSearchTree(pvAddr, pstRoot, szIdx, lGetIdxLen(t)))
        {
            SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
            pthread_rwlock_unlock(prwLock);    
            pstSavm->m_lErrno = UNIQ_IDX_REPT;
            TFree(pvData);
            return RC_FAIL;
        }

        if(NULL == (pstRoot = _pDeleteTree(pvAddr, pstSavm->tblName, pstRoot, pstTree)))
        {
            SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
            pthread_rwlock_unlock(prwLock);    
            pstSavm->m_lErrno = SVR_EXCEPTION;
            TFree(pvData);
            return RC_FAIL;
        }
        
        ((TblDef *)pvAddr)->m_lValid --;
        if(NULL == (pstRoot = pInsertTree(pstSavm, pstRoot, szIdx, lGetIdxLen(t), &pstTruck)))
        {
            SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
            pthread_rwlock_unlock(prwLock);    
            TFree(pvData);
            return RC_FAIL;
        }

        ((TblDef *)pvAddr)->m_lValid ++;
        ((TblDef *)pvAddr)->m_lTreeRoot = pstRoot->m_lSePos;
        if(HAVE_NORL_IDX(t))
            lRet = __lUpdateGroup(pstSavm, pvAddr, t, pstTruck, lData);
        else if(HAVE_HASH_IDX(t))
            lRet = __lUpdateHash(pstSavm, pvAddr, t, pstTruck, lData);
    }
    SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
    if(RC_SUCC != lRet)
    {
        pthread_rwlock_unlock(prwLock);    
        TFree(pvData);
        return RC_FAIL;
    }

    if(RC_SUCC != lRecordWork(pstSavm, pstTruck->m_pvData, OPERATE_UPDATE))
    {
        pthread_rwlock_unlock(prwLock);    
        TFree(pvData);
        return RC_FAIL;
    }

    memcpy(pstTruck->m_pvData, pvData, lGetRowSize(t));
    pthread_rwlock_unlock(prwLock);    
    pstSavm->m_lEffect = 1;

    TFree(pvData);
    return RC_SUCC;
}

/*************************************************************************************************
    description：Update record index
    parameters:
        pstSavm                    --stvm handle
        pstAddr                    --table handle
        t                          --table 
        pvUpdate                   --update data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lReplaceGroup(SATvm *pstSavm, void *pvAddr, TABLE t, void *pvUpdate)
{
    SHList  *pstList = NULL;
    SHTruck *pstTruck = NULL;
    size_t  lOffset, lNext;
    SHTree  *pstTree, *pstRoot;
    void    *pvData,  *pvCond = NULL;
    RWLock  *prwLock = (RWLock *)pGetRWLock(pvAddr);
    char    szOld[MAX_INDEX_LEN], szIdx[MAX_INDEX_LEN];

    memset(szOld, 0, sizeof(szOld));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetGrpNum(t), pGetTblGrp(t), pstSavm->pstVoid, szOld))
        return RC_CONTU;

    if(NULL == (pvData = (void *)malloc(lGetRowSize(t))))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    if(RC_SUCC != pthread_rwlock_wrlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DOWR_ERR;
        TFree(pvData);
        return RC_FAIL;
    }

    if(NULL == (pstRoot = (SHTree *)pGetNode(pvAddr, ((TblDef *)pvAddr)->m_lGroupRoot)))
    {
        pstSavm->m_lErrno = SVR_EXCEPTION;
        goto GROUP_ERROR;
    }

    if(NULL == (pstTree = pSearchTree(pvAddr, pstRoot, szOld, lGetGrpLen(t))))
    {
        TFree(pvData);
        // If don't find it, then insert
        pstSavm->pstVoid = pvUpdate;
        lNext = __lInsert(pstSavm, pvAddr, t, 0);
        pthread_rwlock_unlock(prwLock);
        return lNext;
    }

    pstSavm->m_lEffect = 0;
    pvCond = pstSavm->pstVoid;
    pstSavm->m_lEType = EXE_PLAN_GRP;
    for(pstList = (SHList *)pGetNode(pvAddr, pstTree->m_lData); SELF_POS_UNUSE != pstList->m_lPos;
        pstList = (SHList *)pGetNode(pvAddr, lNext))
    {
        lNext = pstList->m_lNext;
        pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pvCond))
        {
            if(SELF_POS_UNUSE == pstList->m_lNext)    break;
            continue;
        }

        memcpy(pvData, pstTruck->m_pvData, lGetRowSize(t)); 
        if(RC_FAIL == lMergeTruck(pstSavm, &pstSavm->stUpdt, pvUpdate, pvData))
            goto GROUP_ERROR;

        memset(szIdx, 0, sizeof(szIdx));
        if(NULL == pPickIndex(lGetGrpNum(t), pGetTblGrp(t), pvData, szIdx))
        {
            pstSavm->m_lErrno = IDX_FIELD_NIL;
            goto GROUP_ERROR;
        }

        pstSavm->m_lEffect ++;
        pstSavm->pstVoid = pvData;
        SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NULL);
        if(HAVE_UNIQ_IDX(t))
        {
            if(RC_SUCC != __lIndexUpdate(pstSavm, pvAddr, t, pstTruck, pstList->m_lData))
            {
                SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
                goto GROUP_ERROR;
            }
        }

        if(memcmp(szIdx, szOld, MAX_INDEX_LEN))
        {
            -- ((TblDef *)pvAddr)->m_lValid;
            lOffset = lGetListOfs(t) + ((TblDef *)pvAddr)->m_lValid * sizeof(SHList);
            if(RC_SUCC != lReleaseList(pvAddr, t, pstTree, pstList, lOffset, &lNext))
            {
                ((TblDef *)pvAddr)->m_lValid ++;
                pstSavm->m_lErrno = SVR_EXCEPTION;
                SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
                goto GROUP_ERROR;
            }

            if(SELF_POS_UNUSE == pstTree->m_lData)
            {
                pstRoot = _pDeleteGroup(pvAddr, t, pstRoot, pstTree, &pstSavm->m_lEffect);
                ((TblDef *)pvAddr)->m_lGroupRoot = pstRoot->m_lSePos;
            }

            if(NULL == (pstRoot = pInsertGroup(pstSavm, pstRoot, szIdx, lGetGrpLen(t), &pstTruck)))
            {
                SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
                goto GROUP_ERROR;
            }

            ((TblDef *)pvAddr)->m_lValid ++;
            ((TblDef *)pvAddr)->m_lGroupRoot = pstRoot->m_lSePos;
        }

        if(RC_SUCC != lRecordWork(pstSavm, pstTruck->m_pvData, OPERATE_UPDATE)) 
            goto GROUP_ERROR;

        memcpy(pstTruck->m_pvData, pvData, lGetRowSize(t));
        SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);

        if(FIRST_ROW & pstSavm->lFind)    break;
    }

    if(0 == pstSavm->m_lEffect)
    {
        // If don't find it, then insert
        pstSavm->pstVoid = pvUpdate;
        lNext = __lInsert(pstSavm, pvAddr, t, 0);
        pthread_rwlock_unlock(prwLock);    
        TFree(pvData);
        return lNext;
    }

    pthread_rwlock_unlock(prwLock);
    TFree(pvData);
    return RC_SUCC;

GROUP_ERROR:
    pthread_rwlock_unlock(prwLock);
    TFree(pvData);
    return RC_FAIL;
}

/*************************************************************************************************
    description：Update record hash index
    parameters:
        pstSavm                    --stvm handle
        pstAddr                    --table handle
        t                          --table 
        pvUpdate                   --update data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lReplaceHash(SATvm *pstSavm, void *pvAddr, TABLE t, void *pvUpdate)
{
    SHList  *pstList = NULL;
    SHTruck *pstTruck = NULL;
    SHTree  *pstTree, *pstHash;
    void    *pvData = NULL, *pvCond;
    size_t  lOld, lIdx, lOffset, lNext;
    RWLock  *prwLock = (RWLock *)pGetRWLock(pvAddr);
    char    szOld[MAX_INDEX_LEN], szIdx[MAX_INDEX_LEN];

    memset(szOld, 0, sizeof(szOld));
    if(NULL == pGetIndex(&pstSavm->stCond, lGetGrpNum(t), pGetTblGrp(t), pstSavm->pstVoid, szOld))
        return RC_CONTU;

    if(NULL == (pvData = (void *)malloc(lGetRowSize(t))))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    lOld = uGetHash(szOld, lGetGrpLen(t)) % ((TblDef *)pvAddr)->m_lMaxRow;
    lOffset = ((TblDef *)pvAddr)->m_lGroupPos + lOld * sizeof(SHTree);

    if(RC_SUCC != pthread_rwlock_wrlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DOWR_ERR;
        TFree(pvData);
        return RC_FAIL;
    }

    if(NULL == (pstHash = (SHTree *)(pvAddr + lOffset)))
    {
        // If don't find it, then insert
        pstSavm->pstVoid = pvUpdate;
        lNext =  __lInsert(pstSavm, pvAddr, t, 0);
        pthread_rwlock_unlock(prwLock);    
        TFree(pvData);
        return lNext;
    }

    pstSavm->m_lEffect = 0;
    pvCond = pstSavm->pstVoid;
    pstSavm->m_lEType = EXE_PLAN_GRP;
    for(pstList = (SHList *)pGetNode(pvAddr, pstHash->m_lData); SELF_POS_UNUSE != pstList->m_lPos; 
        pstList = (SHList *)pGetNode(pvAddr, lNext))
    {
        lNext = pstList->m_lNext;
        pstTruck = (PSHTruck)pGetNode(pvAddr, pstList->m_lData);
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pvCond))
        {
            if(SELF_POS_UNUSE == pstList->m_lNext)    break;
            continue;
        }

        memcpy(pvData, pstTruck->m_pvData, lGetRowSize(t)); 
        if(RC_FAIL == lMergeTruck(pstSavm, &pstSavm->stUpdt, pvUpdate, pvData))
            goto HASH_ERROR;

        memset(szIdx, 0, sizeof(szIdx));
        if(NULL == pPickIndex(lGetGrpNum(t), pGetTblGrp(t), pvData, szIdx))
        {
            pstSavm->m_lErrno = IDX_FIELD_NIL;
            goto HASH_ERROR;
        }

        pstSavm->m_lEffect ++;
        pstSavm->pstVoid = pvData;
        lIdx = uGetHash(szIdx, lGetGrpLen(t)) % ((TblDef *)pvAddr)->m_lMaxRow;
        SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NULL);
        if(HAVE_UNIQ_IDX(t))
        {
            if(RC_SUCC != __lIndexUpdate(pstSavm, pvAddr, t, pstTruck, pstList->m_lData))
            {
                SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NULL);
                goto HASH_ERROR;
            }
        }

        // Index does not match, rebuild inde
        if(lOld != lIdx)
        {
            -- ((TblDef *)pvAddr)->m_lValid;
            lOffset = lGetListOfs(t) + ((TblDef *)pvAddr)->m_lValid * sizeof(SHList);
            if(RC_SUCC != lReleaseList(pvAddr, t, pstHash, pstList, lOffset, &lNext))
            {
                ((TblDef *)pvAddr)->m_lValid ++;
                pstSavm->m_lErrno = SVR_EXCEPTION;
                SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
                goto HASH_ERROR;
            }

            lOffset = ((TblDef *)pvAddr)->m_lGroupPos + lIdx * sizeof(SHTree);
            if(NULL == (pstTree = (SHTree *)(pvAddr + lOffset)))
            {
                pstSavm->m_lErrno = SVR_EXCEPTION;
                SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
                goto HASH_ERROR;
            }

            if(RC_SUCC != __lInsertHash(pstSavm, pvAddr, t, pstTree, lOffset, &pstTruck))
            {
                pstSavm->m_lErrno = SVR_EXCEPTION;
                SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
                goto HASH_ERROR;
            }

            ((TblDef *)pvAddr)->m_lValid ++;
        }
        if(RC_SUCC != lRecordWork(pstSavm, pstTruck->m_pvData, OPERATE_UPDATE)) 
            goto HASH_ERROR;

        memcpy(pstTruck->m_pvData, pvData, lGetRowSize(t));
        SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);

        if(FIRST_ROW & pstSavm->lFind)    break;
    }

    if(0 == pstSavm->m_lEffect)
    {
        // If don't find it, then insert
        pstSavm->pstVoid = pvUpdate;
        lNext = __lInsert(pstSavm, pvAddr, t, 0);
        pthread_rwlock_unlock(prwLock);    
        TFree(pvData);
        return lNext;
    }

    pthread_rwlock_unlock(prwLock);
    TFree(pvData);
    return RC_SUCC;

HASH_ERROR:
    pthread_rwlock_unlock(prwLock);
    TFree(pvData);
    return RC_FAIL;
}

/*************************************************************************************************
    description：Update record truck 
    parameters:
        pstSavm                    --stvm handle
        pstAddr                    --table handle
        t                          --table 
        pvUpdate                   --update data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lTruckReplace(SATvm *pstSavm, void *pvAddr, TABLE t, void *pvUpdate)
{
    long    lRet = RC_SUCC;
    SHTruck *pstTruck = NULL;
    void    *pvData = NULL, *pvCond;
    size_t  lRow, lOffset = lGetTblData(t);
    RWLock  *prwLock = (RWLock *)pGetRWLock(pvAddr);

    if(NULL == (pvData = (void *)malloc(lGetRowSize(t))))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    if(RC_SUCC != pthread_rwlock_wrlock(prwLock))
    {
        pstSavm->m_lErrno = LOCK_DOWR_ERR;
        TFree(pvData);
        return RC_FAIL;
    }

    pstSavm->m_lEffect = 0;
    pvCond = pstSavm->pstVoid;
    pstSavm->m_lEType = EXE_PLAN_ALL;
    pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset);
    for(lRow = 0; (lRow < ((TblDef *)pvAddr)->m_lValid) && (lOffset < lGetTableSize(t));
        pstTruck = (PSHTruck)pGetNode(pvAddr, lOffset))
    {
        if(IS_TRUCK_NULL(pstTruck))
        {
            lOffset += lGetRowTruck(t);
            continue;
        }

        lRow ++;
        if(RC_MISMA == lFeildMatch(&pstSavm->stCond, pstTruck->m_pvData, pvCond))
        {
            lOffset += lGetRowTruck(t);
            continue;
        }

        memcpy(pvData, pstTruck->m_pvData, lGetRowSize(t)); 
        if(RC_FAIL == lMergeTruck(pstSavm, &pstSavm->stUpdt, pvUpdate, pvData))
            goto REPLACE_ERROR;

        pstSavm->m_lEffect ++;
        pstSavm->pstVoid = pvData;
        SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NULL);
        if(HAVE_UNIQ_IDX(t))
        {
            if(RC_SUCC != __lIndexUpdate(pstSavm, pvAddr, t, pstTruck, lOffset))
            {
                SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
                goto REPLACE_ERROR;
            }
        }
 
        if(HAVE_NORL_IDX(t))
            lRet = __lUpdateGroup(pstSavm, pvAddr, t, pstTruck, lOffset);
        else if(HAVE_HASH_IDX(t))
            lRet = __lUpdateHash(pstSavm, pvAddr, t, pstTruck, lOffset);
        SET_DATA_TRUCK(pstTruck, DATA_TRUCK_NRML);
        if(RC_SUCC != lRet)
            goto REPLACE_ERROR;

        if(RC_SUCC != lRecordWork(pstSavm, pstTruck->m_pvData, OPERATE_UPDATE)) 
            goto REPLACE_ERROR;

        memcpy(pstTruck->m_pvData, pvData, lGetRowSize(t));
        if(FIRST_ROW & pstSavm->lFind)    break;
        lOffset += lGetRowTruck(t);
    }

    if(0 == pstSavm->m_lEffect)
    {
        // If don't find it, so insert
        pstSavm->pstVoid = pvUpdate;
        lRet = __lInsert(pstSavm, pvAddr, t, 0);
        pthread_rwlock_unlock(prwLock);    
        TFree(pvData);
        return lRet;
    }

    pthread_rwlock_unlock(prwLock);
    TFree(pvData);
    return RC_SUCC;

REPLACE_ERROR:
    pthread_rwlock_unlock(prwLock);
    TFree(pvData);
    return RC_FAIL;
}

/*************************************************************************************************
    description：API - lReplace
    parameters:
        pstSavm                    --stvm handle
        pvReplace                  --replace data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lReplace(SATvm *pstSavm, void *pvReplace)
{
    long    lRet;
    void    *pvData;
    RunTime *pstRun = NULL;

    if(!pstSavm || !pstSavm->pstVoid)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pvData = pstSavm->pstVoid;
    if(NULL == (pstRun = (RunTime *)pInitMemTable(pstSavm, pstSavm->tblName)))
        return RC_FAIL;

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        Tremohold(pstSavm, pstRun);
        return _lReplaceByRt(pstSavm, pvReplace);
    }

    if(HAVE_UNIQ_IDX(pstSavm->tblName))
    {
        lRet = _lReplaceIndex(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, pvReplace);
        if(RC_CONTU != lRet)
        {
            vTblDisconnect(pstSavm, pstSavm->tblName);
            pstSavm->pstVoid = pvData;
            return lRet;
        }
    }

    if(HAVE_NORL_IDX(pstSavm->tblName))
    {
        lRet = _lReplaceGroup(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, pvReplace);
        if(RC_CONTU != lRet)
        {
            vTblDisconnect(pstSavm, pstSavm->tblName);
            pstSavm->pstVoid = pvData;
            return lRet;
        }
    }
    else if(HAVE_HASH_IDX(pstSavm->tblName))
    {
        lRet = _lReplaceHash(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, pvReplace);
        if(RC_CONTU != lRet)
        {
            vTblDisconnect(pstSavm, pstSavm->tblName);
            pstSavm->pstVoid = pvData;
            return lRet;
        }
    }

    lRet = _lTruckReplace(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, pvReplace);
    vTblDisconnect(pstSavm, pstSavm->tblName);
    pstSavm->pstVoid = pvData;
    return lRet;
}

/*************************************************************************************************
    description：dump the table data
    parameters:
        pstSavm                    --stvm handle
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lDumpTable(SATvm *pstSavm, TABLE t)
{
    FILE    *fp = NULL;
    char    szFile[512];
    RunTime *pstRun = NULL;
    SHTruck *pstTruck = NULL;
    size_t  lRow = 0, lOffset;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    memset(szFile, 0, sizeof(szFile));
    if(RC_SUCC != lInitSATvm(pstSavm, t))
        return RC_FAIL;

    if(NULL == (pstRun = (RunTime *)pInitHitTest(pstSavm, t)))
        return RC_FAIL;

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        pstSavm->m_lErrno = RMT_NOT_SUPPT;
        return RC_FAIL;
    }

    snprintf(szFile, sizeof(szFile), "%s/%d.sdb", getenv("TVMDBD"), t);
    if(NULL == (fp = fopen(szFile, "wb")))
    { 
        pstSavm->m_lErrno = FILE_NOT_RSET;
        return RC_FAIL;
    }
    fwrite(pGetTblDef(t), sizeof(TblDef), 1, fp);

    lOffset = lGetTblData(t);
    pstRun->m_lCurLine = 0;
    pstSavm->lSize = lGetRowSize(t);
    pstTruck = (PSHTruck)pGetNode(pstRun->m_pvAddr, lOffset);
    for(lRow = 0; (lRow < ((TblDef *)pstRun->m_pvAddr)->m_lValid) && (lOffset < lGetTableSize(t));
        pstTruck = (PSHTruck)pGetNode(pstRun->m_pvAddr, lOffset))
    {
        if(IS_TRUCK_NULL(pstTruck))
        {
            lOffset += lGetRowTruck(t);
            continue;
        }

        fwrite(pstTruck->m_pvData, lGetRowSize(t), 1, fp);
        fwrite((void *)&pstTruck->m_lTimes, sizeof(ulong), 1, fp);

        pstSavm->m_lEffect ++;
        lOffset += lGetRowTruck(t);
    }
    fclose(fp);
    vTableClose(pstSavm);

    fprintf(stdout, "导出表:%s 有效记录:%ld, completed successfully !!\n", sGetTableName(t), 
        pstSavm->m_lEffect);
    return RC_SUCC;
}

/*************************************************************************************************
    description：lMountTable
    parameters:
        pstSavm                    --stvm handle
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lMountTable(SATvm *pstSavm, char *pszFile)
{
    TblDef  stTde;
    FILE    *fp = NULL;
    ulong   uTimes = 0;
    long    lEffect = 0;
    void    *pvData = NULL;
    RunTime *pstRun = NULL;
    RWLock  *prwLock = NULL;

    if(!pszFile || !pstSavm || !strlen(pszFile))
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    if(NULL == (fp = fopen(pszFile, "rb")))
    {
        pstSavm->m_lErrno = FIL_NOT_EXIST;
        return RC_FAIL;
    }

    fread(&stTde, sizeof(TblDef), 1, fp);
    if(RC_SUCC != lInitSATvm(pstSavm, stTde.m_table))
        goto MOUNT_ERROR;

    if(NULL == (pstRun = (RunTime *)pInitHitTest(pstSavm, stTde.m_table)))
        goto MOUNT_ERROR;

    if(stTde.m_lReSize != lGetRowSize(stTde.m_table))
    {
        vTblDisconnect(pstSavm, pstSavm->tblName);
        pstSavm->m_lErrno = VER_NOT_MATCH;
        goto MOUNT_ERROR;
    }

    if(NULL == (pvData = (void *)malloc(stTde.m_lReSize)))
    {
        vTblDisconnect(pstSavm, pstSavm->tblName);
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        goto MOUNT_ERROR;
    }

    pstSavm->pstVoid = pvData;
    pstSavm->lSize   = stTde.m_lReSize;
    prwLock = (RWLock *)pGetRWLock(pstRun->m_pvAddr);
    if(RC_SUCC != pthread_rwlock_wrlock(prwLock))
    {
        vTblDisconnect(pstSavm, pstSavm->tblName);
        pstSavm->m_lErrno = LOCK_DOWR_ERR;
        goto MOUNT_ERROR;
    }

    while(1 == fread(pvData, stTde.m_lReSize, 1, fp))
    {
        fread((void *)&uTimes, sizeof(ulong), 1, fp);
        if(lGetTblRow(stTde.m_table) <= ((TblDef *)pstRun->m_pvAddr)->m_lValid)
        {
            pthread_rwlock_unlock(prwLock);
            vTblDisconnect(pstSavm, pstSavm->tblName);
            pstSavm->m_lErrno = DATA_SPC_FULL;
            goto MOUNT_ERROR;
        }

        lEffect ++;
        if(RC_SUCC != __lInsert(pstSavm, pstRun->m_pvAddr, pstSavm->tblName, uTimes))
        {
            fprintf(stderr, "=>警告, 导入表:%s 第(%ld)条记录错误, %s, 跳过..\n", 
                sGetTableName(stTde.m_table), lEffect, sGetTError(pstSavm->m_lErrno));
            continue;
        }

        pstSavm->m_lEffect ++;
    }
    fclose(fp);
    TFree(pvData);
    pthread_rwlock_unlock(prwLock);
    vTblDisconnect(pstSavm, pstSavm->tblName);

    fprintf(stdout, "导入表:%s 有效记录:%ld, completed successfully !!\n", 
        sGetTableName(stTde.m_table), lEffect);
    return RC_SUCC;

MOUNT_ERROR:
    fclose(fp);
    TFree(pvData);
    return RC_FAIL;
}

/*************************************************************************************************
 * debug
 *************************************************************************************************/
