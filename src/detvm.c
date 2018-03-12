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

/*************************************************************************************************
    macro
 *************************************************************************************************/
#define     DEBUG_HEAD_INFO     1
#define     DEBUG_UNIQ_IDEX     2
#define     DEBUG_GROP_IDEX     4
#define     DEBUG_GROP_LIST     8
#define     DEBUG_DATA_LIST     16  
#define     DEBUG_IDEX_TREE     32
#define     DEBUG_IDEX_DATA     64
#define     DEBUG_IDEX_DALL     128
#define     DEBUG_ALLD_INFO     (DEBUG_HEAD_INFO|DEBUG_UNIQ_IDEX|DEBUG_GROP_IDEX|DEBUG_GROP_LIST|DEBUG_DATA_LIST)

/*************************************************************************************************
   function
 *************************************************************************************************/
extern void    vPrintHex(char *s, long lIdx, bool bf);

/*************************************************************************************************
    description：debug tree node
    parameters:
        pvData                     --memory address
        pstTree                    --tree node
    return:
  *************************************************************************************************/
void    vDebugTree(void *pvData, SHTree *pstTree)
{

    if(!pvData || !pstTree) return ;

    fprintf(stderr, "DugTree:%p-SePos:[%8ld], Idx(%ld):[%15s](%2ld), Color[%ld], lSePos:[%4ld], "
        "lParent[%4ld], left[%4ld], right[%4ld]\n" , (char *)pstTree, (void *)pstTree - pvData,
        pstTree->m_lIdx, pstTree->m_szIdx, pstTree->m_lData, pstTree->m_eColor, pstTree->m_lSePos,
        pstTree->m_lParent, pstTree->m_lLeft, pstTree->m_lRight);

    if(SELF_POS_UNUSE == pstTree->m_lSePos || NODE_NULL == pstTree->m_lSePos)
        return ;

    if(NODE_NULL != pstTree->m_lLeft)
        vDebugTree(pvData, (SHTree *)pGetNode(pvData, pstTree->m_lLeft));

    if(NODE_NULL != pstTree->m_lRight)
        vDebugTree(pvData, (SHTree *)pGetNode(pvData, pstTree->m_lRight));
}

/*************************************************************************************************
    description：debug table
    parameters:
        pvData                     --memory address
        pstTree                    --tree node
    return:
  *************************************************************************************************/
