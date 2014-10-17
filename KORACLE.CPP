/*  Copyright (c) 1995 IBK-Landquart-Switzerland. All rights reserved.
 *
 *  Module      :  KORACLE.CPP
 *  Application :  SQL Class Library
 *  Purpose     :  ORACLE classes implemention
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
#include "koracle.hpp"

/*
 *  O R A C L E   O C I - f u n c t i o n s   ( O R A 7 O 3 2 . L I B )
 */
int _System orlon(          // log on
    KOraLDA *,                      // logon descriptor area
    unsigned char *,                // host data area
    char *,                         // username
    int,                            // size of username
    char *,                         // password
    int,                            // size of password
    int                             // audit
);

int _System ologof(         // log off
    KOraLDA *                       // logon descriptor area
);

int _System oerhms(         // get errormessage
    KOraLDA *,                      // logon descriptor area
    short int,                      // V7 error code
    char *,                         // text buffer
    int                             // size of buffer
);

int _System ocom(           // commit
    KOraLDA *                       // logon descriptor area
);

int _System orol(           // rollback
    KOraLDA *                       // logon descriptor area
);

int _System oopen(          // open cursor
    KOraCDA *,                      // cursor descriptor area
    KOraLDA *,                      // logon descriptor area
    char *,                         // database name (V2 compatibility)
    short int,                      // database name length
    short int,                      // area size
    char *,                         // logon string
    short int                       // logon string length
);

int _System oclose(         // close cursor
    KOraCDA *                       // cursor descriptor area
);

int _System ocan(           // cancel current operation
    KOraCDA *                       // cursor descriptor area
);

int _System oparse(         // parse statement
    KOraCDA *,                      // cursor descriptor area
    char *,                         // sql statement
    long int,                       // length of sql statement
    int,                            // deferred mode flag
    unsigned long int               // V6/V7 mode flag
);

int _System odescr(         // describe select items
    KOraCDA *,                      // cursor descriptor area
    int,                            // position #
    long int *,                     // max.size of column
    short int *,                    // datatype code
    char *,                         // column name
    long int *,                     // name size
    long int *,                     // display size
    short int *,                    // numeric precision
    short int *,                    // numeric scale
    short int *                     // NOT NULL column
);
 
int _System odefin(         // define output variable for select item
    KOraCDA *,                      // cursor descriptor area
    int,                            // position #
    unsigned char *,                // buffer address
    int,                            // buffer size
    int,                            // external datatype
    int,                            // packed dec.scale (unused)
    short int *,                    // NULL indicator var
    char *,                         // packed dec.fmt (unused)
    int,                            // length of above (unused)
    int,                            // packed dec.type (unused)
    unsigned short int *,           // length of fetched data
    unsigned short int *            // column return code
);

int _System obndrv(         // bind program var to sql placeholder
    KOraCDA *,                      // cursor descriptor area
    char *,                         // name of placeholder (":abc")
    int,                            // length of placeholder name
    unsigned char *,                // buffer address
    int,                            // buffer size
    int,                            // external datatype code
    int,                            // packed dec.scale (unused)
    short int *,                    // NULL indicator var
    char *,                         // packed dec.fmt (unused)
    int,                            // length of above (unused)
    int                             // packed dec.type (unused)
);

int _System oexn(           // execute statement
    KOraCDA *,                      // cursor descriptor area
    int,                            // # of rows
    int                             // rows offset
);

int _System ofen(           // fetch rows
    KOraCDA *,                      // cursor descriptor area
    int                             // # of rows
);

static char empty[] = "";
static char oraErr[]  = "ORACLE error (";
static char oraErr0[] = "ORACLE class-library logical error:";
static char bndErr1[]  = "bind value ";
static char bndErr2[]  = ") does not match to ";
static char invType[] = "internal error (invalid type-code supplied)";

static char *typeName[] = {
    "<varTypeInt8>",
    "<varTypeInt16>",
    "<varTypeInt32>",
    "<varTypeFloat32>",
    "<varTypeFloat64>",
    "<varTypeChar>",
    "<varTypeRaw>"
};

#define KORATHROW(lda,cda,sql,oci) \
{\
    KSqlError exc(oraErr);\
    GetOraError(lda,cda,sql,oci,exc);\
    ITHROW(exc);\
} // KORATHROW

#define KORATHROW0(txt) \
{\
    KSqlError exc(oraErr0);\
    exc.setText(txt);\
    ITHROW(exc);\
} // KORATHROW0

