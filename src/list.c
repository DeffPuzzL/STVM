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
/*************************************************************************************************
    description：get the tailf of list
    parameters:
        root                       --list of root
    return:
        void*
 *************************************************************************************************/
CMList* pGetCMTail(CMList *root)
{
    CMList *list = root;

    if(!list)    return NULL;

    while(list->pstNext)
        list = list->pstNext;

    return list;
}

/*************************************************************************************************
    description：insert to list
    parameters:
    return:
        root                       -root node
 *************************************************************************************************/
CMList* pInsertList(CMList *root, void *pszData, long lSize)
{
    CMList *node = NULL, *tail = pGetCMTail(root);

    if(NULL == (node = (CMList *)malloc(sizeof(CMList))))
    {
        vSetTErrno(MALLC_MEM_ERR);
        return root;
    }

    node->m_lSize = lSize;
    if(NULL == (node->m_psvData = (char *)malloc(node->m_lSize)))
    {
        vSetTErrno(MALLC_MEM_ERR);
        return root;
    }

    node->pstNext = NULL;
    node->pstLast = NULL;
    memcpy(node->m_psvData, pszData, node->m_lSize);

    if(!root)
        root = node;
    else
    {
        node->pstLast = tail;
        tail->pstNext = node;            
    }

    return root;
}

/*************************************************************************************************
    description：find node from the list
    parameters:
    return:
        node                       --list node
 *************************************************************************************************/
CMList* pSearchNode(CMList *root, void *pv, long n)
{
    CMList *node;
    
    for(node = root; node; node = node->pstNext)
    {
        if(!memcmp(node->m_psvData, pv, n))
            return node;
    }

    return NULL;
}

/*************************************************************************************************
    description：destroy list
    parameters:
        root                       --list root
    return:
        void*
 *************************************************************************************************/
void    vDestroyList(CMList *root)
{
    CMList    *node = root, *list = NULL;

    while(node)
    {
        list = node;
        node = node->pstNext;
        TFree(list->m_psvData);
        TFree(list);
    }

    root = NULL;
}

/*************************************************************************************************
    description：delete node
    parameters:
        root                       --root node
        psvData                    --node value
        lSize                      --length of value
    return:
        root                       --root node
 *************************************************************************************************/
CMList*    pDeleteNode(CMList *root, void *psvData, long lSize)
{
    CMList    *node = root;

    while(node)
    {
        if(!memcmp(node->m_psvData, psvData, lSize))
            break;
        node = node->pstNext;
    }
    if(!node)   return root;

    if(node->pstNext)
        node->pstNext->pstLast = node->pstLast;

    if(!node->pstLast)
        root = node->pstNext;
    else
        node->pstLast->pstNext = node->pstNext;

    TFree(node->m_psvData);
    TFree(node);

    return root;
}

/*************************************************************************************************
    description：node number
    parameters:
        root                       --root node
    return:
        long                       --count the number of list
 *************************************************************************************************/
long    lListNodeCount(CMList *root)
{
    long    lCount = 0;
    CMList  *node = root;

    while(node)
    {
        ++ lCount;
        node = node->pstNext;
    }

    return lCount;
}

/*************************************************************************************************
    description：Check if rowgrp is repeated
    parameters:
        pstRrp                     --field index list
        pvData                     --check data
        t                          --table 
        lOut                       --number
        psvOut                     --truck list
    return:
        true                       --repeate
        false
 *************************************************************************************************/
bool    bRepeatLstgrp(FdCond *pstRrp, void *pvData, TABLE t, size_t lOut, void *psvOut)
{
    uint     i, j;
    FdKey    *pstFd;
    bool     bRepeat = false;

    if(!psvOut || 0 == lOut)  return false;
 
    for(i = 0; i < lOut; i ++)
    {
        for(j = 0, bRepeat = true; j < pstRrp->uFldcmp; j ++)
        {
            pstFd = &pstRrp->stFdKey[j];
            if(0 == (GROUP_BY & pstFd->uDecorate))
                continue;
        
            if(memcmp(psvOut + (i * lGetRowSize(t)) + pstFd->uFldpos, pvData + pstFd->uFldpos, 
                pstFd->uFldlen))
                bRepeat = false;
        }
    
        if(bRepeat)   return bRepeat;
    }

    return bRepeat;
}

