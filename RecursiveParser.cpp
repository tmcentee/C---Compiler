//
//  RecursiveParser.cpp
//
//  Created by Tyler McEntee on 3/16/14.
//  Copyright (c) 2014 Tyler McEntee. All rights reserved.
//

#include "RecursiveParser.h"
#include "Global.h"
#include "LexicalAnalyzer.h"
#include "SymbolTable.h"
#include "Sym.h"
#include <stdlib.h>
#include <string>
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <sstream>

template <typename T>
string NumberToString ( T Number )
{
   ostringstream ss;
   ss << Number;
   return ss.str();
}

/*******************************************************************************
 ***  FUNCTION RecursiveParser()
 *******************************************************************************
 ***  DESCRIPTION  :  Constructor for Recursive Descent Parser.
 ******************************************************************************/
RecursiveParser::RecursiveParser(Global & global_init, LexicalAnalyzer & lex_init, SymbolTable & symtab_init)
{
   global = &global_init;
   lex = &lex_init;
   symtab = &symtab_init;

   filename = lex->filename;

   //couldn't get rfind() and replace() working together so this is what I did.
   for (int i = 0; i < filename.length(); i++)
   {
      if (filename[i] == '.')
      {
         filename.replace(filename.begin() + i, filename.end(), ".tac");
         i = 50000;
      }
   }

   tempNum = 1;
   strNum = 0;

   funcname = "::EMPTY::";
   base = NULL;

   isSign = false;
   sign = string();

   offset = 2;

   depth = 1;
}

/*******************************************************************************
 ***  FUNCTION ~RecursiveParser()
 *******************************************************************************
 ***  DESCRIPTION  :  Destructor for Recursive Descent Parser.
 ******************************************************************************/
RecursiveParser::~RecursiveParser()
{
}

/*******************************************************************************
 ***  FUNCTION end()
 *******************************************************************************
 ***  DESCRIPTION  :  final actions for RDP
 ******************************************************************************/
void RecursiveParser::end()
{
   writeToFile();

   EntryPtr peek = symtab->lookup("main");

   if (peek->TypeOfEntry != functionEntry)
   {
      cout << "ERROR - Function 'main' not found." << endl;
      raise(SIGINT);
   }
}

/*******************************************************************************
 ***  FUNCTION match()
 *******************************************************************************
 ***  DESCRIPTION  :  Matches a token from the lexer to an expected token from RDP
 ******************************************************************************/
void RecursiveParser::match(Global::Symbol match)
{
   if (match == global->Token)
      lex->GetNextToken();

   else
   {
      cout << "ERROR - Token mismatch at line " << lex->line_num << ". Expected: " << EnumToString(match)
           << " Received: " << EnumToString(global->Token) << endl;
      raise(SIGINT);
   }

   if (global->Token == Global::commentt)
      lex->GetNextToken();

}

/*******************************************************************************
 ***  FUNCTION PROG()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes PROG grammar rule.
 ***
 ***  PROG -> TYPE idt REST PROG |
 ***          const idt = num ; PROG |
 ***          e
 ******************************************************************************/
void RecursiveParser::PROG()
{
   EntryPtr ptr;
   VarType type;
   ParamPtr paramptr = NULL;
   base = NULL;
   bool func;
   string code;

   int local_vars = 0;
   int param_num = 0;

   if (global->Token == Global::intt || global->Token == Global::floatt || global->Token == Global::chart)
   {
      TYPE(type);

      symtab->insert(global->Lexeme, global->Token, depth);
      ptr = symtab->lookup(global->Lexeme);

      funcname = global->Lexeme;

      match(Global::idt);

      if (global->Token == Global::lparent)
      {
         func = true;
         code = "proc " + funcname;
         emit(code);

         //if function
         ptr->TypeOfEntry = functionEntry;
         ptr->function.ReturnType = type;
         ptr->function.ParamList = new ParamNode();
         paramptr = ptr->function.ParamList;
         ptr->function.NumberOfParameters = 0;
         base = paramptr;
      }
      else
      {
         funcname = "::EMPTY::";
         //if variable declaration
         ptr->TypeOfEntry = varEntry;
         ptr->var.TypeOfVariable = type;
         ptr->var.Offset = offset;
         ptr->var.size = getsize(type, offset);
      }

      REST(type, offset, paramptr, local_vars, param_num);

      if (func)
      {
         code = "endp " + funcname;
         emit(code);

         code = "START PROC " + funcname;
         emit(code);

         func = false;
      }

      PROG();
   }

   //rest is for constant
   else if (global->Token == Global::constt)
   {
      match(Global::constt);

      symtab->insert(global->Lexeme, global->Token, depth);
      ptr = symtab->lookup(global->Lexeme);
      ptr->TypeOfEntry = constEntry;

      match(Global::idt);
      match(Global::assignopt);

      if (lex->isFloat)
      {
         ptr->constant.TypeOfConstant = floatType;
         ptr->constant.ValueR = global->ValueR;
      }
      else
      {
         ptr->constant.TypeOfConstant = intType;
         ptr->constant.Value = global->Value;
      }

      match(Global::numt);
      match(Global::semicolont);
      PROG();
   }
   else
      return;
}