#define KORATHROW1() \
{\
    KSqlError exc(oraErr0);\
    exc.setText(sql);\
    exc.setText(bndErr1+col.name+" ("+\
                IString(val)+bndErr2+typeName[col.htype]);\
    ITHROW(exc);\
} // KORATHROW1

#define KORATHROW2() \
{\
    KSqlError exc(oraErr0);\
    exc.setText(sql);\
    exc.setText(bndErr1+col.name+" ("+\
                IString(val)+bndErr2+typeName[col.htype]+\
                "["+IString(col.hsize)+"]");\
    ITHROW(exc);\
} // KORATHROW2

/*
 *  G e t O r a E r r o r
 */
static KSqlError &GetOraError(
    KOraLDA         *lda,           // logon descriptor area
    KOraCDA         *cda,           // cursor descriptor area
    char            *sql,           // sql statement
    char            *oci,           // oci function name
    KSqlError       &exc)           // exception instance
{
    char    buf[1000], *b0, *b1;
    IString func;

    IFUNCTRACE_DEVELOP();
    if (cda) switch (cda->csrfc) {
        case  4: func="oexn";     break;
        case  8: func="odefin";   break;
        case 12: func="ofen";     break;
        case 14: func="oopen";    break;
        case 16: func="oclose";   break;
        case 28: func="obndrv";   break;
        case 52: func="ocan";     break;
        case 54: func="oparse";   break;
        case 60: func="odescr";   break;
        default: func="OCI#"+IString((int)cda->csrfc);
    } else
        if (oci) func = oci;
    exc.appendText( func+"):" );
    if (sql) exc.setText(sql);
    int err = cda ? cda->csrarc : lda->ldarc;
    *buf = 0;
    ITRACE_DEVELOP("oerhms");
    oerhms(lda, err, b0 = b1 = buf, sizeof(buf) );
    while (*b1 && *b1!='\r' && *b1!='\n') b1++;
    while (*b1=='\r' || *b1=='\n') *b1++ = 0;
    exc.setErrorId(err);
    exc.setText(b0);
    while (*b1) {
        b0 = b1;
        while (*b1 && *b1!='\r' && *b1!='\n') b1++;
        while (*b1=='\r' || *b1=='\n') *b1++ = 0;
        exc.setText((char *)b0);
    } // while
    return exc;
} // GetOraError

/*
 *  K O r a L i n k : : K O r a L i n k
 *
 *  Constructor for KOraLink
 */
_Export KOraLink::KOraLink(
    char        *name,
    char        *pass,
    short int   max ) :
    KSqlLink(max)
{
    IFUNCTRACE_DEVELOP();
    ITRACE_DEVELOP("orlon");
    if ( orlon(&lda, hda, name, -1, pass, -1, -1) )
        KORATHROW(&lda,NULL,NULL,"orlon");
} // KOraLink::KOraLink

/*
 *  K O r a L i n k : : ~ K O r a L i n k
 *
 *  Destructor for KOraLink
 */
KOraLink::~KOraLink()
{
    IFUNCTRACE_DEVELOP();
    // release all cursors before logoff
    for (int i = 0; i < csrCount; i++) delete csrPool[i];
    csrCount = 0;
    // now we can retire...
    ITRACE_DEVELOP("ologof");
    if (ologof( &lda )) 
        KORATHROW(&lda,NULL,NULL,"ologof");
} // KOraLink::~KOraLink

/*
 *  K O r a L i n k : : d a t a B a s e N a m e ( )
 *
 *  return database name
 */
char* KOraLink::dataBaseName()
{
    return "ORACLE";
} // KOraLink::dataBaseName

/*
 *  K O r a L i n k : : a l l o c C u r s o r
 *
 *  allocate a cursor
 */
KSqlCursor *KOraLink::allocCursor()
{
    IFUNCTRACE_DEVELOP();
    KOraCursor *csr = new KOraCursor;
    csr->lda = &lda;
    ITRACE_DEVELOP("oopen");
    if ( oopen( &csr->cda, &lda, NULL, -1, -1, NULL, -1 ) )
        KORATHROW(&lda,&csr->cda,NULL,NULL);
    return csr;
} // KOraLink::allocCursor

/*
 *  K O r a L i n k : : c o m m i t
 *
 *  commit changes
 */
void KOraLink::commit()
{
    IFUNCTRACE_DEVELOP();
    ITRACE_DEVELOP("ocom");
    if (ocom( &lda ))
        KORATHROW(&lda,NULL,NULL,"ocom");
} // KOraLink::commit

