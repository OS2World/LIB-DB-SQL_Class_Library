/*  Copyright (c) 1995 IBK-Landquart-Switzerland. All rights reserved.
 *
 *  Module      :  TSS.CPP
 *  Application :  Tiny SQL Shell
 *  Purpose     :  Sample application for SQL-class use
 *  Author      :  Peter Koch, IBK
 *
 *  Date        Description                                 Who
 *  --------------------------------------------------------------------------
 *  June 1995   First release                               P.Koch, IBK
 *  15.06.1995  V1.1: New commands:
 *                      HOST/$, CALL/@, REM/REMARK, LOG
 *                    Define script & parameters or
 *                    run autoexec.tss at start             P.Koch, IBK
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
#include <stdlib.h>
#include <iostream.h>
#include <fstream.h>
#include <strstrea.h>
#include <iomanip.h>
#include <istring.hpp>
#include <idate.hpp>
#include <itime.hpp>
#include <koracle.hpp>

#define MAXNESTING          15              // max. nesting level

static char *dbName[] = {
    "ORACLE",
    "DB/2",
    "INFORMIX",
    "GUPTA"
};

static  KSqlLink*   mainLink;               // main link
static  istream*    istr[MAXNESTING];       // input stream stack
static  ostream*    lstr;                   // logfile stream
static  IString     ipara[MAXNESTING][10];  // parameters %0 ... %9
static  IString     input;                  // user input line
static  IString     lname;                  // logfile name
static  int         connected   = 0;        // connect status
static  int         numwidth    = 40;       // max.numeric width
static  int         charwidth   = 40;       // max.character width
static  int         selectarr   = 20;       // select array size
static  int         pause       = 24;       // pause after x lines
static  int         database    = 0;        // database type
static  int         output      = 0;        // output format
static  int         echo        = 1;        // echo mode
static  int         ipos        = 0;        // input stream ptr

/*
 *  l o g 0
 *
 *  write string to cout and logfile
 */
static void log0(char* msg)
{
    cout << msg;
    if (lname!="") *lstr << msg;
} // log0

/*
 *  l o g
 *
 *  write string to cout and logfile and print linefeed
 */
static void log(char* msg)
{
    cout << msg << '\n';
    if (lname!="") *lstr << msg << '\n';
} // log

#define numeric(csr,index)\
        (csr->selectColumnType(index)<=KSqlCursor::varTypeFloat64)

/*
 *  d i s p W i d t h
 *
 *  return display width for given column
 */
static int dispWidth(KSqlCursor *csr, int index)
{
    int width = csr->selectColumnSize(index);
    if (numeric(csr,index)) {
        width++; // sign
        if (csr->selectColumnScale(index)) width++; // decimal point
        if (numwidth && width>numwidth) width = numwidth;
    } else 
        if (charwidth && width>charwidth) width = charwidth;
    return width;
} // dispWidth

/*
 *  p a u s e c h k
 *
 *  increment linecounter and pause output
 */
static int pausechk(int &pcnt)
{
    if (++pcnt==pause) {
        pcnt = 0;
        cout << "pause...  (enter 'x' to abort) ";
        IString inp = IString::lineFrom(cin).word(1).upperCase();
        if (inp=="X") {
            log("operation aborted.");
            return 1;
        } // if
    } // if 
    return 0;
} // pausechk

/*
 *  d e s c r i b e
 *
 *  describe columns of a table
 */
static void describe()
{
    if (!connected) {
        log("not connected!");
        return;
    } // if
    IString table = input.word(2);
    if (table=="") {
        log("missing table name!");
        return;
    } // if
    static KSqlCursor *csr;
    mainLink->getCursor(&csr);
    csr->parse("SELECT * FROM "+table+" WHERE ROWNUM<1");
    csr->execute();
    int cols = csr->selectColumnCount();
    int pcnt = 0;
    for (int col = 0; col < cols; col++) {
        ostrstream* out = new ostrstream();
        *out << setiosflags(ios::left) << setw(20) << setfill('.')
             << csr->selectColumnName(col)
             << setiosflags(0) << setw(0)  << setfill(' ')
             << ": " << csr->selectColumnDescription(col);
        char* ss = out->str();
        ss[out->pcount()] = 0;
        log(ss);
        delete ss;
        delete out;
        if (pausechk(pcnt)) break;
    } // for
} // describe

/*
 *  v e r t i c a l
 *
 *  show select results in vertical direction
 */
