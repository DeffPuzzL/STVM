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

#include    "tmain.h"

/*************************************************************************************************
    statement
 *************************************************************************************************/
extern char **environ;
static char *g_Last = NULL;
static char **g_Argv = NULL;
static int  g_eRun = 1;
Rowgrp *g_pstDomgrp = NULL, *g_pstTblgrp = NULL;

extern char*   pGetLog();
extern long    lGetBootType();
extern void    vSetBootType(long lType);
void*   pParsePacket(SATvm *pstSavm, void *pstVoid, TFace *pstFace, void *pvBuffer, long lLen);
void*   pProtocaJava(SATvm *pstSavm, void *pstVoid, TFace *pstFace, void *pvBuffer, long lLen);

/*************************************************************************************************
    macro
 *************************************************************************************************/
#define Tlog(...)           vTraceLog(__FILE__, __LINE__, __VA_ARGS__)

#define checkrequest(s,c,f) if(MAX(f->m_lRows, f->m_lDLen) > c->m_lBuffer)   \
                            { \
                               if(MAX(f->m_lRows, f->m_lDLen) > DATA_MAX_LEN) \
                               {  \
                                    s->m_lErrno = RECD_TOO_LONG;  \
                                    goto LISTEN_ERROR; \
                               }  \
                               c->m_lBuffer = MAX(f->m_lRows, f->m_lDLen);  \
                               if(NULL == (c->pstFace = (void *)realloc(c->pstFace, c->m_lBuffer + sizeof(TFace))))  \
                               { \
                                     s->m_lErrno = MALLC_MEM_ERR; \
                                     goto LISTEN_ERROR; \
                               } \
                               f = (TFace *)c->pstFace; \
                               c->pvData = c->pstFace + sizeof(TFace); \
                               if(NULL == (c->pstVoid = (void *)realloc(c->pstVoid, c->m_lBuffer + sizeof(TFace)))) \
                               { \
                                    s->m_lErrno = MALLC_MEM_ERR;  \
                                    goto LISTEN_ERROR; \
                               } \
                            } 
                                 
#define checkbuffer(p, r, n)     if((p->lSize * n + r->m_lCurLine) > r->m_lRowSize) \
                                 { \
                                     if(r->m_lRowSize > DATA_MAX_LEN) \
                                     { \
                                         p->m_lErrno = RECD_TOO_LONG; \
                                         return RC_FAIL; \
                                     } \
                                     r->m_lRowSize = p->lSize + r->m_lCurLine; \
                                     if(NULL == (r->pstVoid = (void *)realloc(r->pstVoid, r->m_lRowSize))) \
                                     {  \
                                         p->m_lErrno = MALLC_MEM_ERR;  \
                                         return RC_FAIL;  \
                                     }  \
                                 }

/*************************************************************************************************
   function
 *************************************************************************************************/
/*************************************************************************************************
    description：get root of domain list
    parameters:
    return:
        void*                      --root
  *************************************************************************************************/
Rowgrp*  pGetDomgrp()
{
    return g_pstDomgrp;
}

/*************************************************************************************************
    description：get root of table list
    parameters:
    return:
        void*                      --root
  *************************************************************************************************/
Rowgrp*  pGetTblgrp()
{
    return g_pstTblgrp;
}

/*************************************************************************************************
    description：checkt the child process is already started
    parameters:
        lPid                       --pid
    return:
        true                       --exist
        false                      --disappear
  *************************************************************************************************/
bool    bExistProcess(long lPid)
{
    errno = 0;
    if(getpgid(lPid) == -1)
    {
        if(ESRCH == errno)    return false;
        else                return true;
    }

    return true;
}

/*************************************************************************************************
    description：trace log
    parameters:
    return:
  *************************************************************************************************/
void    vTraceLog(const char *pszFile, int nLine, const char *fmt, ...)
{
    va_list ap;
    FILE    *fp = NULL;
    char    szMsg[5120];
    struct  timeb   tb;
    struct  tm      *ptm = NULL;

    memset(szMsg, 0, sizeof(szMsg));
    va_start(ap, fmt);
    vsnprintf(szMsg, sizeof(szMsg), fmt, ap);
    va_end(ap);

    if(NULL == (fp = fopen(pGetLog(), "a+")))
    {
        fprintf(stderr, "P(%d), open (%s) failed, err:%s,[%s]\n", getpid(),
            pGetLog(), strerror(errno), szMsg);
        return ;
    }

    ftime(&tb);
    ptm = localtime(&tb.time);
    fprintf(fp, "F=%-8s L=%-5d P=%-7d T=%-7ld T=%04d%02d%02d %02d%02d%02d:%03d  %s\n",
        pszFile, nLine, getpid(), syscall(SYS_gettid), ptm->tm_year + 1900, ptm->tm_mon + 1,
        ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, tb.millitm, szMsg);
    fclose(fp);
    return ;
}

/*************************************************************************************************
    description：Show cloning progress
    parameters:
        nVaild                     --vaild 
        nMax                       --max 
    return:
  *************************************************************************************************/
void    vPrintProgresss(int nValid, int nMax)
{
    double  dPer;
    int     i, nPer;
    static  int len = 0;
    char    szPrint[64];

    for(i = len + 50; i > 0 && nValid > 1; i --)
        fprintf(stdout, "\b");

    dPer = nValid * 100.0 / nMax;
    nPer = nValid * 50 / nMax > 0 ? nValid * 50 / nMax : 1;
    if(dPer < 60.00)
        fprintf(stdout, "\033[42;32m");
    else if(dPer < 70.00)
        fprintf(stdout, "\033[45;35m");
    else if(dPer < 80.00)
        fprintf(stdout, "\033[46;36m");
    else  if(dPer < 90.00)
        fprintf(stdout, "\033[43;33m");
    else
        fprintf(stdout, "\033[41;31m");

    fflush(stdout);
    for(i = 0; i < nPer; i ++)
        fprintf(stdout, "|");

    fprintf(stdout, "\033[0m");
    for(i; i < 50; i ++)
        fprintf(stdout, " ");

    memset(szPrint, 0, sizeof(szPrint));
    len = snprintf(szPrint, sizeof(szPrint), "] %6.3f%%, (%d/%d)", dPer, nValid, nMax);
    fprintf(stdout, "%s", szPrint);
    fflush(stdout);
}

/*************************************************************************************************
    description：Initialize the startup process name
    parameters:
        argc
        **argv
        **envp
    return:
  *************************************************************************************************/
void    vInitTitle(int argc, char **argv, char **envp)
{
    int    i = 0;

    for(i = 0; envp[i] != NULL; i++) // calc envp num
           continue;

    environ = (char **) malloc(sizeof (char *) * (i + 1));
    for (i = 0; envp[i] != NULL; i++)
    {
        environ[i] = malloc(sizeof(char) * strlen(envp[i]) + 1);
        memset(environ[i], 0, sizeof(char) * strlen(envp[i]) + 1);
        strcpy(environ[i], envp[i]);
    }

    environ[i] = NULL;
    g_Argv = argv;
    if (i > 0)
      g_Last = envp[i - 1] + strlen(envp[i - 1]);
    else
      g_Last = argv[argc - 1] + strlen(argv[argc - 1]);
}

/*************************************************************************************************
    description：set current process name
    parameters:
        pname                      --process name
    return:
  *************************************************************************************************/
void    vSetTitile(const char *pname)
{
    int     i;
    char    *p, name[16];
    extern char **g_Argv;
    extern char *g_Last;

    strncpy(name, pname, 16);
    i = strlen(name);
    if (i > g_Last - g_Argv[0] - 2)
    {
        i = g_Last - g_Argv[0] - 2;
        name[i] = '\0';
    }

    (void) strcpy(g_Argv[0], name);
    p = &g_Argv[0][i];
    while (p < g_Last)
        *p++ = '\0';
    g_Argv[1] = NULL;
    prctl(PR_SET_NAME, name);
}

/*************************************************************************************************
    description：get tick time
    parameters:
    return:
        uint64_t
 *************************************************************************************************/
uint64_t get_tick_time()
{
    struct timeval tval;
    uint64_t tick;

    gettimeofday(&tval, NULL);

    tick = tval.tv_sec * 1000L + tval.tv_usec / 1000L;

    return tick;
}

/*************************************************************************************************
    description：Set non-blocking
    parameters:
        skSock                     --socket
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lSetUnBlock(BSock skSock)
{
    int     nOpt = 0;

    if(fcntl(skSock, F_GETFL) < 0)
        return RC_FAIL;

    nOpt = nOpt | O_NONBLOCK;
    if(fcntl(skSock, F_SETFL, nOpt) < 0)
        return RC_FAIL;

    return RC_SUCC;
}

/*************************************************************************************************
    description：Set blocking
    parameters:
        skSock                     --socket
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lSetBlock(BSock skSock)
{
    int     nOpt = 0;

    if(fcntl(skSock, F_GETFL) < 0)
        return RC_FAIL;

    nOpt &= ~O_NONBLOCK;
    if(fcntl(skSock, F_SETFL, nOpt) < 0)
        return RC_FAIL;

    return RC_SUCC;
}

/*************************************************************************************************
    description：connect server
    parameters:
        pstSavm                    --stvm handle
        pszIp                      --ip
        lPort                      --port
        bf                         --blocking and no-blocking
        lTime                      --time out
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
BSock    skConnectServer(SATvm *pstSavm, char *pszIp, long lPort, bool bf, long lTime)
{
    fd_set  set, exp;
    struct  timeval tv;
    struct  linger  lin;
    BSock   skSock;
    struct  sockaddr_in stAdr;
    int     error = -1, len = sizeof(int);

    memset(&stAdr, 0, sizeof(struct sockaddr_in));
    stAdr.sin_family      = AF_INET;
    stAdr.sin_addr.s_addr = inet_addr(pszIp);
    stAdr.sin_port        = htons((u_short)lPort);

    if((skSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        pstSavm->m_lErrno = SOCK_ERR_CRTE;
        return RC_FAIL;
    }

    memset(&lin, 0, sizeof(lin));
    lin.l_onoff = true;
    lin.l_linger = 10;
    if (0 > setsockopt(skSock, SOL_SOCKET, SO_LINGER, &lin, sizeof(lin)))
    {
        close(skSock);
        return RC_FAIL;
    }

    if(RC_SUCC != lSetUnBlock(skSock))
    {
        close(skSock);
        return RC_FAIL;
    }

    if(RC_SUCC == connect(skSock, (struct sockaddr *)(&stAdr), sizeof(struct sockaddr_in)))
    {
        if(!bf)    lSetBlock(skSock);
        return skSock;
    }

    if(errno != EINPROGRESS)
    {
        close(skSock);
        pstSavm->m_lErrno = SOCK_CONN_ERR;
        return RC_FAIL;
    }

    FD_ZERO(&set);
    FD_ZERO(&exp);
    FD_SET(skSock, &set);
    tv.tv_sec = lTime;
    tv.tv_usec = 0;
    errno = 0;
    if(RC_SUCC >= select(skSock + 1, NULL, &set, &exp, &tv))
    {
        close(skSock);
        pstSavm->m_lErrno = SOCK_CONN_TMO;
        return RC_FAIL;
    }

    if(!FD_ISSET(skSock, &set) ||  FD_ISSET(skSock, &exp))  //异常套接字就绪
    {
        close(skSock);
        pstSavm->m_lErrno = SOCK_CONN_ERR;
        return RC_FAIL;
    }

#ifdef HP_UX
    getsockopt(skSock, SOL_SOCKET, SO_ERROR, &error, &len);
#else    // linux
    getsockopt(skSock, SOL_SOCKET, SO_ERROR, &error,(socklen_t*)&len);
#endif
    if(!bf)    lSetBlock(skSock);    //   set block
    if(0 == error)    return skSock;
    
    pstSavm->m_lErrno = SOCK_CONN_ERR;
    close(skSock);
    return RC_FAIL;
}

/*************************************************************************************************
    description：server initail
    parameters:
        pstSavm                    --stvm handle
        lPort                      --port
    return:
        BSock                      --socket
 *************************************************************************************************/
BSock    skServerInitail(SATvm *pstSavm, int lPort)
{
    int     iret = -1;
    int     optval = 1;
    BSock    skSock = -1;
    struct sockaddr_in serveraddr;

    memset(&serveraddr, 0, sizeof(struct sockaddr_in));
    if((skSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        pstSavm->m_lErrno = SOCK_ERR_CRTE;
        return RC_FAIL;
    }

    if(RC_SUCC != lSetUnBlock(skSock))
    {
        close(skSock);
        return RC_FAIL;
    }

    if (0 > setsockopt(skSock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)))
    {
        close(skSock);
        return RC_FAIL;
    }

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(lPort);

    if (0 > bind(skSock, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr_in)))
    {
        close(skSock);
        pstSavm->m_lErrno = SOCK_BIND_ERR;
        return RC_FAIL;
    }

    if (0 > listen(skSock, 1024))
    {
        close(skSock);
        pstSavm->m_lErrno = SOCK_LSEN_ERR;
        return RC_FAIL;
    }

    return skSock;
}

/*************************************************************************************************
    description：send buffer with no-blocking
    parameters:
        pstSavm                    --stvm handle
        skSock                     --socket
        ss                         --buffer
        len                        --the length of buffer
    return:
        int                        --Number of bytes sent
 *************************************************************************************************/
int    lSendUnblock(SATvm *pstSavm, int skSock, char *ss, int len)
{
    long     lByte = 0, lLeft = len, lWrite = 0;

    while(lLeft > 0)
    {
        if((lWrite = send(skSock, ss + lByte, lLeft, MSG_DONTWAIT)) <= 0)
        {
            if(EWOULDBLOCK == errno|| EAGAIN == errno)
                return lByte;
            else
            {
                pstSavm->m_lErrno = SOCK_SEND_ERR;
                return RC_FAIL;
            }
        }
        else
        {
            lLeft -= lWrite;
            lByte += lWrite;
        }
    }

    return lByte;
}

/*************************************************************************************************
    description：recv buffer with no-blocking
    parameters:
        pstSavm                    --stvm handle
        skSock                     --socket
        ss                         --buffer
        len                        --the length of buffer
    return:
        int                        --Number of bytes recv
 *************************************************************************************************/
int     lRecvUnblock(SATvm *pstSavm, int skSock, char *so, int read)
{
    int     lByte = 0, lRecv = 0;

    while(read > lRecv)
    {
        lByte = recv(skSock, so + lRecv, read - lRecv, MSG_DONTWAIT);
        if(lByte < 0)
        {
            if(EAGAIN == errno || EWOULDBLOCK == errno)
                return lRecv;
            else if(errno == ECONNRESET || ENETRESET == errno || ENETDOWN == errno ||
                EINTR == errno)
            {
                pstSavm->m_lErrno = SOCK_IO_RESET;
                return RC_FAIL;
            }
            else
            {
                pstSavm->m_lErrno = SOCK_READ_ERR;
                return RC_FAIL;
            }
        }
        else if(lByte == 0)
        {
            pstSavm->m_lErrno = SOCK_IO_RESET;
            return RC_FAIL;
        }

        lRecv += lByte;
    }

    return lRecv;
}

/*************************************************************************************************
    description：recv buffer with blocking
    parameters:
        skSock                     --socket
        pszRecv                    --buffer
        lRead                      --the byte of read
    return:
        int                        --Number of bytes recv
 *************************************************************************************************/
int    lRecvBuffer(int skSock, char *pszRecv, int lRead)
{
    int    lByte = 0, lRecv = 0;

    errno = 0;
    while(lRead > lRecv)
    {
        lByte = recv(skSock, pszRecv + lRecv, lRead - lRecv, 0);
        if(lByte < 0)
        {
            if(EAGAIN == errno || EWOULDBLOCK == errno)
                return lRecv;

            //  Connection reset by peer
            if(errno == ECONNRESET || ENETRESET == errno || ENETDOWN == errno ||
                EINTR == errno)
                return RC_FAIL;
            else
                return RC_FAIL;
        }
        else if(lByte == 0)
            return RC_CLOSE;

        lRecv += lByte;
    }

    return lRecv;
}

/*************************************************************************************************
    description：send buffer with blocking
    parameters:
        skSock                     --socket
        pszSend                    --buffer
        lSend                      --the byte of send
    return:
        int                        --Number of bytes send
 *************************************************************************************************/
int     lSendBuffer(BSock skSock, void *pszSend, int lSend)
{
    long    lByte = 0, lLeft = lSend, lWrite = 0;

    errno = 0;
    while(lLeft > 0)
    {
        if((lWrite = send(skSock, pszSend + lByte, lLeft, 0)) <= 0)
            return lByte;
        else
        {
            lLeft -= lWrite;
            lByte += lWrite;
        }
    }

    return lByte;
}

/*************************************************************************************************
    description：close socket of domain
    parameters:
        pstDom                     --domain
    return:
 *************************************************************************************************/
void    vCloseSocket(TDomain *pstDom)
{
    shutdown(pstDom->m_skSock, SHUT_RDWR);
    close(pstDom->m_skSock);
    pstDom->m_skSock = -1;
}

/*************************************************************************************************
    description：find table from table list
    parameters:
        t                          --table
    return:
        list                       --rowgrp node
 *************************************************************************************************/
Rowgrp*    pGetTblNode(TABLE t)
{
    Rowgrp  *list = NULL;

    for(list = pGetTblgrp(); list; list = list->pstNext)
    {
        if(!memcmp(&t, list->psvData, sizeof(TABLE)))
            return list;    
    }

    return NULL;
}

/*************************************************************************************************
    description：update remote table perms
    parameters:
        pstDom                     --domain 
        pstIndex                   --retmote table
    return:
 *************************************************************************************************/
void   vUpdateDomPers(TDomain *pstDom, TIndex *pstIndex)
{
    TDomain *pvm;
    Rowgrp  *list, *node = NULL;

    for(list = pGetTblgrp(); list; list = list->pstNext)
    {
        if(memcmp(&pstIndex->m_table, list->psvData, sizeof(TABLE)))
            continue;
 
        for(node = list->pstSSet; node; node = node->pstNext)
        {
            if(NULL == (pvm = (TDomain *)(list->pstFset->psvData)))
                continue;

            if(strcmp(pvm->m_szIp, pstDom->m_szIp) || pvm->m_lPort != pstDom->m_lPort)
                continue;

            pvm->m_lPers = pstIndex->m_lPers;
            return ;
        }
    }
}

/*************************************************************************************************
    description：find domain from rowgrp
    parameters:
        pszIp                      --ip
        lPort                      --port
    return:
        list                       --rowgrp node
 *************************************************************************************************/
Rowgrp*    pGetDomnode(char *pszIp, long lPort)
{
    Rowgrp  *list = NULL;
    TDomain *pstDom = NULL;

    for(list = pGetDomgrp(); list; list = list->pstNext)
    {
        pstDom = (TDomain *)list->psvData;
        if(!strcmp(pstDom->m_szIp, pszIp) && pstDom->m_lPort == lPort)
            return list;
    }

    return NULL;
}

/*************************************************************************************************
    description：find domain 
    parameters:
        pszIp                      --ip
        lPort                      --port
    return:
        domain                     --domain
 *************************************************************************************************/
TDomain*    pGetDomain(char *pszIp, long lPort)
{
    Rowgrp  *list = NULL;
    TDomain *pstDom = NULL;

    for(list = pGetDomgrp(); list; list = list->pstNext)
    {
        pstDom = (TDomain *)list->psvData;
        if(!strcmp(pstDom->m_szIp, pszIp) && pstDom->m_lPort == lPort)
            return pstDom;
    }

    return NULL;
}

