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

#ifndef __TVM_DEFIND_HHHH___
#define __TVM_DEFIND_HHHH___
#include    "tstr.h"

typedef pthread_rwlock_t     RWLock;
typedef pthread_rwlockattr_t RWAttr;
typedef unsigned    int      TABLE;
typedef long        long     llSEQ;
typedef long                 (*TCREATE)(TABLE t);
typedef long                 CREATE;

//#pragma pack(4)

#define TVM_VKERNEL                        "1.2.0.0"
#define TVM_VERSION                        "1.2.2.0"
/*************************************************************************************************
   custom macro
 *************************************************************************************************/
#define PROT_JAVA                           512
#define FIRST_ROW                           256
#define ORDER_DESC                          128
#define ORDER_ASC                           64
#define ORDER_BY                            64
#define GROUP_BY                            32
#define MATCH_MAX                           16
#define MATCH_MIN                           8

#define FIELD_DOUB                          4
#define FIELD_LONG                          2
#define FIELD_CHAR                          1

//  execution plan
#define EXE_PLAN_ALL                        0
#define EXE_PLAN_IDX                        1 
#define EXE_PLAN_GRP                        2  

#define CHK_SELECT                          0
#define IDX_SELECT                          1
#define RCD_SELECT                          2
#define NIL_IGNORE                          8

#define OPERATE_NULL                        0
#define OPERATE_INSERT                      1
#define OPERATE_DELETE                      2
#define OPERATE_UPDATE                      4
#define OPERATE_SELECT                      8
#define OPERATE_TRCATE                      14
#define OPERATE_COUNT                       15
#define OPERATE_GROUP                       16
#define OPERATE_QUERY                       17
#define OPERATE_EXTREM                      18
#define OPERATE_TBDROP                      19
#define OPERATE_RENAME                      20
#define OPERATE_SELSEQ                      21
#define OPERATE_SETSEQ                      22
#define OPERATE_RBDIDX                      23
#define OPERATE_RETLOK                      24
#define OPERATE_DMKEEP                      25
#define OPERATE_DOMPUL                      26
#define OPERATE_DOMPSH                      27
#define OPERATE_DMRECN                      28
#define OPERATE_REFRESH                     29
#define OPERATE_DOMLOFF                     30
#define OPERATE_DOMROFF                     31
#define OPERATE_PULTBL                      32
#define OPERATE_BEGWORK                     33
#define OPERATE_ROLWORK                     34
#define OPERATE_CMTWORK                     35
#define OPERATE_ENDWORK                     36
#define OPERATE_EXEEXIT                     99

#define OPERATE_DEFAULT                     (OPERATE_SELECT | OPERATE_UPDATE | OPERATE_DELETE | OPERATE_INSERT)
/*************************************************************************************************
    Internal definition 
 *************************************************************************************************/
#define SYS_TVM_INDEX                       0x01
#define SYS_TVM_FIELD                       0x02        // field table
#define SYS_TVM_DOMAIN                      0x03        // domain table
#define SYS_TVM_SEQUE                       0x04
#define TVM_MAX_TABLE                       0xFF        // maximum number of creation of the system

#define MAX_STRIG_LEN                       256

#ifndef MAX_INDEX_LEN
#define MAX_INDEX_LEN                       64
#endif

#define MAX_FIELD_LEN                       32          // maxinum length of Field name

#ifndef MAX_FILED_NUM 
#define MAX_FILED_NUM                       32          // maximum number of fields in a table
#endif

#define MAX_REMOTE_IP                       20
#define MAX_TIMESTAMP                       20
#define MAX_FILED_IDX                       8

// resource flag
#define RES_LOCAL_SID                       1
#define RES_REMOT_SID                       2
#define RESOURCE_ROFF                       5
#define RESOURCE_AUTH                       4 
#define RESOURCE_EXCP                       3
#define RESOURCE_ABLE                       2
#define RESOURCE_STOP                       1
#define RESOURCE_INIT                       0

#define IPC_MSG                             0x01
#define IPC_SEM                             0x02 
#define IPC_SHM                             0x03 

#define SEM_INIT                            1
#define SEM_O_V                             1           // Semaphore V operation
#define SEM_O_P                             -1          // Semaphore P operation
#define SEM_RD                              0
#define SEM_WD                              1

#define UNQIUE                              1
#define NORMAL                              16
#define HASHID                              32