/*******************************************************************************
 ***  FUNCTION TYPE()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes TYPE grammar rule.
 ***
 ***  TYPE ->  float |
 ***           intt |
 ***           char
 ******************************************************************************/
void RecursiveParser::TYPE(VarType & type)
{
   switch(global->Token)
   {
      case Global::floatt:
         match(Global::floatt);
         type = floatType;
         break;

      case Global::intt:
         match(Global::intt);
         type = intType;
         break;

      case Global::chart:
         match(Global::chart);
         type = charType;
         break;
   }
}

/*******************************************************************************
 ***  FUNCTION REST()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes REST grammar rule.
 ***
 ***  REST -> ( PARAMLIST ) COMPOUND |
 ***          IDTAIL ; PROG
 ******************************************************************************/
void RecursiveParser::REST(VarType & type, int & offset, ParamPtr & paramptr, int & local_size, int & param_num)
{
   if (global->Token == Global::lparent)
   {
      depth++;
      offset = 2;
      param_num = 0;
      int param_size = 0;

      match(Global::lparent);
      PARAMLIST(offset, paramptr, local_size, param_num, param_size);

      EntryPtr ptr = symtab->lookup(funcname);
      ptr->function.ParamList = base;
      ptr->function.SizeOfLocal = local_size + offset;
      ptr->function.NumberOfParameters = param_num;
      ptr->function.sizeOfParams = param_size;
      if (param_num == 0)
         base = NULL;

//      cout << "Depth change: " << depth-1 << " to " << depth << ": Enter to Continue"<< endl;
//      cin.ignore();
//      system("clear");
//      symtab->writeTable(depth-1);

      match(Global::rparent);
      COMPOUND(offset);

//      cout << "Depth change: " << depth+1 << " to " << depth << ": Enter to Continue"<< endl;
//      cin.ignore();
//      system("clear");
//      symtab->writeTable(depth+1);
   }
   else
   {
      IDTAIL(type, offset);
      match(Global::semicolont);
      PROG();
   }
}

/*******************************************************************************
 ***  FUNCTION PARAMLIST()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes PARAMLIST grammar rule.
 ***
 ***  PARAMLIST -> TYPE idt PARAMTAIL |
 ***               e
 ******************************************************************************/
void RecursiveParser::PARAMLIST(int & offset, ParamPtr & paramptr, int & local_size, int & param_num, int & param_size)
{
   if (global->Token == Global::intt || global->Token == Global::floatt || global->Token == Global::chart)
   {
      EntryPtr ptr;
      VarType type;

      TYPE(type);

      checkduplicate(global->Lexeme, depth);
      symtab->insert(global->Lexeme, global->Token, depth);
      ptr = symtab->lookup(global->Lexeme);
      ptr->TypeOfEntry =  varEntry;
      ptr->var.Offset = offset + typesize(type);
      ptr->var.TypeOfVariable = type;
      ptr->var.size = typesize(type);
      ptr->isParam = true;

      paramptr->typeOfParameter = type;

      param_size += typesize(type);

      local_size += getsize(type, offset);
      param_num++;

      match(Global::idt);
      PARAMTAIL(offset, paramptr, local_size, param_num, param_size);
   }
   else
      return;
}

/*******************************************************************************
 ***  FUNCTION PARAMTAIL()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes PARAMTAIL grammar rule.
 ***
 ***  PARAMTAIL ->   , TYPE idt PARAMTAIL |
 ***                 e
 ******************************************************************************/
void RecursiveParser::PARAMTAIL(int & offset, ParamPtr & paramptr, int & local_size, int & param_num, int & param_size)
{
   if (global->Token == Global::commat)
   {
      EntryPtr ptr;
      VarType type;

      paramptr->Next = new ParamNode();
      paramptr = paramptr->Next;

      match(Global::commat);
      TYPE(type);

      checkduplicate(global->Lexeme, depth);
      symtab->insert(global->Lexeme, global->Token, depth);
      ptr = symtab->lookup(global->Lexeme);
      ptr->TypeOfEntry = varEntry;
      ptr->var.TypeOfVariable = type;
      ptr->var.Offset = offset + typesize(type);
      ptr->var.size = typesize(type);
      ptr->isParam = true;

      param_size += typesize(type);

      paramptr->typeOfParameter = type;

      local_size += getsize(type, offset);
      param_num++;

      match(Global::idt);
      PARAMTAIL(offset, paramptr, local_size, param_num, param_size);
   }
   else
      return;
}