/*************************************************************************************************
    description：insert group node
    parameters:
        pstSavm                    --stvm handle
        pstRrp                     --field index list
        pvData                     --insert data
        lOut                       --number
        psvOut                     --truck list
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lInsertLstgrp(SATvm *pstSavm, FdCond *pstRrp, void *pvData, TABLE t, size_t *plOut, 
            void **ppsvOut)
{
    FdKey    *pstFd;
    size_t   i, lOffset = (*plOut) * lGetRowSize(t);

    if(NULL == (*ppsvOut = (char *)realloc(*ppsvOut, lOffset + lGetRowSize(t))))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    memset(*ppsvOut + lOffset, 0, lGetRowSize(t));
    for(i = 0; i < pstRrp->uFldcmp; i ++)
    {
        pstFd = &pstRrp->stFdKey[i];
        if(0 == (GROUP_BY & pstFd->uDecorate))
            continue;
    
        memcpy(*ppsvOut + lOffset + pstFd->uFldpos, pvData + pstFd->uFldpos, pstFd->uFldlen);
    }

    (*plOut) ++;
    return RC_SUCC;
}

/*************************************************************************************************
    Rowgrp
 *************************************************************************************************/
/*************************************************************************************************
    description：get rowgrp tail
    parameters:
        root                       --root of rowgrp
    return:
        void*                      --tailf of rowgrp
 *************************************************************************************************/
Rowgrp* pRowgrpTail(Rowgrp *root)
{
    Rowgrp *node = root;

    if(!node)    return NULL;

    while(node->pstNext)
        node = node->pstNext;

    return node;
}

/*************************************************************************************************
    description：free rowgrp list
    parameters:
        root                       --root of rowgrp
    return:
 *************************************************************************************************/
void    vDeleteRowgrp(Rowgrp *root)
{
    Rowgrp *list, *node;

    for(list = root, node = root; NULL != list; node = list)
    {
        list = list->pstNext;
        vDeleteRowgrp(node->pstSSet);
        TFree(node->psvData);    
        TFree(node);    
    }

    return ;
}

/*************************************************************************************************
    description：delete node from rowgrp
    parameters:
        root                       --root of rowgrp
        delete                     --delete node
    return:
 *************************************************************************************************/
void    vDropNodegrp(Rowgrp **root, Rowgrp *delete)
{
    Rowgrp *list, *node;

    if(!root)    return ;

    for(list = *root; NULL != list; )
    {
        if(list != delete)    
        {
            list = list->pstNext;
            continue;
        }

        if(list->pstLast)
            list->pstLast->pstNext = list->pstNext;
        else
            *root = list->pstNext;

        if(list->pstNext)
            list->pstNext->pstLast = list->pstLast;
        node = list;
        list = list->pstNext;
        vDeleteRowgrp(node->pstSSet);
        TFree(node->psvData);    
        TFree(node);    
    }

    return ;
}

/*************************************************************************************************
    description：count rowgrp
    parameters:
        root                       --root of rowgrp
        pv                         --data 
        lLen                       --data length
        lCount                     --count
    return:
        long                       --number of count
 *************************************************************************************************/
long    lCountRowgrp(Rowgrp *root, void *pv, long lLen, size_t lCount)
{
    long   n;
    Rowgrp *node, *list;

    for(node = root; NULL != node; node = node->pstNext)
    {
        if(!memcmp(node->psvData, pv, lLen))
        {
            node->lCount += lCount;
            return node->lCount;
        }

        if(0 != (n = lCountRowgrp(node->pstSSet, pv, lLen, lCount)))
            return n;
    }

    return 0;
}

/*************************************************************************************************
    description：insert node from rowgrp
    parameters:
        pstSavm                    --stvm handle
        root                       --root of rowgrp
        pstFSet                    --parent node
        pstSSet                    --child node
        pv                         --data
        n                          --length
        lCount                     --count
    return:
        void*                      --root of rowgrp
 *************************************************************************************************/