/*
 *  K O r a L i n k : : r o l l b a c k
 *
 *  rollback changes
 */
void KOraLink::rollback()
{
    IFUNCTRACE_DEVELOP();
    ITRACE_DEVELOP("orol");
    if (orol( &lda ))
        KORATHROW(&lda,NULL,NULL,"orol");
} // KOraLink::rollback

/*
 *  K O r a C u r s o r : : K O r a C u r s o r
 *
 *  constructor
 */
KOraCursor::KOraCursor()
{
    IFUNCTRACE_DEVELOP();
    memset( &cda, 0, sizeof(cda) );
} // KOraCursor::KOraCursor

/*
 *  K O r a C u r s o r : : ~K O r a C u r s o r
 *
 *  destructor
 */
KOraCursor::~KOraCursor()
{
    IFUNCTRACE_DEVELOP();
    ITRACE_DEVELOP("oclose");
    if ( oclose(&cda) )
        KORATHROW(lda,&cda,NULL,NULL);
} // KOraCursor::~KOraCursor

/*
 *  K O r a C u r s o r : : n e w P a r s e R e q u i r e d
 *
 *  Check if new parse should be performed
 */
int KOraCursor::newParseRequired()
{
    IFUNCTRACE_DEVELOP();
    // if ORACLE error occured on this cursor, force new parse
    return  (cda.csrarc && cda.csrarc!=1403) ? 1 : 0;
} // KOraCursor::newParesRequired

/*
 *  K O r a C u r s o r : : p a r s e
 *
 *  parse statement. bind- and select-column lists are cleared.
 */
KSqlCursor& KOraCursor::parse(
    char*           stmt,           // sql statement
    short int       bndarrsize,     // bind array size
    short int       selarrsize,     // select array size
    long int        longsize )      // max.select size for long types
{
    IFUNCTRACE_DEVELOP();
    if (sql != empty) {
        ITRACE_DEVELOP("ocan");
        ocan( &cda );
        if (cda.csrarc && cda.csrarc != 1003) 
            KORATHROW(lda,&cda,NULL,NULL);
    } // if
    ITRACE_DEVELOP("oparse");
    sql = stmt;
    if ( oparse(&cda,sql,-1,1,2) )
        KORATHROW(lda,&cda,sql,NULL);
    selarr = selarrsize;
    bndarr = bndarrsize;
    selLongSize = longsize<1 ? 2000 : longsize;
    ITRACE_DEVELOP( "Stmt = "+sql );

    // clear bind variable list for subsequent bind-calls
    bnd.drop();
    bndnxt = 0;
    bndnxtfld = 0;

    // clear select item list for subsequent define-calls
    sel.drop();
    selnxt = 0;
    selnxtfld = 0;
    return *this;
} // KOraCursor::parse

/*
 *  K O r a C u r s o r : : d e f i n e
 *
 *  define select item layout
 */
KSqlCursor& KOraCursor::define(
    char*           name,           // bind variable name
    VarType         htype,          // variable type
    long int        hsize )         // max. size of text-types
{
    IFUNCTRACE_DEVELOP();
    ITRACE_DEVELOP( "define: name="+IString(name)+
                        "  type="+IString(htype)+
                        "  size="+IString(hsize) );

    Column &col = sel.add(name,htype,hsize,selarr);

    if ( (col.etype==extTypeLongVarchar || col.etype==extTypeLongVarraw)
         && col.hsize>65532 ) {
        *(long int*)col.buffer = col.bufsiz;
        ITRACE_DEVELOP("odefin");
        if ( odefin(&cda,sel.count(),col.buffer,-1,col.etype,-1,
                    col.ind,NULL,-1,-1,NULL,NULL) )
            KORATHROW(lda,&cda,sql,NULL);
    } else {
        ITRACE_DEVELOP("odefin");
        if ( odefin(&cda,sel.count(),col.buffer,col.bufsiz,col.etype,-1,
                    col.ind,NULL,-1,-1,NULL,NULL) )
            KORATHROW(lda,&cda,sql,NULL);
    } /* endif */
    return *this;
} // KOraCursor::define

/*
 *  K O r a C u r s o r : : b i n d
 *
 *  define bind variable layout
 */