/*******************************************************************************
 ***  FUNCTION COMPOUND()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes COMPOUND grammar rule.
 ***
 ***  COMPOUND -> { DECL STAT_LIST RET_STAT }
 ******************************************************************************/
void RecursiveParser::COMPOUND(int & offset)
{
   match(Global::lbracet);
   DECL(offset);

   STAT_LIST();

   RET_STAT();

   symtab->deleteDepth(depth);

   match(Global::rbracet);
}

/*******************************************************************************
 ***  FUNCTION DECL()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes DECL grammar rule.
 ***
 ***  DECL ->  TYPE IDLIST |
 ***           const idt = num ; DECL |
 ***           e
 ******************************************************************************/
void RecursiveParser::DECL(int & offset)
{
   EntryPtr ptr;
   VarType type;

   if (global->Token == Global::intt || global->Token == Global::floatt || global->Token == Global::chart)
   {
      TYPE(type);
      IDLIST(type, offset);
   }
   else if (global->Token == Global::constt)
   {
      match(Global::constt);

      checkduplicate(global->Lexeme, depth);
      symtab->insert(global->Lexeme, global->Token, depth);
      ptr = symtab->lookup(global->Lexeme);
      ptr->TypeOfEntry = constEntry;

      match(Global::idt);
      match(Global::assignopt);

      if (lex->isFloat)
      {
         ptr->constant.TypeOfConstant = floatType;
         ptr->constant.ValueR = global->ValueR;
      }
      else
      {
         ptr->constant.TypeOfConstant = intType;
         ptr->constant.Value = global->Value;
      }
      match(Global::numt);
      match(Global::semicolont);
      DECL(offset);
   }
   else
      return;
}

/*******************************************************************************
 ***  FUNCTION IDLIST()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes IDLIST grammar rule.
 ***
 ***  IDLIST -> idt IDTAIL ; DECL
 ******************************************************************************/
void RecursiveParser::IDLIST(VarType & type, int & offset)
{
   EntryPtr ptr;

   checkduplicate(global->Lexeme, depth);
   symtab->insert(global->Lexeme, global->Token, depth);
   ptr = symtab->lookup(global->Lexeme);
   ptr->TypeOfEntry = varEntry;
   ptr->var.TypeOfVariable = type;
   ptr->var.Offset = offset;
   ptr->var.size = typesize(type);
   getsize(type, offset);

   match(Global::idt);
   IDTAIL(type, offset);
   match(Global::semicolont);
   DECL(offset);
}

/*******************************************************************************
 ***  FUNCTION IDTAIL()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes IDTAIL grammar rule.
 ***
 ***  IDTAIL ->   , idt IDTAIL |
 ***              e
 ******************************************************************************/
void RecursiveParser::IDTAIL(VarType & type, int & offset)
{
   if (global->Token == Global::commat)
   {
      EntryPtr ptr;

      match(Global::commat);

      checkduplicate(global->Lexeme, depth);
      symtab->insert(global->Lexeme, global->Token, depth);
      ptr = symtab->lookup(global->Lexeme);
      ptr->TypeOfEntry = varEntry;
      ptr->var.TypeOfVariable = type;
      ptr->var.Offset = offset;
      ptr->var.size = typesize(type);
      getsize(type, offset);

      match(Global::idt);
      IDTAIL(type, offset);
   }
}

/*******************************************************************************
 ***  FUNCTION STAT_LIST()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes STAT_LIST grammar rule.
 ***
 ***  STAT_LIST -> STATEMENT ; STAT_LIST |
 ***               e
 ***
 ******************************************************************************/
void RecursiveParser::STAT_LIST()
{
   if (global->Token == Global::idt || global->Token == Global::cint || global->Token == Global::coutt)
   {
      STATEMENT();
      match(Global::semicolont);
      STAT_LIST();
   }
}

/*******************************************************************************
 ***  FUNCTION RET_STAT()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes RET_STAT grammar rule.
 ***
 ***  RET_STAT -> returnt EXPR ;
 ***
 ******************************************************************************/
void RecursiveParser::RET_STAT()
{
   TableEntry eplace;
   string code;

   if (global->Token == Global::returnt)
   {
      match(Global::returnt);
      EXPR(eplace);

      code += "AX = ";

      if (eplace.depth <= 1)
         code += eplace.Lexeme;

      else
      {
         code += "[bp";

         if (eplace.isParam)
            code += "+";
         else
            code += "-";

         code += NumberToString(eplace.var.Offset);
         code += "]";
      }

      emit(code);

      match(Global::semicolont);
   }
   else
   {
      cout << "ERROR: Unexpected symbol " << global->Lexeme << " at line " << lex->line_num << "  Expected: return" << endl;
      raise(SIGINT);
   }
}

