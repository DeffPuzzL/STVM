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
    global
 *************************************************************************************************/
TBoot   g_stBoot = {0};

/*************************************************************************************************
    description：get the config of boot
    parameters:
    return:
        void*
 *************************************************************************************************/
void*   pGetBoot()
{
    return &g_stBoot;
}

/*************************************************************************************************
    description：get log file
    parameters:
    return:
        void*
 *************************************************************************************************/
char*   pGetLog()
{
    return g_stBoot.m_szLog;
}

/*************************************************************************************************
    description：Get the current node
    parameters:
    return:
        void*
 *************************************************************************************************/
char*   sGetNode()
{
    return g_stBoot.m_szNode;
}

/*************************************************************************************************
    description：Set the current node
    parameters:
    return:
 *************************************************************************************************/
void   vSetNode(char *s)
{
    if(!s)    return ;
    strncpy(g_stBoot.m_szNode, s, sizeof(g_stBoot.m_szNode));
}

/*************************************************************************************************
    description：Set STVM boot type 
    parameters:
    return:
 *************************************************************************************************/
void   vSetBootType(long lType)
{
    g_stBoot.m_lBootType = lType;
}

/*************************************************************************************************
    description：Get current type of boot
    parameters:
    return:
        lBootType
 *************************************************************************************************/
long    lGetBootType()
{
    return g_stBoot.m_lBootType;
}

/*************************************************************************************************
    函数说明：获取配置日志名称
    参数说明：
    返回值：
        void                    --日志名
 *************************************************************************************************/
char*    sGetLog()
{
    return g_stBoot.m_szLog;
}

/*************************************************************************************************
    description：create default config 
    parameters:
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lDefaultBoot()
{
    FILE    *fp = NULL;
    char    szPath[512];
    TBoot   *pstBoot = (TBoot *)pGetBoot();

    memset(szPath, 0, sizeof(szPath));
    memset(pstBoot, 0, sizeof(TBoot));
    snprintf(szPath, sizeof(szPath), "%s", getenv("TVMCFG"));

    pstBoot->m_lMaxTable = 255;
    pstBoot->m_lMaxField = 3000;
    pstBoot->m_lMaxDomain = 1024;
    pstBoot->m_lMaxSeque = 1024;
    pstBoot->m_lBootExec = get_nprocs();
    pstBoot->m_lBootPort = 2000;
    pstBoot->m_lBootType = TVM_BOOT_SIMPLE;
    strcpy(pstBoot->m_szNode, "STVM");
    strcpy(pstBoot->m_szLog, "stvm.log");

    if(NULL == (fp = fopen(szPath, "wb")))
    {
        fprintf(stderr, "create default param failed, %s\n", strerror(errno));
        return RC_FAIL;
    }

    fwrite(TVM_RUNCFG_TAG, 4, 1, fp);
    fwrite(pstBoot, sizeof(TBoot), 1, fp);
    fclose(fp);

    return RC_SUCC;
}

/*************************************************************************************************
    description：from the config to boot system
    parameters:
        pstSavm                    --stvm handle
        pstBoot                    --boot paramer
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lGetBootConfig(SATvm *pstSavm, TBoot *pstBoot)
{
    FILE    *fp = NULL;
    char    szPath[512], szVersion[10];

    if(!pstSavm || !pstBoot)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    memset(szPath, 0, sizeof(szPath));
    memset(szVersion, 0, sizeof(szVersion));
    snprintf(szPath, sizeof(szPath), "%s", getenv("TVMCFG"));
    if(NULL == (fp = fopen(szPath, "rb")))
    {
        pstSavm->m_lErrno = FILE_NOTFOUND;
        return RC_FAIL;
    }

    fread(szVersion, 4, 1, fp);
    if(memcmp(szVersion, TVM_RUNCFG_TAG, 4))
    {
        fclose(fp);
        pstSavm->m_lErrno = BOOT_VER_ICMP;
        return RC_FAIL;
    }

    fread(pstBoot, sizeof(TBoot), 1, fp);
    fclose(fp);

    return RC_SUCC;
}

/*************************************************************************************************
    description： Startup initialization
    parameters:
    return:
       void *
 *************************************************************************************************/
TBoot*    pBootInitial()
{
    static   int    i = 0;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

    if(1 == i ++)    return &g_stBoot;

    memset(&g_stBoot, 0, sizeof(TBoot));

    pstSavm->m_lErrno = TVM_DONE_SUCC;
    lGetBootConfig(pstSavm, &g_stBoot);

    return &g_stBoot;
}

