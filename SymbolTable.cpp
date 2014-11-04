//
//  SymbolTable.cpp
//  CSC446_A7
//
//  Created by Tyler McEntee on 3/15/14.
//  Copyright (c) 2014 Tyler McEntee. All rights reserved.
//

#include "SymbolTable.h"
#include <cstring>
#include <iomanip>

/*******************************************************************************
 ***  FUNCTION insert()
 *******************************************************************************
 ***  DESCRIPTION  :  inserts a new TableEntry object into the hash table
 ******************************************************************************/
void SymbolTable::insert(string lex, Global::Symbol token, int depth)
{
   TableEntry new_entry;
   new_entry.Token = token;
   new_entry.Lexeme = lex;
   new_entry.depth = depth;
   
   int hash_val = hash(lex.c_str());
   
   for(int i = 0; i < table[hash_val].size(); i++)
   {
      if(table[hash_val][i].Lexeme == lex)
      {
         if(table[hash_val][i].depth == depth)
         {
            cout << "Error: " << lex << " already defined at depth " << depth << endl;
            return;
         }
      }
   }
   
   table[hash_val].insert(table[hash_val].begin(), new_entry);
}

/*******************************************************************************
 ***  FUNCTION lookup()
 *******************************************************************************
 ***  DESCRIPTION  :  searches hash table for a specific TableEntry object
 ******************************************************************************/
EntryPtr SymbolTable::lookup(string lex)
{
   TableEntry lookup;
   
   for(int i = 0; i < table_size; i++)
   {
      for(int k = 0; k < table[i].size(); k++)
      {
         if(table[i][k].Lexeme == lex)
            return &table[i][k];
      }
   }
   
   lookup.Lexeme = string("::ERROR::");
   
   return &lookup;
}

/*******************************************************************************
 ***  FUNCTION lookup()
 *******************************************************************************
 ***  DESCRIPTION  :  searches hash table for a specific TableEntry object
 ******************************************************************************/
TableEntry SymbolTable::lookupT(string lex)
{
   TableEntry lookup;
   
   for(int i = 0; i < table_size; i++)
   {
      for(int k = 0; k < table[i].size(); k++)
      {
         if(table[i][k].Lexeme == lex)
            return table[i][k];
      }
   }
   
   lookup.Lexeme = string("::ERROR::");
   
   return lookup;
}

/*******************************************************************************
 ***  FUNCTION deleteDepth()
 *******************************************************************************
 ***  DESCRIPTION  : deletes all nodes at a certain depth
 ******************************************************************************/
void SymbolTable::deleteDepth(int depth)
{
   for(int i = 0; i < table_size; i++)
   {
      for(int k = 0; k < table[i].size(); k++)
      {
         //if depth found in vector, erase that node
         if(table[i][k].depth == depth)
            table[i].erase(table[i].begin()+k);
      }
   }
}

/*******************************************************************************
 ***  FUNCTION writeTable()
 *******************************************************************************
 ***  DESCRIPTION  : Prints out all entries at a certain depth
 ******************************************************************************/