/*******************************************************************************
 ***  FUNCTION STATEMENT()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes STATEMENT grammar rule.
 ***
 ***  STATEMENT ->   ASSIGNSTAT |
 ***                 IOSTAT
 ***
 ******************************************************************************/
void RecursiveParser::STATEMENT()
{
   if (global->Token == Global::idt)
      ASSIGNSTAT();

   else
      IOSTAT();
}

/*******************************************************************************
 ***  FUNCTION ASSIGNSTAT()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes ASSIGNSTAT grammar rule.
 ***
 ***  ASSIGNSTAT -> idt = EXPR |
 ***                idt = FUNCTIONCALL
 ***
 ******************************************************************************/
void RecursiveParser::ASSIGNSTAT()
{
   TableEntry left = symtab->lookupT(global->Lexeme);
   string code;
   TableEntry eplace;

   if (left.depth == depth || left.depth == 1)
   {
      match(Global::idt);
      match(Global::assignopt);

         if (global->Token == Global::idt)
         {
            TableEntry peek = symtab->lookupT(global->Lexeme);
            EntryType var = peek.TypeOfEntry;

            if (var == functionEntry)
               FUNCTIONCALL(left);

            else if (var == varEntry || var == constEntry)
            {
               EXPR(eplace);

               if (left.depth <= 1)
                  code += left.Lexeme;

               else
               {
                  code += "[bp";

                  if (left.isParam)
                     code += "+";
                  else
                     code += "-";

                  code += NumberToString(left.var.Offset);
                  code += "]";
               }

               code += " = ";

               if (eplace.TypeOfEntry == constEntry)
                  code += eplace.constant.Value;

               else
               {
                  if (eplace.depth <= 1)
                     code += eplace.Lexeme;

                  else
                  {
                     code += "[bp";

                     if (eplace.isParam)
                        code += "+";
                     else
                        code += "-";

                     code += NumberToString(eplace.var.Offset);
                     code += "]";
                  }
               }

               emit(code);
            }

            else
            {
               cout << "ERROR: Unexpected symbol " << global->Lexeme << " at line " << lex->line_num << endl;
               raise(SIGINT);
            }
         }

         else if (global->Token == Global::numt || global->Token == Global::addopt || global->Token == Global::nott || global->Token == Global::lparent)
         {
            EXPR(eplace);

            if (left.depth <= 1)
               code += left.Lexeme;

            else
            {
               code += "[bp";

               if (left.isParam)
                  code += "+";
               else
                  code += "-";

               code += NumberToString(left.var.Offset);
               code += "]";
            }
            code += " = ";

            if (isSign)
            {
               code += sign;
               isSign = false;
            }

            if (eplace.TypeOfEntry == constEntry)
            {
               if (eplace.constant.TypeOfConstant == floatType)
                  code += NumberToString(eplace.constant.ValueR);

               else
                  code += NumberToString(eplace.constant.Value);
            }
            else
            {
               if (eplace.depth <= 1)
                  code += eplace.Lexeme;

               else
               {
                  code += "[bp";

                  if (eplace.isParam)
                     code += "+";
                  else
                     code += "-";

                  code += NumberToString(eplace.var.Offset);
                  code += "]";
               }
            }

            emit(code);
         }

         else
         {
            cout << "ERROR: Unexpected symbol " << global->Lexeme << " at line " << lex->line_num << endl;
            raise(SIGINT);
         }
   }

   else {}

}

/*******************************************************************************
 ***  FUNCTION IOSTAT()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes IOSTAT grammar rule.
 ***
 ***     IOSTAT ->   IN_STAT | OUT_STAT
 ***
 ******************************************************************************/
void RecursiveParser::IOSTAT()
{
   if (global->Token == Global::cint)
      IN_STAT();

   else if (global->Token == Global::coutt)
      OUT_STAT();

   else
   {
      cout << "ERROR: Unexpected symbol " << global->Lexeme << " at line " << lex->line_num << endl;
      raise(SIGINT);
   }

}

/*******************************************************************************
 ***  FUNCTION IN_STAT()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes IOSTAT grammar rule.
 ***
 ***     IN_STAT ->   cin >> idt IN_END
 ***
 ******************************************************************************/
void RecursiveParser::IN_STAT()
{
   string code;

   code += "rd";

   match(Global::cint);
   match(Global::instreamt);

   EntryPtr peek = symtab->lookup(global->Lexeme);

   if (peek->var.TypeOfVariable == intType)
      code += "i ";
   else
      code += "c ";

   if (peek->depth == 1)
   {
      code += peek->Lexeme;
   }
   else
   {
      code += "[bp";

      if (peek->isParam)
         code += "+";
      else
         code += "-";

      code += NumberToString(peek->var.Offset);
      code += "]";
   }

   emit(code);

   match(Global::idt);
   IN_END();
}

