#ifndef _KORACLE_
#define _KORACLE_
/*  Copyright (c) 1995 IBK-Landquart-Switzerland. All rights reserved.
 *
 *  Module      :  KORACLE.HPP
 *  Application :  SQL Class Library
 *  Purpose     :  ORACLE classes headerfile
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
#ifndef _KSQL_
    #include "ksql.hpp"
#endif

#pragma pack(1)

/*
 *  O R A C L E   C U R S O R   D E S C R I P T O R   A R E A
 */
typedef struct {
    short int           csrrc;              // v2 return code
    short int           csrft;              // function type
    unsigned long int   csrrpc;             // rows processed count
    short int           csrpeo;             // parse error offset
    unsigned char       csrfc;              // function code
    unsigned char       csrfil;             // filler
    short int           csrarc;             // return code
    unsigned char       csrwrn;             // warning flags
    unsigned char       csrflg;             // error flags
    //**** Operating system dependent ****
    unsigned short int  csrcn;              // cursor number
    struct {                                // rowid structure
        struct {
            unsigned long int   tidtrba;    // rba of first blockof table
            unsigned short int  tidpid;     // partition id of table
            unsigned char       tidtbl;     // table id of table
        } ridtid;
        unsigned long int   ridbrba;        // rba of datablock
        unsigned short int  ridsqn;         // sequence number of row in block
    } csrrid;
    unsigned short int  csrose;             // os dependent error code
    unsigned char       csrchk;             // check byte
    unsigned char       crsfill[30];        // private, reserved fill
} KOraCDA;

/*
 *  L O G O N   D E S C R I P T O R   A R E A
 */
typedef struct {
    short int           ldarc2;             // v2 return code
    unsigned short int  filler_1[5];        // filler
    short int           ldarc;              // return code
    unsigned short int  filler_2;           // filler
    //**** Operating system dependent ****
    unsigned char       filler_3[14];       // filler
    unsigned char       filler_4;           //
    unsigned char       osderr1;            // op.sys. dependent error code 
    unsigned char       osderr2;            // op.sys. dependent error code 
    unsigned char       chkbyte;            // check byte 
    unsigned char       filler_5[30];       // oracle internal parameters 
} KOraLDA;

#pragma pack(4)

/*
 *  S Q L   C U R S O R   T Y P E
 */
class KOraCursor : public KSqlCursor {
    friend class KOraLink;
    public:
        enum ExtType { // ORACLE external datatype codes
            extTypeInteger      = 3,
            extTypeFloat        = 4,
            extTypeVarchar      = 9,
            extTypeVarraw       = 15,
            extTypeLongVarchar  = 94,
            extTypeLongVarraw   = 95
        };

    private:                 
        /*
         *  B I N D / S E L E C T   C O L U M N 
         */
        class Column {
            private:
                Column          *next;      // next column if not NULL
 
            public:
                unsigned char   *buffer;    // buffer for data (array)
                short int       *ind;       // indicator var array
                IString         name;       // column name
                IString         descr;      // column description
                int             bufsiz;     // size of one buffer
                int             hsize;      // size/precision
                short int       hscale;     // scale
                VarType         htype;      // program var type
                ExtType         etype;      // ORACLE external datatype
                Column();                   // constructor
                Column(                     // constructor
                    char*,                      // column name
                    VarType,                    // c++ type
                    long int,                   // size for Char/Raw
                    short int );                // arraysize
                ~Column();                  // destructor
                void descrInfo(             // prep. description info
                    short int,                  // ORACLE internal type code
                    long int,                   // displayed size
                    short int,                  // precision for numerics
                    short int );                // scale for numerics
                Column& operator[](int);    // subscriptor
                Column& add(                // add new column to list
                    char*,                      // column name
                    VarType,                    // c++ type
                    long int,                   // size for Char/Raw
                    short int );                // arraysize
                void drop();                // drop column list
                int count();                // return # of columns
        }; // Column
 
        KOraLDA     *lda;           // logon descriptor area
        Column      bnd;            // bind column list
        Column      sel;            // select column list
        KOraCDA     cda;            // cursor descriptor area
        IString     sql;            // sql statement
        short int   bndarr;         // bind array size
        short int   bndnxt;         // next row index
        short int   bndnxtfld;      // next column to fill
        short int   selarr;         // select array size
        short int   selnxt;         // next row to fetch
        short int   selnxtfld;      // next column to fetch
        short int   selcnt;         // valid rows in array
        long int    selLongSize;    // size for undet.long columns

        int newParseRequired();     // check for new parse need
        void bindValue(long int);   // bind (long int) value
        void bindValue(double);     // bind (double) value
        void bindValue(IString&);   // bind (IString) value
        int fetchValue(long int&);  // fetch (long int) value
        int fetchValue(double&);    // fetch (double) value
        int fetchValue(IString&);   // fetch (IString) value
        Column& beginBind();        // begin bind var insertion
        Column& beginFetch();       // begin fetch value operation
        KOraCursor();               // constructor
        ~KOraCursor();              // destructor
        KSqlCursor& parse(          // parse statement
            char *,                     // sql statement
            short int = 1,              // bind array size
            short int = 1,              // select array size
            long int = 2000 );          // max.select size for long types
        KSqlCursor& bind(           // bind vars
            char *,                     // placeholder name
            VarType=varTypeInt32,       // c++ variable type
            long int=0 );               // max.size for Char & Raw
        KSqlCursor& define(         // define vars
                char *,                 // select column name
                VarType=varTypeInt32,   // c++ variable type
                long int=0 );           // max.size for String & Raw
        int fetch();                // fetch next row
        KSqlCursor& execute();      // execute dml statement
        unsigned long int rowsProcessed();  // get # of rows processed
        void getDescrInfo();        // get description information
        int selectColumnCount();    // get # of select columns
        char* selectColumnName(int);// get name of select column
        VarType selectColumnType(int); // get type of select column
        int selectColumnSize(int);  // get size of select column
        int selectColumnScale(int); // get scale of select column
        char* selectColumnDescription(int);// get description of select column
}; // KOraCursor

/*
 *  C l a s s   :   K O r a L i n k
 *
 *  ORACLE V7 Link
 */
class KOraLink : public KSqlLink
{
    private:
        KOraLDA         lda;        // logon descriptor area
        unsigned char   hda[256];   // host descriptor area

        KSqlCursor *allocCursor();  // allocate a cursor
        ~KOraLink();                // logoff
        void commit();              // commit work
        void rollback();            // rollback work
        char* dataBaseName();       // return database name "ORACLE"

    public:
        _Export KOraLink(           // logon
            char *,                     // username[@database]
            char *,                     // password
            short int=32                // cursor pool size
        );
}; // KOraLink

#pragma pack()
#endif
