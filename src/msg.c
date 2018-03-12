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
    ipc msg function
 ************************************************************************************************/
/*************************************************************************************************
    description：create message queue
    parameters:
       pstSavm                     --stvm handle
       bCreate                     --create type
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lCreateQueue(SATvm *pstSavm, bool bCreate)
{
    long    lQid;

    if(bCreate)
    {   
        pstSavm->m_yMey = yGetIPCPath(pstSavm, IPC_MSG);
        if(pstSavm->m_yMey <= RC_FAIL)
            return RC_FAIL;
        lQid = msgget(pstSavm->m_yMey, IPC_CREAT|0600);
    }   
    else
        lQid = msgget(IPC_PRIVATE, IPC_CREAT|0600);
    if(RC_FAIL >= lQid)
    {
        switch(errno)
        {
        case   EEXIST:
            pstSavm->m_lErrno = MSG_ERR_EXIST;
            break;
        case   EACCES:
            pstSavm->m_lErrno = MSG_ERR_ACCES;
            break;
        case   ENOMEM:
            pstSavm->m_lErrno = MSG_ERR_NOMEM;
            break;
        default:
            pstSavm->m_lErrno = MSG_ERR_INVAL;
            break;
        }
        return RC_FAIL;
    }

    return lQid;
}

/*************************************************************************************************
    description：Get the number of message queues
    parameters:
       pstSavm                     --stvm handle
       lQid                        --msg id
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lGetQueueNum(SATvm *pstSavm, long lQid)
{
    struct  msqid_ds    stQueue;

    if(msgctl(lQid, IPC_STAT, &stQueue) <= -1)
    {
        switch(errno)
        {
        case   EFAULT:
            pstSavm->m_lErrno = MSG_ERR_FAULT;
            break;
        case   EIDRM:
            pstSavm->m_lErrno = MSG_ERR_EIDRM;
            break;
        default:
            pstSavm->m_lErrno = MSG_ERR_INVAL;
            break;
        }

        return RC_FAIL;
    }

    return stQueue.msg_qnum;
}

/*************************************************************************************************
    description：Get the maximum queue support byte
    parameters:
       pstSavm                     --stvm handle
       lQid                        --msg id
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lQueueMaxByte(SATvm *pstSavm, long lQid)
{
    struct  msqid_ds    stQueue;

    if(msgctl(lQid, IPC_STAT, &stQueue) <= -1)
    {
        switch(errno)
        {
        case   EFAULT:
            pstSavm->m_lErrno = MSG_ERR_FAULT;
            break;
        case   EIDRM:
            pstSavm->m_lErrno = MSG_ERR_EIDRM;
            break;
        default:
            pstSavm->m_lErrno = MSG_ERR_INVAL;
            break;
        }

        return RC_FAIL;
    }

    return stQueue.msg_qbytes;
}

/*************************************************************************************************
    description：Gets the final processing time in the queue
    parameters:
       pstSavm                     --stvm handle
       lQid                        --msg id
    return:
        long                       --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lQueueRcvTime(SATvm *pstSavm, long lQid)
{
    struct  msqid_ds    stQueue;

    if(msgctl(lQid, IPC_STAT, &stQueue) <= -1)
    {
        switch(errno)
        {
        case   EFAULT:
            pstSavm->m_lErrno = MSG_ERR_FAULT;
            break;
        case   EIDRM:
            pstSavm->m_lErrno = MSG_ERR_EIDRM;
            break;
        default:
            pstSavm->m_lErrno = MSG_ERR_INVAL;
            break;
        }

        return RC_FAIL;
    }

//  return (long)time(&stQueue.msg_rtime);
    return (long)stQueue.msg_rtime > 0 ? (long)stQueue.msg_rtime : (long)stQueue.msg_ctime;
}

/*************************************************************************************************
    description：read from message queue
    parameters:
        pstSavm                     --stvm handle
        lQid                        --msg id
        pstVoid                     --create type
        lSize                       --create type
        lMType                      --msg type
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lEventRead(SATvm *pstSavm, long lQid, void *pstVoid, long lSize, long lMType)
{
    if(RC_SUCC > msgrcv(lQid, pstVoid, lSize - sizeof(long), lMType, 0))
    {
        switch(errno)
        {
        case   E2BIG:
            pstSavm->m_lErrno = MSG_ERR_E2BIG;
            break;
        case   EACCES:
            pstSavm->m_lErrno = MSG_ERR_ACCES;
            break;
        case   EFAULT:
            pstSavm->m_lErrno = MSG_ERR_FAULT;
            break;
        case   EIDRM:
            pstSavm->m_lErrno = MSG_ERR_EIDRM;
            break;
        case   EINTR:
            pstSavm->m_lErrno = MSG_ERR_EINTR;
            break;
        default:
            pstSavm->m_lErrno = MSG_ERR_INVAL;
            break;
        }

        return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：non-blocking read from message queue
    parameters:
        pstSavm                     --stvm handle
        lQid                        --msg id
        pstVoid                     --create type
        lSize                       --create type
        lMType                      --msg type
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lReadNoWait(SATvm *pstSavm, long lQid, void *psvVoid, long lSize, long lMType)
{
    errno = 0;
    if(RC_SUCC > msgrcv(lQid, psvVoid, lSize - sizeof(long), lMType, IPC_NOWAIT))
    {
        switch(errno)
        {
        case   EAGAIN:
        case   ENOMSG:
            return RC_SUCC;
        case   E2BIG:
            pstSavm->m_lErrno = MSG_ERR_E2BIG;
            break;
        case   EACCES:
            pstSavm->m_lErrno = MSG_ERR_ACCES;
            break;
        case   EFAULT:
            pstSavm->m_lErrno = MSG_ERR_FAULT;
            break;
        case   EIDRM:
            pstSavm->m_lErrno = MSG_ERR_EIDRM;
            break;
        case   EINTR:
            pstSavm->m_lErrno = MSG_ERR_EINTR;
            return RC_FAIL;
        default:
            pstSavm->m_lErrno = MSG_ERR_INVAL;
            break;
        }

        return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：write to message queue
    parameters:
        pstSavm                     --stvm handle
        lQid                        --msg id
        pstVoid                     --create type
        lSize                       --create type
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lEventWrite(SATvm *pstSavm, long lQid, void *psvData, long lSize)
{
    if(msgsnd(lQid, psvData, lSize - sizeof(long), 0) < RC_SUCC)
    {
        switch(errno)
        {
        case   EACCES:
            pstSavm->m_lErrno = MSG_ERR_ACCES;
            break;
        case   EAGAIN:
            pstSavm->m_lErrno = MSG_ERR_SNDEG;
            break;
        case   EFAULT:
            pstSavm->m_lErrno = MSG_ERR_FAULT;
            break;
        case   EIDRM:
            pstSavm->m_lErrno = MSG_ERR_EIDRM;
            break;
        case   EINTR:
            pstSavm->m_lErrno = MSG_ERR_EINTR;
            break;
        default:
            pstSavm->m_lErrno = MSG_ERR_INVAL;
            break;
        }

        return RC_FAIL;
    }

    return RC_SUCC;
}

/****************************************************************************************
    code end
 ****************************************************************************************/