/*******************************************************************************
 ***  FUNCTION IN_END()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes IN_END grammar rule.
 ***
 ***     IN_END ->   >> idt IN_END |
 ***                 e
 ***
 ******************************************************************************/
void RecursiveParser::IN_END()
{
   string code;

   code += "rd";

   if (global->Token == Global::instreamt)
   {
      match(Global::instreamt);

      EntryPtr peek = symtab->lookup(global->Lexeme);

      if (peek->var.TypeOfVariable == intType)
         code += "i ";
      else
         code += "c ";

      if (peek->depth == 1)
         code += peek->Lexeme;

      else
      {
         code += "[bp";

         if (peek->isParam)
            code += "+";
         else
            code += "-";

         code += NumberToString(peek->var.Offset);
         code += "]";
      }

      emit(code);

      match(Global::idt);
      IN_END();
   }
}

/*******************************************************************************
 ***  FUNCTION OUT_STAT()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes OUT_STAT grammar rule.
 ***
 ***     OUT_STAT ->   cout << OUT_OPTIONS OUT_END
 ***
 ******************************************************************************/
void RecursiveParser::OUT_STAT()
{
   match(Global::coutt);
   match(Global::outstreamt);
   OUT_OPTIONS();
   OUT_END();
}

/*******************************************************************************
 ***  FUNCTION OUT_OPTIONS()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes OUT_OPTIONS grammar rule.
 ***
 ***     OUT_OPTIONS ->   idt | Literal | endl
 ***
 ******************************************************************************/
void RecursiveParser::OUT_OPTIONS()
{
   string code;

   code += "wr";

   if (global->Token == Global::idt)
   {
      EntryPtr peek = symtab->lookup(global->Lexeme);

      if (peek->var.TypeOfVariable == intType)
         code += "i ";
      else
         code += "c ";

      if (peek->depth == 1)
         code += peek->Lexeme;

      else
      {
         code += "[bp";

         if (peek->isParam)
            code += "+";
         else
            code += "-";

         code += NumberToString(peek->var.Offset);
         code += "]";
      }

      match(Global::idt);
      emit(code);
   }
   else if (global->Token == Global::literalt)
   {
      string temp = "_S" + NumberToString(strNum);

      code += "s";
      code += " " + temp;
      strNum++;

      symtab->insert(temp, Global::literalt, 1);
      EntryPtr peek = symtab->lookup(temp);
      peek->isLiteral = true;
      peek->Literal = global->Lexeme;
      peek->TypeOfEntry = literalEntry;

      match(Global::literalt);
      emit(code);
   }
   else if (global->Token == Global::endlt)
   {
      code += "tln";
      match(Global::endlt);
      emit(code);
   }
   else
   {
      cout << "ERROR: Unexpected symbol " << global->Lexeme << " at line " << lex->line_num << endl;
      raise(SIGINT);
   }
}

/*******************************************************************************
 ***  FUNCTION OUT_END()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes OUT_END grammar rule.
 ***
 ***     OUT_END ->   << OUT_OPTIONS OUT_END | e
 ***
 ******************************************************************************/
void RecursiveParser::OUT_END()
{
   if (global->Token == Global::outstreamt)
   {
      match(Global::outstreamt);
      OUT_OPTIONS();
      OUT_END();
   }
}

/*******************************************************************************
 ***  FUNCTION EXPR()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes EXPR grammar rule.
 ***
 ***  EXPR -> RELATION
 ***
 ******************************************************************************/
void RecursiveParser::EXPR(TableEntry & eplace)
{
   RELATION(eplace);
}

/*******************************************************************************
 ***  FUNCTION RELATION()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes RELATION grammar rule.
 ***
 ***  RELATION -> SIGNOP SIMPLEEXPR
 ***
 ******************************************************************************/
void RecursiveParser::RELATION(TableEntry & eplace)
{
   SIGNOP();
   SIMPLEEXPR(eplace);
}

/*******************************************************************************
 ***  FUNCTION SIMPLEEXPR()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes SIMPLEEXPR grammar rule.
 ***
 ***  SIMPLEEXPR -> TERM MORETERM
 ***
 ******************************************************************************/
void RecursiveParser::SIMPLEEXPR(TableEntry & eplace)
{
   TableEntry tplace;

   TERM(tplace);
   MORETERM(tplace);

   eplace = tplace;
}

/*******************************************************************************
 ***  FUNCTION MORETERM()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes MORETERM grammar rule.
 ***
 ***  MORETERM -> ADDOP TERM MORETERM | e
 ***
 ******************************************************************************/