#define TYPE_SYSTEM                         0x01
#define TYPE_INCORE                         0x02
#define TYPE_CLIENT                         0x03        //  custom
#define TYPE_KEYVAL                         0x04 
#define TVM_NODE_INFO                       "localhost"
#define TVM_RUNCFG_TAG                      "\x01\x33\xC8\x48"

#define TVM_BOOT_CLUSTER                    2
#define TVM_BOOT_LOCAL                      1
#define TVM_BOOT_SIMPLE                     0


#define HAVE_INDEX(t)                       (((TblDef *)pGetTblDef(t))->m_lIType != 0)
#define HAVE_UNIQ_IDX(t)                    (((TblDef *)pGetTblDef(t))->m_lIType & UNQIUE)
#define HAVE_NORL_IDX(t)                    (((TblDef *)pGetTblDef(t))->m_lIType & NORMAL)
#define HAVE_HASH_IDX(t)                    (((TblDef *)pGetTblDef(t))->m_lIType & HASHID)

#define IS_RED(x)                           (x->m_eColor == COLOR_RED)
#define FPOS(t, f)                          ((size_t)&((t *)0)->f)
#define FLEN(t, f)                          (sizeof(((t *)0)->f))
#define NODE_NULL                           g_lNilOfs
#define ReField(t, f)                       (FPOS(t, f) << 16 | FLEN(t, f))
#define REFrom(t)                           (t >> 16)
#define REFLen(t)                           (t & 0xffff)

#define WORK_ERROR_LOG                      "work.err"
#define STVM_SQL_LINE                       ".stvmrc"
#define COLOR_BLK                           0 
#define COLOR_RED                           1
#define SELF_POS_UNUSE                      0 
#define DATA_TRUCK_NULL                     0x00
#define DATA_TRUCK_NRML                     0x01
#define DATA_TRUCK_LOCK                     0x02
#define TABLE_LOCK_READ                     1
#define TABLE_LOCK_WRITE                    2
#define IS_TRUCK_NULL(p)                    ((p)->m_chTag == DATA_TRUCK_NULL)
#define IS_TRUCK_NRML(p)                    ((p)->m_chTag == DATA_TRUCK_NRML)
#define IS_TRUCK_LOCK(p)                    ((p)->m_chTag == DATA_TRUCK_LOCK)
#define SET_DATA_TRUCK(p, type)             ((p)->m_chTag =  type)
#define TFree(p)                             if(p) { free(p); p = NULL; }
#define TFgrp(p)                             do{vDeleteRowgrp(p);p = NULL;}while(0);
#define TFlst(p)                             do{vDestroyList(p);p = NULL;}while(0);
#define TClose(f)                            if(f) { fclose(f); f = NULL; }

/*************************************************************************************************
    错误码定义区
 *************************************************************************************************/
