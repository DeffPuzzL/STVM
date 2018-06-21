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

/************************************************************************************************
   function
 ************************************************************************************************/
extern void    vCondInsInit(FdCond *pstCond, TABLE t);

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


/****************************************************************************************
    code end
 ****************************************************************************************/