static void vertical(KSqlCursor *csr, int cols)
{
    int pcnt = 0;
    while (csr->fetch()) {
        log("");
        if (pausechk(pcnt)) break;
        for (int col = 0; col < cols; col++) {
            ostrstream* out = new ostrstream();
            *out << setiosflags(ios::left) << setw(20) << setfill('.')
                 << csr->selectColumnName(col)
                 << setiosflags(0) << setw(0) << setfill(' ') << ": ";
            int width = dispWidth(csr,col);
            int scale = csr->selectColumnScale(col);
            if (scale>=0) {
                out->setf(ios::fixed);
                out->precision(scale);
            } //
            if (numeric(csr,col)) {
                double val;
                *csr >> val;
                *out << val;
            } else {
                IString val;
                *csr >> val;
                *out << val.subString(1,width).strip() << setiosflags(0);
            } /* endif */
            char* ss = out->str();
            ss[out->pcount()] = 0;
            log(ss);
            delete ss;
            delete out;
            if (pausechk(pcnt)) return;
        } // for
    } // while
} // vertical

/*
 *  h o r i z o n t a l
 *
 *  show select results in horizontal direction
 */
static void horizontal(KSqlCursor *csr, int cols)
{
    // show heading
    for (int col = 0; col < cols; col++) {
        int width = dispWidth(csr,col);
        cout.setf(numeric(csr,col) ? ios::right : ios::left);
        cout << setw(width) << IString(csr->selectColumnName(col))
                                    .subString(1,width)
                                    .strip();
        if (col < cols-1) cout << ' ';
        cout.flags(0);
    } // for
    cout << '\n';
    for (col = 0; col < cols; col++) {
        int width = dispWidth(csr,col);
        cout << setw(width) << setfill('-') << "" << setfill(' ');
        if (col < cols-1) cout << ' ';
    } // for
    cout << '\n';
    // show fetched columns
    int pcnt = 2;
    while (csr->fetch()) {
        for (int col = 0; col < cols; col++) {
            int width = dispWidth(csr,col);
            int scale = csr->selectColumnScale(col);
            if (scale>=0) {
                cout.setf(ios::fixed);
                cout.precision(scale);
            } //
            cout.width(width);
            if (numeric(csr,col)) {
                cout.setf( ios::right );
                double val;
                *csr >> val;
                cout << val;
            } else {
                cout.setf( ios::left );
                IString val;
                *csr >> val;
                 cout << val.subString(1,width).strip();
            } /* endif */
            if (col < cols-1) cout << ' ';
            cout.flags(0);
        } // for
        cout << '\n';
        if (pausechk(pcnt)) break;
    } // while
} // horizontal

/*
 *  e x e c u t e
 *
 *  execute any sql statement
 */
static void execute()
{
    if (!connected) {
        log("not connected!");
        return;
    } // if
    static KSqlCursor *csr;
    mainLink->getCursor(&csr);
    csr->parse(input,1,selectarr)
        .execute();
    int cols = csr->selectColumnCount();
    if (!cols) {
        log( IString(csr->rowsProcessed())+" row(s) processed.");
        return;
    } // if
    if (output) 
        vertical(csr,cols);
    else 
        horizontal(csr,cols);
    log(IString(csr->rowsProcessed())+" row(s) selected.");
} // execute

/*
 *  r o l l b a c k
 *
 *  rollback work
 */
static void rollback()
{
    if (connected) {
        mainLink->rollback();
        log("rolled back.");
    } else
        log("not connected!");
} // rollback

/*
 *  d i s c o n n e c t
 *
 *  disconnect from database
 */
static void disconnect()
{
    if (connected) { 
        rollback();
        delete mainLink;
        log("disconnected.");
        connected = 0;
    } else
        log("not connected!");
} // disconnect

/*
 *  c o n n e c t
 *
 *  connect to database
 */
static void connect()
{
    if (connected) disconnect();
    switch (database) {
        case 0: // ORACLE
            mainLink = new KOraLink( input.word(2), input.word(3) );
            break;
        default: 
            log(IString("sorry, classes for ")+dbName[database]+
                " not yet implemented!");
            return;
    } // switch
    log(IString("connected to ")+mainLink->dataBaseName());
    connected = 1;
} // connect

/*
 *  c o m m i t
 *
 *  commit any changes
 */
static void commit()
{
    if (connected) {
        mainLink->commit();
        log("commited.");
    } else
        log("not connected!");
} // commit