#define TVM_DONE_SUCC                       0           // completed successfully
#define SVR_EXCEPTION                       1           // sever exception
#define IDX_FIELD_NIL                       2           // index field values is null
#define CONDIT_IS_NIL                       3           // condition is null
#define DATA_SPC_FULL                       4           // no space for create data
#define GENER_KEY_ERR                       5           // generate shm key failure
#define SHM_ERR_INVAL                       6           // Invalid parameter or shm has disappeared
#define SHM_ERR_EXIST                       7           // shared memory already exists
#define SHM_ERR_EIDRM                       8           // shared memory has been deleted
#define SHM_ERR_ACCES                       9           // Permission denied
#define SHM_ERR_NOMEM                       10          // Insufficient(shm) core memory
#define VER_NOT_MATCH                       11          // data truck version mismatch
#define BLCK_SIZE_ERR                       12          // size is error to creat data block
#define IDX_DEF_SPILL                       13          // unique Index definition overflow
#define IDX_LEN_SPILL                       14          // unique Index length overflow
#define GRP_DEF_SPILL                       15          // normal Index definition overflow
#define GRP_LEN_SPILL                       16          // normal Index length overflow
#define IDX_TYP_NODEF                       17          // index type not define
#define FLD_DEF_SPILL                       18          // field define overflow
#define IDX_DATA_MISM                       19          // index data mismatch
#define FTYPE_NOT_DEF                       20          // field type not define
#define SHMT_NOT_INIT                       21          // memory has not been initialized
#define UNIQ_IDX_REPT                       22          // unique index repeat
#define IDX_SPC_SPILL                       23          // no space for create index
#define NO_DATA_FOUND                       24          // no data be found
#define MORE_ROWS_SEL                       25          // more then one records be selected
#define MALLC_MEM_ERR                       26          // malloc memory error
#define CURS_IS_INVAL                       27          // cursor invalid
#define TABLE_NOT_DEF                       28          // table not define
#define FIL_NOT_EXIST                       29          // file not exist
#define SEM_CDIT_NULL                       30          // semget condition is null
#define SEM_ERR_INVAL                       31          // Invalid parameter or sem has disappeared
#define SEM_ERR_EXIST                       32          // semaphore already exists
#define SEM_ERR_EIDRM                       33          // semaphore has been deleted
#define SEM_ERR_ACCES                       34          // Permission denied(sem)
#define SEM_ERR_NOMEM                       35          // Insufficient(sem) core memory
#define SEM_ERR_LIMIT                       36          // Semaphore value out of limit
#define SQL_SYNTX_ERR                       37          // SQL syntax is error
#define SQL_NOT_SUPPT                       38          // SQL operation not be supported
#define SQL_TABLE_NIL                       39          // SQL no table name be inputted
#define SQL_FIELD_NIL                       40          // SQL field is not selected
#define SQL_WHERE_NIL                       41          // SQL conditional syntax error
#define SQL_ERR_FIELD                       42          // SQL field syntax error
#define SQL_ERR_WHERE                       43          // SQL where syntax error
#define TBL_NOT_FOUND                       44          // table not found
#define SQL_FAV_MATCH                       45          // SQL fields does not match the value 
#define LOCK_DORD_ERR                       46          // set the read lock failure
#define LOCK_UNRD_ERR                       47          // unlock read lock failure
#define LOCK_DOWR_ERR                       48          // set the write lock failure
#define LOCK_UNWR_ERR                       49          // unlock write lock failure
#define SOCK_CONN_ERR                       50          // socket connect failure
#define SOCK_CONN_TMO                       51          // socket connect timeout
#define SOCK_ERR_CRTE                       52          // create socket failure
#define SOCK_READ_ERR                       53          // socket recv failure
#define SOCK_SEND_ERR                       54          // socket send failure
#define SOCK_BIND_ERR                       55          // socket bind failure
#define SOCK_LSEN_ERR                       56          // socket listen failure
#define SOCK_SEND_TMO                       57          // socket send timeout
#define SOCK_READ_TMO                       58          // socket read timeout
#define SOCK_IO_RESET                       59          // socket reset
#define SOCK_COM_EXCP                       60          // Socket communication anomaly
#define EPOLL_ADD_ERR                       61          // epoll add fd error
#define EPOLL_CRT_ERR                       62          // create epoll fd failure
#define EPOLL_DEL_ERR                       63          // delete epoll fd failure
#define SOCK_ACPT_ERR                       64          // socket accept failure
#define RMT_NOT_SUPPT                       65          // SQL remote does not support
#define FILE_NOTFOUND                       66          // file not found
#define BOOT_PARM_ERR                       67          // boot parameters error
#define BOOT_RELA_ERR                       68          // parameters table related error
#define BOOT_VER_ICMP                       69          // Incompatible version
#define DOM_NOT_REGST                       70          // domain not register
#define DMWORK_NOTSUP                       71          // domain work does not support
#define SEQ_NOT_FOUND                       72          // sequence does not exist
#define FILE_NOT_RSET                       73          // file is not set
#define RECD_TOO_LONG                       74          // record data too long 
#define RESOU_DISABLE                       75          // Resource unavailable 
#define MSG_ERR_EXIST                       76          // message queue already exists
#define MSG_ERR_ACCES                       77          // Permission denied .msg
#define MSG_ERR_NOMEM                       78          // Insufficient(msg) core memory
#define MSG_ERR_INVAL                       79          // Invalid parameter or msg has disappeared
#define MSG_ERR_FAULT                       80          // msg Invalid address
#define MSG_ERR_EIDRM                       81          // message queue has been deleted
#define MSG_ERR_E2BIG                       82          // message text length is greater than msgsz
#define MSG_ERR_EINTR                       83          // Interrupted by signal
#define MSG_ERR_SNDEG                       84          // msgsnd queue overrun
#define INI_ERR_CHLDP                       85          // initial child process failed
#define FLD_NOT_EXIST                       86          // field not exist
#define TBL_ARD_EXIST                       87          // table already exist
#define WORK_NOT_OPEN                       88          // The transaction has not been opened yet
#define WORK_NOT_REGT                       89          // The transaction has not been register
#define DOM_NOT_INITL                       90          // domain not initail
#define FIELD_NOT_DEF                       91          // table field not define
#define FIELD_NOT_SET                       92          // field not set
#define UPDFD_NOT_SET                       93          // update field not set
#define EXTRE_SET_ERR                       94          // extreme set decorate error
#define GROUP_SET_ERR                       95          // group set decorate error