KSqlCursor& KOraCursor::bind(
    char*           name,           // bind variable name
    VarType         htype,          // variable type
    long int        hsize )         // max. size of text-types
{
    IFUNCTRACE_DEVELOP();
    ITRACE_DEVELOP( "bind: name="+IString(name)+
                        "  type="+IString(htype)+
                        "  size="+IString(hsize) );

    Column &col = bnd.add(name,htype,hsize,bndarr);

    if ( (col.etype==extTypeLongVarchar || col.etype==extTypeLongVarraw)
         && col.hsize>65532 ) {
        *(long int*)col.buffer = col.bufsiz;
        ITRACE_DEVELOP("obndrv");
        if ( obndrv(&cda,name,-1,col.buffer,-1,
                    col.etype,-1,col.ind,NULL,-1,-1) )
            KORATHROW(lda,&cda,sql,NULL);
    } else {
        ITRACE_DEVELOP("obndrv");
        if ( obndrv(&cda,name,-1,col.buffer,col.bufsiz,
                    col.etype,-1,col.ind,NULL,-1,-1) )
            KORATHROW(lda,&cda,sql,NULL);
    } /* endif */
    return *this;
} // KOraCursor::bind

/*
 *  K O r a C u r s o r : : b e g i n B i n d
 *
 *  first steps for bind value insertion
 */
KOraCursor::Column& KOraCursor::beginBind()
{
    IFUNCTRACE_DEVELOP();
    if (bndnxt>=bndarr) {
        ITRACE_DEVELOP("oexn");
        if ( oexn(&cda,bndnxt,0) )
            KORATHROW(lda,&cda,sql,NULL);
        bndnxt = 0;
        bndnxtfld = 0;
    } // if
    return bnd[bndnxtfld];
} // KOraCursor::beginBind

/*
 *  K O r a C u r s o r : : b i n d V a l u e ( l o n g   i n t )
 *
 *  provide (long int) bind value
 */
void KOraCursor::bindValue(long int val)
{
    IFUNCTRACE_DEVELOP();
    Column &col = beginBind();

    // verify compatibility with column
    switch (col.htype) {
        case varTypeInt8:
            ((char *)col.buffer)[bndnxt] = val;
            if (((char *)col.buffer)[bndnxt] != val) goto noMatch;
            break;
        case varTypeInt16:
            ((short int *)col.buffer)[bndnxt] = val;
            if (((short int *)col.buffer)[bndnxt] != val) goto noMatch;
            break;
        case varTypeInt32:
            ((long int *)col.buffer)[bndnxt] = val;
            break;
        case varTypeFloat32:
            ((float *)col.buffer)[bndnxt] = val;
            break;
        case varTypeFloat64:
            ((double *)col.buffer)[bndnxt] = val;
            break;
        case varTypeChar:
        case varTypeRaw:
            noMatch:
            KORATHROW1();
        default:
            KORATHROW0(invType);
    } // switch
    col.ind[bndnxt] = val ? 0 : -1;
    if (++bndnxtfld>=bnd.count()) {
        bndnxt++;
        bndnxtfld = 0;
    } // if
} // KOraCursor::bindValue(long int)

/*
 *  K O r a C u r s o r : : b i n d V a l u e ( d o u b l e )
 *
 *  provide (double) bind value
 */
void KOraCursor::bindValue(double val)
{
    IFUNCTRACE_DEVELOP();
    Column &col = beginBind();

    // verify compatibility with column
    switch (col.htype) {
        case varTypeInt8:
            ((char *)col.buffer)[bndnxt] = val;
            if (((char *)col.buffer)[bndnxt] != val) goto noMatch;
            break;
        case varTypeInt16:
            ((short int *)col.buffer)[bndnxt] = val;
            if (((short int *)col.buffer)[bndnxt] != val) goto noMatch;
            break;
        case varTypeInt32:
            ((long int *)col.buffer)[bndnxt] = val;
            if (((long int *)col.buffer)[bndnxt] != val) goto noMatch;
            break;
        case varTypeFloat32:
            ((float *)col.buffer)[bndnxt] = val;
            if (((float *)col.buffer)[bndnxt] != val) goto noMatch;
            break;
        case varTypeFloat64:
            ((double *)col.buffer)[bndnxt] = val;
            break;
        case varTypeChar:
        case varTypeRaw:
            noMatch:
            KORATHROW1();
        default:
            KORATHROW0(invType);
    } // switch
    col.ind[bndnxt] = val ? 0 : -1;
    if (++bndnxtfld>=bnd.count()) {
        bndnxt++;
        bndnxtfld = 0;
    } // if
} // KOraCursor::bindValue(double)

/*
 *  K O r a C u r s o r : : b i n d V a l u e ( I S t r i n g )
 *
 *  provide (IString) bind value
 */