Rowgrp* pInsertRowgrp(SATvm *pstSavm, Rowgrp *root, Rowgrp *pstFset, Rowgrp *pstSSet, void *pv, 
           long n, size_t lCount)
{
    Rowgrp *node = NULL, *tail = (Rowgrp *)pRowgrpTail(root);

    if(NULL == (node = (Rowgrp *)calloc(1, sizeof(Rowgrp))))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return root;
    }

    if(NULL == (node->psvData = (char *)calloc(n + 1, sizeof(char))))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return root;
    }

    node->lLen = n;
    if(0 == lCount)
        node->lCount ++;
    else
        node->lCount = lCount;
    node->pstFset = pstFset;
    node->pstSSet = pstSSet;
    memcpy(node->psvData, pv, node->lLen);

    if(!root)
    {
        root = node;
        tail = node;
        root->lIdx = 0;
        return root;
    }

    node->lIdx = tail->lIdx + 1;
    node->pstLast = tail;
    tail->pstNext = node;            
    tail = node;

    return root;
}

/*************************************************************************************************
    description：get middle of rowgrp
    parameters:
        root                       --root of rowgrp
    return:
        void*                      --root of rowgrp
 *************************************************************************************************/
Rowgrp* pGetListMid(Rowgrp *root)
{
    Rowgrp *fast = NULL, *slow = NULL;

    if(root == NULL || root->pstNext == NULL)  
        return NULL;

    for(slow = root, fast = root->pstNext; fast; slow = slow->pstNext)
    {
        fast = fast->pstNext;  
        if(NULL == fast)
            break;
        fast = fast->pstNext;  
    }

    fast = slow->pstNext;  
    slow->pstNext = NULL; 
    return fast;
}

/*************************************************************************************************
    description：Sort list with ASC.
    parameters:
        root                       --root of rowgrp
    return:
        void*                      --root of rowgrp
 *************************************************************************************************/
Rowgrp* pSortMergeAsc(Rowgrp *pa, Rowgrp *pb, FdKey *pstKey)  
{  
    Rowgrp *pstRes = NULL;  
  
    if(pa == NULL)  
        return pb;  
    else if(pb == NULL)  
        return pa;  
  
    switch(pstKey->uDecorate & 0x0f)
    {
    case FIELD_DOUB:
        switch(pstKey->uFldlen)
        {
        case    4:
            if(*((float *)(pa->psvData)) < *((float *)(pb->psvData)))
            {
                pstRes = pa;  
                pstRes->pstNext = pSortMergeAsc(pa->pstNext, pb, pstKey);  
            }
            else
            {
                pstRes = pb;  
                pstRes->pstNext = pSortMergeAsc(pa, pb->pstNext, pstKey);
            }
            break;
        case    8:
            if(*((double *)(pa->psvData)) < *((double *)(pb->psvData)))
            {
                pstRes = pa;  
                pstRes->pstNext = pSortMergeAsc(pa->pstNext, pb, pstKey);  
            }
            else
            {
                pstRes = pb;  
                pstRes->pstNext = pSortMergeAsc(pa, pb->pstNext, pstKey);
            }
            break;
        default:
            break;
        }
        break;
    case FIELD_LONG:
        switch(pstKey->uFldlen)
        {
        case    2:
            if(*((sint *)(pa->psvData)) < *((sint *)(pb->psvData)))
            {
                pstRes = pa;  
                pstRes->pstNext = pSortMergeAsc(pa->pstNext, pb, pstKey);  
            }
            else
            {
                pstRes = pb;  
                pstRes->pstNext = pSortMergeAsc(pa, pb->pstNext, pstKey);
            }
            break;
        case    4:
            if(*((int *)(pa->psvData)) < *((int *)(pb->psvData)))
            {
                pstRes = pa;  
                pstRes->pstNext = pSortMergeAsc(pa->pstNext, pb, pstKey);  
            }
            else
            {
                pstRes = pb;  
                pstRes->pstNext = pSortMergeAsc(pa, pb->pstNext, pstKey);
            }
            break;
        case    8:
            if(*((llong *)(pa->psvData)) < *((llong *)(pb->psvData)))
            {
                pstRes = pa;  
                pstRes->pstNext = pSortMergeAsc(pa->pstNext, pb, pstKey);  
            }
            else
            {
                pstRes = pb;  
                pstRes->pstNext = pSortMergeAsc(pa, pb->pstNext, pstKey);
            }
            break;
        default:
            break;
        }
        break;
    case FIELD_CHAR:
        if(0 < memcmp(pb->psvData, pa->psvData, pstKey->uFldlen))
        {
            pstRes = pa;  
            pstRes->pstNext = pSortMergeAsc(pa->pstNext, pb, pstKey);  
        }
        else
        {
            pstRes = pb;  
            pstRes->pstNext = pSortMergeAsc(pa, pb->pstNext, pstKey);
        }
        break;
    default:
        break;
    }

    if(pstRes->pstNext)    pstRes->pstNext->pstLast = pstRes;    

    return pstRes;  
}  

