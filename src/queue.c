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

/************************************************************************************************
   function
 ************************************************************************************************/
extern long       _lPushByRt(SATvm *pstSavm);
extern long       _lPopByRt(SATvm *pstSavm, void *psvOut);
extern long       _lPopupByRt(SATvm *pstSavm, size_t lExp, time_t lTime, size_t *plOut, void **pp);

/*************************************************************************************************
    description：push data to queue
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --address
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lPush(SATvm *pstSavm, void *pvAddr)
{
    int     nPos;
    SHTruck *ps = NULL;
    TblDef  *pv = (TblDef *)pvAddr;

    if(pv->m_lValid == pv->m_lMaxRow)
    {
        pstSavm->m_lErrno = DATA_SPC_FULL;
        return RC_FAIL;
    }

    if(pv->m_lMaxRow > (nPos = (int)__sync_add_and_fetch(&pv->m_lListPos, 1)))
        ;
    else if(0 == (nPos = nPos % pv->m_lMaxRow))
        __sync_sub_and_fetch(&pv->m_lListPos, pv->m_lMaxRow);

    ps = (PSHTruck)pGetNode(pvAddr, pv->m_lData + pv->m_lTruck * nPos);
    if(ps->m_chTag != DATA_TRUCK_NULL)
    {
        pstSavm->m_lErrno = DATA_SPC_FULL;
        return RC_FAIL;
    }

    memcpy(ps->m_pvData, pstSavm->pstVoid, pv->m_lReSize);
    SET_DATA_TRUCK(ps, DATA_TRUCK_NRML);
    __sync_add_and_fetch(&pv->m_lValid, 1);
    pstSavm->m_lEffect = 1;
 
    Futex(&pv->m_lValid, FUTEX_WAKE, pv->m_lGroup, NULL);
    return RC_SUCC;
}

/*************************************************************************************************
    description：pop data from queue
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --address
        pvOut                      --out of data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lPop(SATvm *pstSavm, void *pvAddr, void *pvOut, Timesp *tm)
{
    int     nPos;
    SHTruck *ps = NULL;
    extern  int errno;
    TblDef  *pv = (TblDef *)pvAddr;

    while(1)
    {
        if(0 != Futex(&pv->m_lValid, FUTEX_WAIT, 0, tm))
        {
            if(ETIMEDOUT == errno) // timeout
            {
                pstSavm->m_lErrno = NO_DATA_FOUND;
                return RC_FAIL;
            }
            else if(EWOULDBLOCK != errno)
            {
                pstSavm->m_lErrno = MQUE_WAIT_ERR;
                return RC_FAIL;
            }
            else    // EWOULDBLOCK
                ;
        }
 
        if(0 == pv->m_lValid)
            continue;

        if(0 > (int)__sync_sub_and_fetch(&pv->m_lValid, 1))
        {   
            __sync_fetch_and_add(&pv->m_lValid, 1);
            continue;
        }

        break;
    }
    
    /* at least cost one vaild */
    if(pv->m_lMaxRow > (nPos = __sync_add_and_fetch(&pv->m_lListOfs, 1)))
        ;
    else if(0 == (nPos = nPos % pv->m_lMaxRow))
        __sync_sub_and_fetch(&pv->m_lListOfs, pv->m_lMaxRow);
    
    ps = (PSHTruck)pGetNode(pvAddr, pv->m_lData + pv->m_lTruck * nPos);
    if(IS_TRUCK_NULL(ps))
    {
        pstSavm->m_lErrno = SVR_EXCEPTION;
        return RC_FAIL;
    }

    memcpy(pvOut, ps->m_pvData, pv->m_lReSize);
    SET_DATA_TRUCK(ps, DATA_TRUCK_NULL);
    pstSavm->m_lEffect = 1;

    return RC_SUCC;
}

/*************************************************************************************************
    description：pop more data from queue
    parameters:
        pstSavm                    --stvm handle
        pvAddr                     --address
        lExpect                    --Expected number of records
        tm                         --time stamp
        plOut                      --Number of records pop
        ppsvOut                    --data list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    _lPops(SATvm *pstSavm, void *pvAddr, size_t lExpect, Timesp *tm, size_t *plOut, 
            void **ppsvOut)
{
    int     nPos;
    SHTruck *ps = NULL;
    extern  int errno;
    TblDef  *pv = (TblDef *)pvAddr;

    if(NULL == (*ppsvOut = (char *)malloc(lExpect * pv->m_lReSize)))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    for (*plOut = 0; *plOut < lExpect; )
    {
        if(0 != Futex(&pv->m_lValid, FUTEX_WAIT, 0, tm))
        {
            if(ETIMEDOUT == errno)
            {
                if(0 == *plOut)
                {
                    TFree(*ppsvOut);
                    pstSavm->m_lErrno = NO_DATA_FOUND;
                }
                else
                    pstSavm->m_lErrno = MQUE_WAIT_TMO;
                return RC_FAIL;
            }
            else if(EWOULDBLOCK != errno)
            {
                pstSavm->m_lErrno = MQUE_WAIT_ERR;
                return RC_FAIL;
            }
            else    // EWOULDBLOCK
                ;
        }
 
        if(0 == pv->m_lValid)
            continue;

        if(0 > (int)__sync_sub_and_fetch(&pv->m_lValid, 1))
        {   
            __sync_fetch_and_add(&pv->m_lValid, 1);
            continue;
        }
    
        /* at least cost one vaild */
        if(pv->m_lMaxRow > (nPos = __sync_add_and_fetch(&pv->m_lListOfs, 1)))
            ;
        else if(0 == (nPos = nPos % pv->m_lMaxRow))
            __sync_sub_and_fetch(&pv->m_lListOfs, pv->m_lMaxRow);
        
        ps = (PSHTruck)pGetNode(pvAddr, pv->m_lData + pv->m_lTruck * nPos);
        if(IS_TRUCK_NULL(ps))
        {
            *plOut = 0;
            TFree(*ppsvOut);
            pstSavm->m_lErrno = SVR_EXCEPTION;
            return RC_FAIL;
        }

        memcpy(*ppsvOut + (*plOut) * pv->m_lReSize, ps->m_pvData, pv->m_lReSize);
        SET_DATA_TRUCK(ps, DATA_TRUCK_NULL);
        ++ (*plOut);
    }

    pstSavm->m_lEffect  = *plOut;

    return RC_SUCC;
}

