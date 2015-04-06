//
//  SymbolTable.h
//
//  Created by Tyler McEntee on 3/15/14.
//  Copyright (c) 2014 Tyler McEntee. All rights reserved.
//

#ifndef _SYMBOLTABLE_H
#define _SYMBOLTABLE_H

#include <iostream>
#include <string>
#include <vector>
#include "Global.h"
#include "Sym.h"

class SymbolTable {
public:
   void insert(string lex, Global::Symbol token, int depth);
   EntryPtr lookup(string lex);
   TableEntry lookupT(string lex);
   void deleteDepth(int depth);
   void writeTable(int depth);
   void debug(string func);
   
private:
   int hash(const char *str);
   
public:
   static const int table_size = 211;
   vector<TableEntry> table[table_size];
};

string VarTypeString(VarType in);

#endif