/*************************************************************************************************
    description：from the config to initial table index
    parameters:
        pstSavm                    --stvm handle
        plOut                      --number
        ppstIndex                  --out of index group
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lGetLocalIndex(SATvm *pstSavm, long *plOut, TIndex **ppstIndex)
{
    FILE    *fp = NULL;
    char    szPath[512], szVersion[10];

    if(!pstSavm || !ppstIndex || !plOut)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    memset(szPath, 0, sizeof(szPath));
    memset(szVersion, 0, sizeof(szVersion));
    snprintf(szPath, sizeof(szPath), "%s", getenv("TVMCFG"));
    if(NULL == (fp = fopen(szPath, "rb")))
    {
        pstSavm->m_lErrno = FILE_NOTFOUND;
        return RC_FAIL;
    }

    fread(szVersion, 4, 1, fp);
    if(memcmp(szVersion, TVM_RUNCFG_TAG, 4))
    {
        fclose(fp);
        pstSavm->m_lErrno = BOOT_VER_ICMP;
        return RC_FAIL;
    }

    fseek(fp, sizeof(TBoot), SEEK_CUR);
    fread((void *)plOut, sizeof(long), 1, fp);
    if(*plOut <= 0)
    {
        fclose(fp);
        return RC_SUCC;
    }

    if(NULL == (*ppstIndex = (void *)calloc(*plOut, sizeof(TIndex))))
    {
        fclose(fp);
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    fread(*ppstIndex, (*plOut) * sizeof(TIndex), 1, fp);
    fclose(fp);

    return RC_SUCC;
}

/*************************************************************************************************
    description：from the config to initial domain
    parameters:
        pstSavm                    --stvm handle
        plOut                      --number
        ppstIndex                  --out of index group
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lGetDomainIndex(SATvm *pstSavm, long *plOut, TIndex **ppstIndex)
{
    FILE    *fp = NULL;
    char    szPath[512], szVersion[10];

    if(!pstSavm || !ppstIndex || !plOut)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    memset(szPath, 0, sizeof(szPath));
    memset(szVersion, 0, sizeof(szVersion));
    snprintf(szPath, sizeof(szPath), "%s", getenv("TVMCFG"));
    if(NULL == (fp = fopen(szPath, "rb")))
    {
        pstSavm->m_lErrno = FILE_NOTFOUND;
        return RC_FAIL;
    }

    fread(szVersion, 4, 1, fp);
    if(memcmp(szVersion, TVM_RUNCFG_TAG, 4))
    {
        fclose(fp);
        pstSavm->m_lErrno = BOOT_VER_ICMP;
        return RC_FAIL;
    }

    fseek(fp, sizeof(TBoot), SEEK_CUR);
    fread((void *)plOut, sizeof(long), 1, fp);
    if(*plOut <= 0)
    {
        fclose(fp);
        return RC_SUCC;
    }

    fseek(fp, (*plOut) * sizeof(TIndex), SEEK_CUR);
    fread((void *)plOut, sizeof(long), 1, fp);
    if(NULL == (*ppstIndex = (void *)calloc(*plOut, sizeof(TIndex))))
    {
        fclose(fp);
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    fread(*ppstIndex, (*plOut) * sizeof(TIndex), 1, fp);
    fclose(fp);

    return RC_SUCC;
}

/*************************************************************************************************
    description：from the config to initial table of domain
    parameters:
        pstSavm                    --stvm handle
        plOut                      --number
        ppstIndex                  --out of index group
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lGetDomainTable(SATvm *pstSavm, long *plOut, TDomain **ppstDomain)
{
    FILE    *fp = NULL;
    char    szPath[512], szVersion[10];

    if(!pstSavm || !ppstDomain || !plOut)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    memset(szPath, 0, sizeof(szPath));
    memset(szVersion, 0, sizeof(szVersion));
    snprintf(szPath, sizeof(szPath), "%s", getenv("TVMCFG"));
    if(NULL == (fp = fopen(szPath, "rb")))
    {
        pstSavm->m_lErrno = FILE_NOTFOUND;
        return RC_FAIL;
    }

    fread(szVersion, 4, 1, fp);
    if(memcmp(szVersion, TVM_RUNCFG_TAG, 4))
    {
        fclose(fp);
        pstSavm->m_lErrno = BOOT_VER_ICMP;
        return RC_FAIL;
    }

    fseek(fp, sizeof(TBoot), SEEK_CUR);
    fread((void *)plOut, sizeof(long), 1, fp);
    fseek(fp, (*plOut) * sizeof(TIndex), SEEK_CUR);
    fread((void *)plOut, sizeof(long), 1, fp);
    fseek(fp, (*plOut) * sizeof(TIndex), SEEK_CUR);
    fread((void *)plOut, sizeof(long), 1, fp);
    if(*plOut <= 0)
    {
        fclose(fp);
        return RC_SUCC;
    }

    if(NULL == (*ppstDomain = (void *)calloc(*plOut, sizeof(TDomain))))
    {
        fclose(fp);
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    fread(*ppstDomain, (*plOut) * sizeof(TDomain), 1, fp);
    fclose(fp);

    return RC_SUCC;
}

/*************************************************************************************************
    description：parse config field
    parameters:
        pszBuffer                  --file content
        pszTarg                    --target
        nTarg                      --target length
        pszValue                   --The label value
        nValue                     --value max length
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lPraseField(char *pszBuffer, char *pszTarg, long nTarg, char *pszValue, long nValue)
{
    char    szAttr[512];

    memset(szAttr, 0, sizeof(szAttr));
    strncpy(szAttr, pszBuffer, sizeof(szAttr));
    if(!strstr(szAttr, "="))
    {
        fprintf(stdout, "%s\n*may be lost '='\n", szAttr);
        return RC_FAIL;
    }

    strncpy(pszTarg, sgetvalue(szAttr, "=", 1), nTarg);
    strncpy(pszValue, sgetvalue(szAttr, "=", 2), nValue);
    srtrim(pszTarg);
    sltrim(pszValue);
    strimabout(pszValue, "\"", "\"");
    if(!strlen(pszValue))
    {
        fprintf(stdout, "%s\n*config error, The initial value is not set\n", szAttr);
        return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：from the config to initial table index
    parameters:
        pstSavm                    --stvm handle
        ppstRoot                   --content list
        pszFile                    --config file
        pszTarge                   --target
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lParseFile(SATvm *pstSavm, CMList **ppstRoot, char *pszFile, const char *pszTarget)
{
    FILE    *fp = NULL;
    char    szLine[4098];
    bool    bFlag = false;
    CMList  *pstList = NULL;

    if(NULL == (fp = fopen(pszFile, "rb")))
    {
        pstSavm->m_lErrno = FILE_NOTFOUND;
        return RC_FAIL;
    }

    memset(szLine, 0, sizeof(szLine));
    while(fgets(szLine, sizeof(szLine), fp))
    {
        strimcrlf(szLine);
        sltrim(szLine);
        srtrim(szLine);
        if(!strlen(szLine))
            continue;

        if('#' == szLine[0] || !memcmp("//", szLine, 2) || !memcmp("/*", szLine, 2) ||
            !memcmp("＃", szLine, 2) || !memcmp("--", szLine, 2))
            continue;

        if(!strcmp(pszTarget, szLine) && !bFlag)
        {
            bFlag = true;
            memset(szLine, 0, sizeof(szLine));
            continue;
        }
        else if(szLine[0] == '*' && bFlag)
            break;

        if(!bFlag)
        {
            memset(szLine, 0, sizeof(szLine));
            continue;
        }

        if(NULL == (pstList = pInsertList(pstList, (void *)szLine, sizeof(szLine))))
        {
            fclose(fp);
            vDestroyList(pstList);
            return RC_FAIL;
        }
    }

    fclose(fp);
    *ppstRoot = pstList;

    return RC_SUCC;
}

/*************************************************************************************************
    description：parse STVM boot parameter
    parameters:
        pstSavm                    --stvm handle
        pszFile                    --config file
        pstBoot                    --target
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lParseBoot(SATvm *pstSavm, char *pszFile, TBoot *pstBoot)
{
    char    szTarg[128], szValue[64];
    CMList  *pstNode = NULL, *pstRoot = NULL;

    if(RC_SUCC != _lParseFile(pstSavm, &pstRoot, pszFile, "*GLOBLE"))
    {
        fprintf(stderr, "parse file, err:(%d)(%s)\n", pstSavm->m_lErrno, sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    for(pstNode = pstRoot; pstNode; pstNode = pstNode->pstNext)
    {
        memset(szTarg, 0, sizeof(szTarg));
        memset(szValue, 0, sizeof(szValue));
        if(RC_SUCC != lPraseField((char *)pstNode->m_psvData, szTarg, sizeof(szTarg), szValue, 
            sizeof(szValue)))
            goto PBOOT_ERROR;

        if(!strcasecmp(szTarg, "MACHINE"))
            strncpy(pstBoot->m_szNode, szValue, sizeof(pstBoot->m_szNode));
        else if(!strcasecmp(szTarg, "LOGNAME"))
            strncpy(pstBoot->m_szLog, szValue, sizeof(pstBoot->m_szLog));
        else if(!strcasecmp(szTarg, "DEPLOY"))
        {
            if(!strcasecmp(szValue, "cluster"))
                pstBoot->m_lBootType = TVM_BOOT_CLUSTER;
            else if(!strcasecmp(szValue, "local"))
                pstBoot->m_lBootType = TVM_BOOT_LOCAL;
            else    // local
                pstBoot->m_lBootType = TVM_BOOT_SIMPLE;
        }
        else if(!strcasecmp(szTarg, "MAXTABLE"))
        {
            pstBoot->m_lMaxTable = atol(szValue);
            if(pstBoot->m_lMaxTable <= 5)
            {
                fprintf(stdout, "%s\n*Set STVM maximum support table number error\n", 
                    (char *)pstNode->m_psvData);
            }
            else if(pstBoot->m_lMaxTable > 255)
            {
                fprintf(stdout, "%s\n*STVM maximum support table 255\n", 
                    (char *)pstNode->m_psvData);
                goto PBOOT_ERROR;
            }
        }
        else if(!strcasecmp(szTarg, "MAXFILED"))
        {
            pstBoot->m_lMaxField = atol(szValue);
            if(pstBoot->m_lMaxField <= 100)
            {
                fprintf(stdout, "%s\n*Set the number of STVM field details error\n", 
                    (char *)pstNode->m_psvData);
                goto PBOOT_ERROR;
            }
        }
        else if(!strcasecmp(szTarg, "MAXDOMAIN"))
        {
            pstBoot->m_lMaxDomain = atol(szValue);
            if(pstBoot->m_lMaxDomain <= 0)
            {
                fprintf(stdout, "%s\n*Error in setting maximum number of domain\n", 
                   (char *)pstNode->m_psvData);
                goto PBOOT_ERROR;
            }
        }
        else if(!strcasecmp(szTarg, "MAXSEQUE"))
        {
            pstBoot->m_lMaxSeque = atol(szValue);
            if(pstBoot->m_lMaxSeque <= 0)
            {
                fprintf(stdout, "%s\n*Error in setting maximum number of sequences\n", 
                    (char *)pstNode->m_psvData);
                goto PBOOT_ERROR;
            }
        }
        else if(!strcasecmp(szTarg, "SERVER_EXEC"))
        {
            pstBoot->m_lBootExec = atol(szValue);
            if(pstBoot->m_lBootExec <= 0)
            {
                fprintf(stdout, "%s\n*LIS.tvm: startup number set error", 
                    (char *)pstNode->m_psvData);
                goto PBOOT_ERROR;
            }
        }
        else if(!strcasecmp(szTarg, "SERVER_PORT"))
        {
            pstBoot->m_lBootPort = atol(szValue);
            if(pstBoot->m_lBootPort <= 0)
            {
                fprintf(stdout, "%s\n*LIS.tvm: Error starting port setting\n", 
                    (char *)pstNode->m_psvData);
                goto PBOOT_ERROR;
            }
        }
        else
        {
            fprintf(stdout, "%s\n*Invalid parameter\n", (char *)pstNode->m_psvData);
            goto PBOOT_ERROR;
        }
    }

    if(!strlen(pstBoot->m_szNode))
    {
        fprintf(stdout, "MACHINE\n*The local node is not set\n");
        goto PBOOT_ERROR;
    }

    if(pstBoot->m_lMaxTable <= 0)
        pstBoot->m_lMaxTable = TVM_MAX_TABLE;
    if(pstBoot->m_lMaxField <= 0)
        pstBoot->m_lMaxField = 3000;
    if(pstBoot->m_lMaxDomain <= 0)
        pstBoot->m_lMaxDomain = 500;
    if(pstBoot->m_lMaxSeque <= 0)
       pstBoot->m_lMaxSeque = 500;
    if(pstBoot->m_lBootExec <= 0)
        pstBoot->m_lBootExec = get_nprocs() > 0 ? get_nprocs() : 1;
    if(pstBoot->m_lBootPort <= 0)
        pstBoot->m_lBootPort = TVM_PORT_LISTEN;

    vDestroyList(pstRoot);

    return RC_SUCC;

PBOOT_ERROR:
    vDestroyList(pstRoot);
    return RC_FAIL;
}

/*************************************************************************************************
    description：get config domain resource
    parameters:
        lDomain                    --number of domain
        pstDomain                  --domain group
        pszDomain                  --domain
    return:
        void*                      --domain
 *************************************************************************************************/
