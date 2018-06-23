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

#define _GNU_SOURCE
#include    "tvm.h"

/************************************************************************************************
   function
 ************************************************************************************************/
extern void    vCondInsInit(FdCond *pstCond, TABLE t);
extern long    _lDumpTable(SATvm *pstSavm, TABLE t, char *pszFile);

/*************************************************************************************************
    description：dump the unused
    parameters:
        pstSavm                    --stvm handle
        t                          --table
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lUnuseDump(SATvm *pstSavm, TABLE t)
{
    FILE    *fp = NULL;
    char    szFile[512];
    RunTime *pstRun = NULL;
    SHTruck *pstTruck = NULL;
    size_t  lRow = 0, lOffset, lDump = 0;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    memset(szFile, 0, sizeof(szFile));
    if(RC_SUCC != lInitSATvm(pstSavm, t))
        return RC_FAIL;

    vHoldConnect(pstSavm);
    if(NULL == (pstRun = (RunTime *)pInitHitTest(pstSavm, t)))
        return RC_FAIL;

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        pstSavm->m_lErrno = RMT_NOT_SUPPT;
        return RC_FAIL;
    }

    snprintf(szFile, sizeof(szFile), "%s/%d.udb", getenv("TVMDBD"), t);
    if(NULL == (fp = fopen(szFile, "ab")))
    { 
        pstSavm->m_lErrno = FILE_NOT_RSET;
        return RC_FAIL;
    }

    lOffset = lGetTblData(t);
    pstRun->m_lCurLine = 0;
    pstSavm->lSize = lGetRowSize(t);
    pstTruck = (PSHTruck)pGetNode(pstRun->m_pvAddr, lOffset);
    for(lRow = 0; (lRow < ((TblDef *)pstRun->m_pvAddr)->m_lValid) && (lOffset < lGetTableSize(t));
        pstTruck = (PSHTruck)pGetNode(pstRun->m_pvAddr, lOffset))
    {
        if(IS_TRUCK_NULL(pstTruck) || pstTruck->m_lTimes == 0)
        {
            lOffset += lGetRowTruck(t);
            continue;
        }

        fwrite(pstTruck->m_pvData, lGetRowSize(t), 1, fp);
        pstSavm->pstVoid = pstTruck->m_pvData;
        vCondInsInit(&pstSavm->stCond, t);
        lDelete(pstSavm);

        lDump ++;
        lOffset += lGetRowTruck(t);
    }
    fclose(fp);
    vForceDisconnect(pstSavm, t);
    pstSavm->m_lEffect = lDump;

    fprintf(stdout, "export table:%s valid:%ld, completed successfully !!\n", sGetTableName(t), 
        pstSavm->m_lEffect);
    return RC_SUCC;
}

/*************************************************************************************************
    description：exit and backup tables
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lBackupTables(SATvm *pstSavm)
{
    size_t  lOut = 0, i;
    char    szFile[512];
    TIndex  *pstIndex = NULL;

    memset(szFile, 0, sizeof(szFile));
    if(RC_SUCC != lExportTable(SYS_TVM_INDEX, &lOut, (void *)&pstIndex))
        return RC_FAIL;

    snprintf(szFile, sizeof(szFile), "%s/backup", getenv("TVMDBD"));
    if(0 != mkdir(szFile, S_IRWXU | S_IRGRP))
    {
        if(EEXIST != errno)
        {
            vRedeError(pstSavm->m_lErrno = 127, strerror(errno));
            return RC_FAIL;
        }
    }

    for(i = 0; i < lOut; i ++)
    {
        if((TYPE_SYSTEM == pstIndex[i].m_lType || TYPE_INCORE == pstIndex[i].m_lType) &&
            SYS_TVM_SEQUE != pstIndex[i].m_table)
            continue;

        memset(szFile, 0, sizeof(szFile));
        snprintf(szFile, sizeof(szFile), "%s/backup/%d.sdb", getenv("TVMDBD"), 
            pstIndex[i].m_table);
        _lDumpTable(pstSavm, pstIndex[i].m_table, szFile);
    }

    TFree(pstIndex);

    return RC_SUCC;
}

/*************************************************************************************************
    description：boot and restore tables
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lRestoreTables(SATvm *pstSavm)
{
    DIR    *dir;
    struct dirent *pr;
    char    szPath[512], szFile[128];

    memset(szPath, 0, sizeof(szPath));
    memset(szFile, 0, sizeof(szFile));
    snprintf(szPath, sizeof(szPath), "%s/backup", getenv("TVMDBD"));
    if ((NULL == (dir = opendir(szPath))))
    {
        vRedeError(pstSavm->m_lErrno = 127, strerror(errno));
        return RC_FAIL;
    }

    while(pr = readdir(dir))
    {
        if (NULL == strcasestr(pr->d_name, ".sdb"))
            continue;

        if(DT_REG != pr->d_type) 
            continue;

        memset(szFile, 0, sizeof(szFile));
        snprintf(szFile, sizeof(szFile), "%s/%s", szPath, pr->d_name);
        if(RC_SUCC != lMountTable(pstSavm, szFile))
        {
            fprintf(stderr, "Warning:restore table failed, %s", 
                sGetTError(pstSavm->m_lErrno));
            continue;
        }
    }

    closedir(dir);

    return RC_SUCC;
}

/****************************************************************************************
    code end
 ****************************************************************************************/
