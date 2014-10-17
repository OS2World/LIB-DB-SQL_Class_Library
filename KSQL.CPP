/*  Copyright (c) 1995 IBK-Landquart-Switzerland. All rights reserved.
 *
 *  Module      :  KSQL.CPP
 *  Application :  SQL class library
 *  Purpose     :  SQL virtual class implemention
 *  Author      :  Peter Koch, IBK
 *
 *  Date        Description                                 Who
 *  --------------------------------------------------------------------------
 *  June 1995   First release                               P.Koch, IBK
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. All advertising materials mentioning features or use of this software
 *     must display the following acknowledgement:
 *       This product includes software developed by 
 *       Informatik-Buero Koch (IBK) and its contributors.
 *  4. Neither the name of Informatik-Buero Koch (IBK), nor the names of its
 *     contributors may be used to endorse or promote products derived from 
 *     this software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 */
#include <itrace.hpp>
#include "ksql.hpp"

IEXCLASSIMPLEMENT(KSqlError,IException);

static char sqlErr0[] = "SQL class-library logical error:";

#define KSQLTHROW1(col,htype,val) \
{\
    KSqlError exc(sqlErr0);\
    exc.setText(IString("select-column ")+selectColumnName(col)+\
                " value("+IString(val)+") does not fit into <"+htype+">");\
    ITHROW(exc);\
} // KSQLTHROW1

/*
 *  K S q l C u r s o r : : ~ K S q l C u r s o r
 *
 *  Destructor for virtual SQL cursor class
 */
KSqlCursor::~KSqlCursor()
{
    IFUNCTRACE_DEVELOP();
} // KSqlCursor::~KSqlCursor

/*
 *  K S q l C u r s o r : : n e w P a r s e R e q u i r e d
 *
 *  Check if new parse should be performed
 */
int KSqlCursor::newParseRequired()
{
    IFUNCTRACE_DEVELOP();
    return 0;
} // KSqlCursor::newParseRequired

/*
 *  K S q l L i n k : : K S q l L i n k
 *
 *  Constructor for virtual SQL link class
 */
KSqlLink::KSqlLink(short int Max)
{
    IFUNCTRACE_DEVELOP();
    maxCursor = Max;
    csrCount = 0;
    csrPool = new KSqlCursor*[Max];
} // KSqlLink::KSqlLink

/*
 *  K S q l L i n k : : ~ K S q l L i n k
 *
 *  Destructor for virtual SQL link class
 */
KSqlLink::~KSqlLink()
{
    IFUNCTRACE_DEVELOP();
    for (int i = 0; i < csrCount; i++) delete csrPool[i];
    delete [] csrPool;
} // KSqlLink::~KSqlLink

/*
 *  K S q l L i n k : : g e t C u r s o r
 *
 *  Verify or get cursor from pool
 */
int _Export KSqlLink::getCursor(KSqlCursor **csr)
{
    int         ret, prio, old, curr;
    KSqlCursor  *c;

    IFUNCTRACE_DEVELOP();
    // search for active cursor
    for (curr = 0; curr < csrCount; curr++) {
        c = csrPool[curr];
        if ( csr == c->owner ) {
            ret = c->newParseRequired();
            goto gotcursor;
        } /* if */
        if (!c->prio) old = curr; // remember least busy cursor
    } /* for */

    // check if new cursor may be opened
    if (csrCount < maxCursor) {
        csrPool[csrCount] = allocCursor();
        csrPool[curr = csrCount]->prio = csrCount++;
    } else
        curr = old;

    // save owner signature and return SqlCursor ptr
    ret = 1;
    c = *csr = csrPool[curr];
    c->owner = csr;

    gotcursor:
    // mark this cursor most busy 
    if ( c->prio < csrCount-1) {
        prio = c->prio;
        c->prio = csrCount;
        for (curr = 0; curr < csrCount; curr++) {
            c = csrPool[curr];
            if (c->prio >= prio) c->prio--;
        } // for
    } // if

    return ret;
} // KSqlLink::getCursor

/*
 *  K S q l C u r s o r : : o p e r a t o r < < ( < n u m e r i c > )
 *
 *  bind numeric values
 */