/*************************************************************************************************
    description：Initialize the network connection table handle
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmBuffer(SATvm *pstSavm)
{
    RunTime *pstRun;

    if(!pstSavm)    return RC_FAIL;

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(pstRun->m_lRowSize > 0 && pstRun->pstVoid)
        return RC_SUCC;

    pstRun->m_lCurLine = sizeof(pstSavm->stCond) + sizeof(pstSavm->stUpdt) + sizeof(TFace);
    pstRun->m_lRowSize = READ_MAX_LEN > pstRun->m_lCurLine ? READ_MAX_LEN : pstRun->m_lCurLine;
    if(NULL == (pstRun->pstVoid = (void *)calloc(pstRun->m_lRowSize, 1)))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：Update local domain information
    parameters:
        pstSavm                    --stvm handle
        pszId                      --ip
        lPort                      --lPort
        lStatus                    --status
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lUpdateDomain(SATvm *pstSavm, char *pszIp, long lPort, long lStatus)
{
    TDomain   stUpdate, stDomain;

    if(RC_SUCC != lInitSATvm(pstSavm, SYS_TVM_DOMAIN))
        return RC_FAIL;

    updateinit(pstSavm, stUpdate);
    conditinit(pstSavm, stDomain, SYS_TVM_DOMAIN);
    conditnum(pstSavm, stDomain, m_lPort, lPort)
    conditstr(pstSavm, stDomain, m_szIp, pszIp);

    updatenum(pstSavm, stUpdate, m_lStatus, lStatus);
    if(RESOURCE_ABLE != lStatus) 
        updatenum(pstSavm, stUpdate, m_lPers, 0);
    if(RC_SUCC != lUpdate(pstSavm, (void *)&stUpdate))
    {
        Tlog("update domain (%s:%d) failure, %s\n", pszIp, lPort, sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：Update local resources based on remote tables
    parameters:
        pstSavm                    --stvm handle
        pstFace                    --request head
        pstDom                     --domain info
        pstIndx                    --remote resource list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lRemodeIndex(SATvm *pstSavm, TFace *pstFace, TDomain *pstDom, TIndex *pstIndex)
{
    long      i;
    TDomain   stDomain, stRemote;

    pstFace->m_lRows = pstFace->m_lRows / pstFace->m_lDLen;
    if(RC_SUCC != lInitSATvm(pstSavm, SYS_TVM_DOMAIN))
        return RC_FAIL;

    conditinit(pstSavm, stDomain, SYS_TVM_DOMAIN);
    conditnum(pstSavm, stDomain, m_lPort, pstDom->m_lPort);
    conditstr(pstSavm, stDomain, m_szIp, pstDom->m_szIp);
    updatenum(pstSavm, stRemote, m_lStatus, RESOURCE_AUTH);
    updatenum(pstSavm, stRemote, m_lPers, 0);
    if(RC_SUCC != lUpdate(pstSavm, (void *)&stRemote))
    {
        if(NO_DATA_FOUND != pstSavm->m_lErrno)
           return RC_FAIL;
    }

    conditinit(pstSavm, stDomain, SYS_TVM_DOMAIN);
    for(i = 0; i < pstFace->m_lRows; i ++)
    {
        conditnum(pstSavm, stDomain, m_lPort, pstDom->m_lPort);
        conditnum(pstSavm, stDomain, m_mtable, pstIndex[i].m_table);
        conditstr(pstSavm, stDomain, m_szIp, pstDom->m_szIp);

        updateinit(pstSavm, stRemote);
        updatenum(pstSavm, stRemote, m_lLastTime, time(NULL));
        updatenum(pstSavm, stRemote, m_lTryTimes, 0);
        updatenum(pstSavm, stRemote, m_lStatus, RESOURCE_ABLE);
        updatenum(pstSavm, stRemote, m_skSock, pstDom->m_skSock);
        updatenum(pstSavm, stRemote, m_lPers, pstIndex[i].m_lPers);
        updatenum(pstSavm, stRemote, m_mtable, pstIndex[i].m_table);
        updatenum(pstSavm, stRemote, m_lRowSize, pstIndex[i].m_lRowSize);
        updatestr(pstSavm, stRemote, m_szPart, pstIndex[i].m_szPart);
        updatestr(pstSavm, stRemote, m_szTable, pstIndex[i].m_szTable);

        if(RC_SUCC != lUpdate(pstSavm, (void *)&stRemote))
        {
            Tlog("update local resource stat failed, %s, e(%d)", sGetTError(pstSavm->m_lErrno), 
                pstSavm->m_lEType);
            if(NO_DATA_FOUND == pstSavm->m_lErrno)
                continue;
            return RC_FAIL;
        }

        vUpdateDomPers(pstDom, &pstIndex[i]);
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：Upload local resources
    parameters:
        pstSavm                    --stvm handle
        pstFace                    --request head
        skSock                     --domain info
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lLocalIndex(SATvm *pstSavm, TFace *pstFace, BSock skSock)
{
    size_t  lWrite;
    TIndex  stIndex;
    void    *pvOut = NULL, *pvBuffer = NULL;

    pstFace->m_table = SYS_TVM_INDEX;
    pstFace->m_lDLen = sizeof(TIndex);

    if(RC_SUCC != lInitSATvm(pstSavm, pstFace->m_table))
        return RC_FAIL;

    conditinit(pstSavm, stIndex, SYS_TVM_INDEX);
    conditnum(pstSavm, stIndex, m_lType, TYPE_CLIENT);
    conditnum(pstSavm, stIndex, m_lLocal, RES_LOCAL_SID);
    if(RC_SUCC != lQuery(pstSavm, (size_t *)&pstFace->m_lRows, (void *)&pvOut))
    {
        if(NO_DATA_FOUND != pstSavm->m_lErrno)
            return RC_FAIL;
    }
    
    if(pstFace->m_lRows == 0)    
    {
        if(sizeof(TFace) != lSendBuffer(skSock, (void *)pstFace, sizeof(TFace)))
            return RC_FAIL;
        return RC_SUCC;
    }

    pstFace->m_lRows = pstFace->m_lDLen * pstFace->m_lRows;
    lWrite = pstFace->m_lRows + sizeof(TFace);
    if(NULL == (pvBuffer = (void *)malloc(lWrite)))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        TFree(pvOut);
        return RC_FAIL;
    }

    memcpy(pvBuffer, (void *)pstFace, sizeof(TFace));
    memcpy(pvBuffer + sizeof(TFace), (void *)pvOut, pstFace->m_lRows);
    lSendBuffer(skSock, pvBuffer, lWrite);
    TFree(pvOut);
    TFree(pvBuffer);

    return RC_SUCC;
}

/*************************************************************************************************
    description：Join in the domain
    parameters:
        pstSavm                    --stvm handle
        pstFace                    --request head
        lPort                      --local port
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lConnectDomain(SATvm *pstSavm, TDomain *pstDom, long lPort)
{
    TFace   stFace;
    TIndex  *pstIndex = NULL;

    memset(&stFace, 0, sizeof(TFace));
    if(pstDom->m_skSock < 0)
    {
        pstDom->m_skSock = skConnectServer(pstSavm, pstDom->m_szIp, pstDom->m_lPort, 
            false, pstDom->m_lTimeOut);
        if(RC_FAIL == pstDom->m_skSock)
        {
            pstDom->m_lStatus = RESOURCE_STOP;
            Tlog("connect server:%s:%d failed, %s", pstDom->m_szIp, pstDom->m_lPort, 
                sGetTError(pstSavm->m_lErrno));
            goto EXCP_NOTIFY;
        }
    }

    stFace.m_lRows  = 0;
    stFace.m_lFind  = lPort;
    stFace.m_table  = SYS_TVM_INDEX;
    stFace.m_enum   = OPERATE_DOMPUL;
    stFace.m_lErrno = TVM_DONE_SUCC;
    stFace.m_lDLen  = sizeof(TIndex);
    if(sizeof(TFace) != lSendBuffer(pstDom->m_skSock, (void *)&stFace, sizeof(TFace)))
    {
        pstDom->m_lStatus = RESOURCE_EXCP;
        goto EXCP_NOTIFY;
    }

    if(sizeof(TFace) != lRecvBuffer(pstDom->m_skSock, (char *)&stFace, sizeof(TFace)))
    {
        pstDom->m_lStatus = RESOURCE_EXCP;
        goto EXCP_NOTIFY;
    }

    if(DOM_NOT_REGST == stFace.m_lErrno)
    {
        pstDom->m_lStatus = RESOURCE_AUTH;    
        goto EXCP_NOTIFY;
    }
    else if(0 != stFace.m_lErrno && NO_DATA_FOUND != stFace.m_lErrno)
    {    
        pstDom->m_lStatus = RESOURCE_EXCP;
        goto EXCP_NOTIFY;
    }

    if(NULL == (pstIndex = (TIndex *)malloc(stFace.m_lRows)))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    if(stFace.m_lRows != lRecvBuffer(pstDom->m_skSock, (void *)pstIndex, stFace.m_lRows))
    {
        TFree(pstIndex);
        return RC_FAIL;
    }

    if(RC_SUCC != _lRemodeIndex(pstSavm, &stFace, pstDom, pstIndex))
    {
        TFree(pstIndex);
        pstDom->m_lStatus = RESOURCE_EXCP;
        goto EXCP_NOTIFY;
    }
    TFree(pstIndex);

    stFace.m_lFind  = lPort;
    stFace.m_enum   = OPERATE_DOMPSH;
    stFace.m_lErrno = TVM_DONE_SUCC;
    if(RC_SUCC != _lLocalIndex(pstSavm, &stFace, pstDom->m_skSock))
    {
        pstDom->m_lStatus = RESOURCE_EXCP;
        goto EXCP_NOTIFY;
    }

    pstDom->m_lStatus = RESOURCE_ABLE;
    return RC_SUCC;

EXCP_NOTIFY:
    vCloseSocket(pstDom);
    pstDom->m_lTryTimes = pstDom->m_lTryMax;
    pstDom->m_lLastTime = (long)time(NULL);
    lUpdateDomain(pstSavm, pstDom->m_szIp, pstDom->m_lPort, pstDom->m_lStatus);
    return RC_FAIL;
}

/*************************************************************************************************
    description：Notifies the remote domain that the local will be offline
    parameters:
        pstSavm                    --stvm handle
        pstFace                    --request head
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lLocalOffline(SATvm *pstSavm, TFace *pstFace)
{
    Rowgrp   *list = NULL;
    TDomain  *pstDom = NULL;

    for(list = pGetDomgrp(); list; list = list->pstNext)
    {
        if(NULL == (pstDom = (TDomain *)list->psvData))
            continue;

        if(RESOURCE_ABLE != pstDom->m_lStatus)
        {
            vCloseSocket(pstDom);
            continue;
        }

        while(DATA_TRUCK_LOCK == pstDom->m_lLock)
            usleep(10);

        pstDom->m_lLock = DATA_TRUCK_LOCK;
        pstFace->m_lRows = 0;
        pstFace->m_enum  = OPERATE_DOMROFF;
        if(sizeof(TFace) != lSendBuffer(pstDom->m_skSock, (void *)pstFace, sizeof(TFace)))
        {
            vCloseSocket(pstDom);
            pstDom->m_lLock = DATA_TRUCK_NULL;
            continue;
        }

        lRecvBuffer(pstDom->m_skSock, (char *)pstFace, sizeof(TFace));
        vCloseSocket(pstDom);
        pstDom->m_lLock = DATA_TRUCK_NULL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：Set the remote domain offline
    parameters:
        pstSavm                    --stvm handle
        pstCon                     --socket handle
        pstFace                    --request head
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lRemoteOffline(SATvm *pstSavm, SKCon *pstCon, TFace *pstFace)
{
    TDomain  *pstDom = NULL;
    long     lPort = pstFace->m_lFind;

    if(NULL == (pstDom = pGetDomain(pstCon->m_szCltIp, pstFace->m_lFind)))
    {
        Tlog("Unregistered client request: %s:%d", pstCon->m_szCltIp, pstFace->m_lFind);
        return RC_FAIL;
    }

    vCloseSocket(pstDom);
    pstDom->m_lStatus = RESOURCE_ROFF;
    pstDom->m_lTryTimes = pstDom->m_lTryMax;
    return lUpdateDomain(pstSavm, pstDom->m_szIp, lPort, RESOURCE_STOP);
}

/*************************************************************************************************
    description：reconnect the remote domain 
    parameters:
        pstSavm                    --stvm handle
        pstCon                     --socket handle
        pstFace                    --request head
        pv                         --remote domain
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lReconnectDomain(SATvm *pstSavm, SKCon *pstCon, TFace *pstFace, TDomain *pv)
{
    TDomain    *pstDom;

    if(NULL == (pstDom = pGetDomain(pv->m_szIp, pv->m_lPort)))
    {
        Tlog("Unregistered client request: %s:%d", pv->m_szIp, pv->m_lPort);
        return RC_FAIL;
    }

    pstDom->m_lTryTimes = 0;
    pstDom->m_lLastTime = (long)time(NULL);
    return lConnectDomain(pstSavm, pstDom, pstFace->m_lFind);
}

/*************************************************************************************************
    description：Receiving remote resources
    parameters:
        pstSavm                    --stvm handle
        pstCon                     --socket handle
        pstFace                    --request head
        pvData                     --remote resource
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lDomainPull(SATvm *pstSavm, SKCon *pstCon, TFace *pstFace, void *pvData)
{
    TDomain    *pstDom = NULL;

    if(NULL == (pstDom = pGetDomain(pstCon->m_szCltIp, pstFace->m_lFind)))
    {
        Tlog("Unregistered client request: %s:%d", pstCon->m_szCltIp, pstFace->m_lFind);
        return RC_FAIL;
    }

    pstDom->m_lTryTimes = 0;
    pstDom->m_lLastTime = (long)time(NULL);
    return _lRemodeIndex(pstSavm, pstFace, pstDom, (TIndex *)pvData);
}

/*************************************************************************************************
    description：Sending local resources
    parameters:
        pstSavm                    --stvm handle
        pstCon                     --socket handle
        pstFace                    --request head
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lDomainPush(SATvm *pstSavm, SKCon *pstCon, TFace *pstFace)
{
    TDomain    *pstDom = NULL;

    if(NULL == (pstDom = pGetDomain(pstCon->m_szCltIp, pstFace->m_lFind)))
    {
        Tlog("Unregistered client request: %s:%d", pstCon->m_szCltIp, pstFace->m_lFind);
        return RC_FAIL;
    }

    pstDom->m_lTryTimes = 0;
    pstDom->m_lStatus = RESOURCE_ABLE;
    pstDom->m_lLastTime = (long)time(NULL);
    return _lLocalIndex(pstSavm, pstFace, pstCon->m_skSock);
}

/*************************************************************************************************
    description：Sending local table data
    parameters:
        pstSavm                    --stvm handle
        pstCon                     --socket handle
        skSock                     --socket
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lPushTable(SATvm *pstSavm, TFace *pstFace, BSock skSock)
{
    long    lWrite, lRow = 0, lRet;
    void    *pvData = NULL, *pvBuffer = NULL;

    pstFace->m_lRows = 1;
    pstFace->m_lDLen = sizeof(TblDef);
    if(pstFace->m_lDLen != lSendBuffer(skSock, (void *)pGetTblDef(pstFace->m_table), 
        pstFace->m_lDLen))
        return RC_FAIL;

    if(pstFace->m_lFind <= 0)
    {
        pstFace->m_lRows = 0;
        lSendBuffer(skSock, (void *)pstFace, sizeof(TFace));
        return RC_SUCC;
    }

    lWrite = lGetRowSize(pstFace->m_table) * pstFace->m_lFind + sizeof(TFace);
    if(NULL == (pvBuffer = (char *)calloc(1, lWrite)))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }
    pvData = pvBuffer + sizeof(TFace);

    pstSavm->pstVoid = NULL;
    pstSavm->tblName = pstFace->m_table;
    pstSavm->lSize = lGetRowSize(pstFace->m_table);
    if(RC_SUCC != lTableDeclare(pstSavm))
    {
        TFree(pvBuffer);
        return RC_FAIL;
    }

    ((TFace *)pvBuffer)->m_table  = pstFace->m_table;
    ((TFace *)pvBuffer)->m_lRows  = pstFace->m_lFind;
    ((TFace *)pvBuffer)->m_lDLen  = lGetRowSize(pstFace->m_table);

    while(1)
    {
        lRet = lTableFetch(pstSavm, (void *)pvData + lRow * pstFace->m_lDLen);
        if(RC_FAIL == lRet)
        {
            TFree(pvBuffer);
            vTableClose(pstSavm);
            return RC_FAIL;
        }
        else if(RC_NOTFOUND == lRet)
            break;

        if(pstFace->m_lFind != ++ lRow)
            continue;

        lRow = 0;
        if(lWrite != lSendBuffer(skSock, (void *)pvBuffer, lWrite))
        {
            vTableClose(pstSavm);
            return RC_FAIL;
        }
    }
    vTableClose(pstSavm);

    if(0 == lRow)
    {
        TFree(pvBuffer);
        pstFace->m_lRows = 0;
        lSendBuffer(skSock, (void *)pstFace, sizeof(TFace));
        return RC_SUCC;
    }

    ((TFace *)pvBuffer)->m_lRows = lRow;
    lWrite = lGetRowSize(pstFace->m_table) * lRow + sizeof(TFace);
    if(lWrite != lSendBuffer(skSock, (void *)pvBuffer, lWrite))
    {
        TFree(pvBuffer);
        return RC_FAIL;
    }

    ((TFace *)pvBuffer)->m_lRows = 0;
    if(sizeof(TFace) != lSendBuffer(skSock, (void *)pvBuffer, sizeof(TFace)))
    {
        TFree(pvBuffer);
        return RC_FAIL;
    }

    TFree(pvBuffer);
    return RC_SUCC;
}

/*************************************************************************************************
    description：refresh remote domain 
    parameters:
        pstSavm                    --stvm handle
        pstFace                    --request head
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lRefreshDomain(SATvm *pstSavm, TFace *pstFace)
{
    Rowgrp  *list = NULL;
    TDomain *pstDom = NULL;

    for(list = pGetDomgrp(); list; list = list->pstNext)
    {
        if(NULL == (pstDom = (TDomain *)list->psvData))
            continue;

        while(DATA_TRUCK_LOCK == pstDom->m_lLock)
            usleep(10);

        pstDom->m_lLock = DATA_TRUCK_LOCK;
        lConnectDomain(pstSavm, pstDom, pstFace->m_lFind);
        pstDom->m_lLock = DATA_TRUCK_NULL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：Java API - event operation
    parameters:
        pstSavm                    --stvm handle
        pstFace                    --request head
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lProcaOperate(SATvm *pstSavm, SKCon *pstCon, TFace *pstFace, char *pvData)
{





    pstFace->m_lRows  = 0;
    pstFace->m_lErrno = RMT_NOT_SUPPT;
    return RC_FAIL;
}

/*************************************************************************************************
    description：C/C++ API - event operation
    parameters:
        pstSavm                    --stvm handle
        pstCon                     --socket handle
        pstFace                    --request head
        pvData                     --request data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lEventOperate(SATvm *pstSavm, SKCon *pstCon, TFace *pstFace, char *pvData)
{
    size_t  lData;
    void    *pvOut = NULL;
        
    switch(pstFace->m_enum)
    {
    case  OPERATE_SELECT:
        if(RC_SUCC != lSelect(pstSavm, (void *)pvData))
        {
            pstFace->m_lErrno = pstSavm->m_lErrno;
            lData = sizeof(TFace);
        }
        else
        {
            lData = pstFace->m_lDLen + sizeof(TFace);
            pstFace->m_lRows = pstSavm->m_lEffect;
            pstFace->m_lDLen = pstSavm->m_lEType;
        }

        lSendBuffer(pstCon->m_skSock, (void *)pstFace, lData);
        return RC_SUCC;
    case  OPERATE_QUERY:
        if(RC_SUCC != lQuery(pstSavm, (size_t *)&pstFace->m_lRows, (void *)&pvOut))
            pstFace->m_lErrno = pstSavm->m_lErrno;
        else
        {
            lData = pstFace->m_lDLen * pstFace->m_lRows;
            pstFace->m_lDLen = pstSavm->m_lEType;
        }

        if(sizeof(TFace) != lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace)))
        {
            TFree(pvOut);
            return RC_SUCC;
        }

        lSendBuffer(pstCon->m_skSock, pvOut, lData);
        TFree(pvOut);
        return RC_SUCC;
    case  OPERATE_REPLACE:
        pstSavm->m_bWork = pstCon->m_bWork;
        pstSavm->m_pstWork = pstCon->m_pstWork;
        if(RC_SUCC != lReplace(pstSavm, pvData))
            pstFace->m_lErrno = pstSavm->m_lErrno;
        else
        {
            pstFace->m_lRows = pstSavm->m_lEffect;
            pstFace->m_lDLen = pstSavm->m_lEType;
        }
        pstCon->m_pstWork = pstSavm->m_pstWork;
        lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        return RC_SUCC;
    case  OPERATE_UPDATE:
        pstSavm->m_bWork = pstCon->m_bWork;
        pstSavm->m_pstWork = pstCon->m_pstWork;
        if(RC_SUCC != lUpdate(pstSavm, pvData))
            pstFace->m_lErrno = pstSavm->m_lErrno;
        else
        {
            pstFace->m_lRows = pstSavm->m_lEffect;
            pstFace->m_lDLen = pstSavm->m_lEType;
        }
        pstCon->m_pstWork = pstSavm->m_pstWork;
        lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        return RC_SUCC;
    case  OPERATE_DELETE:
        pstSavm->m_bWork = pstCon->m_bWork;
        pstSavm->m_pstWork = pstCon->m_pstWork;
        if(RC_SUCC != lDelete(pstSavm))
            pstFace->m_lErrno = pstSavm->m_lErrno;
        else
        {
            pstFace->m_lDLen = pstSavm->m_lEType;
            pstFace->m_lRows = pstSavm->m_lEffect;
        }
        pstCon->m_pstWork = pstSavm->m_pstWork;
        lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        return RC_SUCC;
    case  OPERATS_REPLACE:
        pstSavm->m_bWork = pstCon->m_bWork;
        pstSavm->m_pstWork = pstCon->m_pstWork;
        if(RC_SUCC == lReplace(pstSavm, pvData))
            pstCon->m_pstWork = pstSavm->m_pstWork;
        return RC_SUCC;
    case  OPERAYS_UPDATE:
        pstSavm->m_bWork = pstCon->m_bWork;
        pstSavm->m_pstWork = pstCon->m_pstWork;
        if(RC_SUCC == lUpdate(pstSavm, pvData))
            pstCon->m_pstWork = pstSavm->m_pstWork;
        return RC_SUCC;
    case  OPERAYS_DELETE:
        pstSavm->m_bWork = pstCon->m_bWork;
        pstSavm->m_pstWork = pstCon->m_pstWork;
        if(RC_SUCC == lDelete(pstSavm))
            pstCon->m_pstWork = pstSavm->m_pstWork;
        return RC_SUCC;
    case  OPERAYS_INSERT:
        pstSavm->m_bWork = pstCon->m_bWork;
        pstSavm->m_pstWork = pstCon->m_pstWork;
        if(RC_SUCC == lInsert(pstSavm))
            pstCon->m_pstWork = pstSavm->m_pstWork;
        return RC_SUCC;
    case  OPERATE_INSERT:
        pstSavm->m_bWork = pstCon->m_bWork;
        pstSavm->m_pstWork = pstCon->m_pstWork;
        if(RC_SUCC != lInsert(pstSavm))
            pstFace->m_lErrno = pstSavm->m_lErrno;
        else 
            pstFace->m_lRows  = pstSavm->m_lEffect;
        pstCon->m_pstWork = pstSavm->m_pstWork;
        lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        return RC_SUCC;
    case  OPERATE_TRCATE:
        if(RC_SUCC != lTruncate(pstSavm, pstFace->m_table))
            pstFace->m_lErrno = pstSavm->m_lErrno;
        else
            pstFace->m_lRows  = pstSavm->m_lEffect;
        lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        return RC_SUCC;
    case  OPERATE_COUNT:
        if(RC_SUCC != lCount(pstSavm, (size_t *)&pstFace->m_lRows))
            pstFace->m_lErrno = pstSavm->m_lErrno;
        else
            pstFace->m_lDLen = pstSavm->m_lEType;

        lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        return RC_SUCC;
    case  OPERATE_EXTREM:
        memset(pvData, 0, pstSavm->lSize);
        if(RC_SUCC != lExtreme(pstSavm, (void *)pvData))
        {
            pstFace->m_lErrno = pstSavm->m_lErrno;
            lData = sizeof(TFace);
        }
        else
        {
            lData = pstFace->m_lDLen + sizeof(TFace);
            pstFace->m_lRows = pstSavm->m_lEffect;
            pstFace->m_lDLen = pstSavm->m_lEType;
        }
        lSendBuffer(pstCon->m_skSock, (void *)pstFace, lData);
        return RC_SUCC;
    case  OPERATE_TBDROP:
        if(RC_SUCC != lDropTable(pstSavm, pstFace->m_table))
            pstFace->m_lErrno = pstSavm->m_lErrno;
        else
            pstFace->m_lRows  = pstSavm->m_lEffect;
        lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        return RC_SUCC;
    case  OPERATE_RENAME:
        if(RC_SUCC != lRenameTable(pstSavm, pstFace->m_table, (TABLE)pstFace->m_lDLen))
            pstFace->m_lErrno = pstSavm->m_lErrno;

        lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        return RC_SUCC;
    case  OPERATE_CLICK:
        if(RC_SUCC != lClick(pstSavm, (ulong *)pvData))
        {
            pstFace->m_lErrno = pstSavm->m_lErrno;
            lData = sizeof(TFace);
        }
        else
        {
            lData = pstFace->m_lDLen + sizeof(ulong);
            pstFace->m_lDLen = pstSavm->m_lEType;
        }

        lSendBuffer(pstCon->m_skSock, (void *)pstFace, lData);
        return RC_SUCC;
    case  OPERATE_SELSEQ:
        if(RC_SUCC != lSelectSeque(pstSavm, (char *)pvData, (ulong *)pvData))
        {
            pstFace->m_lErrno = pstSavm->m_lErrno;
            lData = sizeof(TFace);
        }
        else
            lData = pstFace->m_lDLen + sizeof(ulong);

        lSendBuffer(pstCon->m_skSock, (void *)pstFace, lData);
        return RC_SUCC;
    case  OPERATE_SETSEQ:
        if(RC_SUCC != lSetSequence(pstSavm, (char *)pvData, *((ulong *)(pvData + MAX_INDEX_LEN))))
            pstFace->m_lErrno = pstSavm->m_lErrno;

        lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        return RC_SUCC;
    case  OPERATE_RBDIDX:
        if(RC_SUCC != lRebuildIndex(pstSavm, pstFace->m_table))
            pstFace->m_lErrno = pstSavm->m_lErrno;
    
        lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        return RC_SUCC;
    case  OPERATE_RETLOK:
        if(RC_SUCC != lResetLock(pstSavm, pstFace->m_table))
            pstFace->m_lErrno = pstSavm->m_lErrno;
        lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        return RC_SUCC;
    case  OPERATE_GROUP:
        if(RC_SUCC != lGroup(pstSavm, (size_t *)&pstFace->m_lRows, (void *)&pvOut))
            pstFace->m_lErrno = pstSavm->m_lErrno;
        else
        {
            lData = pstFace->m_lDLen * pstFace->m_lRows;
            pstFace->m_lDLen = pstSavm->m_lEType;
        }

        if(sizeof(TFace) != lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace)))
        {
            TFree(pvOut);
            return RC_SUCC;
        }

        lSendBuffer(pstCon->m_skSock, pvOut, lData);
        TFree(pvOut);
        return RC_SUCC;

/*  work  */
    case  OPERATE_BEGWORK:
        if(pstCon->m_bWork && pstCon->m_uWorker != pstFace->m_lDLen)
            lRollbackWork(pstSavm);
        if(!pstCon->m_bWork)
        {
            pstCon->m_bWork = true;
            pstCon->m_uWorker = pstFace->m_lDLen;
        }
        lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        return RC_SUCC;
    case  OPERATE_ROLWORK:
        if(pstCon->m_uWorker != pstFace->m_lDLen)
        {
            pstFace->m_lErrno = WORK_NOT_REGT;
            lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
            return RC_SUCC;
        }

        pstSavm->m_bWork = pstCon->m_bWork;
        pstSavm->m_pstWork = pstCon->m_pstWork;
        lRollbackWork(pstSavm);
        pstFace->m_lRows  = pstSavm->m_lEffect;
        pstCon->m_pstWork = NULL;
        pstFace->m_lErrno = pstSavm->m_lErrno;
        lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        return RC_SUCC;
    case  OPERATE_CMTWORK:
        if(pstCon->m_uWorker != pstFace->m_lDLen)
        {
            pstFace->m_lErrno = WORK_NOT_REGT;
               lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
            return RC_SUCC;
        }

        pstSavm->m_bWork = pstCon->m_bWork;
        pstSavm->m_pstWork = pstCon->m_pstWork;
        lCommitWork(pstSavm);
        pstCon->m_pstWork = NULL;
        lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        return RC_SUCC;
    case  OPERATE_ENDWORK:
        if(pstCon->m_uWorker != pstFace->m_lDLen)
        {
            pstFace->m_lErrno = WORK_NOT_REGT;
               lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
            return RC_SUCC;
        }

        pstSavm->m_bWork = pstCon->m_bWork;
        pstSavm->m_pstWork = pstCon->m_pstWork;
        lCommitWork(pstSavm);
        pstCon->m_bWork = false;
        pstCon->m_pstWork = NULL;
        lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        return RC_SUCC;

    case  OPERATE_DOMPUL:
        if(RC_SUCC != lDomainPush(pstSavm, pstCon, pstFace))
        {
            pstFace->m_lErrno = pstSavm->m_lErrno;
            lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        }
            
        return RC_SUCC;
    case  OPERATE_DOMPSH:
        lDomainPull(pstSavm, pstCon, pstFace, pvData);
        return RC_SUCC;

    case  OPERATE_DMRECN:
        lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        lReconnectDomain(pstSavm, pstCon, pstFace, (TDomain *)pvData);
        return RC_SUCC;
    case  OPERATE_REFRESH:
        lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        lRefreshDomain(pstSavm, pstFace);
        return RC_SUCC;
    case  OPERATE_DOMLOFF:
        lLocalOffline(pstSavm, pstFace);
        g_eRun = 0;
        pstSavm->m_bWork = pstCon->m_bWork;
        pstSavm->m_pstWork = pstCon->m_pstWork;
        lCommitWork(pstSavm);
        pstCon->m_bWork = false;
        pstCon->m_pstWork = NULL;
        lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        return RC_SUCC;
    case  OPERATE_DOMROFF:
        lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        lRemoteOffline(pstSavm, pstCon, pstFace);
        return RC_SUCC;
    case  OPERATE_PULTBL:
        lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        lPushTable(pstSavm, pstFace, pstCon->m_skSock);
        return RC_SUCC;
    case OPERATE_EXEEXIT:
        g_eRun = 0;
        pstSavm->m_bWork = pstCon->m_bWork;
        pstSavm->m_pstWork = pstCon->m_pstWork;
        lCommitWork(pstSavm);
        pstCon->m_bWork = false;
        pstCon->m_pstWork = NULL;
        lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        return RC_SUCC;
    default:
        pstFace->m_lRows  = 0;
        pstFace->m_lErrno = RMT_NOT_SUPPT;
        Tlog("Unknown request:%d", pstFace->m_enum);
        return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：accepte operation
    parameters:
        pstSavm                    --stvm handle
        epdf                       --socket
        pc                         --socket handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lEpollAccept(SATvm *pstSavm, BSock epfd, SKCon *pc)
{
    socklen_t   kLen;
    epollevt    event;
    BSock       skAccept;
    SKCon       *pstCon = NULL;
    struct sockaddr_in cAddr;

    while (1)
    {
        kLen = sizeof(struct sockaddr_in);
        /*  The phenomenon of "terrors" produced by multiple processes, There is only one 
          process returned to succ and other processes return errno=EAGAIN  */
        if(0 > (skAccept = accept(pc->m_skSock, (struct sockaddr *)&cAddr, &kLen)))
            break;

        if (RC_SUCC != lSetUnBlock(skAccept))
        {
            close(skAccept);
            continue;
        }

        if(NULL == (pstCon = (SKCon *)calloc(sizeof(SKCon), 1)))
        {
            close(skAccept);
            fprintf(stderr, "Create memory, err:(%d)(%s)", errno, strerror(errno));
            return RC_FAIL;
        }

        pstCon->m_lBuffer = READ_MAX_LEN;
        pstCon->pstFace = (void *)calloc(1, pstCon->m_lBuffer + sizeof(TFace));
        pstCon->pstVoid = (void *)calloc(1, pstCon->m_lBuffer + sizeof(TFace));
        if(NULL == pstCon->pstVoid || NULL == pstCon->pstFace)
        {
            close(skAccept);
            fprintf(stderr, "Create memory, err:(%d)(%s)", errno, strerror(errno));
            return RC_FAIL;
        }

        pstCon->m_lRead    = 0;
        pstCon->m_bHead    = false;
        pstCon->m_skSock   = skAccept;
        pstCon->m_lCltPort = ntohs(cAddr.sin_port);
        pstCon->pvData = pstCon->pstFace + sizeof(TFace);
        strncpy(pstCon->m_szCltIp, inet_ntoa(cAddr.sin_addr), sizeof(pstCon->m_szCltIp));

        memset(&event, 0, sizeof(event));
        event.data.ptr = pstCon;
//        event.events   = EPOLLIN | EPOLLET;
        event.events   = EPOLLIN;
        if(0 != epoll_ctl(epfd, EPOLL_CTL_ADD, skAccept, &event))
        {
            close(skAccept);
            pstSavm->m_lErrno = EPOLL_ADD_ERR;
            fprintf(stderr, "add socket (%d) error, err:(%d)(%s)", skAccept,
                errno, strerror(errno));
            return RC_FAIL;
        }
    }   
    
    return RC_SUCC;
}

