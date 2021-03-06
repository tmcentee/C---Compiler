//
//  main.cpp
//
//  Created by Tyler McEntee on 3/31/14.
//  Copyright (c) 2014 Tyler McEntee. All rights reserved.
//
#include "SymbolTable.h"
#include "LexicalAnalyzer.h"
#include "RecursiveParser.h"
#include "CodeGen.h"
#include <cstdlib>
#include <iomanip>
#include "Global.h"

using namespace std;

void print();

int main(int argc, char *argv[])
{
   if (argc < 2)
   {
     cout << "Too few args. Exiting." << endl;
     return 0;
   }

   string filename = argv[1];

   print();

   SymbolTable sym;

   LexicalAnalyzer lex(filename);
   RecursiveParser rdp(lex.global, lex, sym);
   lex.GetNextToken();

   while (lex.global.Token == Global::commentt)
      lex.GetNextToken();

   rdp.PROG();

   if (lex.global.Token == Global::eoft)
      cout << "Success! -    Press ENTER to Continue" << endl;
   else
      cout << "Error: eoft not found." << endl;

   rdp.end();

   cin.ignore();

   CodeGen gen(lex.global, sym, rdp.filename);

   cout << endl << endl << "End of Program:" << endl;
   sym.debug("end of prog");


   return 0;
}


void print()
{
}
