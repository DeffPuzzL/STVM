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

/****************************************************************************************
    senum union
 ****************************************************************************************/
union semun {
    int                    val;  
    struct    semid_ds    *buf;  
    struct    seminfo        *__buf;
};

/************************************************************************************************
    sems function
 ************************************************************************************************/
/*************************************************************************************************
    description：create semaphore
    parameters:
       pstSavm                     --stvm handle
       pstRun                      --table handle
       lSems                       --the number of semaphores
       lValue                      --The index of the semaphores
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lCreateSems(SATvm *pstSavm, RunTime *pstRun, long lSems, long lValue)
{
    long    i = 0;
    union    semun    uSem;
    
    if(lSems <= 0)
    {
        pstSavm->m_lErrno = SEM_CDIT_NULL;
        return RC_FAIL;    
    }

    if(RC_FAIL == (pstRun->m_semID = semget(pstSavm->m_ySey, lSems, IPC_CREAT|0600)))
    {
        switch(errno)
        {
        case   EEXIST:
            pstSavm->m_lErrno = SEM_ERR_EXIST;
            break;
        case   EIDRM:
            pstSavm->m_lErrno = SEM_ERR_EIDRM;
            break;
        case   EACCES:
            pstSavm->m_lErrno = SEM_ERR_ACCES;
            break;
        case   ENOMEM:
            pstSavm->m_lErrno = SEM_ERR_NOMEM;
            break;
        default:
            pstSavm->m_lErrno = SEM_ERR_INVAL;
            break;
        }

        return RC_FAIL;
    }

    for(i = 0, uSem.val = lValue; i < lSems; i ++)
        semctl(pstRun->m_semID, i, SETVAL, uSem);

      return RC_SUCC;
}

/*************************************************************************************************
    description：create semaphore
    parameters:
       pstSavm                     --stvm handle
       semID                       --the idx of semaphores
       lSems                       --the number of semaphores
       evp                         --opereate of P-V 
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lOperateSems(SATvm *pstSavm, long semID, long lSems, Benum evp)
{
    struct    sembuf    se; 

    se.sem_num = lSems;
    se.sem_op  = evp;
    se.sem_flg = SEM_UNDO;

    if(RC_SUCC != semop(semID, &se, 1))
    {
        switch(errno)
        {
        case   EINTR:
            return RC_SUCC;
        case   EEXIST:
            pstSavm->m_lErrno = SEM_ERR_EXIST;
            break;
        case   EIDRM:
            pstSavm->m_lErrno = SEM_ERR_EIDRM;
            break;
        case   EACCES:
            pstSavm->m_lErrno = SEM_ERR_ACCES;
            break;
        case   ENOMEM:
            pstSavm->m_lErrno = SEM_ERR_NOMEM;
            break;
        case   E2BIG:
        case   ERANGE:
            pstSavm->m_lErrno = SEM_ERR_INVAL;
            break;
        default:
            pstSavm->m_lErrno = SEM_ERR_INVAL;
            break;
        }

        return RC_FAIL;
    }

    return RC_SUCC;
}


/****************************************************************************************
    code end
 ****************************************************************************************/