/*************************************************************************************************
     创建表宏函数
 *************************************************************************************************/
#define DEFINE(t, n, p, s)      TABLE tbl = t; long type = 0; \
                                ((TblDef *)pGetTblDef(t))->m_table = t; \
                                ((TblDef *)pGetTblDef(t))->m_lReSize = sizeof(s); \
                                strncpy(((TblDef *)pGetTblDef(t))->m_szPart, p, MAX_FIELD_LEN); \
                                strncpy(((TblDef *)pGetTblDef(t))->m_szTable, n, MAX_FIELD_LEN); \
                                ((TblDef *)pGetTblDef(t))->m_lTruck = sizeof(s) + sizeof(SHTruck);

#define CREATE_IDX(t)           type = t;
#define IDX_FIELD(t, f, a)      if(RC_SUCC != lAddIdxField(tbl, type, FPOS(t, f), FLEN(t, f), a)) \
                                    return RC_FAIL;
#define FIELD(t, f, d, a)       if(RC_SUCC != lSetTableIdx(tbl, FPOS(t, f), FLEN(t, f), d, a, CHK_SELECT)) \
                                    return RC_FAIL;
#define FIELU(t, f, d, a)       if(RC_SUCC != lSetTableIdx(tbl, FPOS(t, f), FLEN(t, f), d, a, IDX_SELECT)) \
                                    return RC_FAIL;
#define FIELR(t, f, d, a)       if(RC_SUCC != lSetTableIdx(tbl, FPOS(t, f), FLEN(t, f), d, a, RCD_SELECT)) \
                                    return RC_FAIL;
#define FINISH                  return RC_SUCC;

/*************************************************************************************************
    Field assignment
 *************************************************************************************************/
#define defineinit(p,s,t)       do{ \
                                   p->stCond.uFldcmp = 0; \
                                   p->stUpdt.uFldcmp = 0; \
                                   p->lSize = sizeof(s); \
                                   p->tblName = t; \
                                   p->pstVoid = (void *)&(s);  \
                                }while(0);

#define insertinit(p,s,t)       do{ \
                                   p->lSize = sizeof(s); \
                                   p->tblName = t; \
                                   p->pstVoid = (void *)&(s);  \
                                }while(0);

#define conditinit(p,s,t)       do{ \
                                   p->stCond.uFldcmp = 0; \
                                   p->stUpdt.uFldcmp = 0; \
                                   p->lSize = sizeof(s); \
                                   p->tblName = t; \
                                   p->lFind = 0;  \
                                   memset(&(s), 0, p->lSize); \
                                   p->pstVoid = (void *)&(s);  \
                                }while(0);

#define conditnull(p,d,t)       do{ \
                                   p->stCond.uFldcmp = 0; \
                                   p->stUpdt.uFldcmp = 0; \
                                   p->lFind = 0;  \
                                   p->lSize = sizeof(d); \
                                   p->tblName = t; \
                                   p->pstVoid = NULL;  \
                                }while(0);

#define stringsetv(p,s,f,...)   vSetCodField(&p->stCond, sizeof((s).f), (void *)(s).f - (void *)&(s)); \
                                snprintf((s).f, sizeof((s).f), __VA_ARGS__);

#define stringset(p,s,f,v)      vSetCodField(&p->stCond, sizeof((s).f), (void *)(s).f - (void *)&(s)); \
                                strncpy((s).f, v, sizeof((s).f));

#define stringcpy(p,s,f,v)      vSetCodField(&p->stCond, sizeof((s).f), (void *)(s).f - (void *)&(s)); \
                                memcpy(&(s) + ((void *)(s).f - (void *)&(s)), (void *)v, sizeof((s).f));

#define numberset(p,s,f,v)      vSetCodField(&p->stCond, sizeof((s).f), (void *)&(s).f - (void *)&(s)); \
                                (s).f = v;

#define decorate(p,d,f,v)       vSetDecorate(&p->stUpdt, FLEN(d, f), FPOS(d, f), v); \
                                p->lFind = (v) & FIRST_ROW;