/*************************************************************************************************
    description：reset remote domain
    parameters:
        pstSavm                    --stvm handle
        pszIp                      --ip
        lPort                      --lport
        skSock                     --socket
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
void    vResetRemote(SATvm *pstSavm, char *pszIp, long lPort, BSock skSock)
{
    TDomain *pv = NULL;
    Rowgrp  *list = NULL;

    for(list = pGetDomgrp(); list; list = list->pstNext)
    {
        pv = (TDomain *)list->psvData;
        if(strcmp(pv->m_szIp, pszIp) || pv->m_lPort != lPort)
            continue;

        pv->m_lTryTimes = 0;
        pv->m_lStatus   = RESOURCE_ABLE;
        pv->m_lLastTime = (long)time(NULL);
    }

    return ;
}

/*************************************************************************************************
    description：Get the event request
    parameters:
        pstSovm                    --stvm handle
        pstCon                     --socket handle
        pstFace                    --request head
        pstVoid                    --request condition
        pvData                     --decorate
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lPollRequest(SATvm *pstSovm, SKCon *pstCon, TFace *pstFace, void *pstVoid, char *pvData)
{
    long    lRet;
    RunTime *pstRun = NULL;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

    if(OPERATE_DMKEEP == pstFace->m_enum)
    {
        lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        vResetRemote(pstSovm, pstCon->m_szCltIp, pstFace->m_lFind, pstCon->m_skSock);
        pstCon->m_lRead = 0;
        pstCon->m_bHead = false;
        return RC_SUCC;
    }

    if(0 > (lRet = lRecvBuffer(pstCon->m_skSock, pvData + pstCon->m_lRead, 
        pstFace->m_lRows - pstCon->m_lRead)))
        return RC_FAIL;
    
    pstCon->m_lRead += lRet;
    if(pstFace->m_lRows != pstCon->m_lRead)    // more data wait to read
        return RC_SUCC;
    
    pstCon->m_lRead = 0;
    pstCon->m_bHead = false;
    pstRun = (RunTime *)pGetRunTime(pstSovm, pstFace->m_table);
    pstRun->m_bAttch = pstSovm->stRunTime[pstFace->m_table].m_bAttch;
    pstRun->m_pvAddr = pstSovm->stRunTime[pstFace->m_table].m_pvAddr;
    if(!pstRun->m_bAttch || !pstRun->m_pvAddr)
    {
//Tlog("initial table:%d, %d, %d", pstFace->m_table, pstFace->m_enum, pstRun->m_bAttch);
        if(RC_SUCC != lInitSATvm(pstSovm, pstFace->m_table))
        {
            pstFace->m_lRows  = 0;
            pstFace->m_lErrno = pstSovm->m_lErrno;
            return lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        }
    }

    if(PROTOCAL_JAVA & pstFace->m_enum)
    {
        pstFace->m_enum = pstFace->m_enum ^ PROTOCAL_JAVA;
        pstFace->m_lDLen = pstRun->m_lRowSize;
        if(NULL == (pvData = pProtocaJava(pstSovm, pstVoid, pstFace, pvData, pstFace->m_lRows)))
        {
            pstFace->m_lErrno = RESOU_DISABLE;
            pvData = (void *)pstFace + sizeof(TFace);
            return lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        }

        pstSovm->pstVoid  = pstVoid;
        pstSovm->lFind    = pstFace->m_lFind;
        pstSovm->tblName  = pstFace->m_table;
        pstSovm->lSize    = pstFace->m_lDLen;
        lRet = lProcaOperate(pstSovm, pstCon, pstFace, pvData);
        pstSavm->stRunTime[pstFace->m_table].m_bAttch = pstRun->m_bAttch;
        pstSavm->stRunTime[pstFace->m_table].m_pvAddr = pstRun->m_pvAddr;
        if(RC_SUCC == lRet)    return RC_SUCC;

        return lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
    }
    else
    {
        if(NULL == (pvData = pParsePacket(pstSovm, pstVoid, pstFace, pvData, pstFace->m_lRows)))
        {
               pstFace->m_lErrno = RESOU_DISABLE;
               pvData = (void *)pstFace + sizeof(TFace);
               return lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
        }

        pstSovm->pstVoid  = pstVoid;
        pstSovm->lFind    = pstFace->m_lFind;
        pstSovm->tblName  = pstFace->m_table;
        pstSovm->lSize    = pstFace->m_lDLen;
        lRet = lEventOperate(pstSovm, pstCon, pstFace, pvData);
        pstSavm->stRunTime[pstFace->m_table].m_bAttch = pstRun->m_bAttch;
        pstSavm->stRunTime[pstFace->m_table].m_pvAddr = pstRun->m_pvAddr;
        if(RC_SUCC == lRet)    return RC_SUCC;

        return lSendBuffer(pstCon->m_skSock, (void *)pstFace, sizeof(TFace));
    }
}

/*************************************************************************************************
    description：server listen
    parameters:
        pvParam 
    return:
 *************************************************************************************************/
void*    vEpollListen(void *pvParam)
{
    long        lRet, i, nWait;
    SKCon       *pstCon = NULL;
    TFace       *pstFace = NULL;
    epollevt    events[MAX_EVENTS];
    BSock       epfd = *((long *)pvParam);
    SATvm       *pstSavm = (SATvm *)calloc(1, sizeof(SATvm));

    pthread_detach(pthread_self());

    vHoldConnect(pstSavm);
    if(RC_SUCC != lTvmBuffer(pstSavm))
        return NULL;

    while(g_eRun)
    {
        nWait = epoll_wait(epfd, events, MAX_EVENTS, 500);
        for(i = 0; i < nWait; i++)
        {   
            pstCon = (SKCon *)events[i].data.ptr;
            if(pstCon->m_isListen)
                lEpollAccept(pstSavm, epfd, pstCon);
            else if(events[i].events & EPOLLIN)
            {
               if(false == pstCon->m_bHead)
               {
                    if(0 > (lRet = lRecvBuffer(pstCon->m_skSock, pstCon->pstFace + pstCon->m_lRead,
                        sizeof(TFace) - pstCon->m_lRead)))
                    {
                        if(pstCon->m_bWork)
                        {
                            pstSavm->m_bWork = pstCon->m_bWork;
                            pstSavm->m_pstWork = pstCon->m_pstWork;
                            lRollbackWork(pstSavm);
                            pstCon->m_bWork = false;
                        }
                        pstCon->m_pstWork = NULL;
                        epoll_ctl(epfd, EPOLL_CTL_DEL, pstCon->m_skSock, &events[i]);
                        TFree(pstCon->pstFace);
                        TFree(pstCon->pstVoid);
                        close(pstCon->m_skSock);
                        continue;
                    }
                    
                    pstCon->m_lRead += lRet;
                    if(sizeof(TFace) != pstCon->m_lRead)    // more data wait to read
                        continue;
                    
                    pstFace = (TFace *)pstCon->pstFace;
                    if(TVM_MAX_TABLE <= pstFace->m_table)
                    {
                        pstCon->m_lRead = 0;
                        pstFace->m_lErrno = RESOU_DISABLE;    
                        goto LISTEN_ERROR;
                    }
                    
                    checkrequest(pstSavm, pstCon, pstFace);
                    pstCon->m_lRead = 0;
                    pstCon->m_bHead = true;
                }

                if(RC_FAIL == lPollRequest(pstSavm, pstCon, pstFace, pstCon->pstVoid, 
                    pstCon->pvData))
                {
                    if(pstCon->m_bWork)
                    {
                        pstSavm->m_bWork = pstCon->m_bWork;
                        pstSavm->m_pstWork = pstCon->m_pstWork;
                        lRollbackWork(pstSavm);
                        pstCon->m_bWork = false;
                    }
                    pstCon->m_pstWork = NULL;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, pstCon->m_skSock, &events[i]);
                    TFree(pstCon->pstFace);
                    TFree(pstCon->pstVoid);
                    close(pstCon->m_skSock);
                }
            }
        }
        continue;

LISTEN_ERROR:
    lSendBuffer(pstCon->m_skSock, pstCon->pstFace, sizeof(TFace));
    continue;
    }

    close(epfd);
    pstSavm->pstVoid = NULL;
    vTvmDisconnect(pstSavm);
    return NULL;
}