void KOraCursor::bindValue(IString &val)
{
    IFUNCTRACE_DEVELOP();
    Column &col = beginBind();

    // verify compatibility with column
    switch (col.htype) {
        case varTypeInt8:
        case varTypeInt16:
        case varTypeInt32:
        case varTypeFloat32:
        case varTypeFloat64:
            noMatch:
            KORATHROW1();
        case varTypeChar:
        case varTypeRaw:
            if (val.length()>col.hsize)
                KORATHROW2();
            if (col.etype==extTypeLongVarchar || col.etype==extTypeLongVarraw) {
                *(long int*)&col.buffer[bndnxt*col.bufsiz] = val.length();
                memcpy(&col.buffer[bndnxt*col.bufsiz+4],(char *)val,val.length());
            } else {
                *(short int*)&col.buffer[bndnxt*col.bufsiz] = val.length();
                memcpy(&col.buffer[bndnxt*col.bufsiz+2],(char *)val,val.length());
            } // if
            break;
        default:
            KORATHROW0(invType);
    } // switch
    col.ind[bndnxt] = val.length() ? 0 : -1;
    if (++bndnxtfld>=bnd.count()) {
        bndnxt++;
        bndnxtfld = 0;
    } // if
} // KOraCursor::bindValue(IString)

/*
 *  K O r a C u r s o r : : e x e c u t e
 *
 *  execute dml statement
 *
 *  REMARK:
 *  oexfet was tested here for select statements, but repeated use was
 *  _not_ possible without reparsing (Pro*C 1.5 and DB 7.0.16.6.0).
 *  So we abandoned use of oexfet because saveing a parse is much more
 *  efficiant than the possible performance gain of oexfet.
 */
KSqlCursor& KOraCursor::execute()
{
    IFUNCTRACE_DEVELOP();
    if (sql.word(1).upperCase()=="SELECT" && !sel.count() )
        // auto-define select columns
        for (short int pos = 1;;pos++) {
            // retrieve column info to next column
            long int    isize, dsize;
            short int   type, prec, scale;
            char        cbuf[80];
            long int    cbufl = sizeof(cbuf)-1;
            ITRACE_DEVELOP("odescr");
            if ( odescr( &cda, pos, &isize, &type,
                         cbuf, &cbufl, &dsize, &prec, &scale, NULL ) ) {
                if (cda.csrarc==1007) break;
                KORATHROW(lda,&cda,sql,NULL);
            } // if

            VarType     htype;          // c++ type code
            switch (type) {
                case 2: {    // NUMBER,   max.    22 bytes
                    int pr = prec ? prec : 38;
                    if (!scale && pr<3)   htype = varTypeInt8;    else
                    if (!scale && pr<5)   htype = varTypeInt16;   else
                    if (!scale && pr<10)  htype = varTypeInt32;   else
                    if (0<prec && pr<7)   htype = varTypeFloat32; else
                                          htype = varTypeFloat64;
                    break;
                }
                case 11:    // ROWID,    max.       6 bytes
                case 23:    // RAW,      max.     255 bytes
                case 106:   // MSLABEL,  max.     255 bytes
                    htype = varTypeRaw;
                    break;
                case 1:     // VARCHAR2, max.    2000 bytes
                case 12:    // DATE,     max.       7 bytes
                case 96:    // CHAR,     max.     255 bytes
                    htype = varTypeChar;
                    break;
                case 8:     // LONG,     max. 2**31-1 bytes
                    htype = varTypeChar;
                    dsize = selLongSize;
                    break;
                case 24:    // LONG RAW, max. 2**31-1 bytes
                    htype = varTypeRaw;
                    dsize  = selLongSize;
                    break;
                default:;
            } // switch
            define(IString(cbuf,cbufl),htype,dsize);
            Column &col = sel[sel.count()-1];
            col.descrInfo(type,dsize,prec,scale);
        } /* for */
    if ( bndnxtfld )
        KORATHROW0("not all bind vars supplied");
    ITRACE_DEVELOP("oexn");
    if ( oexn(&cda, bndnxt?bndnxt:1 ,0) )
        KORATHROW(lda,&cda,sql,NULL);
    selcnt = selnxt = selarr;
    bndnxt = 0;
    return *this;
} // KOraCursor::execute

/*
 *  K O r a C u r s o r : : f e t c h
 *
 *  position fetch ptr to next row in the select array; if passing byond
 *  last row, fetch next bunch of rows into select array and reset ptr's.
 */