/*************************************************************************************************
    description：pop data from queue
    parameters:
        pstSavm                    --stvm handle
        psvOut                     --out data
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lPop(SATvm *pstSavm, void *pvOut, Uenum eWait)
{
    long    lRet;
    RunTime *pstRun = NULL;
    static  Timesp  tm = {0, 1};

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    if(NULL == (pstRun = (RunTime *)pInitMemTable(pstSavm, pstSavm->tblName)))
        return RC_FAIL;

    if(TYPE_MQUEUE != pstRun->m_lType)
    {
        pstSavm->m_lErrno = NOT_SUPPT_OPT;
        vTblDisconnect(pstSavm, pstSavm->tblName);
        return RC_FAIL;
    }

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        Tremohold(pstSavm, pstRun);
        return _lPopByRt(pstSavm, pvOut);
    }

    if(QUE_NOWAIT == eWait)
        lRet = _lPop(pstSavm, pstRun->m_pvAddr, pvOut, &tm);
    else
        lRet = _lPop(pstSavm, pstRun->m_pvAddr, pvOut, NULL);
    vTblDisconnect(pstSavm, pstSavm->tblName);
    return lRet;
}

/*************************************************************************************************
    description：pop more data from queue
    parameters:
        pstSavm                    --stvm handle
        lExpect                    --Expected number of records
        tm                         --time stamp
        plOut                      --Number of records pop
        ppsvOut                    --data list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lPopup(SATvm *pstSavm, size_t lExpect, time_t lTime, size_t *plOut, void **ppsvOut)
{
    long    lRet;
    Timesp  tm = {0};
    RunTime *pstRun = NULL;

    if(!pstSavm)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    if(NULL == (pstRun = (RunTime *)pInitMemTable(pstSavm, pstSavm->tblName)))
        return RC_FAIL;

    if(TYPE_MQUEUE != pstRun->m_lType)
    {
        pstSavm->m_lErrno = NOT_SUPPT_OPT;
        vTblDisconnect(pstSavm, pstSavm->tblName);
        return RC_FAIL;
    }

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        Tremohold(pstSavm, pstRun);
        return _lPopupByRt(pstSavm, lExpect, lTime, plOut, ppsvOut);
    }

    tm.tv_sec = lTime;
    lRet = _lPops(pstSavm, pstRun->m_pvAddr, lExpect, &tm, plOut, ppsvOut);
    vTblDisconnect(pstSavm, pstSavm->tblName);
    return lRet;
}

/*************************************************************************************************
    description：push more data to queue
    parameters:
        pstSavm                    --stvm handle
        plOut                      --Number of records pop
        ppsvOut                    --data list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lPushs(SATvm *pstSavm, size_t *plOut, void **ppsvOut)
{
    long    lRet, i;
    RunTime *pstRun = NULL;

    if(!pstSavm || !pstSavm->pstVoid)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    if(NULL == (pstRun = (RunTime *)pInitMemTable(pstSavm, pstSavm->tblName)))
        return RC_FAIL;

    if(TYPE_MQUEUE != pstRun->m_lType)
    {
        pstSavm->m_lErrno = NOT_SUPPT_OPT;
        vTblDisconnect(pstSavm, pstSavm->tblName);
        return RC_FAIL;
    }

    for(i = 0; i < *plOut; i ++)
    {
        if(RES_REMOT_SID == pstRun->m_lLocal)
        {
            Tremohold(pstSavm, pstRun);
            return _lPushByRt(pstSavm);
        }

        if(RC_SUCC != _lPush(pstSavm, pstRun->m_pvAddr))
            break;
    }

    vTblDisconnect(pstSavm, pstSavm->tblName);
    return lRet;
}

/*************************************************************************************************
    description：push data to queue
    parameters:
        pstSavm                    --stvm handle
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lPush(SATvm *pstSavm)
{
    long    lRet;
    RunTime *pstRun = NULL;

    if(!pstSavm || !pstSavm->pstVoid)
    {
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    if(NULL == (pstRun = (RunTime *)pInitMemTable(pstSavm, pstSavm->tblName)))
        return RC_FAIL;

    if(TYPE_MQUEUE != pstRun->m_lType)
    {
        pstSavm->m_lErrno = NOT_SUPPT_OPT;
        vTblDisconnect(pstSavm, pstSavm->tblName);
        return RC_FAIL;
    }

    if(RES_REMOT_SID == pstRun->m_lLocal)
    {
        Tremohold(pstSavm, pstRun);
        return _lPushByRt(pstSavm);
    }

    lRet = _lPush(pstSavm, pstRun->m_pvAddr);
    vTblDisconnect(pstSavm, pstSavm->tblName);
    return lRet;
}

/****************************************************************************************
    code end
 ****************************************************************************************/
