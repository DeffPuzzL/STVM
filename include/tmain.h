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

#ifndef _HHH_TVM_DOMAIN_HHH__
#define _HHH_TVM_DOMAIN_HHH__
#include "tvm.h"


typedef struct  epoll_event     epollevt;
typedef long    (*FUNCEXEC)(SATvm *pstSavm, void *arg);

/*************************************************************************************************
  
 *************************************************************************************************/
#define SET_BLOCK                           0
#define SET_UNBLOCK                         1
#define READ_BUFFER                         4096            
#define MAX_EVENTS                          5000
#define READ_MAX_LEN                        1024 * 10
#define DATA_MAX_LEN                        1048576    // 1024*1024   Maximum self-protection single record 1M
#define MAX_CON_EVENTS                      65535
#define TVM_PORT_LISTEN                     1801
#define TVM_PORT_DOMAIN                     1800
#define TVM_LOCAL_SERV                      "LIS.tvm"
#define TVM_REMOTE_DOM                      "RDS.tvm"
#define LOCAL_HOST_IP                       "127.0.0.1"


/*************************************************************************************************
    表结构&索引定义区
 *************************************************************************************************/
typedef struct __SOCK_CONNECT
{
    char    m_szCltIp[16];
    int     m_lCltPort;
    BSock   m_skSock;
    int     m_isListen;
    BOOL    m_bWork;
    ulong   m_uWorker;
    CMList  *m_pstWork;
}SKCon;

typedef struct  __TVM_INTERFACE
{
    Benum   m_enum;
    Uenum   m_lFind;
    TABLE   m_table;
    uint    m_lDLen;
    uint    m_lErrno;
    size_t  m_lRows;
}TFace;

/*************************************************************************************************
    macro
 *************************************************************************************************/

/*************************************************************************************************
    function
 *************************************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

extern void       lInitTitle(int argc, char **argv, char **envp);
extern void       vSetTitile(const char *name);
extern void       vDeleteRowgrp(Rowgrp *pstNode);
extern Rowgrp*    pFindRowList(Rowgrp *root, long idx);
extern void       vSortRowgrp(Rowgrp **pproot, TblKey *pstKey, Benum em);
extern long       lCountRowgrp(Rowgrp *pstRoot, void *psvData, long lLen, size_t lCount);
extern long       lSortRowList(SATvm *, size_t lRow, void *pvData, size_t lTruck);
extern long       lConvRowList(SATvm *pstSavm, Rowgrp *root, size_t *plOut, void **ppsvOut);
extern long       lParsRowList(SATvm *pstSavm, void *pszBuffer, long lData, Rowgrp **root);
extern Rowgrp*    pInsertRowgrp(SATvm *, Rowgrp *, Rowgrp *, Rowgrp *, void *, long , size_t );
extern bool       bRepeatLstgrp(FdCond *pstRrp, void *pvData, TABLE t, size_t lOut, void *psvOut);
extern long       lInsertLstgrp(SATvm *, FdCond *, void *, TABLE , size_t *, void **ppsvOut);

extern bool       bExistProcess(long lPid);
extern void       vInitTitle(int argc, char **argv, char **envp);
extern long       lBootLocal(SATvm *pstSavm, TBoot *pstBoot, Benum eMode);
extern long       lBootDomain(SATvm *pstSavm, TBoot *pstBoot, long lMode);
extern long       lMultListen(SATvm *pstSavm, long lPort, long , FUNCEXEC , void *arg, FUNCEXEC );
extern long       lOfflineNotify(SATvm *pstSavm, long lPort);
extern long       lRefreshNotify(SATvm *pstSavm, long lPort);
extern long       lPullNotify(SATvm *pstSavm, TDomain *pstDom, size_t lCount);
extern long       lConnectNotify(SATvm *pstSavm, TDomain *pstDom, long lPort);
extern long       lTvmGetTblIndex(SATvm *pstSavm, char *pszTable, char *pszPart, TIndex *pstIndex);
extern long       lTvmGetTblField(SATvm *pstSavm, TABLE t, size_t *plOut, TField **ppstField);



extern long       lTvmBeginWork(SATvm *pstSavm);
extern long       lTvmRollbackWork(SATvm *pstSavm);
extern long       lTvmCommitWork(SATvm *pstSavm);
extern long       lTvmEndWork(SATvm *pstSavm);
extern void       vTvmDisconnect(SATvm *pstSavm);
extern long       lTvmConnect(SATvm *pstSavm, char *pszIp, long lPort, int times);
extern long       lTvmTruncate(SATvm *pstSavm, TABLE t);
extern long       lTvmGroup(SATvm *pstSavm, size_t *plOut, void **ppvOut);
extern long       lTvmCount(SATvm *pstSavm, size_t *plCount);
extern long       lTvmInsert(SATvm *pstSavm);
extern long       lTvmSelect(SATvm *pstSavm, void *pvOut);
extern long       lTvmQuery(SATvm *pstSavm, size_t *plOut, void **ppvOut);
extern long       lTvmUpdate(SATvm *pstSavm, void *pvData);
extern long       lTvmDelete(SATvm *pstSavm);
extern long       lTvmExtreme(SATvm *pstSavm, void *pvOut);
extern long       lTvmDropTable(SATvm *pstSavm, TABLE t);
extern long       lTvmRenameTable(SATvm *pstSavm, TABLE to, TABLE tn);
extern long       lTvmSelectSeque(SATvm *pstSavm, char *pszSQName, ulong *pulNumber);
extern long       lTvmSetSequence(SATvm *pstSavm, char *pszSQName, ulong uStart);
extern long       lTvmRebuildIndex(SATvm *pstSavm, TABLE t);
extern long       lTvmResetLock(SATvm *pstSavm, TABLE t);

#ifdef __cplusplus
}
#endif

#endif    //    _HHH_TVM_DOMAIN_HHH__