int KOraCursor::fetch()
{
    IFUNCTRACE_DEVELOP();
    if ( !sel.count() )
        KORATHROW0("no select vars available");
    if (++selnxt>=selarr) {                // do we have to fetch more rows ?
        unsigned long rows = cda.csrrpc;
        ITRACE_DEVELOP("ofen");
        if (ofen(&cda,selarr))
            if (cda.csrarc!=1403)
                KORATHROW(lda,&cda,sql,NULL);
        selnxt = 0;
        selcnt = cda.csrrpc - rows;
    } // if
    selnxtfld = 0;
    return selnxt>=selcnt ? 0 : 1;
} // KOraCursor::fetch

/*
 *  K O r a C u r s o r : : b e g i n F e t c h
 *
 *  first steps for fetching a column value
 */
KOraCursor::Column& KOraCursor::beginFetch()
{
    IFUNCTRACE_DEVELOP();
    if (selnxt>=selcnt) 
        KORATHROW0("no more rows to fetch");
    return sel[selnxtfld++];
} // KOraCursor::beginFetch

/*
 *  K O r a C u r s o r : : f e t c h V a l u e ( l o n g   i n t  & )
 *
 *  fetch floating point value from current column
 */
int KOraCursor::fetchValue(long int &val)
{
    IFUNCTRACE_DEVELOP();
    int         ret = selnxtfld;
    Column     &col = beginFetch();
    if (col.ind[selnxt]<0)
        val = 0.0;
    else
    switch (col.htype) {
        case varTypeInt8:
            val = (char)col.buffer[selnxt];
            break;
        case varTypeInt16:
            val = ((short int *)col.buffer)[selnxt];
            break;
        case varTypeInt32:
            val = ((long int *)col.buffer)[selnxt];
            break;
        case varTypeFloat32:
            val = ((float *)col.buffer)[selnxt];
            break;
        case varTypeFloat64:
            val = ((double *)col.buffer)[selnxt];
            break;
        case varTypeChar:
        case varTypeRaw: {
            IString buf;
            if (col.etype==extTypeLongVarchar || col.etype==extTypeLongVarraw) 
                buf = IString(&((char*)col.buffer)[selnxt*col.bufsiz+4],
                              *(long int*)&col.buffer[selnxt*col.bufsiz] );
            else
                buf = IString(&((char*)col.buffer)[selnxt*col.bufsiz+2],
                              *(short int*)&col.buffer[selnxt*col.bufsiz] );
            val = atol(buf);
            break;
        }
        default:
            KORATHROW0(invType);
    } // switch
    return ret;
} // KOraCursor::fetchValue(long int&)

/*
 *  K O r a C u r s o r : : f e t c h V a l u e ( d o u b l e & )
 *
 *  fetch floating point value from current column
 */
int KOraCursor::fetchValue(double &val)
{
    IFUNCTRACE_DEVELOP();
    int         ret = selnxtfld;
    Column     &col = beginFetch();
    if (col.ind[selnxt]<0)
        val = 0.0;
    else
    switch (col.htype) {
        case varTypeInt8:
            val = (char)col.buffer[selnxt];
            break;
        case varTypeInt16:
            val = ((short int *)col.buffer)[selnxt];
            break;
        case varTypeInt32:
            val = ((long int *)col.buffer)[selnxt];
            break;
        case varTypeFloat32:
            val = ((float *)col.buffer)[selnxt];
            break;
        case varTypeFloat64:
            val = ((double *)col.buffer)[selnxt];
            break;
        case varTypeChar:
        case varTypeRaw: {
            IString buf;
            if (col.etype==extTypeLongVarchar || col.etype==extTypeLongVarraw) 
                buf = IString(&((char*)col.buffer)[selnxt*col.bufsiz+4],
                              *(long int*)&col.buffer[selnxt*col.bufsiz] );
            else
                buf = IString(&((char*)col.buffer)[selnxt*col.bufsiz+2],
                              *(short int*)&col.buffer[selnxt*col.bufsiz] );
            val = atof(buf);
            break;
        }
        default:
            KORATHROW0(invType);
    } // switch
    return ret;
} // KOraCursor::fetchValue(double&)

/*
 *  K O r a C u r s o r : : f e t c h V a l u e ( I S t r i n g & )
 *
 *  fetch IString value from current column
 */