/*
 *  h e l p
 *
 *  show help screen
 */
static void help()
{
    static char* txt[] = {
"Program startup:",
"   TSS [filename[.sql] parameter1 parameter2 ... parameter9]",
"       starts TSS and optionaly runs a scriptfile.",
"       Placeholders %1 ... %9 in the scriptfile are substituted by parameters",
"       1...9. Parameters containing blanks must be enclosed in quotes e.g.",
"       \"Carol Smith\". %0 will be substituted by the scriptfile name.",
"       If TSS is started without scriptfile and parameters, autoexec.tss is",
"       executed if found in current directory.",
"",
"SQL instruction execution:",
"   [sql instruction];[remark]",
"       The sql instruction may be spawned over several lines. Execution starts",
"       when the semicolon ist found. Lines starting with a semicolon are trea-",
"       ted as remark lines.",
"",
"TSS commands:",
"   TSS commands may be terminated by a semicolon. The remainder of the line is",
"   treated as remark in this case.",
"",
"   CALL <filename[.sql]> [parameter1 parameter2 ... parameter9]",
"   @<filename[.sql]> [parameter1 parameter2 ... parameter9]",
"       Execute a script file. For parameter description look at \"Program",
"       startup\".  Passing placeholders of parameters which may contain",
"       blanks must also by done enclosed in quotes, e.g. (look at %2):",
"               @adduser \"John Ross\" Ewing \"%2\" %3",
"   COMMIT",
"       Commit all changes made by DML-statements (INSERT,UPDATE,DELETE).",
"   CONNECT [name password]",
"       Connect (logon) to database defined with SET DATABASE.",
"   DISCONNECT",
"       Disconnect (logoff) from database.",
"   DESCRIBE <tablename>",
"       Describe the columns of a table.",
"   EXIT",
"       Terminate TSS immediatly.",
"   HELP or ?",
"       Show this help information.",
"   HOST [command]  or  $[command]",
"       Execute command by host shell (cmd). If no command is given, a host",
"       shell session is opened in interactive mode.  Enter EXIT to return",
"       to TSS.",
"   LOG <filename[.log]|CLOSE>",
"       Start logging to specified logfile.  All output produced by TSS will be",
"       appended to the file until LOG CLOSE is entered.  To print the output",
"       immediatly use \"prn.\" as filename (don't forget the dot!).",
"   REM  or  REMARK",
"       Remark line.  A remark line may also be started by a semicolon.",
"   RETURN",
"       Return to previous calling level.  From top level return acts like",
"       EXIT.",
"   ROLLBACK",
"       All DML changes since last COMMIT or ROLLBACK are rolled back.",
"   SET CHARWIDTH <value>",
"       Set maximum displayed character column width.  A value of 0 denotes",
"       unlimited width.",
"   SET DATABASE <name>",
"       Select database type for next CONNECT, e.g. \"ORACLE\", \"DB/2\"",
"       \"INFORMIX\", \"GUPTA\" etc.",
"   SET ECHO <mode>",
"       Turn echo of TSS input \"ON\" or \"OFF\".  This is fully true for",
"       scriptfile execution only; in interactive mode this parameter effects",
"       logging sole.",
"   SET NUMWIDTH <value>",
"       Set maximum displayed numeric column width. A value of 0 denotes",
"       unlimited width.",
"   SET OUTPUT <mode>",
"       Define presentation mode of select query results:",
"       \"HORIZONTAL\" or \"VERTICAL\".",
"   SET PAUSE <value>",
"       Pause after <value> lines of output.  A value of 0 turns off pausing.",
"   SET SELECTARRAY <value>",
"       Define arraysize for select queries.",
"   SHOW <topic>",
"       Show setting of single or all topic(s):",
"       ALL CHARWIDTH DATABASE ECHO LOG NUMWIDTH OUTPUT PAUSE SELECTARRAY"
    };

    int pcnt = 0;
    for (int ln = 0; ln < sizeof(txt)/sizeof(char*); ln++) {
        log(txt[ln]);
        if (pausechk(pcnt)) break;
    } // for
} // help

/*
 *  s e t
 *
 *  set globals
 */