/*************************************************************************************************
    description：Get the event request
    parameters:
        pstSovm                    --stvm handle
        epfg                       --socket 
        plMultListen               --listen handle
    return:
 *************************************************************************************************/
void    vMultListen(SATvm *pstSavm, BSock epfd, FUNCEXEC plMultListen)
{
    long        i, nWait = 0;
    SKCon       *pstCon = NULL;
    epollevt    events[MAX_EVENTS];

    pthread_detach(pthread_self());
    vHoldConnect(pstSavm);
    while(1)
    {   
        nWait = epoll_wait(epfd, events, MAX_EVENTS, 5000);
        for(i = 0; i < nWait; i++)
        {   
            pstCon = (SKCon *)events[i].data.ptr;
            if(pstCon->m_isListen)
                lEpollAccept(pstSavm, epfd, pstCon);
            else if(events[i].events & EPOLLIN)
            {
                if(RC_CLOSE == plMultListen(pstSavm, pstCon))
                {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, pstCon->m_skSock, &events[i]);
                    close(pstCon->m_skSock);
                }
            }
        }
    }

    close(epfd);
    vHoldRelease(pstSavm);
    return ;
}

/*************************************************************************************************
    description：Fork multi-process listening
    parameters:
        pstSavm                    --stvm handle
        lPort                      --socket handle
        lProcess                   --request head
        plMultInitail              --init handle
        arg                        --request condition
        plMultListen               --listen handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lMultListen(SATvm *pstSavm, long lPort, long lProcess, FUNCEXEC plMultInitail, void *arg, 
            FUNCEXEC plMultListen)
{
    int         i;
    pid_t       lPid;
    epollevt    event;
    BSock       epfd = -1;
    SKCon       *pstCon = NULL;

    if(NULL == (pstCon = (SKCon *)calloc(sizeof(SKCon), 1)))
    {
        fprintf(stderr, "create memory, err:(%d)(%s)", errno, strerror(errno));
        return RC_FAIL;
    }

    pstCon->m_isListen = 1;
    if(0 > (pstCon->m_skSock = skServerInitail(pstSavm, lPort)))
        return RC_FAIL;

    for(i = 0; i < lProcess; i ++)
    {
        if(0 > (lPid = fork()))
        {
            close(pstCon->m_skSock);
            return RC_FAIL;
        }
        else if(lPid > 0)
            continue;
    
        epfd = epoll_create(MAX_EVENTS);

        memset(&event, 0, sizeof(event));
        event.data.ptr = pstCon;
        event.events   = EPOLLIN | EPOLLET;
        if(0 != epoll_ctl(epfd, EPOLL_CTL_ADD, pstCon->m_skSock, &event))
        {
            pstSavm->m_lErrno = EPOLL_ADD_ERR;
            return RC_FAIL;
        }

        if(plMultInitail && RC_SUCC != plMultInitail(pstSavm, (void *)arg))
        {
            pstSavm->m_lErrno = INI_ERR_CHLDP;
            return RC_FAIL;
        }
        
        vMultListen(pstSavm, epfd, plMultListen);
        exit(0);
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：break heart 
    parameters:
        pstSovm                    --stvm handle
        pstDom                     --remote domain
        lPort                      --local port
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lBreakHeart(SATvm *pstSavm, TDomain *pstDom, long lPort)
{
    TFace    stFace;

    if(pstDom->m_skSock < 0)
    {    
        pstDom->m_skSock = skConnectServer(pstSavm, pstDom->m_szIp, pstDom->m_lPort,
            false, pstDom->m_lTimeOut);
        if(RC_FAIL == pstDom->m_skSock)
            return RC_FAIL;
    }

    stFace.m_lFind = lPort;
    stFace.m_enum  = OPERATE_DMKEEP;
    if(sizeof(TFace) != lSendBuffer(pstDom->m_skSock, (void *)&stFace, sizeof(TFace)))
    {
        pstDom->m_lTryTimes ++;
        return RC_FAIL;
    }

    if(sizeof(TFace) != lRecvBuffer(pstDom->m_skSock, (void *)&stFace, sizeof(TFace)))
    {
        pstDom->m_lTryTimes ++;
        return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：Domain activity detection
    parameters:
        pstSovm                    --stvm handle
        lPort                      --socket handle
    return:
 *************************************************************************************************/
void    vDomainEvent(SATvm *pstSavm, long lPort)
{
    Rowgrp    *list = NULL;
    TDomain   *pstDom = NULL;

    while(g_eRun)
    {
        for(sleep(1), list = pGetDomgrp(); list; list = list->pstNext)
        {
            if(NULL == (pstDom = (TDomain *)list->psvData))
                continue;

            if(RESOURCE_ROFF == pstDom->m_lStatus)
                continue;

            if(pstDom->m_lKeepLive > ((long)time(NULL) - pstDom->m_lLastTime))
                continue;

            if(pstDom->m_lTryMax > 0 && pstDom->m_lTryMax <= pstDom->m_lTryTimes)
                continue;

            if(pstDom->m_lStatus != RESOURCE_ABLE)
            {
                lConnectDomain(pstSavm, pstDom, lPort);
                continue;
            }

            if(0 != pthread_mutex_trylock(&list->lock))
                continue;

            pstDom->m_lLastTime = (long)time(NULL);
            pstDom->m_lLock = DATA_TRUCK_LOCK;
            if(RC_SUCC == lBreakHeart(pstSavm, pstDom, lPort))
            {
                pthread_mutex_unlock(&list->lock);
                pstDom->m_lTryTimes = 0;
                pstDom->m_lLock = DATA_TRUCK_NULL;
                pstDom->m_lStatus = RESOURCE_ABLE;
                continue;
            }

            pthread_mutex_unlock(&list->lock);

            vCloseSocket(pstDom);
            pstDom->m_lLock = DATA_TRUCK_NULL;
            if(pstDom->m_lTryMax > 0 && pstDom->m_lTryMax <= pstDom->m_lTryTimes)
            {
                pstDom->m_lStatus = RESOURCE_STOP;
                lUpdateDomain(pstSavm, pstDom->m_szIp, pstDom->m_lPort, RESOURCE_STOP);
            }
        }
    }

    return ;
}

/*************************************************************************************************
    description：close domain
    parameters:
    return:
 *************************************************************************************************/
void    vCloseDomain()
{
    Rowgrp    *list;

    for(list = g_pstDomgrp; list; list = list->pstNext)
    {
        pthread_mutex_destroy(&list->lock);
        close(((TDomain *)list->psvData)->m_skSock);
        ((TDomain *)list->psvData)->m_skSock = -1;
    }

    TFgrp(g_pstDomgrp);
    TFgrp(g_pstTblgrp);
}

/*************************************************************************************************
    description：Cache domain
    parameters:
        pstSavm                    --stvm handle
        eMode                      --cache type
        lPort                      --local port
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lCacheDomain(SATvm *pstSavm, Benum eMode, long lPort)
{
    size_t   i, lOut = 0;
    TDomain  *pstDom = NULL, stDomain;
    Rowgrp   *list = NULL, *node = NULL;

    if(RC_SUCC != lInitSATvm(pstSavm, SYS_TVM_DOMAIN))
    {
        fprintf(stderr, "init domain error, %s\n", sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    conditnull(pstSavm, TDomain, SYS_TVM_DOMAIN);
    decorate(pstSavm, TDomain, m_szIp, GROUP_BY | ORDER_ASC);
    decorate(pstSavm, TDomain, m_lPort, GROUP_BY | ORDER_ASC);
    decorate(pstSavm, TDomain, m_lGroup, GROUP_BY);
    decorate(pstSavm, TDomain, m_lTryMax, GROUP_BY);
    decorate(pstSavm, TDomain, m_lTimeOut, GROUP_BY);
    decorate(pstSavm, TDomain, m_lKeepLive, GROUP_BY);
    if(RC_SUCC != lGroup(pstSavm, &lOut, (void *)&pstDom))
    {
        if(NO_DATA_FOUND != pstSavm->m_lErrno)
        {    
            fprintf(stderr, "get domain error, %s", sGetTError(pstSavm->m_lErrno));
            pthread_exit(NULL);
            return RC_FAIL;
        }
    }

    for(i = 0; i < lOut; i ++)
    {
        pstDom[i].m_skSock = -1;
        pstDom[i].m_lLastTime = time(NULL);
        if(0 == eMode)  lConnectDomain(pstSavm, &pstDom[i], lPort);
        
        if(NULL == (g_pstDomgrp = pInsertRowgrp(pstSavm, g_pstDomgrp, NULL, NULL,
             (void *)&pstDom[i], sizeof(TDomain), 0)))
        {
            TFree(pstDom);
            fprintf(stderr, "add domain list error, %s", strerror(errno));
            pthread_exit(NULL);
            return RC_FAIL;
        }
    }
    TFree(pstDom);

    conditnull(pstSavm, TDomain, SYS_TVM_DOMAIN);
    decorate(pstSavm, TDomain, m_table, GROUP_BY | ORDER_ASC);
    if(RC_SUCC != lGroup(pstSavm, &lOut, (void *)&pstDom))
    {
        if(NO_DATA_FOUND == pstSavm->m_lErrno)
            return RC_SUCC;
        return RC_FAIL;
    }

    for(i = 0; i < lOut; i ++)
    {
        if(NULL == (g_pstTblgrp = pInsertRowgrp(pstSavm, pGetTblgrp(), NULL, NULL, 
             (void *)&pstDom[i].m_table, sizeof(TABLE), 0)))
        {
            TFree(pstDom);
            return RC_FAIL;
        }
    }

    TFree(pstDom);

    for(list = pGetTblgrp(); list; list = list->pstNext)
    {
        conditinit(pstSavm, stDomain, SYS_TVM_DOMAIN);
        conditnum(pstSavm, stDomain, m_table, *((TABLE *)list->psvData));
        if(RC_SUCC != lQuery(pstSavm, &lOut, (void *)&pstDom))
            return RC_FAIL;

        for(i = 0; i < lOut; i ++)
        {
            if(NULL == (list->pstSSet = pInsertRowgrp(pstSavm, list->pstSSet, list, NULL,
                (void *)&pstDom[i], sizeof(TDomain), 0)))
            {
                TFree(pstDom);
                return RC_FAIL;
            }
        }

        TFree(pstDom);
    }

    for(list = pGetTblgrp(); list; list = list->pstNext)
    {
        for(node = list->pstSSet; node; node = node->pstNext)
        {
            if(NULL == (pstDom = (TDomain *)node->psvData))
                continue;

            node->pstFset = pGetDomnode(pstDom->m_szIp, pstDom->m_lPort);
            pthread_mutex_init(&node->pstFset->lock, NULL);
        }
    }

    vSetBootType(TVM_BOOT_LOCAL);
    return RC_SUCC;
}

/*************************************************************************************************
    description：Open domain as local mode
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lOpenDomain(SATvm *pstSavm)
{
    return lCacheDomain(pstSavm, 1, 0);
}

/*************************************************************************************************
    description：load remote resourc3
    parameters:
        pstSavm                    --stvm handle
        eMode                      --cache type
        lPort                      --local port
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
void    vRemoteResouce(SATvm *pstSavm, Benum eMode, long lPort)
{
    TDomain  *pstDom = NULL;
    Rowgrp   *list, *node = NULL;
    long     lType = lGetBootType();

    if(RC_SUCC != lCacheDomain(pstSavm, eMode, lPort))
        return ;

    Tlog("=-=-=-=-=-=-=-=-=-=-=-=- Domain node announcement -=-=-=-=-=-=-=-=-=-=-=-=-=");
    for(list = pGetDomgrp(); list; list = list->pstNext)
    {
        pstDom = (TDomain *)list->psvData;
        Tlog("NODE:%X, %s:%d, Group:%d, Try:%d, Time:%d, Keep:%d", list, pstDom->m_szIp, 
            pstDom->m_lPort, pstDom->m_lGroup, pstDom->m_lTryMax, pstDom->m_lTimeOut, 
            pstDom->m_lKeepLive);
    }

    Tlog("=-=-=-=-=-=-=-=-=-=- Remote table resource announcement -=-=-=-=-=-=-=-=-=-=-=");
    for(list = pGetTblgrp(); list; list = list->pstNext)
    {
        Tlog("NODE:%X, TABLE:%d", list, *((TABLE *)list->psvData));
        for(node = list->pstSSet; node; node = node->pstNext)
        {
            pstDom = (TDomain *)node->psvData;
            Tlog("\t>>TABLE:%s, PART:%s, OWNER:%s, pstFset:%X", pstDom->m_szTable, 
                pstDom->m_szPart, pstDom->m_szOwner, node->pstFset);
        }
    }

    vSetBootType(lType);
    vDomainEvent(pstSavm, lPort);
    return ;
}

/*************************************************************************************************
    description：boot local process
    parameters:
        pstSavm                    --stvm handle
        pstBoot                    --boot parameter
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lBootLocal(SATvm *pstSavm, TBoot *pstBoot, Benum eMode)
{
    int          i;
    pid_t        lPid;
    epollevt     event;
    BSock        epfd = -1;
    pthread_t    *tPid = NULL;
    SKCon        *pstCon = NULL;

    if(!pstBoot || pstBoot->m_lBootExec < 1)    //    线程数量
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    if(TVM_BOOT_SIMPLE == pstBoot->m_lBootType)
        return RC_SUCC;

/*
    pthread_mutex_init(&mutex_thread_read, NULL);
    pthread_cond_init(&cond_thread_read, NULL);
*/
    if(NULL == (pstCon = (SKCon *)calloc(sizeof(SKCon), 1)))
    {
        fprintf(stderr, "create memory, err:(%d)(%s)", errno, strerror(errno));
        return RC_FAIL;
    }

    pstCon->m_isListen = 1;
    if(0 > (pstCon->m_skSock = skServerInitail(pstSavm, pstBoot->m_lBootPort)))
        return RC_FAIL;

    vSetTitile(TVM_LOCAL_SERV);
    fprintf(stdout, "Boot process(%s), total %ld process\n", TVM_LOCAL_SERV, pstBoot->m_lBootExec);
    if(0 > (lPid = fork()))
    {
        close(pstCon->m_skSock);
        return RC_FAIL;
    }
    else if(lPid > 0)
    {
        usleep(500);
        return RC_SUCC;
    }

    epfd = epoll_create(MAX_EVENTS);

    memset(&event, 0, sizeof(event));
    event.data.ptr = pstCon;
//    event.events   = EPOLLIN | EPOLLET;
    event.events   = EPOLLIN;

    if(0 != epoll_ctl(epfd, EPOLL_CTL_ADD, pstCon->m_skSock, &event))
    {
        fprintf(stderr, "add socket (%d) error, err:(%d)(%s)", pstCon->m_skSock,
            errno, strerror(errno));
        return -1;
    }
    
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTRAP, SIG_IGN);
    tPid = malloc(sizeof(pthread_t) * pstBoot->m_lBootExec);
    vHoldConnect(pstSavm);
    for(i = 0; i < pstBoot->m_lBootExec; i ++)
    {
        if(0 != pthread_create(&tPid[i], NULL, vEpollListen, (void*)&epfd))
        {
            fprintf(stderr, "create thread error, %s\n", strerror(errno));
            exit(-1);
        }
    }

    fprintf(stdout, "   process %s id=%d ... success\n", TVM_LOCAL_SERV, getpid());
    fflush(stdout);

    vRemoteResouce(pstSavm, eMode, pstBoot->m_lBootPort);
    vTvmDisconnect(pstSavm);
    for(i = 0; i < pstBoot->m_lBootExec; i ++)
    {
        for(usleep(1000);ESRCH != pthread_kill(tPid[i], 0); usleep(1000));
    }

    vCloseDomain();
    TFree(tPid);
    Tlog("Service thread exits");
    exit(-1);
}

/*************************************************************************************************
 *   Domain maintenance interface
 *************************************************************************************************/
