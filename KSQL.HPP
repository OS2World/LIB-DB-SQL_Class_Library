#ifndef _KSQL_
#define _KSQL_
/*  Copyright (c) 1995 IBK-Landquart-Switzerland. All rights reserved.
 *
 *  Module      :  KSQL.HPP
 *  Application :  SQL Class Library
 *  Purpose     :  SQL virtual classes headerfile
 *  Author      :  P. Koch, IBK
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
#ifndef _ISTRING_
    #include <istring.hpp>
#endif

#ifndef _IEXCEPT_
    #include <iexcept.hpp>
#endif

#pragma pack(4)

/*
 *  C l a s s   :   K S q l C u r s o r
 *
 *  Virtual class for SQL-cursors
 */
class KSqlCursor
{
    friend class KSqlLink;
    private:
        KSqlCursor  **owner;            // current owner
        short int   prio;               // priority for recycling

        virtual int  
            newParseRequired();         // check for new parse need
        virtual void 
            bindValue(long int) = 0;    // bind (long int) value
        virtual void 
            bindValue(double) = 0;      // bind (double) value
        virtual void 
            bindValue(IString&) = 0;    // bind (IString) value
        virtual int
            fetchValue(double&) = 0;    // fetch (long int) value
        virtual int
            fetchValue(long int&) = 0;  // fetch (double) value
        virtual int
            fetchValue(IString&) = 0;   // fetch (IString) value

    public:
        enum VarType {                  // variable types
            varTypeInt8,                    //  8 bit int   (char)
            varTypeInt16,                   // 16 bit int   (short int)
            varTypeInt32,                   // 32 bit int   (long int)
            varTypeFloat32,                 // 32 bit fp    (float)
            varTypeFloat64,                 // 64 bit fp    (double
            varTypeChar,                    // xlated char  (char* or IString)
            varTypeRaw                      // raw char     (char* or IString)
        };
        virtual ~KSqlCursor();          // destructor
        virtual KSqlCursor& _Export 
            parse(                      // parse statement
                char *,                     // sql statement
                short int=1,                // bind array size
                short int=1,                // select array size
                long int=2000 ) = 0;        // max.select size for long types
        virtual KSqlCursor& _Export 
            bind(                       // bind vars
                char *,                     // placeholder name
                VarType=varTypeInt32,       // c++ variable type
                long int=0 ) = 0;           // max.size for Char & Raw
        virtual KSqlCursor& _Export 
            define(                     // define vars
                char *,                     // select column name
                VarType=varTypeInt32,       // c++ variable type
                long int=0 ) = 0;           // max.size for String & Raw
        virtual KSqlCursor& _Export 
            execute() = 0;              // execute dml statement
        virtual int _Export 
            fetch() = 0;                // fetch next row
        virtual unsigned long int _Export
            rowsProcessed() = 0;        // get # of processed rows
        virtual int _Export
            selectColumnCount() = 0;    // get # of select columns
        virtual char* _Export
            selectColumnName(int) = 0;  // get name of select column
        virtual VarType _Export
            selectColumnType(int) = 0;  // get type of select column
        virtual int _Export
            selectColumnSize(int) = 0;  // get size of select column
        virtual int _Export
            selectColumnScale(int) = 0; // get scale of select column
        virtual char* _Export
            selectColumnDescription(int)// get description of select column
            = 0;

        // bind value operators
        
        KSqlCursor& _Export operator<<(char);
        KSqlCursor& _Export operator<<(signed char);
        KSqlCursor& _Export operator<<(unsigned char);
        KSqlCursor& _Export operator<<(short int);
        KSqlCursor& _Export operator<<(unsigned short int);
        KSqlCursor& _Export operator<<(long int);
        KSqlCursor& _Export operator<<(unsigned long int);
        KSqlCursor& _Export operator<<(int);
        KSqlCursor& _Export operator<<(unsigned int);
        KSqlCursor& _Export operator<<(float);
        KSqlCursor& _Export operator<<(double);
        KSqlCursor& _Export operator<<(char *);
        KSqlCursor& _Export operator<<(signed char *);
        KSqlCursor& _Export operator<<(unsigned char *);
        KSqlCursor& _Export operator<<(IString);

        // fetch value operators
        KSqlCursor& _Export operator>>(char&);
        KSqlCursor& _Export operator>>(signed char&);
        KSqlCursor& _Export operator>>(unsigned char&);
        KSqlCursor& _Export operator>>(short int&);
        KSqlCursor& _Export operator>>(unsigned short int&);
        KSqlCursor& _Export operator>>(long int&);
        KSqlCursor& _Export operator>>(unsigned long int&);
        KSqlCursor& _Export operator>>(int&);
        KSqlCursor& _Export operator>>(unsigned int&);
        KSqlCursor& _Export operator>>(float&);
        KSqlCursor& _Export operator>>(double&);
        KSqlCursor& _Export operator>>(char *);
        KSqlCursor& _Export operator>>(signed char *);
        KSqlCursor& _Export operator>>(unsigned char *);
        KSqlCursor& _Export operator>>(IString&);
}; // KSqlCursor

/*
 *  C l a s s   :   K S q l L i n k
 *
 *  Virtual class for SQL-links
 */
class KSqlLink
{
    protected:
        KSqlCursor  
            **csrPool;                  // cursor pool array
        short int   
            maxCursor,                  // pool size
            csrCount;                   // currently allocated cursors

        virtual KSqlCursor 
            *allocCursor() = 0;         // allocate a cursor
        KSqlLink(                       // constructor (logon)
            short int );                    // cursor pool size

    public:
        virtual 
            ~KSqlLink();                // destructor (logoff)
        int _Export 
            getCursor(                  // get cursor from pool 
                KSqlCursor** );             // address of csr ptr
        virtual void _Export 
            commit() = 0;               // commit work
        virtual void _Export 
            rollback() = 0;             // rollback work
        virtual char* _Export
            dataBaseName() = 0;         // tell database type
}; // KSqlLink

IEXCLASSDECLARE(KSqlError,IException);

#pragma pack()

#endif
