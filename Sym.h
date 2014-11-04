#ifndef SYM_H
#define SYM_H
#include "Global.h"
#include <string>

const int TableSize = 211; //use a prime number

enum VarType {charType, intType, floatType, literalType};

enum EntryType {constEntry, varEntry, functionEntry, literalEntry};

struct ParamNode;

typedef ParamNode * ParamPtr;

struct ParamNode
{
   VarType typeOfParameter;
   ParamPtr Next;
};

struct TableEntry;

typedef TableEntry * EntryPtr; //pointer to actual table entry

struct TableEntry
{
   Global::Symbol Token;
   string Lexeme;
   string Literal;
   int depth;
   EntryType TypeOfEntry; // tag field for the union
   bool isParam;
   bool isLiteral;
   TableEntry() { isParam = false; isLiteral = false; }
   
   union
   {
      struct
      {
         VarType TypeOfVariable;
         int Offset;
         int size;
      } var;
      
      struct
      {
         VarType TypeOfConstant; //int or float constant
         int Offset;
         union
         {
            int Value;
            float ValueR;
         };
      } constant;
      
      struct
      { //this is the entry for a function
         int SizeOfLocal;
         int NumberOfParameters;
         int sizeOfParams;
         VarType ReturnType;
         ParamPtr ParamList; //linked list of paramter types
      } function;
      
   }; //end of union
};

#endif