/*************************************************************************************************
    description：Notification reconnect the domain
    parameters:
        pstSavm                    --stvm handle
        pstDom                     --remote domain
        lPort                      --local port
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lConnectNotify(SATvm *pstSavm, TDomain *pstDom, long lPort)
{
    long    lWrite;
    void    *pvData = NULL;

    lWrite = sizeof(TFace) + sizeof(TDomain);
    if(NULL == (pvData = (char *)calloc(1, lWrite)))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    pstDom->m_skSock = skConnectServer(pstSavm, LOCAL_HOST_IP, lPort, false, 5);
    if(RC_FAIL == pstDom->m_skSock)
    {
        fprintf(stderr, "Connect server %s:%ld error, %s\n", LOCAL_HOST_IP, lPort, 
            sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    ((TFace *)pvData)->m_lFind  = lPort;
    ((TFace *)pvData)->m_enum   = OPERATE_DMRECN;
    ((TFace *)pvData)->m_table  = SYS_TVM_INDEX;
    ((TFace *)pvData)->m_lDLen  = sizeof(TDomain);
    ((TFace *)pvData)->m_lRows  = sizeof(TDomain);
    memcpy(pvData + sizeof(TFace), pstDom, sizeof(TDomain));
    if(lWrite != lSendBuffer(pstDom->m_skSock, (void *)pvData, lWrite))
    {
        vCloseSocket(pstDom);
        return RC_FAIL;
    }

    if(sizeof(TFace) != lRecvBuffer(pstDom->m_skSock, (char *)pvData, sizeof(TFace)))
    {
        vCloseSocket(pstDom);
        return RC_FAIL;
    }

    vCloseSocket(pstDom);
    pstSavm->m_lErrno = ((TFace *)pvData)->m_lErrno;
    if(0 != pstSavm->m_lErrno)
        return RC_FAIL;
    return RC_SUCC;
}

/*************************************************************************************************
    description：Notification refresh the domain
    parameters:
        pstSavm                    --stvm handle
        lPort                      --local port
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lRefreshNotify(SATvm *pstSavm, long lPort)
{
    BSock    skSock;
    TFace    stFace;

    stFace.m_lRows  = 0;
    stFace.m_lDLen  = 0;
    stFace.m_lFind  = lPort;
    stFace.m_table  = SYS_TVM_INDEX;
    stFace.m_enum   = OPERATE_REFRESH;
    skSock = skConnectServer(pstSavm, LOCAL_HOST_IP, lPort, false, 5);
    if(RC_FAIL == skSock)
    {
        fprintf(stderr, "Connect server %s:%ld error, %s\n", LOCAL_HOST_IP, lPort,
            sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    if(sizeof(TFace) != lSendBuffer(skSock, (void *)&stFace, sizeof(TFace)))
    {
        close(skSock);
        return RC_FAIL;
    }

    if(sizeof(TFace) != lRecvBuffer(skSock, (char *)&stFace, sizeof(TFace)))
    {
        close(skSock);
        return RC_FAIL;
    }
    close(skSock);

    pstSavm->m_lErrno = stFace.m_lErrno;
    if(0 != pstSavm->m_lErrno)
        return RC_FAIL;

    return RC_SUCC;
}

/*************************************************************************************************
    description：Notification offline 
    parameters:
        pstSavm                    --stvm handle
        lPort                      --local port
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lOfflineNotify(SATvm *pstSavm, long lPort)
{
    BSock    skSock;
    TFace    stFace;

    stFace.m_lRows  = 0;
    stFace.m_lDLen  = 0;
    stFace.m_lFind  = lPort;
    stFace.m_table  = SYS_TVM_INDEX;
    stFace.m_enum   = OPERATE_DOMLOFF;
    if(RC_FAIL == (skSock = skConnectServer(pstSavm, LOCAL_HOST_IP, lPort, false, 5)))
    {
        fprintf(stderr, "Connect server %s:%ld error, %s\n", LOCAL_HOST_IP, lPort,
            sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    if(sizeof(TFace) != lSendBuffer(skSock, (void *)&stFace, sizeof(TFace)))
    {
        close(skSock);
        return RC_FAIL;
    }

    if(sizeof(TFace) != lRecvBuffer(skSock, (char *)&stFace, sizeof(TFace)))
    {
        close(skSock);
        return RC_FAIL;
    }
    close(skSock);

    pstSavm->m_lErrno = stFace.m_lErrno;
    if(0 != pstSavm->m_lErrno)
        return RC_FAIL;

    return RC_SUCC;
}

/*************************************************************************************************
    description：Notifies to clone remote table
    parameters:
        pstSavm                    --stvm handle
        pstDom                     --remote domain
        lCount                     --record
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lPullNotify(SATvm *pstSavm, TDomain *pstDom, size_t lCount)
{
    TblDef  stDet;
    TFace   stFace;
    void    *pvData = NULL;
    size_t  lRecv, i, lRow = 0, lValid;

    memset(&stDet, 0, sizeof(TblDef));
    memset(&stFace, 0, sizeof(TFace));
    stFace.m_lRows  = 0;
    stFace.m_lFind  = lCount;
    stFace.m_enum   = OPERATE_PULTBL;
    stFace.m_table  = pstDom->m_mtable;
    if(RC_FAIL == (pstDom->m_skSock = skConnectServer(pstSavm, pstDom->m_szIp, pstDom->m_lPort, 
        false, pstDom->m_lTimeOut)))
    {
        fprintf(stderr, "Connect server %s:%ld error, %s\n", LOCAL_HOST_IP, pstDom->m_lPort,
            sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    if(sizeof(TFace) != lSendBuffer(pstDom->m_skSock, (void *)&stFace, sizeof(TFace)))
        goto ERR_PULLNOTIFY;

    if(sizeof(TFace) != lRecvBuffer(pstDom->m_skSock, (void *)&stFace, sizeof(TFace)))
        goto ERR_PULLNOTIFY;

    pstSavm->m_lErrno = stFace.m_lErrno;
    if(0 != pstSavm->m_lErrno)
        goto ERR_PULLNOTIFY;

    fprintf(stdout, "\nCopying table(%s)(%d) define ..", pstDom->m_szTable, pstDom->m_table);
    fflush(stdout);
    usleep(5000);
    if(sizeof(TblDef) != lRecvBuffer(pstDom->m_skSock, (void *)&stDet, sizeof(TblDef)))
        goto ERR_PULLNOTIFY;

    lValid = stDet.m_lValid;
    stDet.m_lValid = 0;
    stDet.m_lGroup = 0;
    if(RC_SUCC != lCustomTable(pstSavm, pstDom->m_table, stDet.m_lMaxRow, &stDet))
    {
        fprintf(stderr, "Create table(%d) failed, err:(%d)(%s)\n", pstDom->m_table, 
            pstSavm->m_lErrno, sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    fprintf(stdout, "\b\bcompleted .\nCopy table(%s) success, table maxrow:%ld, valid:%ld "
        " completed .\n", stDet.m_szTable, stDet.m_lMaxRow, lValid);
    fflush(stdout);
    if(NULL == (pstSavm = (SATvm *)pInitSATvm(pstDom->m_table)))
    {
        fprintf(stderr, "initial table(%s) error, err:(%d)(%s)\n", pstDom->m_szTable, 
            pstSavm->m_lErrno, sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }
 
    fprintf(stdout, "Start Copy table(%s)(%s)rows:[", pstDom->m_szTable, pstDom->m_szPart);
    fprintf(stdout, "\033[?25l");

    if(NULL == (pvData = (char *)malloc(lCount * stDet.m_lReSize))) 
        goto ERR_PULLNOTIFY;

    pstSavm->lSize  = stDet.m_lReSize;
    while(1)
    {
        if(sizeof(TFace) != lRecvBuffer(pstDom->m_skSock, (void *)&stFace, sizeof(TFace)))
            goto ERR_PULLNOTIFY;
        
        if(0 == stFace.m_lRows)    
            break;

        lRecv = stDet.m_lReSize * stFace.m_lRows;
        if(lRecv != lRecvBuffer(pstDom->m_skSock, (char *)pvData, lRecv))
            goto ERR_PULLNOTIFY;
        
        for(i = 0; i < stFace.m_lRows; i ++)
        {    
            pstSavm->pstVoid = (void *)pvData + i * stDet.m_lReSize;
            if(RC_SUCC != lInsert(pstSavm))
                goto ERR_PULLNOTIFY;

            ++ lRow;
            vPrintProgresss(lRow, lValid);
        }
    }
    TFree(pvData);
    fprintf(stdout, "\n");
    fprintf(stdout, "\033[?25h");
    fprintf(stdout, "Copy (%s)(%s) success，rows(%ld), completed successfully!!!\n", 
        pstDom->m_szTable, pstDom->m_szPart, lRow);
    fflush(stdout);

    vCloseSocket(pstDom);
    return RC_SUCC;

ERR_PULLNOTIFY:
    fprintf(stdout, "\n");
    fprintf(stdout, "\033[?25h");
    fflush(stdout);
    TFree(pvData);
    vCloseSocket(pstDom);
    return RC_FAIL;
}


/*************************************************************************************************
    Remote table access 
 *************************************************************************************************/