void RecursiveParser::MORETERM(TableEntry & rplace)
{
   TableEntry tplace;
   TableEntry temp;
   string code;

   if (global->Token == Global::addopt || global->Token == Global::relopt)
   {
      temp = tempVar();

      if (temp.depth <= 1)
         code += temp.Lexeme;

      else
      {
         code += "[bp-" + NumberToString(temp.var.Offset);
         code += "]";
      }

      code += " = ";

      if (rplace.TypeOfEntry == varEntry)
      {
         if (rplace.depth <= 1)
            code += rplace.Lexeme;

         else
         {
            code += "[bp";

            if (rplace.isParam)
               code += "+";
            else
               code += "-";

            code += NumberToString(rplace.var.Offset);
            code += "]";
         }
      }
      else if (rplace.TypeOfEntry == constEntry)
      {
         if (rplace.constant.TypeOfConstant == floatType)
            code += NumberToString(rplace.constant.ValueR);

         else
            code += NumberToString(rplace.constant.Value);
      }

      code += " " + global->Lexeme + " ";

      ADDOP();

      TERM(tplace);

      if (tplace.TypeOfEntry == varEntry)
      {
         if (tplace.depth <= 1)
            code += tplace.Lexeme;

         else
         {
            code += "[bp";

            if (tplace.isParam)
               code += "+";
            else
               code += "-";

            code += NumberToString(tplace.var.Offset);
            code += "]";
         }
      }
      else if (tplace.TypeOfEntry == constEntry)
      {
         if (tplace.constant.TypeOfConstant == floatType)
            code += NumberToString(tplace.constant.ValueR);

         else
            code += NumberToString(tplace.constant.Value);
      }

      rplace = temp;
      emit(code);
      MORETERM(rplace);
   }
}

/*******************************************************************************
 ***  FUNCTION TERM()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes TERM grammar rule.
 ***
 ***  TERM -> FACTOR MOREFACTOR
 ***
 ******************************************************************************/
void RecursiveParser::TERM(TableEntry & tplace)
{
   TableEntry fplace;

   FACTOR(fplace);
   MOREFACTOR(fplace);

   tplace = fplace;
}

/*******************************************************************************
 ***  FUNCTION MOREFACTOR()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes MOREFACTOR grammar rule.
 ***
 ***  MOREFACTOR -> MULOP FACTOR MOREFACTOR
 ***
 ******************************************************************************/
void RecursiveParser::MOREFACTOR(TableEntry & rplace)
{
   TableEntry tplace;
   string code;

   if (global->Token == Global::mulopt || global->Token == Global::relopt || global->Token == Global::idt || global->Token == Global::numt || global->Token == Global::lparent)
   {
      TableEntry temp = tempVar();

      if (temp.depth <= 1)
         code += temp.Lexeme;

      else
      {
         code += "[bp-";
         code += NumberToString(temp.var.Offset);
         code += "]";
      }

      code += " = ";

      if (isSign)
      {
         code += sign;
         isSign = false;
      }

      if (rplace.TypeOfEntry == constEntry)
      {
         if (rplace.constant.TypeOfConstant == floatType)
            code += NumberToString(rplace.constant.ValueR);

         else
            code += NumberToString(rplace.constant.Value);
      }
      else
      {
         if (rplace.depth <= 1)
            code += rplace.Lexeme;

         else
         {
            code += "[bp";

            if (rplace.isParam)
               code += "+";
            else
               code += "-";

            code += NumberToString(rplace.var.Offset);
            code += "]";
         }
      }

      code += " " + global->Lexeme + " ";

      MULOP();

      FACTOR(tplace);

      if (tplace.TypeOfEntry == constEntry)
      {
         if (tplace.constant.TypeOfConstant == floatType)
            code += NumberToString(tplace.constant.ValueR);

         else
            code += NumberToString(tplace.constant.Value);
      }
      else
      {
         if (tplace.depth <= 1)
            code += tplace.Lexeme;

         else
         {
            code += "[bp";

            if (tplace.isParam)
               code += "+";
            else
               code += "-";

            code += NumberToString(tplace.var.Offset);
            code += "]";
         }
      }

      rplace = temp;
      emit(code);
      MOREFACTOR(rplace);
   }
}

/*******************************************************************************
 ***  FUNCTION FACTOR()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes FACTOR grammar rule.
 ***
 ***  FACTOR ->   idt        |
 ***              numt       |
 ***            ( EXPR )
 ***
 ******************************************************************************/
