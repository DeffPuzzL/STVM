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

#ifndef _HHH_TVM_BASE_STR_HHH__
#define _HHH_TVM_BASE_STR_HHH__
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>
#include <stdarg.h>
#include <getopt.h>
#include <unistd.h>
#include <math.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/timeb.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/inotify.h>
#include <strings.h>
#include <iconv.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <libgen.h>

typedef short       int     sint;
typedef unsigned    int     uint;
typedef unsigned    long    ulong;
typedef long        long    llong;
typedef unsigned    char    ushar;
typedef unsigned    short   ushort;
typedef unsigned    char    Byte;
typedef long        int     Benum;
typedef unsigned    int     Uenum;
typedef int                 BSock;

#ifndef BOOL
typedef unsigned    int    BOOL;
#endif

#ifndef TRUE
#define TRUE                1
#endif

#ifndef FALSE
#define FALSE                0
#endif

#ifndef __cplusplus
#ifndef bool
typedef unsigned    char  bool;
#endif

#ifndef true
#define true                1
#endif

#ifndef false
#define false                0
#endif

#endif    // __cplusplus


#define RC_NOTFOUND                         24
#define RC_MATCH                            2
#define    RC_MISMA                            1
#define RC_SUCC                             0
#define RC_FAIL                             -1
#define RC_CONTU                            -5
#define RC_CLOSE                            -9

#define LOG_DEFU_SIZE                       20480000
#define LOG_DEFU_NAME                       "tvm.log"

typedef pthread_mutex_t  pmutex;
/*************************************************************************************************
    function
 *************************************************************************************************/
typedef struct  __COMLIST
{
    long    m_lSize;
    void    *m_psvData;
    struct  __COMLIST   *pstNext;
    struct  __COMLIST   *pstLast;
}CMList;

typedef struct  __ROWGROUP
{
    long    lIdx;
    long    lLen;
    size_t  lCount;
    pmutex  lock;
    void    *psvData;
    struct  __ROWGROUP   *pstSSet;
    struct  __ROWGROUP   *pstFset;
    struct  __ROWGROUP   *pstNext;
    struct  __ROWGROUP   *pstLast;
}Rowgrp;

#ifdef __cplusplus
extern "C" {
#endif

#define MAX(a, b)                           ((a) > (b) ? (a) : (b))
#define MIN(a, b)                           ((a) > (b) ? (b) : (a))
#define bool(x)                             (x == 0 ? false : true)
#define BOOL(x)                             (x == 0 ? false : true)

extern CMList*     pGetCMTail(CMList *pstRoot);
extern CMList*     pSearchNode(CMList *pstRoot, void *psvData, long lSize);
extern CMList*     pInsertList(CMList *pstRoot, void *pszData, long lSize);
extern CMList*     pDeleteNode(CMList *pstRoot, void *psvData, long lSize);
extern void        vDestroyList(CMList *pstRoot);

extern char*       supper(char *s);
extern char*       slower(char *s);
extern char*       strimcrlf(char *p);
extern char*       sltrim(char *p);
extern char*       srtrim(char *p);
extern char*       strimall(char *p);
extern char*       strimfield(char *s);
extern char*       strimcpy(char *d, char *s, int l);
extern char*       strimabout(char *s, char *o, char *d);
extern long        lfieldnum(char *p, char *k);
extern char*       sfieldvalue(char *p, char *k, int id);
extern long        lgetstrnum(char *p, char *s);
extern char*       sgetvalue(char *p, char *s, int id);
extern char*       sfieldreplace(char *p, char o, char d);

#ifdef __cplusplus
}
#endif

#endif    //    _HHH_TVM_BASE_STR_HHH__