/*************************************************************************************************
    description：remote - Select
    parameters:
        pstSavm                    --stvm handle
        psvOut                     --out of data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lSelectByRt(SATvm *pstSavm, void *psvOut)
{
    long     lRet;
    TDomain  *pvm, *pnoe;
    Rowgrp   *list = NULL, *node = NULL;

    if(NULL == (node = pGetTblNode(pstSavm->tblName)))
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = RESOU_DISABLE;
    switch(lGetBootType())
    {
    case TVM_BOOT_CLUSTER:
        for(list = node->pstSSet; list; list = list->pstNext)
        {
            if(!list->pstFset)
                continue;
        
            if(NULL == (pvm = (TDomain *)(list->pstFset->psvData)))
                continue;
               
            pnoe = (TDomain *)list->psvData;
            if(0 == (OPERATE_SELECT & pnoe->m_lPers) || RESOURCE_ABLE != pvm->m_lStatus ||
                pnoe->m_lRelia < 0)
                continue;
            
            pstSavm->m_skSock = pvm->m_skSock;
            pstSavm->tblName  = pnoe->m_mtable;
            pthread_mutex_lock(&list->pstFset->lock);
            lRet = lTvmSelect(pstSavm, psvOut);
            if(RC_SUCC == lRet || SOCK_COM_EXCP != pstSavm->m_lErrno)
            {
                pvm->m_lTryTimes = 0;
                pvm->m_lLastTime = time(NULL);
            }

            pthread_mutex_unlock(&list->pstFset->lock);
            return lRet;
        }
        return RC_FAIL;
    default:
        for(list = node->pstSSet; list; list = list->pstNext)
        {
            if(NULL == (pvm = (TDomain *)(list->psvData)))
                continue;

            if(0 == (OPERATE_SELECT & pvm->m_lPers))
                continue;
            
            if(RC_SUCC != lTvmConnect(pstSavm, pvm->m_szIp, pvm->m_lPort, pvm->m_lTimeOut))
                continue;

            pstSavm->tblName = pvm->m_mtable;
            lRet = lTvmSelect(pstSavm, psvOut);
            if(RC_SUCC == lRet || SOCK_COM_EXCP != pstSavm->m_lErrno)
            {
                close(pstSavm->m_skSock);
                ((RunTime *)pGetRunTime(pstSavm, 0))->m_lRowSize = 0;
                TFree(((RunTime *)pGetRunTime(pstSavm, 0))->pstVoid);
                return lRet;
            }

            close(pstSavm->m_skSock);
        }

        ((RunTime *)pGetRunTime(pstSavm, 0))->m_lRowSize = 0;
        TFree(((RunTime *)pGetRunTime(pstSavm, 0))->pstVoid);
        return RC_FAIL;
    }
}

/*************************************************************************************************
    description：remote - Query
    parameters:
        pstSavm                    --stvm handle
        plOut                      --number
        psvOut                     --out of data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lQueryByRt(SATvm *pstSavm, size_t *plOut, void **ppsvOut)
{
    long     lRet;
    TDomain  *pvm, *pnoe;
    Rowgrp   *list = NULL, *node = NULL;

    if(NULL == (node = pGetTblNode(pstSavm->tblName)))
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = RESOU_DISABLE;
    switch(lGetBootType())
    {
    case TVM_BOOT_CLUSTER:
        for(list = node->pstSSet; list; list = list->pstNext)
        {
            if(!list->pstFset)
                continue;

            if(NULL == (pvm = (TDomain *)(list->pstFset->psvData)))
                continue;

            pnoe = (TDomain *)list->psvData;
            if(0 == (OPERATE_SELECT & pnoe->m_lPers) || RESOURCE_ABLE != pvm->m_lStatus ||
                pnoe->m_lRelia < 0)
                continue;

            pstSavm->m_skSock = pvm->m_skSock;
            pstSavm->tblName  = pnoe->m_mtable;
            pthread_mutex_lock(&list->pstFset->lock);
            lRet = lTvmQuery(pstSavm, plOut, ppsvOut);
            if(RC_SUCC == lRet || SOCK_COM_EXCP != pstSavm->m_lErrno)
            {
                pvm->m_lTryTimes = 0;
                pvm->m_lLastTime = time(NULL);
            }

            pthread_mutex_unlock(&list->pstFset->lock);
            return lRet;
        }

        return RC_FAIL;
    default:
        for(list = node->pstSSet; list; list = list->pstNext)
        {
            if(NULL == (pvm = (TDomain *)(list->psvData)))
                continue;

            if(0 == (OPERATE_SELECT & pvm->m_lPers))
                continue;

            if(RC_SUCC != lTvmConnect(pstSavm, pvm->m_szIp, pvm->m_lPort, pvm->m_lTimeOut))
                continue;

            pstSavm->tblName = pvm->m_mtable;
            lRet = lTvmQuery(pstSavm, plOut, ppsvOut);
            if(RC_SUCC == lRet || SOCK_COM_EXCP != pstSavm->m_lErrno)
            {
                close(pstSavm->m_skSock);
                ((RunTime *)pGetRunTime(pstSavm, 0))->m_lRowSize = 0;
                TFree(((RunTime *)pGetRunTime(pstSavm, 0))->pstVoid);
                return lRet;
            }

            close(pstSavm->m_skSock);
        }

        ((RunTime *)pGetRunTime(pstSavm, 0))->m_lRowSize = 0;
        TFree(((RunTime *)pGetRunTime(pstSavm, 0))->pstVoid);
        return RC_FAIL;
    }
}

/*************************************************************************************************
    description：remote - Delete
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lDeleteByRt(SATvm *pstSavm)
{
    TDomain  *pvm, *pnoe;
    long     lRet = RC_FAIL;
    Rowgrp   *list = NULL, *node = NULL;

    if(NULL == (node = pGetTblNode(pstSavm->tblName)))
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = RESOU_DISABLE;
    switch(lGetBootType())
    {
    case TVM_BOOT_CLUSTER:
        for(list = node->pstSSet; list; list = list->pstNext)
        {
            if(!list->pstFset)
                continue;

            if(NULL == (pvm = (TDomain *)(list->pstFset->psvData)))
                continue;

            pnoe = (TDomain *)list->psvData;
            if(0 == (OPERATE_DELETE & pnoe->m_lPers) || RESOURCE_ABLE != pvm->m_lStatus)
                continue;

            pstSavm->m_skSock = pvm->m_skSock;
            pstSavm->tblName  = pnoe->m_mtable;
            pthread_mutex_lock(&list->pstFset->lock);
            if(RC_SUCC == lTvmDelete(pstSavm))
            {
                lRet = RC_SUCC;
                pvm->m_lTryTimes = 0;
                pvm->m_lLastTime = time(NULL);
            }
            else if(SOCK_COM_EXCP == pstSavm->m_lErrno)
            {
                pnoe->m_lRelia --;
                Tlog("Delete err: %s, T(%d), F(%s:%d), R(%d)", sGetTError(pstSavm->m_lErrno),
                    pstSavm->tblName, pvm->m_szIp, pvm->m_lPort, pnoe->m_lRelia);
            }

            pthread_mutex_unlock(&list->pstFset->lock);
        }
        return lRet;
    default:
        for(list = node->pstSSet; list; list = list->pstNext)
        {
            if(NULL == (pvm = (TDomain *)(list->psvData)))
                continue;

            if(0 == (OPERATE_DELETE & pvm->m_lPers))
                continue;

            if(RC_SUCC != lTvmConnect(pstSavm, pvm->m_szIp, pvm->m_lPort, pvm->m_lTimeOut))
                continue;

            pstSavm->tblName = pvm->m_mtable;
            lRet = lTvmDelete(pstSavm);
            if(RC_SUCC == lRet || SOCK_COM_EXCP != pstSavm->m_lErrno)
            {
                close(pstSavm->m_skSock);
                ((RunTime *)pGetRunTime(pstSavm, 0))->m_lRowSize = 0;
                TFree(((RunTime *)pGetRunTime(pstSavm, 0))->pstVoid);
                return lRet;
            }

            close(pstSavm->m_skSock);
        }

        ((RunTime *)pGetRunTime(pstSavm, 0))->m_lRowSize = 0;
        TFree(((RunTime *)pGetRunTime(pstSavm, 0))->pstVoid);
        return lRet;
    }
}

/*************************************************************************************************
    description：remote - Truncate
    parameters:
        pstSavm                    --stvm handle
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lTruncateByRt(SATvm *pstSavm, TABLE t)
{
    TDomain  *pvm, *pnoe;
    long     lRet = RC_FAIL;
    Rowgrp   *list = NULL, *node = NULL;

    if(NULL == (node = pGetTblNode(t)))
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = RESOU_DISABLE;
    switch(lGetBootType())
    {
    case TVM_BOOT_CLUSTER:
        for(list = node->pstSSet; list; list = list->pstNext)
        {
            if(!list->pstFset)
                continue;

            if(NULL == (pvm = (TDomain *)(list->pstFset->psvData)))
                continue;

            pnoe = (TDomain *)list->psvData;
            if(0 == (OPERATE_DELETE & pnoe->m_lPers) || RESOURCE_ABLE != pvm->m_lStatus)
                continue;

            pstSavm->m_skSock = pvm->m_skSock;
            pstSavm->tblName  = pnoe->m_mtable;
            pthread_mutex_lock(&list->pstFset->lock);
            if(RC_SUCC == lTvmTruncate(pstSavm, pstSavm->tblName))
            {
                lRet = RC_SUCC;
                pvm->m_lTryTimes = 0;
                pvm->m_lLastTime = time(NULL);
            }
            else if(SOCK_COM_EXCP == pstSavm->m_lErrno)
            {
                pnoe->m_lRelia --;
                Tlog("Truncate err: %s, T(%d), F(%s:%d), R(%d)", sGetTError(pstSavm->m_lErrno),
                    pstSavm->tblName, pvm->m_szIp, pvm->m_lPort, pnoe->m_lRelia);
            }

            pthread_mutex_unlock(&list->pstFset->lock);
        }
        return lRet;
    default:
        for(list = node->pstSSet; list; list = list->pstNext)
        {
            if(NULL == (pvm = (TDomain *)(list->psvData)))
                continue;

            if(0 == (OPERATE_DELETE & pvm->m_lPers))
                continue;

            if(RC_SUCC != lTvmConnect(pstSavm, pvm->m_szIp, pvm->m_lPort, pvm->m_lTimeOut))
                continue;

            pstSavm->tblName = pvm->m_mtable;
            lRet = lTvmTruncate(pstSavm, pstSavm->tblName);
            if(RC_SUCC == lRet || SOCK_COM_EXCP != pstSavm->m_lErrno)
            {
                close(pstSavm->m_skSock);
                ((RunTime *)pGetRunTime(pstSavm, 0))->m_lRowSize = 0;
                TFree(((RunTime *)pGetRunTime(pstSavm, 0))->pstVoid);
                return lRet;
            }

            close(pstSavm->m_skSock);
        }

        ((RunTime *)pGetRunTime(pstSavm, 0))->m_lRowSize = 0;
        TFree(((RunTime *)pGetRunTime(pstSavm, 0))->pstVoid);
       
        return lRet;
    }
}

/*************************************************************************************************
    description：remote - Insert
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lInsertByRt(SATvm *pstSavm)
{
    TDomain  *pvm, *pnoe;
    long     lRet = RC_FAIL;
    Rowgrp   *list = NULL, *node = NULL;

    if(NULL == (node = pGetTblNode(pstSavm->tblName)))
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = RESOU_DISABLE;
    switch(lGetBootType())
    {
    case TVM_BOOT_CLUSTER:
        for(list = node->pstSSet; list; list = list->pstNext)
        {
            if(!list->pstFset)
                continue;
            if(NULL == (pvm = (TDomain *)(list->pstFset->psvData)))
                continue;

            pnoe = (TDomain *)list->psvData;
            if(0 == (OPERATE_INSERT & pnoe->m_lPers) || RESOURCE_ABLE != pvm->m_lStatus)
                continue;

            pstSavm->m_skSock = pvm->m_skSock;
            pstSavm->tblName  = pnoe->m_mtable;
            pthread_mutex_lock(&list->pstFset->lock);
            if(RC_SUCC == lTvmInsert(pstSavm))
            {
                lRet = RC_SUCC;
                pvm->m_lTryTimes = 0;
                pvm->m_lLastTime = time(NULL);
            }
            else if(SOCK_COM_EXCP == pstSavm->m_lErrno)
            {
                pnoe->m_lRelia --;
                Tlog("Insert err: %s, T(%d), F(%s:%d), R(%d)", sGetTError(pstSavm->m_lErrno), 
                    pstSavm->tblName, pvm->m_szIp, pvm->m_lPort, pnoe->m_lRelia);
            }

            pthread_mutex_unlock(&list->pstFset->lock);
        }
        return lRet;
    default:
        for(list = node->pstSSet; list; list = list->pstNext)
        {
            if(NULL == (pvm = (TDomain *)(list->psvData)))
                continue;

            if(0 == (OPERATE_INSERT & pvm->m_lPers))
                continue;

            if(RC_SUCC != lTvmConnect(pstSavm, pvm->m_szIp, pvm->m_lPort, pvm->m_lTimeOut))
                continue;

            pstSavm->tblName = pvm->m_mtable;
            lRet = lTvmInsert(pstSavm);
            if(RC_SUCC == lRet || SOCK_COM_EXCP != pstSavm->m_lErrno)
            {
                close(pstSavm->m_skSock);
                ((RunTime *)pGetRunTime(pstSavm, 0))->m_lRowSize = 0;
                TFree(((RunTime *)pGetRunTime(pstSavm, 0))->pstVoid);
                return lRet;
            }

            close(pstSavm->m_skSock);
        }

        ((RunTime *)pGetRunTime(pstSavm, 0))->m_lRowSize = 0;
        TFree(((RunTime *)pGetRunTime(pstSavm, 0))->pstVoid);
        return lRet;
    }
}

/*************************************************************************************************
    description：remote - Update
    parameters:
        pstSavm                    --stvm handle
        psvUpdate                  --update 
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lUpdateByRt(SATvm *pstSavm, void *pvUpdate)
{
    TDomain  *pvm, *pnoe;
    long     lRet = RC_FAIL;
    Rowgrp   *list = NULL, *node = NULL;

    if(NULL == (node = pGetTblNode(pstSavm->tblName)))
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = RESOU_DISABLE;
    switch(lGetBootType())
    {
    case TVM_BOOT_CLUSTER:
        for(list = node->pstSSet; list; list = list->pstNext)
        {
            if(!list->pstFset)
                continue;

            if(NULL == (pvm = (TDomain *)(list->pstFset->psvData)))
                continue;

            pnoe = (TDomain *)list->psvData;
            if(0 == (OPERATE_UPDATE & pnoe->m_lPers) || RESOURCE_ABLE != pvm->m_lStatus)
                continue;

            pstSavm->m_skSock = pvm->m_skSock;
            pstSavm->tblName  = pnoe->m_mtable;
            pthread_mutex_lock(&list->pstFset->lock);
            if(RC_SUCC == lTvmUpdate(pstSavm, pvUpdate))
            {
                lRet = RC_SUCC;
                pvm->m_lTryTimes = 0;
                pvm->m_lLastTime = time(NULL);
            }
            else if(SOCK_COM_EXCP == pstSavm->m_lErrno)
            {
                pnoe->m_lRelia --;
                Tlog("Update err: %s, T(%d), F(%s:%d), R(%d)", sGetTError(pstSavm->m_lErrno),
                     pstSavm->tblName, pvm->m_szIp, pvm->m_lPort, pnoe->m_lRelia);
            }

            pthread_mutex_unlock(&list->pstFset->lock);
        }
        return lRet;
    default:
        for(list = node->pstSSet; list; list = list->pstNext)
        {
            if(NULL == (pvm = (TDomain *)(list->psvData)))
                continue;

            if(0 == (OPERATE_UPDATE & pvm->m_lPers))
                continue;

            if(RC_SUCC != lTvmConnect(pstSavm, pvm->m_szIp, pvm->m_lPort, pvm->m_lTimeOut))
                continue;

            pstSavm->tblName = pvm->m_mtable;
            lRet = lTvmUpdate(pstSavm, pvUpdate);
            if(RC_SUCC == lRet || SOCK_COM_EXCP != pstSavm->m_lErrno)
            {
                close(pstSavm->m_skSock);
                ((RunTime *)pGetRunTime(pstSavm, 0))->m_lRowSize = 0;
                TFree(((RunTime *)pGetRunTime(pstSavm, 0))->pstVoid);
                return lRet;
            }

            close(pstSavm->m_skSock);
        }

        ((RunTime *)pGetRunTime(pstSavm, 0))->m_lRowSize = 0;
        TFree(((RunTime *)pGetRunTime(pstSavm, 0))->pstVoid);
        return lRet;
    }
}

/*************************************************************************************************
    description：remote - Replace
    parameters:
        pstSavm                    --stvm handle
        psvUpdate                  --update 
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lReplaceByRt(SATvm *pstSavm, void *pvReplace)
{
    TDomain  *pvm, *pnoe;
    long     lRet = RC_FAIL;
    Rowgrp   *list = NULL, *node = NULL;

    if(NULL == (node = pGetTblNode(pstSavm->tblName)))
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = RESOU_DISABLE;
    switch(lGetBootType())
    {
    case TVM_BOOT_CLUSTER:
        for(list = node->pstSSet; list; list = list->pstNext)
        {
            if(!list->pstFset)
                continue;

            if(NULL == (pvm = (TDomain *)(list->pstFset->psvData)))
                continue;

            pnoe = (TDomain *)list->psvData;
            if(0 == (OPERATE_UPDATE & pnoe->m_lPers) || 0 == (OPERATE_INSERT & pnoe->m_lPers)
                || RESOURCE_ABLE != pvm->m_lStatus)
                continue;

            pstSavm->m_skSock = pvm->m_skSock;
            pstSavm->tblName  = pnoe->m_mtable;
            pthread_mutex_lock(&list->pstFset->lock);
            if(RC_SUCC == lTvmReplace(pstSavm, pvReplace))
            {
                lRet = RC_SUCC;
                pvm->m_lTryTimes = 0;
                pvm->m_lLastTime = time(NULL);
            }
            else if(SOCK_COM_EXCP == pstSavm->m_lErrno)
            {
                pnoe->m_lRelia --;
                Tlog("Update err: %s, T(%d), F(%s:%d), R(%d)", sGetTError(pstSavm->m_lErrno),
                     pstSavm->tblName, pvm->m_szIp, pvm->m_lPort, pnoe->m_lRelia);
            }

            pthread_mutex_unlock(&list->pstFset->lock);
        }
        return lRet;
    default:
        for(list = node->pstSSet; list; list = list->pstNext)
        {
            if(NULL == (pvm = (TDomain *)(list->psvData)))
                continue;

            if(0 == (OPERATE_UPDATE & pnoe->m_lPers) || 0 == (OPERATE_INSERT & pnoe->m_lPers))
                continue;

            if(RC_SUCC != lTvmConnect(pstSavm, pvm->m_szIp, pvm->m_lPort, pvm->m_lTimeOut))
                continue;

            pstSavm->tblName = pvm->m_mtable;
            lRet = lTvmReplace(pstSavm, pvReplace);
            if(RC_SUCC == lRet || SOCK_COM_EXCP != pstSavm->m_lErrno)
            {
                close(pstSavm->m_skSock);
                ((RunTime *)pGetRunTime(pstSavm, 0))->m_lRowSize = 0;
                TFree(((RunTime *)pGetRunTime(pstSavm, 0))->pstVoid);
                return lRet;
            }

            close(pstSavm->m_skSock);
        }

        ((RunTime *)pGetRunTime(pstSavm, 0))->m_lRowSize = 0;
        TFree(((RunTime *)pGetRunTime(pstSavm, 0))->pstVoid);
        return lRet;
    }
}

/*************************************************************************************************
    description：remote - Group
    parameters:
        pstSavm                    --stvm handle
        plOut                      --number
        psvOut                     --out of data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lGroupByRt(SATvm *pstSavm, size_t *plOut, void **ppvOut)
{
    long     lRet;
    TDomain  *pvm, *pnoe;
    Rowgrp   *list = NULL, *node = NULL;

    if(NULL == (node = pGetTblNode(pstSavm->tblName)))
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = RESOU_DISABLE;
    switch(lGetBootType())
    {
    case TVM_BOOT_CLUSTER:
        for(list = node->pstSSet; list; list = list->pstNext)
        {
            if(!list->pstFset)
                continue;

            if(NULL == (pvm = (TDomain *)(list->pstFset->psvData)))
                continue;

            pnoe = (TDomain *)list->psvData;
            if(0 == (OPERATE_SELECT & pnoe->m_lPers) || RESOURCE_ABLE != pvm->m_lStatus ||
                pnoe->m_lRelia < 0)
                continue;

            pstSavm->m_skSock = pvm->m_skSock;
            pstSavm->tblName  = pnoe->m_mtable;
            pthread_mutex_lock(&list->pstFset->lock);
            lRet = lTvmGroup(pstSavm, plOut, ppvOut);
            if(RC_SUCC == lRet || SOCK_COM_EXCP != pstSavm->m_lErrno)
            {
                pvm->m_lTryTimes = 0;
                pvm->m_lLastTime = time(NULL);
            }

            pthread_mutex_unlock(&list->pstFset->lock);
            return lRet;
        }
        return RC_FAIL;
    default:
        for(list = node->pstSSet; list; list = list->pstNext)
        {
            if(NULL == (pvm = (TDomain *)(list->psvData)))
                continue;

            if(0 == (OPERATE_SELECT & pvm->m_lPers))
                continue;

            if(RC_SUCC != lTvmConnect(pstSavm, pvm->m_szIp, pvm->m_lPort, pvm->m_lTimeOut))
                continue;

            pstSavm->tblName = pvm->m_mtable;
            lRet = lTvmGroup(pstSavm, plOut, ppvOut);
            if(RC_SUCC == lRet || SOCK_COM_EXCP != pstSavm->m_lErrno)
            {
                close(pstSavm->m_skSock);
                ((RunTime *)pGetRunTime(pstSavm, 0))->m_lRowSize = 0;
                TFree(((RunTime *)pGetRunTime(pstSavm, 0))->pstVoid);
                return lRet;
            }

            close(pstSavm->m_skSock);
        }

        ((RunTime *)pGetRunTime(pstSavm, 0))->m_lRowSize = 0;
        TFree(((RunTime *)pGetRunTime(pstSavm, 0))->pstVoid);
        return RC_FAIL;
    }
}

/*************************************************************************************************
    description：remote - Count
    parameters:
        pstSavm                    --stvm handle
        plOut                      --count
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lCountByRt(SATvm *pstSavm, size_t *plCount)
{
    long     lRet;
    TDomain  *pvm, *pnoe;
    Rowgrp   *list = NULL, *node = NULL;

    if(NULL == (node = pGetTblNode(pstSavm->tblName)))
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = RESOU_DISABLE;
    switch(lGetBootType())
    {
    case TVM_BOOT_CLUSTER:
        for(list = node->pstSSet; list; list = list->pstNext)
        {
            if(!list->pstFset)
                continue;

            if(NULL == (pvm = (TDomain *)(list->pstFset->psvData)))
                continue;

            pnoe = (TDomain *)list->psvData;
            if(0 == (OPERATE_SELECT & pnoe->m_lPers) || RESOURCE_ABLE != pvm->m_lStatus ||
                pnoe->m_lRelia < 0)
                continue;

            pstSavm->m_skSock = pvm->m_skSock;
            pstSavm->tblName  = pnoe->m_mtable;
            pthread_mutex_lock(&list->pstFset->lock);
            lRet = lTvmCount(pstSavm, plCount);
            if(RC_SUCC == lRet || SOCK_COM_EXCP != pstSavm->m_lErrno)
            {
                pvm->m_lTryTimes = 0;
                pvm->m_lLastTime = time(NULL);
            }

            pthread_mutex_unlock(&list->pstFset->lock);
            return lRet;
        }
        return RC_FAIL;
    default:
        for(list = node->pstSSet; list; list = list->pstNext)
        {
            if(NULL == (pvm = (TDomain *)(list->psvData)))
                continue;

            if(0 == (OPERATE_SELECT & pvm->m_lPers))
                continue;

            if(RC_SUCC != lTvmConnect(pstSavm, pvm->m_szIp, pvm->m_lPort, pvm->m_lTimeOut))
                continue;

            pstSavm->tblName = pvm->m_mtable;
            lRet = lTvmCount(pstSavm, plCount);
            if(RC_SUCC == lRet || SOCK_COM_EXCP != pstSavm->m_lErrno)
            {
                close(pstSavm->m_skSock);
                ((RunTime *)pGetRunTime(pstSavm, 0))->m_lRowSize = 0;
                TFree(((RunTime *)pGetRunTime(pstSavm, 0))->pstVoid);
                return lRet;
            }

            close(pstSavm->m_skSock);
        }

        ((RunTime *)pGetRunTime(pstSavm, 0))->m_lRowSize = 0;
        TFree(((RunTime *)pGetRunTime(pstSavm, 0))->pstVoid);
        return RC_FAIL;
    }
}

/*************************************************************************************************
    description：remote - Extreme
    parameters:
        pstSavm                    --stvm handle
        psvOut                     --out of data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lExtremeByRt(SATvm *pstSavm, void *psvOut)
{
    long     lRet;
    TDomain  *pvm, *pnoe;
    Rowgrp   *list = NULL, *node = NULL;

    if(NULL == (node = pGetTblNode(pstSavm->tblName)))
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = RESOU_DISABLE;
    switch(lGetBootType())
    {
    case TVM_BOOT_CLUSTER:
        for(list = node->pstSSet; list; list = list->pstNext)
        {
            if(!list->pstFset)
                continue;

            if(NULL == (pvm = (TDomain *)(list->pstFset->psvData)))
                continue;

            pnoe = (TDomain *)list->psvData;
            if(0 == (OPERATE_SELECT & pnoe->m_lPers) || RESOURCE_ABLE != pvm->m_lStatus ||
                pnoe->m_lRelia < 0)
                continue;

            pstSavm->m_skSock = pvm->m_skSock;
            pstSavm->tblName  = pnoe->m_mtable;
            pthread_mutex_lock(&list->pstFset->lock);
            lRet = lTvmExtreme(pstSavm, psvOut);
            if(RC_SUCC == lRet || SOCK_COM_EXCP != pstSavm->m_lErrno)
            {
                pvm->m_lTryTimes = 0;
                pvm->m_lLastTime = time(NULL);
            }

            pthread_mutex_unlock(&list->pstFset->lock);
            return lRet;
        }
        return RC_FAIL;
    default:
        for(list = node->pstSSet; list; list = list->pstNext)
        {
            if(NULL == (pvm = (TDomain *)(list->psvData)))
                continue;

            if(0 == (OPERATE_SELECT & pvm->m_lPers))
                continue;

            if(RC_SUCC != lTvmConnect(pstSavm, pvm->m_szIp, pvm->m_lPort, pvm->m_lTimeOut))
                continue;

            pstSavm->tblName = pvm->m_mtable;
            lRet = lTvmExtreme(pstSavm, psvOut);
            if(RC_SUCC == lRet || SOCK_COM_EXCP != pstSavm->m_lErrno)
            {
                close(pstSavm->m_skSock);
                ((RunTime *)pGetRunTime(pstSavm, 0))->m_lRowSize = 0;
                TFree(((RunTime *)pGetRunTime(pstSavm, 0))->pstVoid);
                return lRet;
            }

            close(pstSavm->m_skSock);
        }

        ((RunTime *)pGetRunTime(pstSavm, 0))->m_lRowSize = 0;
        TFree(((RunTime *)pGetRunTime(pstSavm, 0))->pstVoid);
        return RC_FAIL;
    }
}

/*************************************************************************************************
    description：remote - Droptable
    parameters:
        pstSavm                    --stvm handle
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
void    _vDropTableByRt(SATvm *pstSavm, TABLE t)
{
    Rowgrp   *node = NULL;

    if(NULL == (node = pGetTblNode(t)))
        return ;
    
    memset(node->psvData, 0, sizeof(TABLE));
//    vDropNodegrp(&g_pstTblgrp, node);
}

/*************************************************************************************************
    description：remote - Rename
    parameters:
        pstSavm                    --stvm handle
        to                         --from table
        tn                         --to table 
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lRenameTableByRt(SATvm *pstSavm, TABLE to, TABLE tn)
{
    TDomain  stDom, stUpd;
    Rowgrp   *node = NULL;

    if(NULL == (node = pGetTblNode(to)))
        return RC_SUCC;

    if(RC_SUCC != lInitSATvm(pstSavm, SYS_TVM_DOMAIN))
        return RC_FAIL;

    updateinit(pstSavm, stUpd);
    conditinit(pstSavm, stDom, SYS_TVM_DOMAIN);
    conditnum(pstSavm, stDom, m_table, to)
    updatenum(pstSavm, stUpd, m_table, tn);

    if(RC_SUCC != lUpdate(pstSavm, &stUpd))
        return RC_FAIL;

    memcpy(node->psvData, &tn, sizeof(TABLE));
    return RC_SUCC;
}

/*************************************************************************************************
    description：disconnect server
    parameters:
        pstSavm                    --stvm handle
    return:
 *************************************************************************************************/
void    vTvmDisconnect(SATvm *pstSavm)
{
    RunTime *pstRun;

    if(!pstSavm)    return ;

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    pstRun->m_lRowSize = 0;
    close(pstSavm->m_skSock);
    TFree(pstRun->pstVoid);

    vHoldRelease(pstSavm);
}

/*************************************************************************************************
    description：connect server
    parameters:
        pstSavm                    --stvm handle
        pszIp                      --server ip
        lPort                      --server port
        times                      --time out
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmConnect(SATvm *pstSavm, char *pszIp, long lPort, int times)
{
    RunTime *pstRun;

    if(!pstSavm || !pszIp)
        return RC_FAIL;

    pstSavm->m_lTimes = times;
    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    pstSavm->m_skSock = skConnectServer(pstSavm, pszIp, lPort, false, times);
    if(RC_FAIL == pstSavm->m_skSock)
    {
        Tlog("Connect server %s:%d failed, %s", pszIp, lPort, sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    return lTvmBuffer(pstSavm);
}

/*************************************************************************************************
    description：parse Java packed
    parameters:
        pstSavm                    --stvm handle
        pstVoid                    --conditon
        pstFace                    --request head
        pvBuffer                   --recv buffer
        lLen                       --the length of buffer
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
void*    pProtocaJava(SATvm *pstSavm, void *pstVoid, TFace *pstFace, void *pvBuffer, long lLen)
{
    void     *pvData, *q;
    int      i, len, n, m, k;
    FdCond   *pstCond = &pstSavm->stCond;
    char     szLen[5] = {0}, szField[MAX_FIELD_LEN];
    TblKey   *pstKey, *pv = pGetTblKey(pstFace->m_table);

    if(pstFace->m_lRows == 0)
        return pvBuffer;

//    pstFace->m_lDLen
//  szName[256]="020020tx_date=201708120016tx_name=DefPuzzL";
    pstCond->uFldcmp = 0;
    memcpy(szLen, pvBuffer, 2);
    for(i = 0, lLen -= 2, pvData = pvBuffer + 2, k = atol(szLen); i < k; i ++)
    {
        memcpy(szLen, pvData, 4);
        pvData += 4;

        if(lLen < (len = strlen(szLen)))
        {
            pstSavm->m_lErrno = SQL_SYNTX_ERR;
            return NULL;
        }
        lLen -= (len + 4);

        if(NULL == (q = strstr(pvData, "=")))
        {
            pstSavm->m_lErrno = SQL_SYNTX_ERR;
            return NULL;
        }

        n = q - pvData;
        m = MIN(n, MAX_FIELD_LEN);
        memcpy(szField, pvData, m);
        szField[m] = 0x00;
        if(NULL == (pstKey = pFindField(pv, lGetIdxNum(pstFace->m_table), szField)))
        {
            pstSavm->m_lErrno = FLD_NOT_EXIST;
            return NULL;
        }

        vSetCodField(pstCond, pstKey->m_lLen, pstKey->m_lFrom);
        memcpy(pstVoid + pstKey->m_lFrom, pvData + n + 1, m);
        pvData += len;
    }

    switch(pstFace->m_enum)
    {
    case OPERATE_INSERT:
        return pvBuffer;
    case OPERATE_REPLACE:
    case OPERATE_UPDATE:
        pstCond = &pstSavm->stUpdt;
        pstCond->uFldcmp = 0;
        memset(szLen, 0, sizeof(szLen));
        memcpy(szLen, pvBuffer, 2);
        pstVoid += pstFace->m_lDLen;
        for(i = 0, lLen -= 2, pvData += 2, k = atol(szLen); i < k; i ++)
        {
            memcpy(szLen, pvData, 4);
            pvData += 4;
        
            if(lLen < (len = strlen(szLen))) 
            {
                pstSavm->m_lErrno = SQL_SYNTX_ERR;
                return NULL;
            }
            lLen -= (len + 4);
        
            if(NULL == (q = strstr(pvData, "=")))
            {
                pstSavm->m_lErrno = SQL_SYNTX_ERR;
                return NULL;
            }
        
            n = q - pvData;
            m = MIN(n, MAX_FIELD_LEN);
            memcpy(szField, pvData, m);
            szField[m] = 0x00;
            if(NULL == (pstKey = pFindField(pv, lGetIdxNum(pstFace->m_table), szField)))
            {
                pstSavm->m_lErrno = FLD_NOT_EXIST;
                return NULL;
            }
        
            vSetCodField(pstCond, pstKey->m_lLen, pstKey->m_lFrom);
            memcpy(pstVoid + pstKey->m_lFrom, pvData + n + 1, m);
            pvData += len;
        }

        if(lLen < 0)  
        {
            pstSavm->m_lErrno = SQL_SYNTX_ERR;
            return NULL;
        }

        return memcpy(pvBuffer, pstVoid, pstFace->m_lDLen);
    case OPERATE_SELECT:
    case OPERATE_QUERY:
    case OPERATE_DELETE:
    case OPERATE_TRCATE:
    case OPERATE_GROUP:
    case OPERATE_EXTREM:
        if(lLen < 0)
        {
            pstSavm->m_lErrno = SQL_SYNTX_ERR;
            return NULL;
        }

        pstCond = &pstSavm->stUpdt;
        pstCond->uFldcmp = 0;
        memset(szLen, 0, sizeof(szLen));
        memcpy(szLen, pvBuffer, 2);
        for(i = 0, lLen -= 2, pvData += 2, k = atol(szLen); i < k; i ++)
        {
            memcpy(szLen, pvData, 4);
            pvData += 4;
        
            if(lLen < (len = strlen(szLen))) 
            {
                pstSavm->m_lErrno = SQL_SYNTX_ERR;
                return NULL;
            }

            lLen -= (len + 4);
            if(NULL == (q = strstr(pvData, "=")))
            {
                pstSavm->m_lErrno = SQL_SYNTX_ERR;
                return NULL;
            }
        
            n = q - pvData;
            m = MIN(n, MAX_FIELD_LEN);
            memcpy(szField, pvData, m);
            szField[m] = 0x00;
            if(NULL == (pstKey = pFindField(pv, lGetIdxNum(pstFace->m_table), szField)))
            {
                pstSavm->m_lErrno = FLD_NOT_EXIST;
                return NULL;
            }
        
            m = ((char *)pvData)[n + 1];
            vSetDecorate(pstCond, pstKey->m_lLen, pstKey->m_lFrom, (Uenum)m);    
            pstSavm->lFind = m & FIRST_ROW;
            pvData += len;
        }

        return pvBuffer;
    default:
        return pvBuffer;
    }

    pstSavm->m_lErrno = SQL_SYNTX_ERR;
    return NULL;
}

/*************************************************************************************************
    description：parse packed
    parameters:
        pstSavm                    --stvm handle
        pstVoid                    --conditon
        pstFace                    --request head
        pvBuffer                   --recv buffer
        lLen                       --the length of buffer
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
void*    pParsePacket(SATvm *pstSavm, void *pstVoid, TFace *pstFace, void *pvBuffer, long lLen)
{
    register uint i;
    FdKey    *pstFld;
    void     *pvData = pvBuffer;
    FdCond   *pstCond = &pstSavm->stCond;

    if(pstFace->m_lRows == 0)
        return pvBuffer;

    switch(pstFace->m_enum)
    {
    case OPERAYS_INSERT:
    case OPERATE_INSERT:
        return memcpy(pstVoid, pvData, pstFace->m_lDLen);
    case OPERAYS_UPDATE:
    case OPERATE_UPDATE:
    case OPERATS_REPLACE:
    case OPERATE_REPLACE:
        memcpy(&pstCond->uFldcmp, pvData, sizeof(uint));
        for(i = 0, pvData += sizeof(uint); i < pstCond->uFldcmp; i ++)
        {
            pstFld = &pstCond->stFdKey[i];
            memcpy(pstFld, pvData, sizeof(FdKey));
            pvData += sizeof(FdKey);
            memcpy(pstVoid + pstFld->uFldpos, pvData, pstFld->uFldlen);
            pvData += pstFld->uFldlen;
        }

        pstCond = &pstSavm->stUpdt;
        memcpy(&pstCond->uFldcmp, pvData, sizeof(uint));
        if(0 == pstCond->uFldcmp)    return pvBuffer;

        for(i = 0, pvData += sizeof(uint), pstVoid += pstFace->m_lDLen; i < pstCond->uFldcmp; i ++)
        {
            pstFld = &pstCond->stFdKey[i];
            memcpy(pstFld, pvData, sizeof(FdKey));
            pvData += sizeof(FdKey);
            memcpy(pstVoid + pstFld->uFldpos, pvData, pstFld->uFldlen);
            pvData += pstFld->uFldlen;
        }

        return memcpy(pvBuffer, pstVoid, pstFace->m_lDLen);
    case OPERATE_SELECT:
    case OPERATE_QUERY:
    case OPERATE_DELETE:
    case OPERAYS_DELETE:
    case OPERATE_TRCATE:
    case OPERATE_GROUP:
    case OPERATE_COUNT:
    case OPERATE_CLICK:
    case OPERATE_EXTREM:
        memcpy(&pstCond->uFldcmp, pvData, sizeof(uint));
        for(i = 0, pvData += sizeof(uint); i < pstCond->uFldcmp; i ++)    
        {
            pstFld = &pstCond->stFdKey[i];
            memcpy(pstFld, pvData, sizeof(FdKey));
            pvData += sizeof(FdKey);
            memcpy(pstVoid + pstFld->uFldpos, pvData, pstFld->uFldlen);
            pvData += pstFld->uFldlen;
        } 

        pstCond = &pstSavm->stUpdt;
        memcpy(&pstCond->uFldcmp, pvData, sizeof(uint));
        if(0 == pstCond->uFldcmp)    return pvBuffer;

        pvData += sizeof(uint);
        memcpy(pstCond->stFdKey, pvData, sizeof(FdKey) * pstCond->uFldcmp);
        return pvBuffer;
/*
    case OPERATE_TBDROP:
    case OPERATE_RENAME:
    case OPERATE_SELSEQ:
    case OPERATE_SETSEQ:
    case OPERATE_RETLOK:
    case OPERATE_RBDIDX:
    case OPERATE_DOMPSH:
    case OPERATE_DOMPUL:
    ....
*/
    default:
        return pvBuffer;
    }

    return NULL;
}