#define DEFSHIFTLEFTNUM(VarType,DestType) \
KSqlCursor& _Export KSqlCursor::operator<<(VarType val)\
{\
    IFUNCTRACE_DEVELOP();\
    bindValue((DestType)val);\
    return *this;\
}
DEFSHIFTLEFTNUM(char, long int)
DEFSHIFTLEFTNUM(signed char, long int)
DEFSHIFTLEFTNUM(unsigned char, long int)
DEFSHIFTLEFTNUM(short int, long int)
DEFSHIFTLEFTNUM(unsigned short int, long int)
DEFSHIFTLEFTNUM(long int, long int)
DEFSHIFTLEFTNUM(unsigned long int, long int)
DEFSHIFTLEFTNUM(int, long int)
DEFSHIFTLEFTNUM(unsigned int, long int)
DEFSHIFTLEFTNUM(float, double)
DEFSHIFTLEFTNUM(double, double)

/*
 *  K S q l C u r s o r : : o p e r a t o r < < ( < c h a r a c t e r > )
 *
 *  bind char values
 */
#define DEFSHIFTLEFTCHAR(VarType) \
KSqlCursor& _Export KSqlCursor::operator<<(VarType val)\
{\
    IFUNCTRACE_DEVELOP();\
    IString tmp((char *)val);\
    bindValue(tmp);\
    return *this;\
}
DEFSHIFTLEFTCHAR(char *)
DEFSHIFTLEFTCHAR(signed char *)
DEFSHIFTLEFTCHAR(unsigned char *)

/*
 *  K S q l C u r s o r : : o p e r a t o r < < ( I S t r i n g )
 *
 *  bind IString values
 */
KSqlCursor& _Export KSqlCursor::operator<<(IString val)
{
    IFUNCTRACE_DEVELOP();
    bindValue(val);
    return *this;
} // KSqlCursor::operator<<(IString val)

/*
 *  K S q l C u r s o r : : o p e r a t o r > > ( < n u m e r i c > )
 *
 *  fetch numeric value
 */
#define DEFSHIFTRIGHTNUM(VarType,VarName,DestType) \
KSqlCursor& _Export KSqlCursor::operator>>(VarType& val)\
{\
    IFUNCTRACE_DEVELOP();\
    DestType dst;\
    int col = fetchValue(dst);\
    val=dst;\
    if (val!=dst)\
        KSQLTHROW1(col,VarName,dst);\
    return *this;\
}
DEFSHIFTRIGHTNUM(char,"char",long int)
DEFSHIFTRIGHTNUM(signed char,"signed char",long int)
DEFSHIFTRIGHTNUM(unsigned char,"unsigned char",long int)
DEFSHIFTRIGHTNUM(short int,"short int",long int)
DEFSHIFTRIGHTNUM(unsigned short int,"unsigned short int",long int)
DEFSHIFTRIGHTNUM(long int,"long int",long int)
DEFSHIFTRIGHTNUM(unsigned long int,"unsigned long int",long int)
DEFSHIFTRIGHTNUM(int,"int",long int)
DEFSHIFTRIGHTNUM(unsigned int,"unsigned int",long int)
DEFSHIFTRIGHTNUM(float,"float",double)
DEFSHIFTRIGHTNUM(double,"double",double)

/*
 *  K S q l C u r s o r : : o p e r a t o r > > ( c h a r   * )
 *
 *  fetch (char *) value
 */
#define DEFSHIFTRIGHTCHAR(VarType) \
KSqlCursor& _Export KSqlCursor::operator>>(VarType *val)\
{\
    IFUNCTRACE_DEVELOP();\
    IString str;\
    fetchValue(str);\
    strcpy((char *)val,str);\
    return *this;\
}
DEFSHIFTRIGHTCHAR(char)
DEFSHIFTRIGHTCHAR(signed char)
DEFSHIFTRIGHTCHAR(unsigned char)

/*
 *  K S q l C u r s o r : : o p e r a t o r > > ( I S t r i n g   & v a l )
 *
 *  fetch (IString) value
 */
KSqlCursor& _Export KSqlCursor::operator>>(IString &val)
{
    IFUNCTRACE_DEVELOP();
    fetchValue(val);
    return *this;
} // KSqlCursor& operator>>(IString&)