#define stringreset(s,f,v)      strncpy((s).f, v, sizeof((s).f));
#define stringresetv(s,f,...)   snprintf((s).f, sizeof((s).f), __VA_ARGS__);
#define stringrecpy(s,f,v)      memcpy((s).f, v, sizeof((s).f));
#define numberreset(s,f,v)      (s).f = v;

// UPDATE Field assignment
#define updateinit(s)           memset(&(s), 0, sizeof(s));

#define stringupd(p,s,f,v)      vSetCodField(&p->stUpdt, sizeof((s).f), (void *)(s).f - (void *)&(s)); \
                                strncpy((s).f, v, sizeof((s).f));

#define stringupy(p,s,f,v)      vSetCodField(&p->stUpdt, sizeof((s).f), (void *)(s).f - (void *)&(s)); \
                                memcpy(&(s) + ((void *)(s).f - (void *)&(s)), (void *)v, sizeof((s).f));

#define numberupd(p,s,f,v)      vSetCodField(&p->stUpdt, sizeof((s).f), (void *)&(s).f - (void *)&(s)); \
                                (s).f = v; 

/*************************************************************************************************
    Table structure & index definition area  
 *************************************************************************************************/
typedef struct  __SH_DATA_TRUCK
{
    ulong   m_lTimes;
    char    m_chTag;
    char    m_pvData[0];
}SHTruck, *PSHTruck;

typedef struct  __SH_RBTREE
{
    size_t  m_lSePos;
    char    m_szIdx[MAX_INDEX_LEN];
    long    m_lIdx;
    size_t  m_lData;
    long    m_eColor;
    size_t  m_lParent;
    size_t  m_lLeft;
    size_t  m_lRight;
}SHTree;

typedef struct  __SH_LIST
{
    size_t    m_lPos; 
    size_t    m_lNode;
    size_t    m_lData;
    size_t    m_lNext;
    size_t    m_lLast;
}SHList;

typedef struct __TBL_COM_KEY
{
    long    m_lFrom;
    long    m_lLen; 
    long    m_lAttr;
    long    m_lIsPk;
    char    m_szField[MAX_FIELD_LEN]; 
}TblKey;

typedef struct  __TBL_HEAD_DEF
{
    RWLock  m_rwlock;                         //  rwlock
    long    m_lGroup;                         //  index group
    size_t  m_lMaxRow;                        //  maximum support
    size_t  m_lValid;                         //  number of valid
    long    m_lIdxLen;                        //  unique index length
    size_t  m_lNodeNil;                       //  NIL
    size_t  m_lTreePos;                       //  unique tree position
    size_t  m_lTreeRoot;                      //  unique tree root
    long    m_lGrpLen;                        //  index length
    size_t  m_lGroupPos;                      //  index position
    size_t  m_lGroupRoot;                     //  index root
    size_t  m_lListPos;                       //  list position
    size_t  m_lListOfs;                       //  list offset
    size_t  m_lData;                          //  data offset
    uint    m_lIType;                         //  index type
    uint    m_lIdxUp;                         //  unique index field
    TblKey  m_stIdxUp[MAX_FILED_IDX];         //  unique index 
    uint    m_lGrpUp;                         //  index field
    TblKey  m_stGrpUp[MAX_FILED_IDX];         //  index
    size_t  m_lTable;                         //  table size
    long    m_lReSize;                        //  row size
    size_t  m_lTruck;                         //  truck size
    llSEQ   m_lExSeQ;                         //  extern sequence
    long    m_lExtern;                        //  extern table space(standby)
    long    m_lIdxNum;                        //  Number of fields
    TblKey  m_stKey[MAX_FILED_NUM];           //  fields
    TABLE   m_table;                          //  table
    char    m_szTable[MAX_FIELD_LEN];         //  table name
    char    m_szPart[MAX_FIELD_LEN];          //    
    SHTree  m_stNil;
}TblDef;

static    long    g_lNilOfs = FPOS(TblDef, m_stNil);

typedef struct __SQL_FIELD
{
    TblKey  m_stKey;   
    struct  __SQL_FIELD   *pstNext;
}SQLFld;

/*************************************************************************************************
    TVM engine starts the required table (do not move)
 *************************************************************************************************/