/*************************************************************************************************
    description：build condition packed
    parameters:
        pvData                     --send buffer
        pvBuffer                   --condition
        pstCond                    --cond field
        plRows                     --length
    return:
 *************************************************************************************************/
void    vBuildPacket(void *pvData, void *pvBuffer, FdCond *pstCond, uint *plRows)
{
    uint    i;
    FdKey   *pstFld;

    if(!pvBuffer)
    {
        memset(pvData + (*plRows), 0, sizeof(uint));
        (*plRows) += sizeof(uint);
        return; 
    }

    memcpy(pvData + (*plRows), (void *)&pstCond->uFldcmp, sizeof(uint));
    for(i = 0, (*plRows) += sizeof(uint); i < pstCond->uFldcmp; i ++)
    {
        pstFld = &pstCond->stFdKey[i];
        memcpy(pvData + (*plRows), (void *)pstFld, sizeof(FdKey));
        (*plRows) += sizeof(FdKey);
        memcpy(pvData + (*plRows), pvBuffer + pstFld->uFldpos, pstFld->uFldlen);
        (*plRows) += pstFld->uFldlen;
    }

    return ;
}

/*************************************************************************************************
    description：parse decorate packed
    parameters:
        pvData                     --send buffer
        pstCond                    --decorate field
        plRows                     --length
    return:
 *************************************************************************************************/
void    vAppendCond(void *pvData, FdCond *pstCond, uint *plRows)
{
    memcpy(pvData + (*plRows), (void *)&pstCond->uFldcmp, sizeof(uint));
    (*plRows) += sizeof(uint);
    if(pstCond->uFldcmp <= 0)
        return ;

    memcpy(pvData + (*plRows), (void *)&pstCond->stFdKey, pstCond->uFldcmp * sizeof(FdKey));
    (*plRows) += sizeof(FdKey) * pstCond->uFldcmp;
    return ;
}

