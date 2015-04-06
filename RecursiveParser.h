//
//  RecursiveParser.h
//
//  Created by Tyler McEntee on 2/16/14.
//  Copyright (c) 2014 Tyler McEntee. All rights reserved.
//

#ifndef _RecursiveParser_h
#define _RecursiveParser_h

#include <iostream>
#include <iomanip>
#include "Global.h"
#include "LexicalAnalyzer.h"
#include "SymbolTable.h"
using namespace std;

/*******************************************************************************
 ***  CLASS : RecursiveParser
 *******************************************************************************
 ***  DESCRIPTION  :  Class to implement a Recursive Descent Parser for the C-- Lang.
 ******************************************************************************/
class RecursiveParser {
public:
   RecursiveParser(Global &, LexicalAnalyzer &, SymbolTable &);
   ~RecursiveParser();
   
   void match(Global::Symbol);
   
   void end();
   
   void PROG();
   void TYPE(VarType &);
   void REST(VarType &, int &, ParamPtr &, int &, int &);
   void PARAMLIST(int & offset, ParamPtr &, int &, int &, int &);
   void PARAMTAIL(int & offset, ParamPtr &, int &, int &, int &);
   void COMPOUND(int & offset);
   void DECL(int & offset);
   void IDLIST(VarType &,int & offset);
   void IDTAIL(VarType &,int & offset);
   void STAT_LIST();
   void RET_STAT();
   void STATEMENT();
   void ASSIGNSTAT();
   void IOSTAT();
   void IN_STAT();
   void IN_END();
   void OUT_STAT();
   void OUT_OPTIONS();
   void OUT_END();
   
   void EXPR(TableEntry &);
   void RELATION(TableEntry &);
   void SIMPLEEXPR(TableEntry &);
   void MORETERM(TableEntry &);
   void TERM(TableEntry &);
   void MOREFACTOR(TableEntry &);
   void FACTOR(TableEntry &);
   void ADDOP();
   void MULOP();
   void SIGNOP();
   
   void FUNCTIONCALL(TableEntry);
   void PARAMS();
   void PARAMSTAIL();
   
   void checkduplicate(string check, int depth);
   
   
   string filename;
   
private:
   Global *global;
   LexicalAnalyzer *lex;
   SymbolTable *symtab;
   vector<string> tac_code;
   int tempNum;
   int strNum;
   int offset;
   
   bool isSign;
   string sign;
   
   string funcname;
   ParamPtr base;
   
   void emit(string code);
   void writeToFile();
   TableEntry tempVar();
   int getsize(VarType type, int & offset);
   int typesize(VarType type);
   
   void debug_print(string location)
   {
      cout << left << setw(20) << location;
      cout << setw(15) << left << EnumToString(global->Token);
      cout << setw(35) << global->Lexeme;
      if (global->Token == Global::numt)
      {
         if (lex->isFloat)
            cout << setw(35) << global->ValueR;
         else
            cout << setw(35) << global->Value;
      }
      if (global->Token == Global::literalt)
         cout << setw(35) << global->Literal;
      
      cout << endl;
   }
   
   void global_debug()
   {
      cout << "Global" << endl;
      cout << "----------------------" << endl;
      cout << "Lexeme: " << global->Lexeme << endl;
      cout << "Token: " << EnumToString(global->Token) << endl;
      cout << "Value: " << global->Value << endl;
      cout << "ValueR: " << global->ValueR << endl << endl;
      
   }
   
   int depth;
};



#endif /* defined(___46_A2__RecursiveParser__) */