/*************************************************************************************************
    description：Sort list with DESC.
    parameters:
        root                       --root of rowgrp
    return:
        void*                      --root of rowgrp
 *************************************************************************************************/
Rowgrp* pSortMergeDes(Rowgrp *pa, Rowgrp *pb, FdKey *pstKey)  
{  
    Rowgrp *pstRes = NULL;  
  
    if(pa == NULL)  
        return pb;  
    else if(pb == NULL)  
        return pa;  
  
    switch(pstKey->uDecorate & 0x0f)
    {
    case FIELD_DOUB:
        switch(pstKey->uFldlen)
        {
        case    4:
            if(*((float *)(pa->psvData)) > *((float *)(pb->psvData)))
            {
                pstRes = pa;  
                pstRes->pstNext = pSortMergeDes(pa->pstNext, pb, pstKey);  
            }
            else
            {
                pstRes = pb;  
                pstRes->pstNext = pSortMergeDes(pa, pb->pstNext, pstKey);
            }
            break;
        case    8:
            if(*((double *)(pa->psvData)) > *((double *)(pb->psvData)))
            {
                pstRes = pa;  
                pstRes->pstNext = pSortMergeDes(pa->pstNext, pb, pstKey);  
            }
            else
            {
                pstRes = pb;  
                pstRes->pstNext = pSortMergeDes(pa, pb->pstNext, pstKey);
            }
            break;
        default:
            break;
        }
        break;
    case FIELD_LONG:
        switch(pstKey->uFldlen)
        {
        case    2:
            if(*((sint *)(pa->psvData)) > *((sint *)(pb->psvData)))
            {
                pstRes = pa;  
                pstRes->pstNext = pSortMergeDes(pa->pstNext, pb, pstKey);  
            }
            else
            {
                pstRes = pb;  
                pstRes->pstNext = pSortMergeDes(pa, pb->pstNext, pstKey);
            }
            break;
        case    4:
            if(*((int *)(pa->psvData)) > *((int *)(pb->psvData)))
            {
                pstRes = pa;  
                pstRes->pstNext = pSortMergeDes(pa->pstNext, pb, pstKey);  
            }
            else
            {
                pstRes = pb;  
                pstRes->pstNext = pSortMergeDes(pa, pb->pstNext, pstKey);
            }
            break;
        case    8:
            if(*((llong *)(pa->psvData)) > *((llong *)(pb->psvData)))
            {
                pstRes = pa;  
                pstRes->pstNext = pSortMergeDes(pa->pstNext, pb, pstKey);  
            }
            else
            {
                pstRes = pb;  
                pstRes->pstNext = pSortMergeDes(pa, pb->pstNext, pstKey);
            }
            break;
        default:
            break;
        }
        break;
    case FIELD_CHAR:
        if(0 > memcmp(pb->psvData, pa->psvData, pstKey->uFldlen))
        {
            pstRes = pa;  
            pstRes->pstNext = pSortMergeDes(pa->pstNext, pb, pstKey);  
        }
        else
        {
            pstRes = pb;  
            pstRes->pstNext = pSortMergeDes(pa, pb->pstNext, pstKey);
        }
        break;
    default:
        break;
    }

    if(pstRes->pstNext)    pstRes->pstNext->pstLast = pstRes;    

    return pstRes;  
}  

/*************************************************************************************************
    description：Sort list by field
    parameters:
        root                       --root 
        pstFd                      --field
    return:
 *************************************************************************************************/
void   _vSortField(Rowgrp **root, FdKey *pstFd)
{  
    Rowgrp  *slow = NULL;

    if(NULL == *root || NULL == (*root)->pstNext)  
        return;  
      
    slow = pGetListMid(*root);  
    _vSortField(root, pstFd);
    _vSortField(&slow, pstFd);

    if(ORDER_DESC & pstFd->uDecorate)
        *root = pSortMergeDes(*root, slow, pstFd);
    else
        *root = pSortMergeAsc(*root, slow, pstFd);  
    return ;
}  

/*************************************************************************************************
    description：Sort subset
    parameters:
        root                       --root 
        pstFd                      --field
    return:
 *************************************************************************************************/