static void set()
{
    IString topic = input.word(2).upperCase();
    if (topic=="CHARWIDTH")
        charwidth = atoi(input.word(3));
    else
    if (topic=="DATABASE") {
        IString name = input.word(3).upperCase();
        for (int i=0; i < sizeof(dbName)/sizeof(char*); i++) 
            if (name==dbName[i]) {
                database = i;
                return;
            } // if
        log("unknown database type: "+name);
    } else
    if (topic=="ECHO") {
        IString name = input.word(3).upperCase();
        if (name=="OFF" || name=="")
            echo = 0;
        else
        if (name=="ON")
            echo = 1;
        else
            log("unknown echo mode: "+name);
    } else
    if (topic=="NUMWIDTH") 
        numwidth = atoi(input.word(3));
    else
    if (topic=="OUTPUT") {
        IString name = input.word(3).upperCase();
        if (name=="HORIZONTAL" || name=="")
            output = 0;
        else
        if (name=="VERTICAL")
            output = 1;
        else
            log("unknown direction: "+name);
    } else
    if (topic=="PAUSE")
        pause = atoi(input.word(3));
    else
    if (topic=="SELECTARRAY")
        selectarr = atoi(input.word(3));
    else
        log("unknown topic: "+topic);
} // set

/*
 *  s h o w
 *
 *  show current settings
 */
static void show()
{
    IString topic = input.word(2).upperCase();
    if (topic=="") topic = "ALL";
    if (topic=="CHARWIDTH" || topic=="ALL")
        log(        "CHARWIDTH   = "+IString(charwidth));
    if (topic=="DATABASE" || topic=="ALL")
        log(IString("DATABASE    = ")+dbName[database]);
    if (topic=="ECHO" || topic=="ALL")
        log(IString("ECHO        = ")+(echo?"ON":"OFF"));
    if (topic=="LOG" || topic=="ALL")
        log(IString("LOG         = ")+((lname=="")?"CLOSED":(char*)lname));
    if (topic=="NUMWIDTH" || topic=="ALL")
        log(        "NUMWIDTH    = "+IString(numwidth));
    if (topic=="OUTPUT" || topic=="ALL")
        log(IString("OUTPUT      = ")+(output?"VERTICAL":"HORIZONTAL"));
    if (topic=="PAUSE" || topic=="ALL")
        log(        "PAUSE       = "+IString(pause));
    if (topic=="SELECTARRAY" || topic=="ALL")
        log(        "SELECTARRAY = "+IString(selectarr));
} // show

/*
 *  h o s t
 *
 *  execute host command
 */
static void host()
{
    IString cmd = input.removeWords(1,1).strip();
    if (cmd=="") {
        log("Type \"EXIT\" to return to TSS");
        cout << flush;
        system("cmd");
    } else {
        cout << flush;
        system(cmd);
    } // if
} // host

/*
 *  s k i p b l a n k s
 *
 *  skip blanks and tabs
 */
static char *skipblank( char *c )
{
    while ( *c == ' ' || *c == 0x09 ) c++;
    return c;
} // skipblank

/*
 *  g e t p a r a m
 *
 *  get next parameter from line
 */
static char *getparam( char **c )
{
    char    *e, *f;

    *c = skipblank(*c);
    if ( **c == '"' ) {
        e = ++(*c);
        while ( **c && **c != '"' ) (*c)++;
        f = *c;
        if ( **c ) (*c)++;
    } else {
        e = *c;
        while ( **c &&
                **c != ' ' && **c != 0x09 &&
                **c != '=' && **c != ',' ) (*c)++;
        f = *c;
    } /* if */
    *c = skipblank(*c);
    *f = 0;
    return e;
} // getparam

/*
 *  r e d i r e c t
 *
 *  redirect input to file
 */
static void redirect(int msg)
{
    if (ipos>=MAXNESTING-1) {
        log("nesting of input streams too deep!");
        return;
    } // if
    ipara[++ipos][0] = input.word(2);
    if (ipara[ipos][0].lastIndexOf('.')<=ipara[ipos][0].lastIndexOf('\\'))
        ipara[ipos][0] += ".sql";
    char *line = input.removeWords(1,2);
    for (int p = 1; p < 10; p++) ipara[ipos][p] = getparam(&line);
    istr[ipos] = new ifstream(ipara[ipos][0]);
    if (!*istr[ipos]) {
        if (msg) 
            log(IString("input file ")+ipara[ipos][0]+" could not be opened!");
        ipos--;
    } // if
} // redirect

/*
 *  s e t l o g
 *
 *  handle log command
 */