int KOraCursor::fetchValue(IString &val)
{
    IFUNCTRACE_DEVELOP();
    int         ret = selnxtfld;
    Column &col = beginFetch();
    if (col.ind[selnxt]<0) 
        val = empty;
    else
    switch (col.htype) {
        case varTypeInt8:
            val = (int)((char *)col.buffer)[selnxt];
            break;
        case varTypeInt16:
            val = ((short int *)col.buffer)[selnxt];
            break;
        case varTypeInt32:
            val = ((long int *)col.buffer)[selnxt];
            break;
        case varTypeFloat32:
            val = ((float *)col.buffer)[selnxt];
            break;
        case varTypeFloat64:
            val = ((double *)col.buffer)[selnxt];
            break;
        case varTypeChar:
        case varTypeRaw:
            if (col.etype==extTypeLongVarchar || col.etype==extTypeLongVarraw) 
                val = IString(&((char*)col.buffer)[selnxt*col.bufsiz+4],
                              *(long int*)&col.buffer[selnxt*col.bufsiz] );
            else
                val = IString(&((char*)col.buffer)[selnxt*col.bufsiz+2],
                              *(short int*)&col.buffer[selnxt*col.bufsiz] );
            break;
        default:
            KORATHROW0(invType);
    } // switch
    return ret;
} // KOraCursor::fetchValue(IString&)

/*
 *  K O r a C u r s o r : : r o w s P r o c e s s e d
 *
 *  get # of processed rows
 */
unsigned long int KOraCursor::rowsProcessed()
{
    IFUNCTRACE_DEVELOP();
    return cda.csrrpc;
} // KOraCursor::rowsProcessed

/*
 *  K O r a C u r s o r : : s e l e c t C o l u m n C o u n t
 *
 *  get # of select columns
 */
int KOraCursor::selectColumnCount()
{
    IFUNCTRACE_DEVELOP();
    return sel.count();
} // KOraCursor::selectColumnCount

/*
 *  K O r a C u r s o r : : s e l e c t C o l u m n N a m e
 *
 *  get name of select column
 */
char* KOraCursor::selectColumnName(int index)
{
    IFUNCTRACE_DEVELOP();
    return sel[index].name;
} // KOraCursor::selectColumnName

/*
 *  K O r a C u r s o r : : s e l e c t C o l u m n D e s c r i p t i o n
 *
 *  get description of select column
 */
char* KOraCursor::selectColumnDescription(int index)
{
    IFUNCTRACE_DEVELOP();
    return sel[index].descr;
} // KOraCursor::selectColumnDescription

/*
 *  K O r a C u r s o r : : s e l e c t C o l u m n T y p e
 *
 *  get type of select column
 */
KSqlCursor::VarType KOraCursor::selectColumnType(int index)
{
    IFUNCTRACE_DEVELOP();
    return sel[index].htype;
} // KOraCursor::selectColumnType

/*
 *  K O r a C u r s o r : : s e l e c t C o l u m n S i z e
 *
 *  get size/precision of select column
 */
int KOraCursor::selectColumnSize(int index)
{
    IFUNCTRACE_DEVELOP();
    return sel[index].hsize;
} // KOraCursor::selectColumnSize

/*
 *  K O r a C u r s o r : : s e l e c t C o l u m n S c a l e
 *
 *  get scale of select column (for numerics only)
 */
int KOraCursor::selectColumnScale(int index)
{
    IFUNCTRACE_DEVELOP();
    return sel[index].hscale;
} // KOraCursor::selectColumnScale

/*
 *  K O r a C u r s o r : : C o l u m n : : C o l u m n
 *
 *  constructor for column
 */
KOraCursor::Column::Column()
{
    IFUNCTRACE_DEVELOP();
    next  = NULL;
    buffer = NULL;
    ind    = NULL;
} // Column::Column

/*
 *  K O r a C u r s o r : : C o l u m n : : C o l u m n
 *
 *  constructor for column
 */