/*************************************************************************************************
    description：API - select
    parameters:
        pstSavm                    --stvm handle
        pvOut                      --out data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmSelect(SATvm *pstSavm, void *pvOut)
{
    RunTime *pstRun;
    TFace   *pstFace;
    uint    lWrite = sizeof(TFace);

    if(!pstSavm || !pstSavm->pstVoid)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(!pstRun->pstVoid)
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstFace = (TFace *)pstRun->pstVoid;
    pstFace->m_lFind  = pstSavm->lFind;
    pstFace->m_lDLen  = pstSavm->lSize;
    pstFace->m_lErrno = TVM_DONE_SUCC;
    pstFace->m_enum   = OPERATE_SELECT;
    pstFace->m_table  = pstSavm->tblName;

    checkbuffer(pstSavm, pstRun, 1);
    vBuildPacket(pstRun->pstVoid, pstSavm->pstVoid, &pstSavm->stCond, &lWrite);
    vAppendCond(pstRun->pstVoid, &pstSavm->stUpdt, &lWrite);
    pstFace->m_lRows = lWrite - sizeof(TFace);

    if(lWrite != lSendBuffer(pstSavm->m_skSock, (char *)pstRun->pstVoid, lWrite))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }
        
    if(sizeof(TFace) != lRecvBuffer(pstSavm->m_skSock, (char *)pstRun->pstVoid, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    pstSavm->m_lErrno  = pstFace->m_lErrno;
    if(0 != pstSavm->m_lErrno)
        return RC_FAIL;

    pstSavm->m_lEffect = pstFace->m_lRows;
    pstSavm->m_lEType  = pstFace->m_lDLen;
    if(pstSavm->lSize != lRecvBuffer(pstSavm->m_skSock, (char *)pvOut, pstSavm->lSize))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：API - query
    parameters:
        pstSavm                    --stvm handle
        plOut                      --number
        ppvOut                     --out data list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmQuery(SATvm *pstSavm, size_t *plOut, void **ppvOut)
{
    RunTime *pstRun;
    TFace   *pstFace;
    uint    lWrite = sizeof(TFace);

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(!pstRun->pstVoid)
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstFace = (TFace *)pstRun->pstVoid;
    pstFace->m_lFind  = pstSavm->lFind;
    pstFace->m_lDLen  = pstSavm->lSize;
    pstFace->m_lErrno = TVM_DONE_SUCC;
    pstFace->m_enum   = OPERATE_QUERY;
    pstFace->m_table  = pstSavm->tblName;

    checkbuffer(pstSavm, pstRun, 1);
    vBuildPacket(pstRun->pstVoid, pstSavm->pstVoid, &pstSavm->stCond, &lWrite);
    vAppendCond(pstRun->pstVoid, &pstSavm->stUpdt, &lWrite);
    pstFace->m_lRows = lWrite - sizeof(TFace);

    if(lWrite != lSendBuffer(pstSavm->m_skSock, (char *)pstRun->pstVoid, lWrite))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }
        
    if(sizeof(TFace) != lRecvBuffer(pstSavm->m_skSock, (char *)pstRun->pstVoid, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = pstFace->m_lErrno;
    if(0 != pstSavm->m_lErrno)
        return RC_FAIL;

    pstRun->m_lRowSize = pstSavm->lSize * pstFace->m_lRows;
    if(NULL == (*ppvOut = (void *)malloc(pstRun->m_lRowSize)))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    if(pstRun->m_lRowSize != lRecvBuffer(pstSavm->m_skSock, (char *)*ppvOut, pstRun->m_lRowSize))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    *plOut = pstFace->m_lRows;
    pstSavm->m_lEffect = pstFace->m_lRows;
    pstSavm->m_lEType  = pstFace->m_lDLen;
    return RC_SUCC;
}

/*************************************************************************************************
    description：API - delete by asynch
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lAsyDelete(SATvm *pstSavm)
{
    RunTime *pstRun;
    TFace   *pstFace;
    uint    lWrite = sizeof(TFace);

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(!pstRun->pstVoid)
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstFace = (TFace *)pstRun->pstVoid;
    pstFace->m_lFind  = pstSavm->lFind;
    pstFace->m_lDLen  = pstSavm->lSize;
    pstFace->m_lErrno = TVM_DONE_SUCC;
    pstFace->m_enum   = OPERAYS_DELETE;
    pstFace->m_table  = pstSavm->tblName;

    checkbuffer(pstSavm, pstRun, 1);
    vBuildPacket(pstRun->pstVoid, pstSavm->pstVoid, &pstSavm->stCond, &lWrite);
    vAppendCond(pstRun->pstVoid, &pstSavm->stUpdt, &lWrite);
    pstFace->m_lRows = lWrite - sizeof(TFace);

    if(lWrite != lSendBuffer(pstSavm->m_skSock, (void *)pstRun->pstVoid, lWrite))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }
        
    return RC_SUCC;
}

/*************************************************************************************************
    description：API - update by asynch
    parameters:
        pstSavm                    --stvm handle
        pvData                     --update data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lAsyUpdate(SATvm *pstSavm, void *pvData)
{
    RunTime *pstRun;
    FdCond  *pstCond;
    TFace   *pstFace;
    uint    lWrite = sizeof(TFace);

    if(!pstSavm || !pvData)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(!pstRun->pstVoid)
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstFace = (TFace *)pstRun->pstVoid;
    pstFace->m_lFind  = pstSavm->lFind;
    pstFace->m_lDLen  = pstSavm->lSize;
    pstFace->m_lErrno = TVM_DONE_SUCC;
    pstFace->m_enum   = OPERAYS_UPDATE;
    pstFace->m_table  = pstSavm->tblName;

    checkbuffer(pstSavm, pstRun, 2);
    pstCond = &pstSavm->stUpdt;
    if(0 == pstCond->uFldcmp)
    {
        pstSavm->m_lErrno = UPDFD_NOT_SET;
        return RC_FAIL;
    }

    vBuildPacket(pstRun->pstVoid, pstSavm->pstVoid, &pstSavm->stCond, &lWrite);
    vBuildPacket(pstRun->pstVoid, pvData, pstCond, &lWrite);
    pstFace->m_lRows  = lWrite - sizeof(TFace);

    if(lWrite != lSendBuffer(pstSavm->m_skSock, (void *)pstRun->pstVoid, lWrite))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }
        
    return RC_SUCC;
}

/*************************************************************************************************
    description：API - replace by asynch
    parameters:
        pstSavm                    --stvm handle
        pvData                     --update data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lAsyReplace(SATvm *pstSavm, void *pvData)
{
    RunTime *pstRun;
    FdCond  *pstCond;
    TFace   *pstFace;
    uint    lWrite = sizeof(TFace);

    if(!pstSavm || !pvData)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(!pstRun->pstVoid)
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstFace = (TFace *)pstRun->pstVoid;
    pstFace->m_lFind  = pstSavm->lFind;
    pstFace->m_lDLen  = pstSavm->lSize;
    pstFace->m_lErrno = TVM_DONE_SUCC;
    pstFace->m_enum   = OPERATS_REPLACE;
    pstFace->m_table  = pstSavm->tblName;

    checkbuffer(pstSavm, pstRun, 2);
    pstCond = &pstSavm->stUpdt;
    if(0 == pstCond->uFldcmp)
    {
        pstSavm->m_lErrno = UPDFD_NOT_SET;
        return RC_FAIL;
    }

    vBuildPacket(pstRun->pstVoid, pstSavm->pstVoid, &pstSavm->stCond, &lWrite);
    vBuildPacket(pstRun->pstVoid, pvData, pstCond, &lWrite);
    pstFace->m_lRows  = lWrite - sizeof(TFace);

    if(lWrite != lSendBuffer(pstSavm->m_skSock, (void *)pstRun->pstVoid, lWrite))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }
        
    return RC_SUCC;
}

/*************************************************************************************************
    description：API - insert by asynch
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lAsyInsert(SATvm *pstSavm)
{
    uint    lWrite;
    RunTime *pstRun;
    TFace   *pstFace;

    if(!pstSavm || !pstSavm->pstVoid)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(!pstRun->pstVoid)
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstFace = (TFace *)pstRun->pstVoid;
    pstFace->m_lFind  = pstSavm->lFind;
    pstFace->m_lDLen  = pstSavm->lSize;
    pstFace->m_lErrno = TVM_DONE_SUCC;
    pstFace->m_enum   = OPERAYS_INSERT;
    pstFace->m_table  = pstSavm->tblName;

    pstFace->m_lRows  = pstSavm->lSize;
    lWrite = pstFace->m_lRows + sizeof(TFace);

    checkbuffer(pstSavm, pstRun, 1);
    memcpy(pstRun->pstVoid + sizeof(TFace), pstSavm->pstVoid, pstSavm->lSize);
    if(lWrite != lSendBuffer(pstSavm->m_skSock, (void *)pstRun->pstVoid, lWrite))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }
        
    return RC_SUCC;
}

/*************************************************************************************************
    description：API - insert
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmInsert(SATvm *pstSavm)
{
    RunTime *pstRun;
    TFace   *pstFace;
    uint    lWrite = sizeof(TFace);

    if(!pstSavm || !pstSavm->pstVoid)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(!pstRun->pstVoid)
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstFace = (TFace *)pstRun->pstVoid;
    pstFace->m_lFind  = pstSavm->lFind;
    pstFace->m_lDLen  = pstSavm->lSize;
    pstFace->m_lErrno = TVM_DONE_SUCC;
    pstFace->m_enum   = OPERATE_INSERT;
    pstFace->m_table  = pstSavm->tblName;

    pstFace->m_lRows  = pstSavm->lSize;
    lWrite = pstFace->m_lRows + sizeof(TFace);

    checkbuffer(pstSavm, pstRun, 1);
    memcpy(pstRun->pstVoid + sizeof(TFace), pstSavm->pstVoid, pstSavm->lSize);
    if(lWrite != lSendBuffer(pstSavm->m_skSock, (void *)pstRun->pstVoid, lWrite))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }
        
    if(sizeof(TFace) != lRecvBuffer(pstSavm->m_skSock, (char *)pstRun->pstVoid, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = pstFace->m_lErrno;
    if(0 != pstSavm->m_lErrno)
        return RC_FAIL;

    pstSavm->m_lEffect = pstFace->m_lRows;
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
long    lTvmDelete(SATvm *pstSavm)
{
    RunTime *pstRun;
    TFace   *pstFace;
    uint    lWrite = sizeof(TFace);

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(!pstRun->pstVoid)
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstFace = (TFace *)pstRun->pstVoid;
    pstFace->m_lFind  = pstSavm->lFind;
    pstFace->m_lDLen  = pstSavm->lSize;
    pstFace->m_lErrno = TVM_DONE_SUCC;
    pstFace->m_enum   = OPERATE_DELETE;
    pstFace->m_table  = pstSavm->tblName;

    checkbuffer(pstSavm, pstRun, 1);
    vBuildPacket(pstRun->pstVoid, pstSavm->pstVoid, &pstSavm->stCond, &lWrite);
    vAppendCond(pstRun->pstVoid, &pstSavm->stUpdt, &lWrite);
    pstFace->m_lRows = lWrite - sizeof(TFace);

    if(lWrite != lSendBuffer(pstSavm->m_skSock, (void *)pstRun->pstVoid, lWrite))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }
        
    if(sizeof(TFace) != lRecvBuffer(pstSavm->m_skSock, (char *)pstRun->pstVoid, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = pstFace->m_lErrno;
    if(0 != pstSavm->m_lErrno)
        return RC_FAIL;

    pstSavm->m_lEffect = pstFace->m_lRows;
    pstSavm->m_lEType  = pstFace->m_lDLen;
    return RC_SUCC;
}

/*************************************************************************************************
    description：API - update
    parameters:
        pstSavm                    --stvm handle
        pvData                     --update data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmUpdate(SATvm *pstSavm, void *pvData)
{
    RunTime *pstRun;
    FdCond  *pstCond;
    TFace   *pstFace;
    uint    lWrite = sizeof(TFace);

    if(!pstSavm || !pvData)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(!pstRun->pstVoid)
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstFace = (TFace *)pstRun->pstVoid;
    pstFace->m_lFind  = pstSavm->lFind;
    pstFace->m_lDLen  = pstSavm->lSize;
    pstFace->m_lErrno = TVM_DONE_SUCC;
    pstFace->m_enum   = OPERATE_UPDATE;
    pstFace->m_table  = pstSavm->tblName;

    checkbuffer(pstSavm, pstRun, 2);
    pstCond = &pstSavm->stUpdt;
    if(0 == pstCond->uFldcmp)
    {
        pstSavm->m_lErrno = UPDFD_NOT_SET;
        return RC_FAIL;
    }

    vBuildPacket(pstRun->pstVoid, pstSavm->pstVoid, &pstSavm->stCond, &lWrite);
    vBuildPacket(pstRun->pstVoid, pvData, pstCond, &lWrite);
    pstFace->m_lRows  = lWrite - sizeof(TFace);

    if(lWrite != lSendBuffer(pstSavm->m_skSock, (void *)pstRun->pstVoid, lWrite))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }
        
    if(sizeof(TFace) != lRecvBuffer(pstSavm->m_skSock, (char *)pstRun->pstVoid, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = pstFace->m_lErrno;
    if(0 != pstSavm->m_lErrno)
        return RC_FAIL;

    pstSavm->m_lEffect = pstFace->m_lRows;
    pstSavm->m_lEType  = pstFace->m_lDLen;
    return RC_SUCC;
}

/*************************************************************************************************
    description：API - update
    parameters:
        pstSavm                    --stvm handle
        pvData                     --update data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmReplace(SATvm *pstSavm, void *pvData)
{
    RunTime *pstRun;
    FdCond  *pstCond;
    TFace   *pstFace;
    uint    lWrite = sizeof(TFace);

    if(!pstSavm || !pvData)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(!pstRun->pstVoid)
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstFace = (TFace *)pstRun->pstVoid;
    pstFace->m_lFind  = pstSavm->lFind;
    pstFace->m_lDLen  = pstSavm->lSize;
    pstFace->m_lErrno = TVM_DONE_SUCC;
    pstFace->m_enum   = OPERATE_REPLACE;
    pstFace->m_table  = pstSavm->tblName;

    checkbuffer(pstSavm, pstRun, 2);
    pstCond = &pstSavm->stUpdt;
    if(0 == pstCond->uFldcmp)
    {
        pstSavm->m_lErrno = UPDFD_NOT_SET;
        return RC_FAIL;
    }

    vBuildPacket(pstRun->pstVoid, pstSavm->pstVoid, &pstSavm->stCond, &lWrite);
    vBuildPacket(pstRun->pstVoid, pvData, pstCond, &lWrite);
    pstFace->m_lRows = lWrite - sizeof(TFace);

    if(lWrite != lSendBuffer(pstSavm->m_skSock, (void *)pstRun->pstVoid, lWrite))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }
        
    if(sizeof(TFace) != lRecvBuffer(pstSavm->m_skSock, (char *)pstRun->pstVoid, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = pstFace->m_lErrno;
    if(0 != pstSavm->m_lErrno)
        return RC_FAIL;

    pstSavm->m_lEffect = pstFace->m_lRows;
    pstSavm->m_lEType  = pstFace->m_lDLen;
    return RC_SUCC;
}

/*************************************************************************************************
    description：API - truncate
    parameters:
        pstSavm                    --stvm handle
        t                          --t
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmTruncate(SATvm *pstSavm, TABLE t)
{
    RunTime *pstRun;
    TFace   *pstFace;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(!pstRun->pstVoid)
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstFace = (TFace *)pstRun->pstVoid;
    pstFace->m_table  = t;
    pstFace->m_lRows  = 0;
    pstFace->m_lErrno = TVM_DONE_SUCC;
    pstFace->m_enum   = OPERATE_TRCATE;

    if(sizeof(TFace) != lSendBuffer(pstSavm->m_skSock, (void *)pstFace, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }
        
    if(sizeof(TFace) != lRecvBuffer(pstSavm->m_skSock, (void *)pstFace, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = pstFace->m_lErrno;
    if(0 != pstSavm->m_lErrno)
        return RC_FAIL;

    pstSavm->m_lEffect = pstFace->m_lRows;
    return RC_SUCC;
}

/*************************************************************************************************
    description：API - group
    parameters:
        pstSavm                    --stvm handle
        plOut                      --number
        ppvOut                     --out data list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmGroup(SATvm *pstSavm, size_t *plOut, void **ppvOut)
{
    RunTime *pstRun;
    TFace   *pstFace;
    FdCond  *pstCond;
    uint    lWrite = sizeof(TFace);

    if(!pstSavm || !ppvOut)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(!pstRun->pstVoid)
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstFace = (TFace *)pstRun->pstVoid;
    pstFace->m_lFind  = pstSavm->lFind;
    pstFace->m_lDLen  = pstSavm->lSize;
    pstFace->m_lErrno = TVM_DONE_SUCC;
    pstFace->m_enum   = OPERATE_GROUP;
    pstFace->m_table  = pstSavm->tblName;

    checkbuffer(pstSavm, pstRun, 1);
    pstCond = &pstSavm->stUpdt;
    if(0 == pstCond->uFldcmp)
    {
        pstSavm->m_lErrno = GROUP_SET_ERR;
        return RC_FAIL;
    }

    vBuildPacket(pstRun->pstVoid, pstSavm->pstVoid, &pstSavm->stCond, &lWrite);
    vAppendCond(pstRun->pstVoid, &pstSavm->stUpdt, &lWrite);
    pstFace->m_lRows = lWrite - sizeof(TFace);
    if(lWrite != lSendBuffer(pstSavm->m_skSock, (char *)pstRun->pstVoid, lWrite))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }
        
    if(sizeof(TFace) != lRecvBuffer(pstSavm->m_skSock, (char *)pstRun->pstVoid, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = pstFace->m_lErrno;
    if(0 != pstSavm->m_lErrno)
        return RC_FAIL;

    pstRun->m_lRowSize = pstSavm->lSize * pstFace->m_lRows;
    if(NULL == (*ppvOut = (void *)malloc(pstRun->m_lRowSize)))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    if(pstRun->m_lRowSize != lRecvBuffer(pstSavm->m_skSock, (char *)*ppvOut, pstRun->m_lRowSize))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    *plOut = pstFace->m_lRows;
    pstSavm->m_lEffect = pstFace->m_lRows;
    pstSavm->m_lEType  = pstFace->m_lDLen;
    return RC_SUCC;
}

/*************************************************************************************************
    description：API - count
    parameters:
        pstSavm                    --stvm handle
        plcount                    --count
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmCount(SATvm *pstSavm, size_t *plCount)
{
    RunTime *pstRun;
    TFace   *pstFace;
    uint    lWrite = sizeof(TFace);

    if(!pstSavm || !plCount)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(!pstRun->pstVoid)
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstFace = (TFace *)pstRun->pstVoid;
    pstFace->m_lFind  = pstSavm->lFind;
    pstFace->m_lDLen  = pstSavm->lSize;
    pstFace->m_lErrno = TVM_DONE_SUCC;
    pstFace->m_enum   = OPERATE_COUNT;
    pstFace->m_table  = pstSavm->tblName;

    checkbuffer(pstSavm, pstRun, 1);
    vBuildPacket(pstRun->pstVoid, pstSavm->pstVoid, &pstSavm->stCond, &lWrite);
    vAppendCond(pstRun->pstVoid, &pstSavm->stUpdt, &lWrite);
    pstFace->m_lRows  = lWrite - sizeof(TFace);

    if(lWrite != lSendBuffer(pstSavm->m_skSock, (char *)pstRun->pstVoid, lWrite))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    if(sizeof(TFace) != lRecvBuffer(pstSavm->m_skSock, (char *)pstRun->pstVoid, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = pstFace->m_lErrno;
    if(0 != pstSavm->m_lErrno)
        return RC_FAIL;

    *plCount = pstFace->m_lRows;
    pstSavm->m_lEType = pstFace->m_lDLen;
    return RC_SUCC;
}

/*************************************************************************************************
    description：API - extreme
    parameters:
        pstSavm                    --stvm handle
        pvOut                      --out data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmExtreme(SATvm *pstSavm, void *pvOut)
{
    RunTime *pstRun;
    TFace   *pstFace;
    FdCond  *pstCond;
    uint    lWrite = sizeof(TFace);

    if(!pstSavm || !pvOut)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(!pstRun->pstVoid)
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstFace = (TFace *)pstRun->pstVoid;
    pstFace->m_lFind  = pstSavm->lFind;
    pstFace->m_lDLen  = pstSavm->lSize;
    pstFace->m_lErrno = TVM_DONE_SUCC;
    pstFace->m_enum   = OPERATE_EXTREM;
    pstFace->m_table  = pstSavm->tblName;

    checkbuffer(pstSavm, pstRun, 1);
    pstCond = &pstSavm->stUpdt;
    if(0 == pstCond->uFldcmp)
    {
        pstSavm->m_lErrno = GROUP_SET_ERR;
        return RC_FAIL;
    }

    vBuildPacket(pstRun->pstVoid, pstSavm->pstVoid, &pstSavm->stCond, &lWrite);
    vAppendCond(pstRun->pstVoid, pstCond, &lWrite);
    pstFace->m_lRows  = lWrite - sizeof(TFace);

    if(lWrite != lSendBuffer(pstSavm->m_skSock, (char *)pstRun->pstVoid, lWrite))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    if(sizeof(TFace) != lRecvBuffer(pstSavm->m_skSock, (char *)pstRun->pstVoid, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = pstFace->m_lErrno;
    if(0 != pstSavm->m_lErrno)
        return RC_FAIL;

    pstSavm->m_lEType = pstFace->m_lDLen;
    if(pstSavm->lSize != lRecvBuffer(pstSavm->m_skSock, (char *)pvOut, pstSavm->lSize))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：API - droptable
    parameters:
        pstSavm                    --stvm handle
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmDropTable(SATvm *pstSavm, TABLE t)
{
    RunTime *pstRun;
    TFace   *pstFace;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(!pstRun->pstVoid)
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstFace = (TFace *)pstRun->pstVoid;
    pstFace->m_lRows  = 0;
    pstFace->m_table  = t;
    pstFace->m_lErrno = TVM_DONE_SUCC;
    pstFace->m_enum   = OPERATE_TBDROP;

    if(sizeof(TFace) != lSendBuffer(pstSavm->m_skSock, (void *)pstRun->pstVoid, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }
        
    if(sizeof(TFace) != lRecvBuffer(pstSavm->m_skSock, (char *)pstRun->pstVoid, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = pstFace->m_lErrno;
    if(0 != pstSavm->m_lErrno)
        return RC_FAIL;

    pstSavm->m_lEffect = pstFace->m_lRows;
    return RC_SUCC;
}

/*************************************************************************************************
    description：API - renametable
    parameters:
        pstSavm                    --stvm handle
        to                         --from
        tn                         --to table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmRenameTable(SATvm *pstSavm, TABLE to, TABLE tn)
{
    RunTime *pstRun;
    TFace   *pstFace;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(!pstRun->pstVoid)
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstFace = (TFace *)pstRun->pstVoid;
    pstFace->m_lRows  = 0;
    pstFace->m_table  = to;
    pstFace->m_lDLen  = tn;
    pstFace->m_lErrno = TVM_DONE_SUCC;
    pstFace->m_enum   = OPERATE_RENAME;

    if(sizeof(TFace) != lSendBuffer(pstSavm->m_skSock, (void *)pstRun->pstVoid, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }
        
    if(sizeof(TFace) != lRecvBuffer(pstSavm->m_skSock, (char *)pstRun->pstVoid, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = ((TFace *)pstRun->pstVoid)->m_lErrno;
    if(0 != pstSavm->m_lErrno)
        return RC_FAIL;

    pstSavm->m_lEffect = ((TFace *)pstRun->pstVoid)->m_lRows;
    return RC_SUCC;
}

/*************************************************************************************************
    description：API - click
    parameters:
        pstSavm                    --stvm handle
        plcount                    --count
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmClick(SATvm *pstSavm, ulong *pulHits)
{
    RunTime *pstRun;
    TFace   *pstFace;
    uint    lWrite = sizeof(TFace);

    if(!pstSavm || !pulHits)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(!pstRun->pstVoid)
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstFace = (TFace *)pstRun->pstVoid;
    pstFace->m_lFind  = pstSavm->lFind;
    pstFace->m_lDLen  = pstSavm->lSize;
    pstFace->m_lErrno = TVM_DONE_SUCC;
    pstFace->m_enum   = OPERATE_CLICK;
    pstFace->m_table  = pstSavm->tblName;

    checkbuffer(pstSavm, pstRun, 1);
    vBuildPacket(pstRun->pstVoid, pstSavm->pstVoid, &pstSavm->stCond, &lWrite);
    vAppendCond(pstRun->pstVoid, &pstSavm->stUpdt, &lWrite);
    pstFace->m_lRows  = lWrite - sizeof(TFace);

    if(lWrite != lSendBuffer(pstSavm->m_skSock, (char *)pstRun->pstVoid, lWrite))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    if(sizeof(TFace) != lRecvBuffer(pstSavm->m_skSock, (char *)pstRun->pstVoid, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = pstFace->m_lErrno;
    if(0 != pstSavm->m_lErrno)
        return RC_FAIL;

    pstSavm->m_lEType  = pstFace->m_lDLen;
    if(sizeof(ulong) != lRecvBuffer(pstSavm->m_skSock, (void *)pstRun->pstVoid, sizeof(ulong)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    memcpy((void *)pulHits, pstRun->pstVoid, sizeof(ulong));
    return RC_SUCC;
}

/*************************************************************************************************
    description：API - selectseque
    parameters:
        pstSavm                    --stvm handle
        pszSQName                  --seque name
        pulNumber                  --oout seque
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmSelectSeque(SATvm *pstSavm, char *pszSQName, ulong *pulNumber)
{
    RunTime *pstRun;
    TFace   *pstFace;
    uint    lWrite = sizeof(TFace) + MAX_INDEX_LEN;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(!pstRun->pstVoid)
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstFace = (TFace *)pstRun->pstVoid;
    pstFace->m_table  = SYS_TVM_SEQUE;
    pstFace->m_lDLen  = MAX_INDEX_LEN;
    pstFace->m_lErrno = TVM_DONE_SUCC;
    pstFace->m_enum   = OPERATE_SELSEQ;
    pstFace->m_lRows  = MAX_INDEX_LEN;

    strncpy(pstRun->pstVoid + sizeof(TFace), pszSQName, MAX_INDEX_LEN);
    if(lWrite != lSendBuffer(pstSavm->m_skSock, (char *)pstRun->pstVoid, lWrite))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }
        
    if(sizeof(TFace) != lRecvBuffer(pstSavm->m_skSock, (char *)pstRun->pstVoid, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = pstFace->m_lErrno;
    if(0 != pstSavm->m_lErrno)
        return RC_FAIL;

    if(sizeof(ulong) != lRecvBuffer(pstSavm->m_skSock, (void *)pstRun->pstVoid, sizeof(ulong)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    memcpy((void *)pulNumber, pstRun->pstVoid, sizeof(ulong));
    return RC_SUCC;
}

/*************************************************************************************************
    description：API - setsequence
    parameters:
        pstSavm                    --stvm handle
        pszSQName                  --seque name
        uStart                     --out seque
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmSetSequence(SATvm *pstSavm, char *pszSQName, ulong uStart)
{
    RunTime *pstRun;
    TFace   *pstFace;
    uint    lWrite = sizeof(TFace) + MAX_INDEX_LEN;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(!pstRun->pstVoid)
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstFace = (TFace *)pstRun->pstVoid;
    pstFace->m_table  = SYS_TVM_SEQUE;
    pstFace->m_lDLen  = MAX_INDEX_LEN;
    pstFace->m_lErrno = TVM_DONE_SUCC;
    pstFace->m_enum   = OPERATE_SETSEQ;

    strncpy(pstRun->pstVoid + sizeof(TFace), pszSQName, MAX_INDEX_LEN);
    *((ulong *)(pstRun->pstVoid + lWrite)) = uStart;
    lWrite += sizeof(ulong);
    pstFace->m_lRows  = lWrite - sizeof(TFace);
    if(lWrite != lSendBuffer(pstSavm->m_skSock, (void *)pstRun->pstVoid, lWrite))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    if(sizeof(TFace) != lRecvBuffer(pstSavm->m_skSock, (char *)pstRun->pstVoid, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = ((TFace *)pstRun->pstVoid)->m_lErrno;
    if(0 != pstSavm->m_lErrno)
        return RC_FAIL;

    return RC_SUCC;
}

/*************************************************************************************************
    description：API - rebuildindex
    parameters:
        pstSavm                    --stvm handle
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmRebuildIndex(SATvm *pstSavm, TABLE t)
{
    RunTime *pstRun;
    TFace   *pstFace;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(!pstRun->pstVoid)
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstFace = (TFace *)pstRun->pstVoid;
    pstFace->m_lRows  = 0;
    pstFace->m_table  = t;
    pstFace->m_lErrno = TVM_DONE_SUCC;
    pstFace->m_enum   = OPERATE_RBDIDX;

    if(sizeof(TFace) != lSendBuffer(pstSavm->m_skSock, (void *)pstRun->pstVoid, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }
        
    if(sizeof(TFace) != lRecvBuffer(pstSavm->m_skSock, (char *)pstRun->pstVoid, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = ((TFace *)pstRun->pstVoid)->m_lErrno;
    if(0 != pstSavm->m_lErrno)
        return RC_FAIL;

    return RC_SUCC;
}

/*************************************************************************************************
    description：API - Relock
    parameters:
        pstSavm                    --stvm handle
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmResetLock(SATvm *pstSavm, TABLE t)
{
    RunTime *pstRun;
    TFace   *pstFace;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(!pstRun->pstVoid)
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstFace = (TFace *)pstRun->pstVoid;
    pstFace->m_lRows  = 0;
    pstFace->m_table  = t;
    pstFace->m_lErrno = TVM_DONE_SUCC;
    pstFace->m_enum   = OPERATE_RETLOK;

    if(sizeof(TFace) != lSendBuffer(pstSavm->m_skSock, (void *)pstFace, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }
        
    if(sizeof(TFace) != lRecvBuffer(pstSavm->m_skSock, (char *)pstFace, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    pstSavm->m_lErrno = ((TFace *)pstRun->pstVoid)->m_lErrno;
    if(0 != pstSavm->m_lErrno)
        return RC_FAIL;

    return RC_SUCC;
}

/*************************************************************************************************
    description：API - work
    parameters:
        pstSavm                    --stvm handle
        eWork                      --ework
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lTvmWork(SATvm *pstSavm, Benum eWork)
{
    RunTime *pstRun;
    TFace   *pstFace;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    pstRun = (RunTime *)pGetRunTime(pstSavm, 0);
    if(!pstRun->pstVoid)
    {
        pstSavm->m_lErrno = DOM_NOT_INITL;
        return RC_FAIL;
    }

    pstFace = (TFace *)pstRun->pstVoid;
    pstFace->m_lRows  = 0;
    pstFace->m_enum   = eWork;
    pstFace->m_table  = SYS_TVM_INDEX;
    pstFace->m_lErrno = TVM_DONE_SUCC;
//#ifdef   __linux__
    pstFace->m_lDLen = syscall(SYS_gettid);
//#endif

    if(sizeof(TFace) != lSendBuffer(pstSavm->m_skSock, (void *)pstFace, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }
        
    if(sizeof(TFace) != lRecvBuffer(pstSavm->m_skSock, (char *)pstFace, sizeof(TFace)))
    {
        pstSavm->m_lErrno = SOCK_COM_EXCP;
        return RC_FAIL;
    }

    pstSavm->m_lErrno  = pstFace->m_lErrno;
    pstSavm->m_lEffect = pstFace->m_lRows;
    if(0 != pstSavm->m_lErrno)
        return RC_FAIL;

    return RC_SUCC;
}

/*************************************************************************************************
    description：API - begin work
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmBeginWork(SATvm *pstSavm)
{
    return _lTvmWork(pstSavm, OPERATE_BEGWORK);
}

/*************************************************************************************************
    description：API - rollback work
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmRollbackWork(SATvm *pstSavm)
{
    return _lTvmWork(pstSavm, OPERATE_ROLWORK);
}

/*************************************************************************************************
    description：API - commit work
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmCommitWork(SATvm *pstSavm)
{
    return _lTvmWork(pstSavm, OPERATE_CMTWORK);
}

/*************************************************************************************************
    description：API - end work
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmEndWork(SATvm *pstSavm)
{
    return _lTvmWork(pstSavm, OPERATE_ENDWORK);
}

/*************************************************************************************************
    description：API - exit
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmExit(SATvm *pstSavm)
{
    return _lTvmWork(pstSavm, OPERATE_EXEEXIT);
}

/*************************************************************************************************
    description：Gets the remote table index, user STVM-SQL
    parameters:
        pstSavm                    --stvm handle
        pszTable                   --table name
        pszPart                    --part 
        pstIndex                   --table index
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmGetTblIndex(SATvm *pstSavm, char *pszTable, char *pszPart, TIndex *pstIndex)
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
    if(RC_SUCC != lTvmSelect(pstSavm, (void *)pstIndex))
    {
        if(NO_DATA_FOUND == pstSavm->m_lErrno)
            pstSavm->m_lErrno = TBL_NOT_FOUND;
        return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：Gets the remote table field, user STVM-SQL
    parameters:
        pstSavm                    --stvm handle
        t                          --t
        plOut                      --number 
        ppstField                  --table field
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lTvmGetTblField(SATvm *pstSavm, TABLE t, size_t *plOut, TField **ppstField)
{   
    TField  stField; 
    
    conditinit(pstSavm, stField, SYS_TVM_FIELD)
    conditnum(pstSavm, stField, m_table, t);

    return lTvmQuery(pstSavm, plOut, (void **)ppstField);
}

/*************************************************************************************************
 * code end
 *************************************************************************************************/