void    vSubsetSort(Rowgrp *root, FdCond *pstExm, uint ug)
{
    Rowgrp  *node;
    FdKey   *pstFd;

    for(node = root, pstFd = &pstExm->stFdKey[ug]; node; node = node->pstNext)
    {
        if(0 == (pstFd->uDecorate & (ORDER_ASC | ORDER_DESC)))
            continue;

        _vSortField(&node->pstSSet, pstFd);
        vSubsetSort(node->pstSSet, pstExm, ++ ug);    
    }

    return ;
}

/*************************************************************************************************
    description：Sort Rowgrp
    parameters:
        root                       --root 
        pstFd                      --field
        t                          --t
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long   lSortRowgrp(Rowgrp **root, FdCond *pstExm, TABLE t)
{
    FdKey   *pstFd = &pstExm->stFdKey[0];

    if(!bSetCondAttr(pstExm, t, ORDER_ASC | ORDER_DESC))
        return RC_SUCC;
    
    if(pstFd->uDecorate & (ORDER_ASC | ORDER_DESC))
        _vSortField(root, pstFd);
    vSubsetSort(*root, pstExm, 1);
    return RC_SUCC;
}

/*************************************************************************************************
    description：compare data by field
    parameters:
        s                          --data 
        p                          --to be compared
        pstCond                    --field list
        uNice                      --next index of field
    return:
        TRUE                       --success
        FALSE                      --failure
 *************************************************************************************************/
bool    bCompare(void *s, void *p, FdCond *pstCond, uint uNice)
{
    FdKey   *pstKey;

    if(pstCond->uFldcmp <= uNice)
        return TRUE;

    pstKey = &pstCond->stFdKey[uNice];
    if(pstKey->uDecorate & ORDER_ASC)
    {
        switch(pstKey->uDecorate & 0x0f)
        {
        case FIELD_DOUB:
            switch(pstKey->uFldlen)
            {
            case    4:
                if(*((float *)(s + pstKey->uFldpos)) > *((float *)(p + pstKey->uFldpos)))
                    return TRUE;
                else if(*((float *)(s + pstKey->uFldpos)) == *((float *)(p + pstKey->uFldpos)))
                    return bCompare(s, p, pstCond, ++ uNice);
                else
                    return FALSE;
            case    8:
                if(*((double *)(s + pstKey->uFldpos)) > *((double *)(p + pstKey->uFldpos)))
                    return TRUE;
                else if(*((float *)(s + pstKey->uFldpos)) == *((float *)(p + pstKey->uFldpos)))
                    return bCompare(s, p, pstCond, ++ uNice);
                else
                    return FALSE;
            default:
                return FALSE;
            }
            break;
        case FIELD_LONG:
            switch(pstKey->uFldlen)
            {
            case    2:
                if(*((sint *)(s + pstKey->uFldpos)) > *((sint *)(p + pstKey->uFldpos)))
                    return TRUE;
                else if(*((sint *)(s + pstKey->uFldpos)) == *((sint *)(p + pstKey->uFldpos)))
                    return bCompare(s, p, pstCond, ++ uNice);
                else
                    return FALSE;
            case    4:
                if(*((int *)(s + pstKey->uFldpos)) > *((int *)(p + pstKey->uFldpos)))
                    return TRUE;
                else if(*((int *)(s + pstKey->uFldpos)) == *((int *)(p + pstKey->uFldpos)))
                    return bCompare(s, p, pstCond, ++ uNice);
                else
                    return FALSE;
            case    8:
                if(*((llong *)(s + pstKey->uFldpos)) > *((llong *)(p + pstKey->uFldpos)))
                    return TRUE;
                else if(*((llong *)(s + pstKey->uFldpos)) == *((llong *)(p + pstKey->uFldpos)))
                    return bCompare(s, p, pstCond, ++ uNice);
                else
                    return FALSE;
            default:
                return FALSE;
            }
            break;
        case FIELD_CHAR:
            if(0 < memcmp(s + pstKey->uFldpos, p + pstKey->uFldpos, pstKey->uFldlen))
                return TRUE;
            else if(0 == memcmp(s + pstKey->uFldpos, p + pstKey->uFldpos, pstKey->uFldlen))
                return bCompare(s, p, pstCond, ++ uNice);
            else
                return FALSE;
            break;
        default:
            break;
        }
    }
    else if(pstKey->uDecorate & ORDER_DESC)
    {
        switch(pstKey->uDecorate & 0x0f)
        {
        case FIELD_DOUB:
            switch(pstKey->uFldlen)
            {
            case    4:
                if(*((float *)(s + pstKey->uFldpos)) < *((float *)(p + pstKey->uFldpos)))
                    return TRUE;
                else if(*((float *)(s + pstKey->uFldpos)) == *((float *)(p + pstKey->uFldpos)))
                    return bCompare(s, p, pstCond, ++ uNice);
                else
                    return FALSE;
            case    8:
                if(*((double *)(s + pstKey->uFldpos)) < *((double *)(p + pstKey->uFldpos)))
                    return TRUE;
                else if(*((float *)(s + pstKey->uFldpos)) == *((float *)(p + pstKey->uFldpos)))
                    return bCompare(s, p, pstCond, ++ uNice);
                else
                    return FALSE;
            default:
                return FALSE;
            }
            break;
        case FIELD_LONG:
            switch(pstKey->uFldlen)
            {
            case    2:
                if(*((sint *)(s + pstKey->uFldpos)) < *((sint *)(p + pstKey->uFldpos)))
                    return TRUE;
                else if(*((sint *)(s + pstKey->uFldpos)) == *((sint *)(p + pstKey->uFldpos)))
                    return bCompare(s, p, pstCond, ++ uNice);
                else
                    return FALSE;
            case    4:
                if(*((int *)(s + pstKey->uFldpos)) < *((int *)(p + pstKey->uFldpos)))
                    return TRUE;
                else if(*((int *)(s + pstKey->uFldpos)) == *((int *)(p + pstKey->uFldpos)))
                    return bCompare(s, p, pstCond, ++ uNice);
                else
                    return FALSE;
            case    8:
                if(*((llong *)(s + pstKey->uFldpos)) < *((llong *)(p + pstKey->uFldpos)))
                    return TRUE;
                else if(*((llong *)(s + pstKey->uFldpos)) == *((llong *)(p + pstKey->uFldpos)))
                    return bCompare(s, p, pstCond, ++ uNice);
                else
                    return FALSE;
            default:
                return FALSE;
            }
            break;
        case FIELD_CHAR:
            if(0 > memcmp(s + pstKey->uFldpos, p + pstKey->uFldpos, pstKey->uFldlen))
                return TRUE;
            else if(0 == memcmp(s + pstKey->uFldpos, p + pstKey->uFldpos, pstKey->uFldlen))
                return bCompare(s, p, pstCond, ++ uNice);
            else
                return FALSE;
            break;
        default:
            break;
        }
    }

    return bCompare(s, p, pstCond, ++ uNice);    
}