KOraCursor::Column::Column(
    char*       _name,          // column name
    VarType     _htype,         // c++ type code
    long int    _hsize,         // size for Char & Raw
    short int   arrsize )       // arraysize
{
    IFUNCTRACE_DEVELOP();
    next  = NULL;
    name  = _name;
    hsize = _hsize;
    hscale = 0;
    switch (htype = _htype) {
        case varTypeInt8:
            bufsiz  = sizeof(signed char);
            etype   = extTypeInteger;
            break;
        case varTypeInt16:
            bufsiz  = sizeof(short int);
            etype   = extTypeInteger;
            break;
        case varTypeInt32:
            bufsiz  = sizeof(long int);
            etype   = extTypeInteger;
            break;
        case varTypeFloat32:
            bufsiz  = sizeof(float);
            etype   = extTypeFloat;
            break;
        case varTypeFloat64:
            bufsiz  = sizeof(double);
            etype   = extTypeFloat;
            break;
        case varTypeChar:
            if (_hsize < 1) _hsize = 2000;
            if (_hsize > 2000) {
                bufsiz  = _hsize+4;
                hsize   = _hsize;
                etype   = extTypeLongVarchar;
            } else {
                bufsiz  = _hsize+2;
                hsize   = _hsize;
                etype   = extTypeVarchar;
            } // if
            break;
        case varTypeRaw:
            if (_hsize < 1) _hsize = 2000;
            if (_hsize > 2000) {
                bufsiz  = _hsize+4;
                hsize   = _hsize;
                etype   = extTypeLongVarraw;
            } else {
                bufsiz  = _hsize+2;
                hsize   = _hsize;
                etype   = extTypeVarraw;
            } // if
            break;
        default: 
            KORATHROW0("bind/define: unknown VarType");
    } // switch 
    if (double(bufsiz)*arrsize > INT_MAX)
        KORATHROW0("bind/define: array buffer size exceeds INT_MAX");
    buffer = new unsigned char[bufsiz*arrsize];
    ind    = new short int[arrsize];
} // Column::Column

/*
 *  K O r a C u r s o r : : C o l u m n : : ~ C o l u m n
 *
 *  destructor for column
 */
KOraCursor::Column::~Column()
{
    IFUNCTRACE_DEVELOP();
    delete next;
    delete [] buffer;
    delete [] ind;   
} // Column::~Column

/*
 *  K S q l C u r s o r : : C o l u m n : : d e s c r I n f o
 *
 *  prepare description info for select column
 */
void KOraCursor::Column::descrInfo(
    short int       type,           // ORACLE internal type code
    long int        size,           // displayed size
    short int       prec,           // precision for numerics
    short int       scale )         // scale for numerics
{
    hscale = 0;
    switch (type) {
        case 1: 
            descr = "VARCHAR2("+IString(size)+")";
            break;
        case 2:     
            if (scale<=-127) {
                descr = "FLOAT";
                if (prec<38)
                    descr += "("+IString(prec)+")";
            } else {
                descr = "NUMBER";
                if (prec) {
                    descr += "("+IString(prec);
                    if (scale)
                        descr += ","+IString(scale);
                    descr += ")";
                } // if
            } /* endif */
            hsize = prec ? prec : 38;
            hscale = scale>=0 ? scale : scale>-127 ? 0 : -1;
            break;
        case 8:
            descr = "LONG";
            break;
        case 11:
            descr = "ROWID";
            break;
        case 12:
            descr = "DATE";
            break;
        case 23:
            descr = "RAW("+IString(size)+")";
            break;
        case 24:
            descr = "LONG RAW";
            break;
        case 96:
            descr = "CHAR("+IString(size)+")";
            break;
        case 106:
            descr = "MLSLABEL";
            break;
        default:
            KORATHROW0("unknown ORACLE-internal datatype");
    } // switch
} // descrInfo

/*
 *  K O r a C u r s o r : : C o l u m n : : o p e r a t o r [ ]
 *
 *  subscription operator for columns
 */
KOraCursor::Column& KOraCursor::Column::operator[](int index)
{
    IFUNCTRACE_DEVELOP();
    Column *col = next;
    for (int colCnt=0; colCnt<index && col; colCnt++, col=col->next );
    if (!col)
        KORATHROW0("invalid column # ("+IString(index)+")");
    return *col;
} // Column::operator[]

/*
 *  K O r a C u r s o r : : C o l u m n : : a d d
 *
 *  add new column to list
 */
KOraCursor::Column& KOraCursor::Column::add(
    char*       _name,          // column name
    VarType     _htype,         // c++ type code
    long int    _hsize,         // size for Char & Raw
    short int   arrsize )       // arraysize
{
    IFUNCTRACE_DEVELOP();
    for (Column *col = this; col->next; col=col->next );
    col->next = new Column(_name,_htype,_hsize,arrsize);
    return *col->next;
} // Column::add

/*
 *  K O r a C u r s o r : : C o l u m n : : d r o p
 *
 *  Drop column list
 */
void KOraCursor::Column::drop()
{
    IFUNCTRACE_DEVELOP();
    delete next;
    next = NULL;
} // Column::drop

/*
 *  K O r a C u r s o r : : C o l u m n : : c o u n t
 *
 *  return # of columns in list
 */
int KOraCursor::Column::count()
{
    IFUNCTRACE_DEVELOP();
    Column *col = next;
    for (int colCnt=0; col; colCnt++, col=col->next );
    return colCnt;
} // Column::count
