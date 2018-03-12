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

#include    "tstr.h"

/************************************************************************************************
    strs function
 ************************************************************************************************/
/*************************************************************************************************
    description：uppercase string 
    parameters:
        s                           --string
    return:
        s
 *************************************************************************************************/
char    *supper(char *s)
{
    long    i, l = strlen(s), fg = 0;

    for(i=0; i < l; i++)
        s[i] = toupper(s[i]);

    return s;
}

/*************************************************************************************************
    description：lowercas string 
    parameters:
        s                           --string
    return:
        s
 *************************************************************************************************/
char    *slower(char *s)
{
    long    i, l = strlen(s), fg = 0;

    for(i = 0; i < l; i++)
        s[i] = tolower(s[i]);

    return s;
}

/*************************************************************************************************
    description：drop CRLF from string
    parameters:
        p                           --string
    return:
        p
 *************************************************************************************************/
char*   strimcrlf(char *p)
{
    int     i = 0, j = 0, m = 0;
    char    *s = NULL;

    if(!p)           return p;
    if(!strlen(p))   return p;

    s = p;
    m = strlen(p);
    for(i = 0; i < m; i ++)
    {
        if(p[i] == 10 || p[i] == 13)
            continue;
        s[j] = p[i];
        j ++;
    }
    s[j] = 0x00;
    return s;
}

/*************************************************************************************************
    description：trim the left space from string
    parameters:
        p                           --string
    return:
        p
 *************************************************************************************************/
char*    sltrim(char *p)
{
    char    *s = p;
    long    i, k = 0, l;

    if(!p || (0 == (l = strlen(p))))
        return p;

    for(i = 0; i < l; i ++, k ++)
    {
        if(p[i] != ' ' && '\t' != p[i])
            break;
    }

    if(0 == k)    return p;

    for(i = 0; i < l - k; i ++)
        p[i] = s[i + k];
    p[i] = 0x00;

    return p;
}

/*************************************************************************************************
    description：trim the right space from string
    parameters:
        p                           --string
    return:
        p
 *************************************************************************************************/
char*    srtrim(char *p)
{
    long    i, k = 0, l = 0;

    if(!p || 0 == (l = strlen(p)))
        return p;

    for(i = l - 1; i >= 0; i --)
    {
        if(p[i] == ' ' || '\t' == p[i])
            continue;
        break;
    }

//    p[i + 1] = '\0';
    memset(p + i + 1, 0, l - i - 1);

    return p;
}

/*************************************************************************************************
    description：trimall space from string
    parameters:
        p                           --string
    return:
        p
 *************************************************************************************************/
char*    strimall(char *p)
{
    long    i, k = 0, l = 0;
    char    *q = p;

    if(!p || !strlen(p))
        return p;

    l = strlen(p);
    for(i = 0; i < l; i ++)
    {
        if(p[i] == ' ')
            continue;
        q[k ++] = p[i];
    }
    memset(q + k, 0, l - k);
    p = q;

    return q;
}

/*************************************************************************************************
    description：trim the left space from field string
    parameters:
        p                           --string
    return:
        p
 *************************************************************************************************/
char*   strimfield(char *s)
{
    register int    i, n, m;
    BOOL    bf = false;

    if(!s || 0 == (n = strlen(s)))
        return s;

    for(i = 0, m = 0; i < n; i ++)
    {
        if(s[i] == '\"')
            bf = !bf;

        if(bf)  s[m ++] = s[i];
        else if(s[i] != ' ' && s[i] != '\t')
            s[m ++] = s[i];
    }

    s[m] = 0x00;

    return s;
}

/*************************************************************************************************
    description：calcute the number of field string
    parameters:
        p                           --string
        k                           --string
    return:
        p
 *************************************************************************************************/
long    lfieldnum(char *p, char *k)
{
    char    *y = p;
    BOOL    bf = false;
    long    idx, i, m, n;

    if(!p || !k) return 0;

    for(i = 0, idx = 0, m = strlen(p), n = strlen(k); i < m; i ++)
    {
        if(p[i] == '\\')
        {
            i ++;
            continue;
        }

        if(p[i] == '\"' || p[i] == '\'')
            bf = !bf;

        if(bf)    continue;

        if(!memcmp(p + i, k, n))
        {
            ++ idx;
            for(i += n; i < m; i ++)
            {
                if(p[i] != ' ' && p[i] != '\t' && memcmp(p + i, k, n))
                    break;
            }

            y = p + i;
            i --;
        }
    }

    for(i = y - p ; i < m; i ++)
    {
        if(y[i] != ' ' && y[i] != '\t')
            return ++ idx;
    }

    return idx;
}

/*************************************************************************************************
    description：is or not gbk-Character set
    parameters:
        s                           --string
    return:
        1                           --yes
        0                           --no
 *************************************************************************************************/