typedef struct  __SYS_TVM_INDEX
{
    TABLE   m_table;                          //  table
    long    m_lType;                          //  table type
    char    m_szTable[MAX_FIELD_LEN];         //  table name
    char    m_szPart[MAX_FIELD_LEN];          //  partition name
    char    m_szOwner[MAX_FIELD_LEN];         //  owner
    key_t   m_yKey;
    long    m_shmID;                          //  Memory Key
    long    m_semID;                          //  semaphore key
    long    m_lPid;                           //  pid
    long    m_lValid;                         //  valid
    long    m_lMaxRows;                       //  Table maximum support record number.
    long    m_lRowSize;                       //  truck size
    long    m_lLocal;                         //  Local/remote
    uint    m_lState;                         //  available
    long    m_lPers;                          //  permissions
    char    m_szTime[MAX_TIMESTAMP];          //  create time
}TIndex;

typedef struct  __SYS_TVM_FIELD
{
    TABLE   m_table;                          //  table
    long    m_lSeq;                           //  filed seq
    char    m_szOwner[MAX_FIELD_LEN];         //  owner
    char    m_szTable[MAX_FIELD_LEN];         //  table name
    char    m_szField[MAX_FIELD_LEN];         //  field name
    long    m_lAttr;                          //  attr
    long    m_lFrom;                          //  field from
    long    m_lLen;                           //  field length
    long    m_lIsPk;
}TField;

typedef struct  __SYS_TVM_DOMAIN
{
    BSock   m_skSock;
    TABLE   m_table;
    TABLE   m_mtable;
    long    m_lLock;
    long    m_lGroup;
    long    m_lKeepLive;
    long    m_lLastTime;
    long    m_lTimeOut;
    long    m_lTryMax;
    long    m_lTryTimes;
    long    m_lRowSize;
    long    m_lStatus;                         //  remote domain state
    long    m_lPers;                           //  perms
    long    m_lPort;
    long    m_lRelia;
    char    m_szIp[MAX_REMOTE_IP];
    char    m_szTable[MAX_FIELD_LEN];
    char    m_szPart[MAX_FIELD_LEN];
    char    m_szOwner[MAX_FIELD_LEN];
}TDomain;

typedef struct  __SYS_TVM_SEQUE
{
    char    m_szSQName[MAX_INDEX_LEN];         // Name of sequence
    uint    m_uIncrement;
}TSeque;

/*************************************************************************************************
    Operating handle
 *************************************************************************************************/
typedef struct __TBL_FILED_KEY
{
    Uenum   uDecorate;
    uint    uFldpos;
    uint    uFldlen;
}FdKey;

typedef struct __TBL_CONDIT_FLD
{
    uint    uFldcmp;
    FdKey   stFdKey[MAX_FILED_NUM];
}FdCond;

typedef struct  __TVM_WORK
{
    TABLE   m_table;
    long    m_lRowSize;
    long    m_lOperate;
    FdCond  m_stCond;
    FdCond  m_stUpdt;
    void    *m_pvData;
    void    *m_pvNew;
}TWork;

typedef struct  __TVM_RUNTIME
{
    void    *pstVoid;
    uint    m_lState;
    uint    m_lLocal;
    long    m_shmID;                          //  Memory Key
    long    m_semID;                          //  semaphore key
    long    m_lRowSize;                       //  Record block size
    bool    m_bAttch;                         //  Does it initialize
    void    *m_pvAddr;
    long    m_lCurLine;                       //  cursor line
    long    m_lCurType;                       //  cursor type
    void    *m_pvCurAddr;                     //  cursor address
}RunTime;

typedef struct  __TVM_OPERATE
{
    size_t  lSize;                            //  struck size
    Uenum   lFind;                            //  find type
    Uenum   bSearch;                          //  find table type
    TABLE   tblName;                          //  table
    void    *pstVoid;                         //  condition
    FdCond  stCond;
    FdCond  stUpdt;

    bool    m_bWork;                          //  work
    bool    m_bPrew;                          //  work on or off
    bool    m_bHold;                          //  memory hold
    bool    m_bCreat;
    CMList  *m_pstWork;                       //  work list
    uint    m_lTimes;
    BSock   m_skSock;
    Uenum   m_lEType;
    key_t   m_yKey;
    key_t   m_ySey;
    key_t   m_yMey;
    Uenum   m_lErrno;
    size_t  m_lEffect;                        //  effect record line
    bool    m_bCache;
    char    m_szMsg[256];                     //  Custom message
    char    m_szNode[MAX_FIELD_LEN];
    RunTime stRunTime[TVM_MAX_TABLE];         //  table handle
}SATvm;