void    vDebugTable(TABLE t, long eType)
{
    long    i = 0, j = 0;
    RunTime    *pstRun = NULL;
    TblKey  *pstKey = NULL;
    SHTree  *pstTree = NULL;
    SHList  *pstList = NULL;
    SHTruck *pstTruck = NULL;
    TIndex  *pstIndex = NULL;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

    if(NULL == (pstSavm = (SATvm *)pInitSATvm(t)))
    {
        fprintf(stderr, "initial table (%d) failed\n", t);
        return ;
    }

    if(NULL == (pstRun = pInitHitTest(pstSavm, pstSavm->tblName)))
    {
        fprintf(stderr, "hit test table (%d) failed, err:(%d)(%s)\n", t,pstSavm->m_lErrno, 
            sGetTError(pstSavm->m_lErrno));
        return ;
    }

    if(eType & DEBUG_HEAD_INFO)
    {
        fprintf(stdout, "\n==========================================TABLE HEAND INFO============="
        "============================\n");
        fprintf(stdout, "TABLE:%d, NAME:%s\tSHTree(%ld),SHList(%ld),TblDef(%ld)\n"
            "extern:%ld, Group:%ld, MaxRow:%ld, Valid:%ld, lNodeNil:%ld, lIType:%d, "
            "Table:%ld\nIdxLen:%ld, TreePos:%ld,  TreeRoot:%ld, GrpLen:%ld, GroupPos:%ld, "
            "GroupRoot:%ld\nListPos:%ld, ListOfs:%ld, Data:%ld, ReSize:%ld, Truck:%ld\n", 
            ((TblDef *)pGetTblDef(t))->m_table, ((TblDef *)pGetTblDef(t))->m_szTable,
            sizeof(SHTree), sizeof(SHList), sizeof(TblDef), ((TblDef *)pGetTblDef(t))->m_lExtern, 
            ((TblDef *)pGetTblDef(t))->m_lGroup, ((TblDef *)pGetTblDef(t))->m_lMaxRow, 
            ((TblDef *)pGetTblDef(t))->m_lValid, ((TblDef *)pGetTblDef(t))->m_lNodeNil,
            ((TblDef *)pGetTblDef(t))->m_lIType, 
            ((TblDef *)pGetTblDef(t))->m_lTable, ((TblDef *)pGetTblDef(t))->m_lIdxLen, 
            ((TblDef *)pGetTblDef(t))->m_lTreePos, ((TblDef *)pGetTblDef(t))->m_lTreeRoot,
            ((TblDef *)pGetTblDef(t))->m_lGrpLen, ((TblDef *)pGetTblDef(t))->m_lGroupPos, 
            ((TblDef *)pGetTblDef(t))->m_lGroupRoot, ((TblDef *)pGetTblDef(t))->m_lListPos,
            ((TblDef *)pGetTblDef(t))->m_lListOfs, ((TblDef *)pGetTblDef(t))->m_lData, 
            ((TblDef *)pGetTblDef(t))->m_lReSize, ((TblDef *)pGetTblDef(t))->m_lTruck);

        pstTree = &((TblDef *)pGetTblDef(t))->m_stNil;
        fprintf(stdout, ">>NODE_NULL POS:[%8ld], Idx:[%s](%ld)(%ld), Color[%ld], lSePos:[%4ld], lParent[%4ld]"
            ", left[%4ld], right[%4ld]\n" , (void *)pstTree - (void *)pGetTblDef(t), pstTree->m_szIdx, 
            pstTree->m_lIdx, pstTree->m_lData, pstTree->m_eColor, pstTree->m_lSePos, pstTree->m_lParent, 
            pstTree->m_lLeft, pstTree->m_lRight); 

        fprintf(stdout, "==========UNIQ INDEX FIELD=========\n");
        for(i = 0, pstKey = pGetTblIdx(t); i < lGetIdxNum(t); i ++)
        {
            fprintf(stdout, "From:%4ld, len:%3ld, attr:%ld, IsPk:%ld, field:%s\n", pstKey[i].m_lFrom,
                pstKey[i].m_lLen, pstKey[i].m_lAttr, pstKey[i].m_lIsPk, pstKey[i].m_szField);
        }

        fprintf(stdout, "==========GROUP INDEX FIELD========\n");
        for(i = 0, pstKey = pGetTblGrp(t); i < lGetGrpNum(t); i ++)
        {
            fprintf(stdout, "From:%4ld, len:%3ld, attr:%ld, IsPk:%ld, field:%s\n", pstKey[i].m_lFrom,
                pstKey[i].m_lLen, pstKey[i].m_lAttr, pstKey[i].m_lIsPk, pstKey[i].m_szField);
        }

        fprintf(stdout, "==================== TABLE FIELD ====================\n");
        for(i = 0, pstKey = pGetTblKey(t); i < lGetFldNum(t); i ++)
        {
            fprintf(stdout, "From:%4ld, len:%3ld, attr:%ld, IsPk:%ld, field:%s\n", pstKey[i].m_lFrom,
                pstKey[i].m_lLen, pstKey[i].m_lAttr, pstKey[i].m_lIsPk, pstKey[i].m_szField);
        }
    }                

    if(eType & DEBUG_UNIQ_IDEX)
    {
        fprintf(stdout, "\n===================================UNIQUE_INDEX====================="
            "==============lValid(%ld)=========================\n", lGetTblValid(t));
        if(eType & DEBUG_IDEX_DALL)
        {
            for(i = 0; i < lGetTblRow(t); i ++)
            {
                pstTree = (SHTree *)(pstRun->m_pvAddr + lGetIdxPos(t) + i * sizeof(SHTree));
                vPrintHex(pstTree->m_szIdx, pstTree->m_lIdx, 0);
                fprintf(stdout, "NODE:[%ld](%02ld), Idx:[%4s](%ld)(%2ld), Color[%ld], lSePos:[%4ld], "
                    "lParent[%4ld], left[%4ld], right[%4ld]\n" , (void *)pstTree - pstRun->m_pvAddr, 
                     i, pstTree->m_szIdx, pstTree->m_lIdx, pstTree->m_lData, pstTree->m_eColor,
                    pstTree->m_lSePos, pstTree->m_lParent, pstTree->m_lLeft, pstTree->m_lRight);
            }
        }
        else
        {
            for(i = 0; i < lGetTblValid(t); i ++)
            {
                pstTree = (SHTree *)(pstRun->m_pvAddr + lGetIdxPos(t) + i * sizeof(SHTree));
                if(SELF_POS_UNUSE == pstTree->m_lSePos || NODE_NULL == pstTree->m_lSePos)
                    continue;

                vPrintHex(pstTree->m_szIdx, pstTree->m_lIdx, 0);
                fprintf(stdout, "NODE:[%6ld]->(%02ld), Idx:[%15s](%6ld), Color[%ld], lSePos:[%4ld], "
                    "lParent[%4ld], left[%4ld], right[%4ld]\n" , (void *)pstTree - pstRun->m_pvAddr, i, 
                    pstTree->m_szIdx, pstTree->m_lData, pstTree->m_eColor, pstTree->m_lSePos,
                    pstTree->m_lParent, pstTree->m_lLeft, pstTree->m_lRight);
            }
        }
    }

    if(eType & DEBUG_GROP_IDEX)
    {
        fprintf(stdout, "\n===================================INDEX_GROUP====================="
            "==============Valid:%ld, Group:%ld================\n", lGetTblValid(t), lGetTblGroup(t));
        if(eType & DEBUG_IDEX_DALL)
        {
            for(i = 0; i < lGetTblRow(t); i ++)
            {
                pstTree = (SHTree *)(pstRun->m_pvAddr + lGetGrpPos(t) + i * sizeof(SHTree));
                vPrintHex(pstTree->m_szIdx, pstTree->m_lIdx, 0);
                fprintf(stdout, "NODE:[%ld](%02ld), Idx:[%4s](%ld)(%2ld), Color[%ld], lSePos:[%4ld],"
                    " lParent[%4ld], left[%4ld], right[%4ld]\n" , (void *)pstTree - pstRun->m_pvAddr, 
                    i, pstTree->m_szIdx, pstTree->m_lIdx, pstTree->m_lData, pstTree->m_eColor,
                    pstTree->m_lSePos, pstTree->m_lParent, pstTree->m_lLeft, pstTree->m_lRight);
            }
        }
        else
        {
            for(i = 0; i < lGetTblValid(t); i ++)
            {
                pstTree = (SHTree *)(pstRun->m_pvAddr + lGetGrpPos(t) + i * sizeof(SHTree));
                if(SELF_POS_UNUSE == pstTree->m_lSePos || NODE_NULL == pstTree->m_lSePos)
                    continue;
            
                vPrintHex(pstTree->m_szIdx, pstTree->m_lIdx, 0);
                fprintf(stdout, "NODE:[%ld](%02ld), Idx:[%4s](%ld)(%2ld), Color[%ld], lSePos:[%4ld],"
                    " lParent[%4ld], left[%4ld], right[%4ld]\n" , (void *)pstTree - pstRun->m_pvAddr, 
                    i, pstTree->m_szIdx, pstTree->m_lIdx, pstTree->m_lData, pstTree->m_eColor,
                    pstTree->m_lSePos, pstTree->m_lParent, pstTree->m_lLeft, pstTree->m_lRight);
            }
        }
    }

    if(eType & DEBUG_GROP_LIST)
    {
        fprintf(stdout, "\n=================================INDEX_LIST========================"
            "==============Valid(%ld)=============\n", lGetTblValid(t));
        if(eType & DEBUG_IDEX_DALL)
        {
            for(i = 0, j = lGetListOfs(t); i < lGetTblRow(t); i ++)
            {
                pstList = (SHList *)(pstRun->m_pvAddr + j + i * sizeof(SHList));
                fprintf(stdout, "LIST:[%8ld][%02ld], lSePos:[%4ld], lData[%8ld], Node[%8ld], "
                    "Next[%8ld], Last[%8ld]\n" , (void *)pstList - pstRun->m_pvAddr, i, 
                    pstList->m_lPos, pstList->m_lData, pstList->m_lNode, pstList->m_lNext, 
                    pstList->m_lLast);
            }
        }
        else
        {
            for(i = 0, j = lGetListOfs(t); i < lGetTblValid(t); i ++)
            {
                pstList = (SHList *)(pstRun->m_pvAddr + j + i * sizeof(SHList));
                if(SELF_POS_UNUSE == pstList->m_lPos)
                    continue;
            
                fprintf(stdout, "LIST:[%8ld][%02ld], lSePos:[%4ld], lData[%8ld], Node[%8ld], "
                    "Next[%8ld], Last[%8ld]\n" , (void *)pstList - pstRun->m_pvAddr, i, 
                    pstList->m_lPos, pstList->m_lData, pstList->m_lNode,
                    pstList->m_lNext, pstList->m_lLast);
            }
        }
    }

    if(eType & DEBUG_IDEX_TREE)
    {
        fprintf(stdout, "\n=================================TREE DEUBG========================"
            "==============Valid(%ld)=============\n", lGetTblValid(t));
        vDebugTree(pstRun->m_pvAddr, pGetNode(pstRun->m_pvAddr, lGetIdxRoot(t)));
    }

    if(eType & DEBUG_IDEX_DATA)
    {
        fprintf(stdout, "\n===================================UNIQUE_INDEX====================="
            "==============lValid(%ld)=========================\n", lGetTblValid(t));
        for(i = 0; i < lGetTblRow(t); i ++)
        {
            pstTruck = (void *)(pstRun->m_pvAddr + lGetTblData(t) + i * lGetRowTruck(t));
            fprintf(stdout, "SePos[%ld]\n", (void *)pstTruck - pstRun->m_pvAddr);
            vPrintHex(pstTruck->m_pvData, lGetRowSize(t), 0);
        }
    }

    vTblDisconnect(pstSavm, pstSavm->tblName);
    fprintf(stdout, "=========================================================================="
        "===================================\n");
}

