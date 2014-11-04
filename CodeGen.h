//
//  CodeGen.h
//  A8
//
//  Created by Tyler McEntee on 4/30/14.
//  Copyright (c) 2014 Tyler McEntee. All rights reserved.
//

#ifndef __A8__CodeGen__
#define __A8__CodeGen__

#include "SymbolTable.h"
#include "Global.h"
#include <iostream>
#include <sstream>

class CodeGen
{
public:
   CodeGen(Global &, SymbolTable &, string);
   ~CodeGen();
   void init();
   void data_init();
   void code_init();
   void proc_init();
   void final_init();
   void assign();
   void fnCall();
   void pushParam();
   void addop();
   void subop();
   void mulop();
   void divop();
   void readop();
   void printop();
   void retop();
   
private:
   Global *global;
   SymbolTable *symtab;
   string filename;
   stringstream tacstream;
   string curLine;
   char ch;
   int stream_pos;
   int str_offset;
   int str_num;
   int var_offset;
   bool done;
   
   void GetNextLine();
   void GetNextChar();
   
   vector<string> asm_code;
   
   void emit(string);
   void writeToFile();
};

#endif /* defined(__A8__CodeGen__) */