int     bIsgbk(const char *s)
{
    if((ushar)s[0] > 0x80 && (ushar)s[1] >= 0x40)
        return 1;
    return 0;
}

/*************************************************************************************************
    description：Gets the field value of the specified character position
    parameters:
        p                           --string
        s                           --characters
        id                          --position
    return:
        char                        --values
 *************************************************************************************************/
char*    sfieldvalue(char *p, char *k, int id)
{
    char    *y = p;
    BOOL    bf = false;
    long    idx, i = 0, m = 0, n = 0;
    static  char    szOut[1024]; 
    
    memset(szOut, 0, sizeof(szOut));
    if(!p || !k || id <= 0) return szOut;
    
    for(i = 0, idx = 0, m = strlen(p), n = strlen(k); i < m; i ++)
    {
        if(p[i] == '\\')
        {
            i ++;
            continue;
        }   
        
        if(p[i] == '\"' || p[i] == '\'')
        {
            bf = !bf;
            continue;
        }   
        if(bf)  continue;
        
        if(!memcmp(p + i, k, n))
        {
            if(++idx == id)
                break;
                
            for(i += n; i < m; i ++)
            {
                if(p[i] != ' ' && p[i] != '\t' && memcmp(p + i, k, n))
                    break; 
            }       
            
            y = p + i;
            i --;
        }   
    }   
    
    if(idx + 1 < id) return szOut;
    memcpy(szOut, y, i - (long )(y - p));
    
    return szOut; 
}

/*************************************************************************************************
    description：Gets the number of specified characters
    parameters:
        p                           --string
        s                           --characters
    return:
        long                        --number
 *************************************************************************************************/
long    lgetstrnum(char *p, char *s)
{
    char    *q = p;
    long    i, m, n, k;

    if(!p || 0 == (n = strlen(s)) || 0 == (m = strlen(p)))
        return 0;

    for(i = 0, k = 0; i < m; i += n, q += n)
    {
        if(0 == memcmp(q, s, n))
            k ++;    
    }

    return k;
}

/*************************************************************************************************
    description：Gets the value of the specified character position
    parameters:
        p                           --string
        s                           --characters
        id                          --position
    return:
        char                        --values
 *************************************************************************************************/
char*   sgetvalue(char *p, char *s, int id)
{
    char    *y = p;
    long    i, m, n, idx = 0;
    static  char    szOut[1024];

    memset(szOut, 0, sizeof(szOut));
    if(!p || !s || id <= 0 || (n = strlen(s)) <= 0)
        return szOut;

    for(i = 0, idx = 0, m = strlen(p); i < m; i ++)
    {
        if(!memcmp(p + i, s, n))
        {
            if(i > 0 && bIsgbk(p + i - 1))
               continue;

            if(++ idx == id)
                break;

            y = p + i + n;
        }
    }

    if(idx + 1 < id) return szOut;
    memcpy(szOut, y, i - (int)(y - p));

    return szOut;
}

/*************************************************************************************************
    description：Characters to replace
    parameters:
        p                           --The original string
        o                           --key characters
        d                           --target character
    return:
        char                        --values
 *************************************************************************************************/
char*    sfieldreplace(char *p, char o, char d)
{
    char    *s = p;    
    bool    bf = false;
    long    idx, i, j = 0, m = 0;

    if(!p) return p;

    for(i = 0, idx = 0, m = strlen(p); i < m; i ++)
    {
        if(p[i] == '\"' || p[i] == '\'')
            bf = !bf;

        if(bf)
        {
            s[j ++] = p[i];
            continue;
        }

        if(o == p[i])
        {
            if(i > 0 && bIsgbk(p + i - 1))
                continue;

            p[i] = d;
        }

        if((j == 0 && ' ' == p[i]) || (j > 0 && ' ' == s[j - 1] && p[i] == ' '))
            continue;
        s[j ++] = p[i];
    }
    s[j] = 0x00;

    return s;
}

/*************************************************************************************************
    description：Get the value between o-d
    parameters:
        p                           --The original string
        o                           --key characters
        d                           --target character
    return:
        char*                       --values
 *************************************************************************************************/
char*   strimabout(char *s, char *o, char *d)
{
    long    l = 0;
    char    *p = NULL, *q = NULL, *m = NULL;

    if(!s || !o || !d)              return NULL;
    if(0 == (l = strlen(s)))        return NULL;
    if(NULL == (p = strstr(s, o)))  return NULL;

    for(l; l > 0; l --)
    {
        if(NULL != (q = strstr(s + l, d)))
            break;
    }
    if(!q)  return NULL;

    l = strlen(o);
    if((p - q) >= 0)    return NULL;

    if(((p - q + l) == 0))
    {
        s[0] = 0x00;
        return NULL;
    }

    for(m = s, p += l; p != q; *p ++, *m ++)
        *m = *p;
    *m = 0x00;

    return s;
}

/****************************************************************************************
    code end
****************************************************************************************/