typedef    struct    __TVM_BOOT_PARAM
{
    long    m_lMaxTable;
    long    m_lMaxField;
    long    m_lMaxDomain;
    long    m_lMaxSeque;
    long    m_lBootExec;
    long    m_lBootPort;
    long    m_lBootType;
    char    m_szNode[MAX_FIELD_LEN];
    char    m_szLog[MAX_STRIG_LEN];
}TBoot;

/*************************************************************************************************
    内部函数
 *************************************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
extern    char*    sGetLog();
extern    char*    sGetNode();
extern    void*    pGetBoot();
extern    void*    pGetSATvm();
extern    long     lDefaultBoot();
extern    TBoot*   pBootInitial();
extern    size_t   lGetTblRow(TABLE t);
extern    long     lGetPermit(TABLE t);
extern    long     lGetRowSize(TABLE t);
extern    TblDef*  pGetTblDef(TABLE t);
extern    TblKey*  pGetTblIdx(TABLE t);
extern    uint     lGetIdxNum(TABLE t);
extern    uint     lGetGrpNum(TABLE t);
extern    size_t   lGetTblData(TABLE t);
extern    size_t   lGetListOfs(TABLE t);
extern    size_t   lGetRowTruck(TABLE t);
extern    size_t   lGetIdxPos(TABLE t);
extern    size_t   lGetGrpPos(TABLE t);
extern    long     lGetFldNum(TABLE t);
extern    size_t   lGetIdxRoot(TABLE t);
extern    long     lGetTblGroup(TABLE t);
extern    size_t   lGetTblValid(TABLE t);
extern    TblKey*  pGetTblGrp(TABLE t);
extern    TblKey*  pGetTblKey(TABLE t);
extern    RWLock*  pGetRWLock(char* pvAddr);
extern    void     vRedeError(long err, char *s);
extern    void*    pGetAddr(SATvm *pstSavm, TABLE t);
extern    RunTime* pGetRunTime(SATvm *pstSavm, TABLE t);
extern    void*    pGetNode(void *pvData, size_t lOfs);
extern    void*    pInitMemTable(SATvm *pstSavm, TABLE t);
extern    void*    pInitHitTest(SATvm *pstSavm, TABLE t);
extern    long     lTableMaxRow(SATvm *pstSavm, TABLE t);
extern    key_t    yGetIPCPath(SATvm *pstSavm, Benum em);
extern    long     lGetBootConfig(SATvm *pstSavm, TBoot *pstBoot);
extern    long     lAddIdxField(TABLE, long, long, long, long);
extern    long     lSetTableIdx(TABLE, long, long, char*, long, long);
extern    long     lUpdIndexPart(SATvm *pstSavm, TABLE t, char *pszPart);
extern    TblKey*  pFindField(TblKey *pstIdx, long lNum, char *pszField);
extern    long     lGetTblField(TABLE t, size_t *plOut, TField **ppstField);
extern    void     vSetCodField(FdCond *pstCond, uint ulen, uint uPos);
extern    bool     bSetCondAttr(FdCond *pstCond, TABLE t, Uenum eCheck);
extern    void     vSetDecorate(FdCond *pstCond, uint ulen, uint uPos, Uenum em);
extern    long     lGetDomainIndex(SATvm *pstSavm, long *plOut, TIndex **ppstIndex);
extern    long     lGetDomainTable(SATvm *pstSavm, long *plOut, TDomain **ppstDomain);
extern    long     lGetLocalIndex(SATvm *pstSavm, long *plOut, TIndex **ppstIndex);
extern    long     lGetTblIndex(SATvm *pstSavm, char *pszTable, char *pszPart, TIndex *pstIndex);

/*************************************************************************************************
    config make&unmake
 *************************************************************************************************/
extern    long    lUnmakeConfig(char *pszFile);
extern    long    lMakeConfig(char *pszFile);

/*************************************************************************************************
    IPC Message and semaphore
 *************************************************************************************************/
extern    long     lGetQueueNum(SATvm *pstSavm, long lQid);
extern    long     lQueueMaxByte(SATvm *pstSavm, long lQid);
extern    long     lQueueRcvTime(SATvm *pstSavm, long lQid);
extern    long     lCreateQueue(SATvm *pstSavm, bool bCreate);
extern    long     lOperateSems(SATvm *pstSavm, long semID, long lSems, Benum evp);
extern    long     lEventWrite(SATvm *pstSavm, long lQid, void *psvData, long lSize);
extern    long     lCreateSems(SATvm *pstSavm, RunTime *pstRun, long lSems, long lValue);
extern    long     lEventRead(SATvm *pstSavm, long lQid, void *pstVoid, long lSize, long lMType);
extern    long     lReadNoWait(SATvm *pstSavm, long lQid, void *psvVoid, long lSize, long lMType);