/*************************************************************************************************
    description：QSort list row
    parameters:
        pvData                     --data 
        low                        --low water
        high                       --high water
        lTruck                     --the length of truck
        pstCond                    --field list
        pvKey                      --next index of field
    return:
 *************************************************************************************************/
void    vQsortRow(void *pvData, long low, long high, size_t lTurck, FdCond *pstCond, void *pvKey)
{
    long  first = low, last = high;

    if(low >= high)
        return;

    memcpy(pvKey, pvData + first * lTurck, lTurck);
    while(first < last)
    {
        while(first < last && bCompare(pvData + last * lTurck, pvKey, pstCond, 0))
            -- last;

        memcpy(pvData + first * lTurck, pvData + last * lTurck, lTurck);

        while(first < last && bCompare(pvKey, pvData + first * lTurck, pstCond, 0))
            ++ first;
        memcpy(pvData + last * lTurck, pvData + first * lTurck, lTurck);
    }

    memcpy(pvData + first * lTurck, pvKey, lTurck);
    vQsortRow(pvData, low, first - 1, lTurck, pstCond, pvKey);
    vQsortRow(pvData, first + 1, high, lTurck, pstCond, pvKey);
}

/*************************************************************************************************
    description：sort rowlist
    parameters:
        pstSavm                    --stvm handle 
        lRow                       --rows
        pvData                     --data list
        lTruck                     --the length of truck
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lSortRowList(SATvm *pstSavm, size_t lRow, void *pvData, size_t lTruck)
{
    void    *pvKey;
    FdCond  *pstCond = &pstSavm->stUpdt;

    if(!bSetCondAttr(pstCond, pstSavm->tblName, ORDER_ASC | ORDER_DESC))
        return RC_SUCC;

    if(NULL == (pvKey = (char *)malloc(lTruck)))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    vQsortRow(pvData, 0, (long)(lRow - 1), lTruck, pstCond, pvKey);
    TFree(pvKey);
    return RC_SUCC;
}

/*************************************************************************************************
    description：Collapse the rowgrp node to buffer.
    parameters:
        root                       --rowgrp root
    return:
 *************************************************************************************************/