TDomain*    pGetResourse(long lDomain, TDomain *pstDomain, char *pszDomain)
{
    long    i = 0;

    for(i = 0; i < lDomain; i ++)
    {
        if(!strcmp(pstDomain[i].m_szOwner, pszDomain))
            return &pstDomain[i];
    }

    return NULL;
}

/*************************************************************************************************
    description：parse local table config
    parameters:
        pstSavm                    --stvm handle
        pszFile                    --config
        plOut                      --number
        ppstIndex                  --out of index group
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lParseIndex(SATvm *pstSavm, char *pszFile, long *plOut, TIndex **ppstIndex)
{
    long    i, j, n;
    TIndex  *pstIndex = NULL;
    CMList  *pstNode = NULL, *pstRoot = NULL;
    char    szTarg[128], szValue[64], szAttr[1024];

    memset(szTarg, 0, sizeof(szTarg));
    memset(szAttr, 0, sizeof(szAttr));
    memset(szValue, 0, sizeof(szValue));
    if(RC_SUCC != _lParseFile(pstSavm, &pstRoot, pszFile, "*LOCAL_RESOURCE"))
    {
        fprintf(stderr, "parse file, err:(%d)(%s)\n", pstSavm->m_lErrno, sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    for(pstNode = pstRoot, j = 0; pstNode; pstNode = pstNode->pstNext)
    {
        sfieldreplace((char *)pstNode->m_psvData, '\t', ' ');
        if(!strlen((char *)pstNode->m_psvData)) continue;

        if(NULL == (pstIndex = (TIndex *)realloc(pstIndex, (++ j) * sizeof(TIndex))))
        {
            pstSavm->m_lErrno = MALLC_MEM_ERR;
            return RC_FAIL;
        }

        memset(&pstIndex[j - 1], 0, sizeof(TIndex));
        for(i = 0, n = lfieldnum((char *)pstNode->m_psvData, " "); i < n; i ++)
        {
            memset(szTarg, 0, sizeof(szTarg));
            memset(szValue, 0, sizeof(szValue));
            if(RC_SUCC != lPraseField(sfieldvalue((char *)pstNode->m_psvData, " ",  i + 1),
                szTarg, sizeof(szTarg), szValue, sizeof(szValue)))
                goto PINDEX_ERROR;

            if(!strcasecmp(szTarg, "TABLE"))
                pstIndex[j - 1].m_table = atol(szValue);
            else if(!strcasecmp(szTarg, "PERMIT"))
                pstIndex[j - 1].m_lPers = atol(szValue);
            else
            {
                fprintf(stdout, "%s\n*Invalid parameter\n", (char *)pstNode->m_psvData);
                goto PINDEX_ERROR;
            }
        }

        if(pstIndex[j - 1].m_table <= 0)
        {
            fprintf(stdout, "%s\n*Table setting error\n", (char *)pstNode->m_psvData);
            goto PINDEX_ERROR;
        }

        if(pstIndex[j - 1].m_lPers <= 0)
            pstIndex[j - 1].m_lPers = OPERATE_DEFAULT;
    }
    *plOut = j;
    *ppstIndex = pstIndex;

    vDestroyList(pstRoot);
    return RC_SUCC;

PINDEX_ERROR:
    TFree(*ppstIndex);
    vDestroyList(pstRoot);
    return RC_FAIL;
}

/*************************************************************************************************
    description：parse domain
    parameters:
        pstSavm                    --stvm handle
        pszFile                    --config
        plCount                    --number
        ppstDom                    --out of domain group
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lParseResouce(SATvm *pstSavm, char *pszFile, long *plCout, TDomain **ppstDom)
{
    TDomain *pv;
    long    i, n;
    char    szTarg[128], szValue[64];
    CMList  *pstNode = NULL, *pstRoot = NULL;

    if(RC_SUCC != _lParseFile(pstSavm, &pstRoot, pszFile, "*REMOTE_DOMAIN"))
    {
        fprintf(stderr, "parse file, err:(%d)(%s)\n", pstSavm->m_lErrno, sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    for(pstNode = pstRoot, *plCout = 0; pstNode; pstNode = pstNode->pstNext)
    {
        sfieldreplace((char *)pstNode->m_psvData, '\t', ' ');
        if(!strlen((char *)pstNode->m_psvData)) continue;

        if(NULL == (*ppstDom = (TDomain *)realloc(*ppstDom, (++ (*plCout)) * sizeof(TDomain))))
        {
            pstSavm->m_lErrno = MALLC_MEM_ERR;
            return RC_FAIL;
        }

        pv = &(*ppstDom)[(*plCout) - 1];
        memset(pv, 0, sizeof(TDomain));
        for(i = 0, n = lfieldnum((char *)pstNode->m_psvData, " "); i < n; i ++)
        {
            memset(szTarg, 0, sizeof(szTarg));
            memset(szValue, 0, sizeof(szValue));
            if(RC_SUCC != lPraseField(sfieldvalue((char *)pstNode->m_psvData, " ",  i + 1), 
                szTarg, sizeof(szTarg), szValue, sizeof(szValue)))
                goto PDOMAIN_ERROR;

            if(!strcasecmp(szTarg, "DOMAINID"))
                strncpy(pv->m_szOwner, szValue, sizeof(pv->m_szOwner));
            else if(!strcasecmp(szTarg, "GROUP"))
                pv->m_lGroup = atol(szValue);
            else if(!strcasecmp(szTarg, "WSADDR"))
            {
                strncpy(pv->m_szIp, sgetvalue(szValue, ":", 1), sizeof(pv->m_szIp));
                pv->m_lPort = atol(sgetvalue(szValue, ":", 2));
            }
            else if(!strcasecmp(szTarg, "TIMEOUT"))
                pv->m_lTimeOut = atol(szValue);
            else if(!strcasecmp(szTarg, "MAXTRY"))
                pv->m_lTryMax = atol(szValue);
            else if(!strcasecmp(szTarg, "KEEPALIVE"))
                pv->m_lKeepLive = atol(szValue);
        }

        if(!strlen(pv->m_szOwner))
        {
            fprintf(stdout, "%s\n*域名未设置\n", (char *)pstNode->m_psvData);
            goto PDOMAIN_ERROR;
        }

        if(!strlen(pv->m_szIp))
        {
            fprintf(stdout, "%s\n*域地址未设置\n", (char *)pstNode->m_psvData);
            goto PDOMAIN_ERROR;
        }

        if(pv->m_lPort <= 0)
        {
            fprintf(stdout, "%s\n*The domain port is set incorrectly or unset\n", 
                (char *)pstNode->m_psvData);
            goto PDOMAIN_ERROR;
        }

        pv->m_lGroup   = pv->m_lGroup > 0 ? pv->m_lGroup : 1;
        pv->m_lTimeOut = pv->m_lTimeOut > 0 ? pv->m_lTimeOut : 2;
        pv->m_lTryMax  = pv->m_lTryMax > 0 ? pv->m_lTryMax : 3;
        pv->m_lKeepLive= pv->m_lKeepLive > 0 ? pv->m_lKeepLive : 30;
    }
    TFlst(pstRoot);
    return RC_SUCC;

PDOMAIN_ERROR:
    TFree(*ppstDom);
    TFlst(pstRoot);
    return RC_FAIL;
}

/*************************************************************************************************
    description：parse the table of domain 
    parameters:
        pszNode                    --node
        pszBuffer                  --content
        plOut                      --number
        ppIndx                     --out of index
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lParseTable(char *pszNode, char *pszBuffer, long *plOut, TIndex **ppIndx)
{
    long    i, n;
    TIndex  *pv = NULL;
    char    szTarg[128], szValue[64];

    if(NULL == (*ppIndx = (TIndex *)realloc(*ppIndx, (++ (*plOut)) * sizeof(TIndex))))
        return RC_FAIL;

    pv = &(*ppIndx)[(*plOut) - 1];
    memset(pv, 0, sizeof(TIndex));
    sfieldreplace(pszBuffer, '\t', ' ');
    for(i = 0, n = lfieldnum(pszBuffer, " "); i < n; i ++)
    {
        memset(szTarg, 0, sizeof(szTarg));
        memset(szValue, 0, sizeof(szValue));
        if(RC_SUCC != lPraseField(sfieldvalue(pszBuffer, " ",  i + 1), szTarg, sizeof(szTarg), 
            szValue, sizeof(szValue)))
            return RC_FAIL;

        if(!strcasecmp(szTarg, "TABLE"))
            pv->m_table = atol(szValue);
        else if(!strcasecmp(szTarg, "TABLENAME"))
            strncpy(pv->m_szTable, szValue, sizeof(pv->m_szTable));
        else if(!strcasecmp(szTarg, "PART"))
            strncpy(pv->m_szPart, szValue, sizeof(pv->m_szPart));

        if(!strlen(pv->m_szPart))
            strcpy(pv->m_szPart, pszNode); 
        strcpy(pv->m_szOwner, pszNode); 
    }
    pv->m_lLocal = RES_REMOT_SID; 
    pv->m_lPers  = OPERATE_NULL;
    pv->m_lType  = TYPE_CLIENT;
    strncpy(pv->m_szTime, sGetUpdTime(), sizeof(pv->m_szTime));

    return RC_SUCC;
}

/*************************************************************************************************
    description：parse domain resource
    parameters:
        pszBuffer                  --content
        pstIndex                   --index
        pstDom                     --domain
        plDom                      --number
        ppstDom                    --domain list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lParseDomain(char *pszBuffer, TIndex *pstIndex, TDomain *pstDom, long *plDom, 
            TDomain **ppstDom)
{
    long    i, n;
    TDomain *pv = NULL;
    char    szTarg[128], szValue[64];

    if(NULL == (*ppstDom = (TDomain *)realloc(*ppstDom, (++ (*plDom)) * sizeof(TDomain))))
        return RC_FAIL;

    pv = &(*ppstDom)[(*plDom) - 1];
    memset(pv, 0, sizeof(TDomain));
    for(i = 0, n = lfieldnum(pszBuffer, " "); i < n; i ++)
    {
        memset(szTarg, 0, sizeof(szTarg));
        memset(szValue, 0, sizeof(szValue));
        if(RC_SUCC != lPraseField(sfieldvalue(pszBuffer, " ",  i + 1), szTarg, sizeof(szTarg), 
            szValue, sizeof(szValue)))
            return RC_FAIL;

        if(!strcasecmp(szTarg, "MTABLE"))
            pv->m_mtable = atol(szValue);
        else if(!strcasecmp(szTarg, "DOMAINID"))
            strncpy(pv->m_szOwner, szValue, sizeof(pv->m_szOwner));
        else
        {
            fprintf(stdout, "%s\n*Invalid parameter\n", pszBuffer);
            return RC_FAIL;
        }
    }
    
    pv->m_lTryMax   = 0;
    pv->m_lLastTime = 0;
    pv->m_lStatus   = RESOURCE_INIT;
    pv->m_table     = pstIndex->m_table;
    pv->m_lPort     = pstDom->m_lPort;
    pv->m_lGroup    = pstDom->m_lGroup;
    pv->m_lKeepLive = pstDom->m_lKeepLive;
    pv->m_lTimeOut  = pstDom->m_lTimeOut;
    pv->m_lTryMax   = pstDom->m_lTryMax;
    strcpy(pv->m_szIp, pstDom->m_szIp);
    strcpy(pv->m_szTable, pstIndex->m_szTable);
    strcpy(pv->m_szOwner, pstDom->m_szOwner);
    Tdefstr(pv->m_szPart, pstDom->m_szOwner, sizeof(pv->m_szPart));

    return RC_SUCC;
}

/*************************************************************************************************
    description：parse domain table of remote
    parameters:
        pstSavm                    --stvm handle
        pszFile                    --config
        pstBoot                    --boot paramter
        plOut                      --number of index
        ppstIndex                  --out of index list
        plDom                      --number of domain
        ppstDom                    --out of domain list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lParseRemote(SATvm *pstSavm, char *pszFile, TBoot *pstBoot, long *plOut, TIndex **ppstIndex, 
            long *plDom, TDomain **ppstDom)
{
    long    i, lResource;
    TIndex  *pv = NULL;
    CMList  *pstNode = NULL, *pstRoot = NULL;
    TDomain *pstRes = NULL, *pstResouce = NULL;
    char    szTarg[128], szValue[64], *p = NULL;

    if(RC_SUCC != lParseResouce(pstSavm, pszFile, &lResource, &pstResouce))
        return RC_FAIL;

    if(RC_SUCC != _lParseFile(pstSavm, &pstRoot, pszFile, "*REMOTE_TABLE"))
    {
        fprintf(stderr, "parse file, err:(%d)(%s)\n", pstSavm->m_lErrno, sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    if(!pstRoot)
    {
        for(i = 0; i < lResource; i ++)
        {
           pstResouce[i].m_table = SYS_TVM_INDEX;
           pstResouce[i].m_mtable = SYS_TVM_INDEX;
           strcpy(pstResouce[i].m_szTable, "SYS_TVM_INDEX");
           strcpy(pstResouce[i].m_szPart, pstResouce[i].m_szOwner);
        }

        *plOut = 0; 
        *ppstIndex = NULL;
        *plDom = lResource;
        *ppstDom = pstResouce;
        return RC_SUCC;
    }
    else
    {
        for(pstNode = pstRoot, *plDom = 0, *plOut = 0; pstNode; pstNode = pstNode->pstNext)
        {
            sfieldreplace((char *)pstNode->m_psvData, '\t', ' ');
            if(!strncmp((char *)pstNode->m_psvData, "TABLE", 5))
            {
                if(RC_SUCC != lParseTable(pstBoot->m_szNode, (char *)pstNode->m_psvData, 
                    plOut, ppstIndex))
                    goto PREMOTE_ERROR;
                pv = &(*ppstIndex)[(*plOut) - 1];
                continue;
            }
        
            if(NULL == (p = strstr((char *)pstNode->m_psvData, "DOMAINID")))
            {
                fprintf(stderr, "set error:%s\n", (char *)pstNode->m_psvData);
                goto PREMOTE_ERROR;
            }
        
            if(NULL == (pstRes = pGetResourse(lResource, pstResouce, 
                strimabout(sfieldvalue(p, "=", 2), "\"", "\""))))
            {
                fprintf(stderr, "No domain (%s) is found\n", sfieldvalue(p, "=", 2));
                goto PREMOTE_ERROR;
            }
        
            if(RC_SUCC != lParseDomain((char *)pstNode->m_psvData, pv, pstRes, plDom, ppstDom))
            {
                TFlst(pstRoot);
                return RC_FAIL;
            }
        }
    }
    TFree(pstResouce);
    TFlst(pstRoot);

    return RC_SUCC;

PREMOTE_ERROR:
    TFlst(pstRoot);
    TFree(*ppstIndex);
    TFree(pstResouce);
    return RC_FAIL;
}

/*************************************************************************************************
    description：Domain uniqueness checks
    parameters:
        lCount                     --number
        pstDomain                  --domain list
    return:
        true                       --repeat
        false
 *************************************************************************************************/