static void setlog()
{
    IString word2(input.word(2).upperCase());
    IString spf("logfile ");
    if (word2=="CLOSE" && lname=="") {
        log("log is already OFF!");
        return;
    } //
    if (lname!="") {
        log(spf+lname+" closed at "+IDate().asString()+" "+ITime().asString());
        delete lstr;
        lname = "";
    } // if
    if (word2=="CLOSE") return;
    lname = input.word(2);
    if (lname.lastIndexOf('.')<=lname.lastIndexOf('\\')) lname += ".log";
    lstr = new ofstream(lname,ios::out|ios::app);
    if (*lstr)
        log(spf+lname+" opened at "+IDate().asString()+" "+ITime().asString());
    else {
        log(spf+lname+" could not be opened!");
        lname = "";
    } // if
} // setlog

/*
 *  e x p a n d
 *
 *  expand parameters on input line
 */
static void expand(IString &line)
{
    for (int p = 0; p < 10; p++) {
        if (ipara[ipos][p]=="") break;
        line.change("%"+IString(p), ipara[ipos][p]);
    } /* endfor */
} // expand

/*
 *  m a i n
 */
main(int argc, char *argv[], char *envp[])
{
    cout << "Tini SQL Shell V1.1\n";
    cout << "Copyright (C) 1995 by IBK-Landquart-Switzerland\n";
    cout << "Enter \"?\" or \"HELP\" for instructions.\n";
    istr[ipos] = &cin;
    ipara[ipos][0] = "standard input";
    if (argc > 1) {
        input = IString("@ ")+argv[1];
        for (int p = 2; p < argc; p++) input += IString(" \"")+argv[p]+"\"";
        cout << input << '\n';
        redirect(1);
    } else {
        input = "@ autoexec.tss";
        redirect(0);
    } // if
    for (;;) {
        int semiColon;
        IString word1;
        if (!ipos) cout << "TSS> ";
        input = IString::lineFrom(*istr[ipos]);
        expand(input);
        if (istr[ipos]->eof()) {
            log(IString("eof on ")+ipara[ipos][0]);
            if (ipos) {
                delete istr[ipos--];
                continue;
            } else
                goto ende;
        } // if
        if (ipos && echo) cout << "TSS> " << input << '\n';
        if (lname!="" && echo) *lstr << "TSS> " << input << '\n';
        if (semiColon = input.lastIndexOf(';')) 
            input = input.subString(1,semiColon-1);
        input = input.strip();
        switch (input[1]) {
            case '?':
            case '$':
            case '@':
                input.insert(" ",1);
            default:;
        } // switch
        word1 = input.word(1).upperCase();
        if (word1=="RETURN") {
            if (ipos) {
                delete istr[ipos--];
                continue;
            } else
                goto ende;
        } // if
        if (word1=="EXIT") {
            ende:
            try {
                if (connected) disconnect();
                if (lname!="") {
                    input = "LOG CLOSE";
                    setlog();
                } // if
            } // try
            catch ( IException err ) {
                // show full exception information
                err.logExceptionData();
            } // catch
            break;
        } else  {
            try {
                if (word1=="REM" || word1=="REMARK") ; else
                if (word1=="LOG"       ) setlog(); else
                if (word1=="CONNECT"   ) connect(); else
                if (word1=="DISCONNECT") disconnect(); else
                if (word1=="DESCRIBE"  ) describe(); else
                if (word1=="COMMIT"    ) commit(); else
                if (word1=="ROLLBACK"  ) rollback(); else
                if (word1=="HELP" || word1=="?") help(); else
                if (word1=="HOST" || word1=="$") host(); else
                if (word1=="CALL" || word1=="@") redirect(1); else
                if (word1=="SET"       ) set(); else
                if (word1=="SHOW"      ) show(); else
                if (word1!=""          ) {
                    while (!semiColon) {
                        if (!ipos) cout << "   > ";
                        IString nxt = IString::lineFrom(*istr[ipos]);
                        expand(nxt);
                        if (istr[ipos]->eof()) break;
                        if (ipos && echo) cout << "   > " << nxt << '\n';
                        if (lname!="" && echo) *lstr << "   > " << nxt << '\n';
                        if (semiColon = nxt.lastIndexOf(';'))
                            nxt = nxt.subString(1,semiColon-1);
                        input += " "+nxt.strip();
                    } // while
                    execute();
                } // if
            } // try 
            catch ( IException err ) {
               // show text only
               for (int i = err.textCount()-1; i >= 0; i--)
                    cout << err.text(i) << '\n';
            } // catch
        } // if
    } // for
    return 0;
} // main