void    _vConvRowList(Rowgrp *root, long lParant, void *pszBuffer, size_t *plOffset)
{
    Rowgrp  *node;

    for(node = root; NULL != node; node = node->pstNext)
    {   
        memcpy(pszBuffer + *plOffset, &lParant, sizeof(long));        *plOffset += sizeof(long);
        memcpy(pszBuffer + *plOffset, &node->lLen, sizeof(long));     *plOffset += sizeof(long);
        memcpy(pszBuffer + *plOffset, &node->lCount, sizeof(size_t)); *plOffset += sizeof(size_t);
        memcpy(pszBuffer + *plOffset, node->psvData, node->lLen);     *plOffset += node->lLen;

        _vConvRowList(node->pstSSet, node->lIdx, pszBuffer, plOffset);
    }   
}

/*************************************************************************************************
    description：Collapse the rowgrp
    parameters:
        root                       --rowgrp root
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
 *************************************************************************************************/
long    lConvRowList(SATvm *pstSavm, Rowgrp *root, size_t *plOut, void **ppsvOut)
{
    Rowgrp  *node;
    size_t  lOffset = 0;

    if(!root)
    {   
        *plOut = 0;
        return RC_SUCC;
    }   

    if(!ppsvOut || !plOut)
    {   
        pstSavm->m_lErrno = CONDIT_IS_NIL;
        return RC_FAIL;
    }

    for(node = root; NULL != node->pstNext; node = node->pstNext);

    *plOut = (node->lIdx + 1) * (sizeof(long) * 2 + sizeof(size_t) + node->lLen);
    if(NULL == (*ppsvOut = (char *)calloc(1, *plOut)))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        return RC_FAIL;
    }

    _vConvRowList(root, 0, *ppsvOut, &lOffset);

    return RC_SUCC;
}

/*************************************************************************************************
    description：find node from the rowgrp list by idx
    parameters:
        root                      --rowgrp root
        idx                       --idx of node
    return:
        node                      --rowgrp node
 *************************************************************************************************/
Rowgrp* pFindRowList(Rowgrp *root, long idx)
{
    Rowgrp *node, *list;

    for(node = root; NULL != node; node = node->pstNext)
    {
        if(node->lIdx == idx)
            return node;

        if(NULL != (list = pFindRowList(node->pstSSet, idx)))
            return list;
    }

    return NULL;
}

/*************************************************************************************************
    description：Collapse the buffer to rowgrp list
    parameters:
        pstSavm                    --stvm handle
        pszBuffer                  --buffer 
        lData                      --the length of buffer
        root                      --rowgrp root
    return:
 *************************************************************************************************/
long    lParsRowList(SATvm *pstSavm, void *pszBuffer, long lData, Rowgrp **root)
{
    Rowgrp row, *node;
    long   lOffset, idx;

    for(lOffset = 0; lOffset < lData; lOffset += row.lLen)
    {
        memcpy(&idx, pszBuffer + lOffset, sizeof(long));          lOffset += sizeof(long);
        memcpy(&row.lLen, pszBuffer + lOffset, sizeof(long));     lOffset += sizeof(long);
        memcpy(&row.lCount, pszBuffer + lOffset, sizeof(size_t)); lOffset += sizeof(size_t);
        if(0 == idx)
        {
            if(NULL == (*root = (Rowgrp *)pInsertRowgrp(pstSavm, *root, NULL, NULL, 
                pszBuffer + lOffset, row.lLen, row.lCount)))
            {
                TFgrp(*root);
                return RC_FAIL;
            }

            continue;
        }

        if(NULL == (node = pFindRowList(*root, idx)))
            continue;

        if(NULL == (node = pInsertRowgrp(pstSavm, node, NULL, NULL, pszBuffer + lOffset,
            row.lLen, row.lCount)))
        {
            TFgrp(*root);
            return RC_FAIL;
        }
    }

    return RC_SUCC;
}

/****************************************************************************************
    code end
 ****************************************************************************************/