/*************************************************************************************************
    description：get action
    parameters:
    return:
  *************************************************************************************************/
void    vGetAction(char *s, long *plAction)
{
    int     i, nLen;

    if(!s)    return ;

    for(i = 0, nLen = strlen(s); i < nLen; i ++)
    {
        switch(s[i])
        {
        case 'h':
            *plAction |= DEBUG_HEAD_INFO;                
            break;
        case 'u':
            *plAction |= DEBUG_UNIQ_IDEX;                
            break;
        case 'g':
            *plAction |= DEBUG_GROP_IDEX;                
            break;
        case 'l':
            *plAction |= DEBUG_GROP_LIST;                
            break;
        case 'd':
            *plAction |= DEBUG_DATA_LIST;                
            break;
        case 't':
            *plAction |= DEBUG_IDEX_TREE;                
            break;
        case 'e':
            *plAction |= DEBUG_IDEX_DALL;                
            break;
        case 'a':
            *plAction |= DEBUG_IDEX_DATA;                
            break;
        default:
            break;
        }
    }
}

/*************************************************************************************************
    description：print func
    parameters:
    return:
  *************************************************************************************************/
void    vPrintFunc(char *s)
{
    fprintf(stdout, "\nUsage:\t%s -[tpu][hugldtui]\n", s);
    fprintf(stdout, "\t-t\t\t--table\n");
    fprintf(stdout, "\t-p[hugldta]\t--debug\n");
    fprintf(stdout, "\t-u(ui)\t\t--reset lock\n");
    fprintf(stdout, "\n");
}