bool    bDomIsRepeat(long lCount, TDomain *pstDomain)
{
    int        i, j;

    for(i = 0; i < lCount; i ++)
    {
        for(j = 0; j < lCount; j ++)
        {
            if(i == j)    continue;

            if(pstDomain[i].m_lPort == pstDomain[j].m_lPort && 
                !strcmp(pstDomain[i].m_szIp, pstDomain[j].m_szIp) &&
                !strcmp(pstDomain[i].m_szPart, pstDomain[j].m_szPart) &&
                !strcmp(pstDomain[i].m_szTable, pstDomain[j].m_szTable))
            {
                if(SYS_TVM_INDEX == pstDomain[i].m_table)
                {
                    fprintf(stderr, "*domain(%s)(%s:%ld)repeate\n", pstDomain[i].m_szOwner, 
                        pstDomain[i].m_szIp, pstDomain[i].m_lPort);
                }
                else
                {
                    fprintf(stderr, "*table(%s)(%s)repeate\n", pstDomain[i].m_szTable, 
                        pstDomain[i].m_szPart);
                }
                return true;
            }
        }
    }

    return false;
}

/*************************************************************************************************
    description：Compile configuration file
    parameters:
        pszFile                    --config file
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lMakeConfig(char *pszFile)
{
    TBoot   stBoot;
    FILE    *fp = NULL;
    TIndex  *pstIndex = NULL;
    TDomain    *pstDomain = NULL;
    long    i, lOut = 0, lCount = 0;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

    if(!pszFile || !strlen(pszFile))
    {
        fprintf(stderr, "The configuration file is not set\n");
        return RC_FAIL;
    }

    if(RC_SUCC != access(pszFile, R_OK | F_OK ))
    {
        fprintf(stderr, "Insufficient authority(%s), please confirm!!!\n\n", pszFile);
        return RC_FAIL;
    }

    memset(&stBoot, 0, sizeof(TBoot));
    if(NULL == (fp = fopen(getenv("TVMCFG"), "wb")))
    {
        fprintf(stderr, "open (%s) failure, err:(%d)(%s)", getenv("TVMCFG"), 
            errno, strerror(errno));
        return RC_FAIL;
    }

    fwrite(TVM_RUNCFG_TAG, 4, 1, fp);
    if(RC_SUCC != lParseBoot(pstSavm, pszFile, &stBoot))
        goto CREATE_ERROR;

    fwrite(&stBoot, sizeof(stBoot), 1, fp);
    if(TVM_BOOT_CLUSTER != stBoot.m_lBootType)    //    单机部署
    {
        fclose(fp);
        fprintf(stdout, "create completed successfully!!!\n");
        return RC_FAIL;
    }

    if(RC_SUCC != lParseIndex(pstSavm, pszFile, &lOut, &pstIndex))
        goto CREATE_ERROR;

    fwrite(&lOut, sizeof(long), 1, fp);
    fwrite(pstIndex, sizeof(TIndex), lOut, fp);    
    TFree(pstIndex);

    if(RC_SUCC != lParseRemote(pstSavm, pszFile, &stBoot, &lOut, &pstIndex, 
        &lCount, &pstDomain))
        goto CREATE_ERROR;

    if(bDomIsRepeat(lCount, pstDomain))
        goto CREATE_ERROR;

    fwrite(&lOut, sizeof(long), 1, fp);
    if(lOut > 0)
    {
        fwrite(pstIndex, sizeof(TIndex), lOut, fp);    
        TFree(pstIndex);
    }

    fwrite(&lCount, sizeof(long), 1, fp);
    if(lCount > 0)
    {
        fwrite(pstDomain, sizeof(TDomain), lCount, fp);
        TFree(pstDomain);
    }
    fclose(fp);

    fprintf(stdout, "create completed successfully!!!\n");

    return RC_SUCC;

CREATE_ERROR:
    TFree(pstDomain);
    TFree(pstIndex);
    fclose(fp);
    return RC_FAIL;
}

/*************************************************************************************************
    description：Decompilate compilation configuration
    parameters:
        pszFile                    --config file
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lUnmakeConfig(char *pszFile)
{
    char    ch;
    TBoot   stBoot;
    FILE    *fp = NULL;
    bool    bf = false;
    TIndex  *pstIndex = NULL;
    TDomain *pstDomain = NULL;
    long    i, j, lOut = 0, lCount = 0;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

    if(RC_SUCC == access(pszFile, F_OK))
    {
        fprintf(stderr, "The configuration already exists, Confirm the cover(Y/N)?:");
        ch = getchar();
        if(ch != 'y' && 'Y' != ch)
            return RC_SUCC;
    }

    if(NULL == (fp = fopen(pszFile, "w")))
    {
        fprintf(stderr, "open file error, %s\n", strerror(errno));
        return RC_FAIL;
    }

    if(RC_SUCC != lGetBootConfig(pstSavm, &stBoot))
        goto UNMAKE_ERR;

    fprintf(fp, "*GLOBLE\n");
    fprintf(fp, "MACHINE=\"%s\"\n", stBoot.m_szNode);
    fprintf(fp, "MAXTABLE=%ld\n", stBoot.m_lMaxTable);
    fprintf(fp, "MAXFILED=%ld\n", stBoot.m_lMaxField);
    fprintf(fp, "MAXDOMAIN=%ld\n", stBoot.m_lMaxDomain);
    fprintf(fp, "MAXSEQUE=%ld\n", stBoot.m_lMaxSeque);
    fprintf(fp, "SERVER_EXEC=%ld\n", stBoot.m_lBootExec);
    fprintf(fp, "SERVER_PORT=%ld\n", stBoot.m_lBootPort);
    fprintf(fp, "LOGNAME=\"%s\"\n\n",  stBoot.m_szLog);

    fprintf(fp, "*LOCAL_RESOURCE\n");
    if(RC_SUCC != lGetLocalIndex(pstSavm, &lCount, (void *)&pstIndex))
        goto UNMAKE_ERR;

    for(i = 0; i < lCount; i ++)
        fprintf(fp, "TABLE=%d PERMIT=%ld\n", pstIndex[i].m_table, pstIndex[i].m_lPers);
    TFree(pstIndex);

    fprintf(fp, "\n*REMOTE_DOMAIN");
    if(RC_SUCC != lGetDomainIndex(pstSavm, &lCount, &pstIndex))
        goto UNMAKE_ERR;

    if(RC_SUCC != lGetDomainTable(pstSavm, &lOut, &pstDomain))
        goto UNMAKE_ERR;
    
    for(i = 0; i < lCount; i ++)
    {
        fprintf(fp, "\nTABLE=%d TABLENAME=\"%s\" PART=\"%s\" ", pstIndex[i].m_table, 
            pstIndex[i].m_szTable, pstIndex[i].m_szPart);
        for(j = 0, bf = false; j < lOut; j ++)
        {
            if(strcmp(pstDomain[j].m_szTable, pstIndex[i].m_szTable) ||     
                strcmp(pstDomain[j].m_szPart, pstIndex[i].m_szPart))
                continue;

            if(!bf)
            {
                bf = !bf;
                fprintf(fp, "GROUP=%ld TIMEOUT=%ld MAXTRY=%ld KEEPALIVE=%ld\n", 
                    pstDomain[j].m_lGroup, pstDomain[j].m_lTimeOut, pstDomain[j].m_lTryMax, 
                    pstDomain[j].m_lKeepLive);
            }

            fprintf(fp, "\tDOMAINID=\"%s\" WSADDR=\"%s:%ld\"\n", pstDomain[j].m_szOwner, 
                pstDomain[j].m_szIp, pstDomain[j].m_lPort);
        }
    }
    TFree(pstIndex);
    TFree(pstDomain);

    fclose(fp);
    fprintf(stdout, "导出文件(%s)成功，completed successfully!!!\n", pszFile);

    return RC_SUCC;
UNMAKE_ERR:
    fclose(fp);
    TFree(pstIndex);
    TFree(pstDomain);
       fprintf(stderr, "get config err:(%d)(%s)\n", pstSavm->m_lErrno, sGetTError(pstSavm->m_lErrno));
    return RC_FAIL;
}

/**************************************************************************************************
    code end
 **************************************************************************************************/