void SymbolTable::writeTable(int depth)
{
   cout << "TABLE AT DEPTH: " << depth << endl;
   cout << "------------------------------------" << endl;
   
   for(int i = 0; i < table_size; i++)
   {
      for(int k = 0; k < table[i].size(); k++)
      {
         int curr_depth = table[i][k].depth;
         //if depth found in vector, print that node
         if(table[i][k].depth == depth)
         {
            //if depth found in vector, print that node
            cout << setw(9) << left << "Lexeme:" << setw(16) << table[i][k].Lexeme << setw(7) << "Depth:" << setw(3) << table[i][k].depth;
            if(table[i][k].TypeOfEntry == constEntry)
            {
               if(table[i][k].constant.TypeOfConstant == floatType)
                  cout << setw(21) << "Type: constType"<< setw(7) << "Class:" << setw(12) << "floatType" << setw(9) << "ValueR:" << table[i][k].constant.ValueR;
               else
                  cout << setw(21) << "Type: constType"<< setw(7) << "Class:" << setw(12) << "intType" << setw(9) << "Value:" << table[i][k].constant.Value;
            }
            else if(table[i][k].TypeOfEntry == varEntry)
            {
               cout << setw(21) << "Type: varEntry" << setw(7) << "Class:" << setw(12) << VarTypeString(table[i][k].var.TypeOfVariable) << setw(9) << "Offset:" << table[i][k].var.Offset;
            }
            else if(table[i][k].TypeOfEntry == functionEntry)
            {
               cout << setw(21) << "Type: functionEntry" << setw(7) << "Class:" << setw(12) << VarTypeString(table[i][k].function.ReturnType) << setw(9) << "#Params:" << table[i][k].function.NumberOfParameters;
               
               ParamPtr ptr = table[i][k].function.ParamList;
               
               
               if(ptr != NULL)
               {
                  cout << endl << setw(25) << " " << "Params: ";
                  
                  while(ptr != NULL)
                  {
                     cout << setw(9) << VarTypeString(ptr->typeOfParameter);
                     ptr = ptr->Next;
                  }
               }
            }
            
            else
               cout << setw(12) << "undefined";
            
            cout << endl;
         }
      }
   }
}



/*******************************************************************************
 ***  FUNCTION hash()
 *******************************************************************************
 ***  DESCRIPTION  : hashes a string for use in the hash table
 ******************************************************************************/
int SymbolTable::hash(const char *str)
{
   int hashVal = 0;
   
   while (*str)
      hashVal = hashVal << 1 ^ *str++;
   
   return hashVal % table_size;
}


/*****DEBUG*******/

void SymbolTable::debug(string func)
{
   cout << "CURRENT TABLE STRUCTRE - After: " << func <<  endl;
   cout << "----------------------" << endl;
   
   for(int i = 0; i < table_size; i++)
   {
      cout << flush;
      
      for(int k = 0; k < table[i].size(); k++)
      {
         //if depth found in vector, print that node
         cout << setw(9) << left << "Lexeme:" << setw(16) << table[i][k].Lexeme << setw(7) << "Depth:" << setw(3) << table[i][k].depth;
         if(table[i][k].TypeOfEntry == constEntry)
         {
            if(table[i][k].constant.TypeOfConstant == floatType)
               cout << setw(21) << "Type: constType"<< setw(7) << "Class:" << setw(12) << "floatType" << setw(9) << "ValueR:" << table[i][k].constant.ValueR;
            else
               cout << setw(21) << "Type: constType"<< setw(7) << "Class:" << setw(12) << "intType" << setw(9) << "Value:" << table[i][k].constant.Value;
         }
         else if(table[i][k].TypeOfEntry == varEntry)
         {
            cout << setw(21) << "Type: varEntry" << setw(7) << "Class:" << setw(12) << VarTypeString(table[i][k].var.TypeOfVariable) << setw(9) << "Offset:" << table[i][k].var.Offset;
         }
         else if(table[i][k].TypeOfEntry == literalEntry)
         {
            cout << setw(21) << "Type: literalEntry" << setw(7) << "Contents:" << setw(12) << table[i][k].Literal;
         }
         else if(table[i][k].TypeOfEntry == functionEntry)
         {
            cout << setw(21) << "Type: functionEntry" << setw(7) << "Class:" << setw(12) << VarTypeString(table[i][k].function.ReturnType) << setw(9) << "#Params:" << table[i][k].function.NumberOfParameters;
            
            ParamPtr ptr = table[i][k].function.ParamList;
            
            
            if(ptr != NULL && table[i][k].function.NumberOfParameters != 0)
            {
               cout << endl << setw(25) << " " << "Params: ";
               
               while(ptr != NULL)
               {
                  cout << setw(9) << VarTypeString(ptr->typeOfParameter);
                  ptr = ptr->Next;
               }
            }
         }
         
         else
            cout << setw(12) << "undefined";
         
         cout << endl;
      }
   }
}

string VarTypeString(VarType in)
{
   switch(in)
   {
         case floatType:
         return string("floatType");
         break;
         
         case intType:
         return string("intType");
         break;
         
         case charType:
         return string("charType");
         break;
   }
   
   return string("_ERROR_");
}