void RecursiveParser::FACTOR(TableEntry & tplace)
{
   TableEntry rplace;
   string code;

   if (global->Token == Global::idt)
   {
      EntryPtr peek = symtab->lookup(global->Lexeme);

      if (peek->depth == depth || peek->depth == 1)
      {
         tplace = symtab->lookupT(global->Lexeme);
         match(Global::idt);
      }
      else
      {
         cout << "ERROR: Undefined variable: " << global->Lexeme << " at line " << lex->line_num << endl;
         raise(SIGINT);
      }
   }
   else if (global->Token == Global::numt)
   {
      TableEntry temp = tempVar();

      if (temp.depth <= 1)
         code += temp.Lexeme;

      else
      {
         code += "[bp-";
         code += NumberToString(temp.var.Offset);
         code += "]";
      }

      code += " = ";

      if (lex->isFloat)
         code += NumberToString(global->ValueR);

      else
         code += NumberToString(global->Value);

      tplace = temp;
      emit(code);

      match(Global::numt);
   }
   else if (global->Token == Global::lparent)
   {
      match(Global::lparent);
      EXPR(rplace);
      match(Global::rparent);

      tplace = rplace;
   }
   else
   {
      cout << "ERROR: Unexpected symbol " << global->Lexeme << " at line " << lex->line_num << endl;
      raise(SIGINT);
   }
}

/*******************************************************************************
 ***  FUNCTION ADDOP()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes ADDOP grammar rule.
 ******************************************************************************/
void RecursiveParser::ADDOP()
{
   if (global->Token == Global::addopt)
      match(Global::addopt);

   else if (global->Token == Global::relopt)
      match(Global::relopt);

   else {}
}

/*******************************************************************************
 ***  FUNCTION MULOP()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes MULOP grammar rule.
 ******************************************************************************/
void RecursiveParser::MULOP()
{
   if (global->Token == Global::mulopt)
      match(Global::mulopt);

   else if (global->Token == Global::relopt)
      match(Global::relopt);

   else {}
}

/*******************************************************************************
 ***  FUNCTION SIGNOP()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes SIGNOP grammar rule.
 ******************************************************************************/
void RecursiveParser::SIGNOP()
{
   if (global->Token == Global::nott)
   {
      isSign = true;
      sign = global->Lexeme;
      match(Global::nott);

   }
   else if (global->Token == Global::addopt)
   {
      isSign = true;
      sign = global->Lexeme;
      match(Global::addopt);
   }
   else
   {
      isSign = false;
      sign = string();
   }
}

/*******************************************************************************
 ***  FUNCTIONCALL ()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes FUNCTIONCALL grammar rule.
 ***
 ***  FUNCTIONCALL ->   idt ( PARAMS )
 ***
 ******************************************************************************/
void RecursiveParser::FUNCTIONCALL(TableEntry left1)
{
   if (global->Token == Global::idt)
   {
      EntryPtr peek = symtab->lookup(global->Lexeme);

      if (peek->TypeOfEntry == functionEntry)
      {
         string code = "call " + global->Lexeme + '\n';

         EntryPtr left = symtab->lookup(left1.Lexeme);

         if (left->depth <= 1)
            code += left->Lexeme;

         else
         {
            code += "[bp";

            if (left->isParam)
               code += "+";
            else
               code += "-";

            code += NumberToString(left->var.Offset);
            code += "]";
         }

         code += " = AX";

         match(Global::idt);
         match(Global::lparent);
         PARAMS();
         match(Global::rparent);

         emit(code);
      }
      else
      {
         cout << "ERROR: Undefined function " << global->Lexeme << " at line " << lex->line_num << endl;
         raise(SIGINT);
      }
   }
   else
   {
      cout << "ERROR: Unexpected symbol " << global->Lexeme << " at line " << lex->line_num << endl;
      raise(SIGINT);
   }
}

/*******************************************************************************
 ***  FUNCTION PARAMS()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes PARAMS grammar rule.
 ***
 ***  PARAMS ->   idt PARAMSTAIL  |
 ***              num PARAMSTAIL  |
 ***              e
 ***
 ******************************************************************************/
void RecursiveParser::PARAMS()
{
   if (global->Token == Global::idt)
   {
      EntryPtr peek = symtab->lookup(global->Lexeme);

      if (peek->depth == depth || peek->depth == 1)
      {
         string code;

         code += "push ";

         if (peek->depth <= 1)
            code += peek->Lexeme;

         else
         {
            code += "[bp";

            if (peek->isParam)
               code += "+";
            else
               code += "-";

            code += NumberToString(peek->var.Offset);
            code += "]";
         }

         match(Global::idt);
         PARAMSTAIL();
         emit(code);
      }
      else
      {
         cout << "ERROR: " << global->Lexeme << " at line " << lex->line_num << " not defined in current scope." << endl;
         raise(SIGINT);
      }
   }
   else if (global->Token == Global::numt)
   {
      string code;

      code += "push ";

      if (lex->isFloat)
         code += NumberToString(global->ValueR);
      else
         code += NumberToString(global->Value);

      match(Global::numt);
      PARAMSTAIL();
      emit(code);
   }
   else {}
}