/*************************************************************************************************
    api
 *************************************************************************************************/
extern    long     lShutdownTvm();
extern    long     lStartupTvm(TBoot *pstBoot);

extern    char*    sGetUpdTime();
extern    long     lGetTErrno();
extern    void     vSetTvmMsg(SATvm *pstSavm, char *fmt, ...);
extern    char*    sGetTvmMsg(SATvm *pstSavm);
extern    void     vSetTErrno(long err);
extern    char*    sGetTError(long err);
extern    size_t   lGetEffect();
extern    bool     bIsTvmBoot();
extern    void*    pInitSATvm(TABLE t);
extern    long     lInitSvCache(SATvm *pstSavm);
extern    void     vInitSATvm(SATvm *pstSavm);
extern    bool     bTableIsExist(TABLE t);
extern    bool     bPartIsExist(char *pszTable, char *pszPart);
extern    long     lInitSATvm(SATvm *pstSavm, TABLE t);
extern    void*    pPartSatvm(SATvm *pstSavm, char *pszTable, char *pszPart);

extern    long     lResetLock(SATvm *pstSavm, TABLE t);
extern    long     lRebuildIndex(SATvm *pstSavm, TABLE t);
extern    void     vHoldConnect(SATvm *pstSavm);
extern    void     vHoldRelease(SATvm *pstSavm);
extern    void     vTblDisconnect(SATvm *pstSamo, TABLE t);
extern    void     vForceDisconnect(SATvm *pstSamo, TABLE t);
extern    void     vBeginWork(SATvm *pstSavm);
extern    void     vEndWork(SATvm *pstSavm);
extern    long     lCommitWork(SATvm *pstSavm);
extern    long     lRollbackWork(SATvm *pstSavm);
extern    long     lDropTable(SATvm *pstSavm, TABLE t);
extern    long     lImportFile(TABLE t, char *pszFile, char *pszFlag);
extern    long     lExportFile(TABLE t, char *pszFile, char *pszFlag);
extern    long     lImportTable(TABLE t, size_t lCount, void *psvOut);
extern    long     lExportTable(TABLE t, size_t *plOut, void **ppsvOut);

extern    long     lRenameTable(SATvm *pstSavm, TABLE to, TABLE tn);
extern    long     lCreateSeque(SATvm *pstSavm, char *pszSQName, uint uIncre);
extern    long     lSelectSeque(SATvm *pstSavm, char *pszSQName, ulong *pulNumber);
extern    long     lSetSequence(SATvm *pstSavm, char *pszSQName, ulong uStart);
extern    long     lCustomTable(SATvm *pstSavm, TABLE t, size_t lRow, TblDef *pstDef);
extern    long     lCreateTable(SATvm *pstSavm, TABLE t, size_t lRow, TCREATE pfCreateFunc);
extern    long     lInsertTrans(SATvm *pstSavm, size_t *plOffset, llSEQ *pllSeq);


extern    long     lDelete(SATvm *pstSavm);
extern    long     lInsert(SATvm *pstSavm);
extern    long     lTruncate(SATvm *pstSavm, TABLE t);
extern    long     lSelect(SATvm *pstSavm, void *psvOut);
extern    long     lUpdate(SATvm *pstSavm, void *psvUpd);
extern    long     lCount(SATvm *pstSavm, size_t *plCount);
extern    long     lExtreme(SATvm *pstSavm, void *psvOut);
extern    long     lGroup(SATvm *pstSavm, size_t *plOut, void **ppsvOut);
extern    long     lQuery(SATvm *pstSavm, size_t *plOut, void **ppsvOut);

extern    long     lTableDeclare(SATvm *pstSavm);
extern    long     lTableFetch(SATvm *pstSavm, void *psvOut);
extern    long     lNextFetch(SATvm *pstSavm, void **ppvOAddr);
extern    void     vTableClose(SATvm *pstSavm);
#ifdef __cplusplus
}
#endif

/*************************************************************************************************
    code end
 *************************************************************************************************/
#endif    //    __TVM_DEFIND_HHHH___