/*************************************************************************************************
    description：debug action
    parameters:
    return:
  *************************************************************************************************/
void    vDebugAction(TABLE t, int iAction, char *pszApp, char *pszParam)
{
    long    lRet, lDebug = 0;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

    if(3 == iAction)
    {
        if(RC_SUCC != lResetLock((SATvm *)pGetSATvm(), t))
        {
            fprintf(stderr, "reset the table lock (%d) failed, err:(%d)(%s)\n", t,
                pstSavm->m_lErrno, sGetTError(pstSavm->m_lErrno));
            return ;
        }
    }
    else if(5 == iAction)
    {
        vGetAction(pszParam, &lDebug);
        vDebugTable(t, lDebug);
    }
    else
        vPrintFunc(pszApp);

    return ;
}

/*************************************************************************************************
    description：main
    parameters:
    return:
  *************************************************************************************************/
int     main(int argc, char *argv[])
{
    TABLE   t;
    char    szCom[256];
    int     iChoose = 0, iAction = 0;

    memset(szCom, 0, sizeof(szCom));
    while(-1 != (iChoose = getopt(argc, argv, "t:p:u::v?::")))
    {
        switch(iChoose)
        {
        case    't':
            iAction |= 1;
            t = atol(optarg);
            break;
        case    'u':
            if(!optarg)
                strcpy(szCom, "u");
            else
                strcpy(szCom, optarg);
            iAction |= 2;
            break;
        case    'p':
            iAction |= 4;
            strcpy(szCom, optarg);
            break;
        case    'v':
        case    '?':
        default:
            vPrintFunc(basename(argv[0]));
            return RC_FAIL;
        }
    }

    vDebugAction(t, iAction, basename(argv[0]), szCom);

    return RC_SUCC;
}

/*************************************************************************************************
    code end
  *************************************************************************************************/