/*******************************************************************************
 ***  FUNCTION ;()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes PARAMSTAIL grammar rule.
 ***
 ***  PARAMSTAIL ->   , idt PARAMSTAIL  |
 ***                  , num PARAMSTAIL  |
 ***                  e
 ******************************************************************************/
void RecursiveParser::PARAMSTAIL()
{

   if (global->Token == Global::commat)
   {
      match(Global::commat);

      if (global->Token == Global::idt)
      {
         EntryPtr peek = symtab->lookup(global->Lexeme);

         if (peek->depth == depth || peek->depth == 1)
         {
            string code;

            code += "push ";

            if (peek->depth <= 1)
               code += peek->Lexeme;

            else
            {
               code += "[bp";

               if (peek->isParam)
                  code += "+";
               else
                  code += "-";

               code += NumberToString(peek->var.Offset);
               code += "]";
            }

            match(Global::idt);
            PARAMSTAIL();
            emit(code);
         }
         else
         {
            cout << "ERROR: " << global->Lexeme << " at line " << lex->line_num << " not defined in current scope." << endl;
            raise(SIGINT);
         }
      }
      else if (global->Token == Global::numt)
      {
         string code;

         code += "push ";

         if (lex->isFloat)
            code += NumberToString(global->ValueR);
         else
            code += NumberToString(global->Value);

         match(Global::numt);
         PARAMSTAIL();
         emit(code);
      }

      else {}

   }
}

/*******************************************************************************
 ***  FUNCTION emit()
 *******************************************************************************
 ***  DESCRIPTION  :  emit function for TAC, prints and adds to tac_code vector
 ******************************************************************************/
void RecursiveParser::emit(string code)
{
   cout << code << endl;
   tac_code.push_back(code);
}

/*******************************************************************************
 ***  FUNCTION writeToFile()
 *******************************************************************************
 ***  DESCRIPTION  :  writes TAC to TAC file
 ******************************************************************************/
void RecursiveParser::writeToFile()
{
   ofstream tac_file;
   tac_file.open(filename.c_str(), ios::trunc);

   for (int i = 0; i < tac_code.size(); i++)
      tac_file << tac_code[i] << endl;

   tac_file.close();
}

/*******************************************************************************
 ***  FUNCTION getsize()
 *******************************************************************************
 ***  DESCRIPTION  : gets the size of the VarType for the offset and function size
 ******************************************************************************/
int RecursiveParser::getsize(VarType type, int & offset)
{
   offset += typesize(type);
   return typesize(type);
}

/*******************************************************************************
 ***  FUNCTION typesize()
 *******************************************************************************
 ***  DESCRIPTION  : gets the size of the VarType for the offset and function size
 ******************************************************************************/
int RecursiveParser::typesize(VarType type)
{
   if (type == intType)
      return 2;
   else if (type == floatType)
      return 4;
   else if (type == charType)
      return 1;
   else
      return 0;
}

/*******************************************************************************
 ***  FUNCTION tempVar()
 *******************************************************************************
 ***  DESCRIPTION  : creates a tempVar for TAC
 ******************************************************************************/
TableEntry RecursiveParser::tempVar()
{
   TableEntry temp;
   EntryPtr tvar;

   string var;
   var = "_t" + NumberToString(tempNum);
   tempNum++;

   symtab->insert(var, Global::idt, depth);

   tvar = symtab->lookup(var);
   tvar->TypeOfEntry = varEntry;
   tvar->var.Offset = offset;
   tvar->var.TypeOfVariable = intType;

   temp = symtab->lookupT(var);
   temp.TypeOfEntry = varEntry;
   temp.var.TypeOfVariable = intType;
   temp.var.Offset = offset;

   offset += 2;

   for (int i = 0; i < symtab->table_size; i++)
   {
      for (int k = 0; k < symtab->table[i].size(); k++)
      {
         //if depth found in vector, erase that node
         if (symtab->table[i][k].depth == depth-1 && symtab->table[i][k].TypeOfEntry == functionEntry)
            symtab->table[i][k].function.SizeOfLocal += 2;
      }
   }

   return temp;
}


/*******************************************************************************
 ***  FUNCTION checkduplicate()
 *******************************************************************************
 ***  DESCRIPTION  : checks if there's a duplicate lexeme at the current depth
 ******************************************************************************/
void RecursiveParser::checkduplicate(string check, int depth)
{
   EntryPtr ptr = symtab->lookup(check);

   if (ptr->Lexeme == check && ptr->depth == depth)
      cout << "ERROR: Duplicate identifier " << check << " at line: " << lex->line_num << endl;

}
