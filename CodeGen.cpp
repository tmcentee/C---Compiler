//
//  CodeGen.cpp
//
//  Created by Tyler McEntee on 4/30/14.
//  Copyright (c) 2014 Tyler McEntee. All rights reserved.
//

#include <fstream>
#include <sstream>
#include <cctype>
#include <string>
#include "CodeGen.h"
using namespace std;

template <typename T>
string num2str ( T Number )
{
   ostringstream ss;
   ss << Number;
   return ss.str();
}

/*******************************************************************************
 ***  FUNCTION CodeGen()
 *******************************************************************************
 ***  DESCRIPTION  : constructor
 ******************************************************************************/
CodeGen::CodeGen(Global & global_init, SymbolTable & symtab_init, string tacFile)
{
   global = &global_init;
   symtab = &symtab_init;
   
   stream_pos = 0;
   str_offset = 0;
   str_num = 0;
   var_offset = 0;
   
   done = false;
   
   filename = tacFile;
   
   //couldn't get rfind() and replace() working together so this is what I did.
   for (int i = 0; i < filename.length(); i++)
   {
      if (filename[i] == '.')
      {
         filename.replace(filename.begin() + i, filename.end(), ".asm");
         i = 50000;
      }
   }
   
   ifstream source_file;
   source_file.open(tacFile.c_str());
   
   tacstream << source_file.rdbuf();
   source_file.close();
   
   GetNextChar();
   
   init();
}

/*******************************************************************************
 ***  FUNCTION ~CodeGen()
 *******************************************************************************
 ***  DESCRIPTION  : desctrutor
 ******************************************************************************/
CodeGen::~CodeGen()
{
   writeToFile();
}

/*******************************************************************************
 ***  FUNCTION init()
 *******************************************************************************
 ***  DESCRIPTION  :  starts the TAC -> ASM conversion process
 ******************************************************************************/
void CodeGen::init()
{
   string code;
   
   code += "\t.model small\n\t.stack 100h\n\t.data";
   emit(code);
   
   data_init();
   code_init();
   
   //Get first line in the file.
   GetNextLine();
   
   while (curLine[0] == 'p' && !done)
      proc_init();
      
   
   final_init();
   
}

/*******************************************************************************
 ***  FUNCTION data_init()
 *******************************************************************************
 ***  DESCRIPTION  :  sets up the .data segment for ASM
 ******************************************************************************/
void CodeGen::data_init()
{
   string code;
   
   for (int i = 0; i < symtab->table_size; i++)
   {
      code = "";
      
      for (int k = 0; k < symtab->table[i].size(); k++)
      {
         if (symtab->table[i][k].depth == 1 && symtab->table[i][k].TypeOfEntry != functionEntry)
         {
            if (symtab->table[i][k].TypeOfEntry == constEntry)
            {
               code += symtab->table[i][k].Lexeme + "\tEQU\t";
               code += num2str(symtab->table[i][k].constant.Value) + "\n";
               emit(code);
            }
            else if (symtab->table[i][k].TypeOfEntry == literalEntry)
            {
               code += symtab->table[i][k].Lexeme + "\tDB\t";
               code += symtab->table[i][k].Literal + "," + '"' + "$" + '"';
               str_num++;
               emit(code);
            }
            else
            {
               code += symtab->table[i][k].Lexeme + "\tDW\t";
               code += "?";
               emit(code);
            }
         }
      }
   }
}

/*******************************************************************************
 ***  FUNCTION final_init()
 *******************************************************************************
 ***  DESCRIPTION  :  sets up the final main segment for ASM
 ******************************************************************************/
void CodeGen::final_init()
{
   string code;
   
   code = "\n\n_startproc PROC\n\tmov ax, @data\n\tmov ds, ax\n\tcall main\n\tmov ax, 4c00h";
   code += "\n\tint 21h\n_startproc ENDP\n\tEND _startproc";
   
   emit(code);
}

/*******************************************************************************
 ***  FUNCTION GetNextChar()
 *******************************************************************************
 ***  DESCRIPTION  :  Gets a single char from the file stream
 ******************************************************************************/
void CodeGen::GetNextChar()
{
   ch = tacstream.get();

   if (stream_pos > tacstream.str().length()) 
      done = true;

   stream_pos++;
}

/*******************************************************************************
 ***  FUNCTION GetNextLine()
 *******************************************************************************
 ***  DESCRIPTION  :  Gets the next line in the file stream
 ******************************************************************************/
void CodeGen::GetNextLine()
{
   curLine = string();
   
   while (ch != '\n' && !done)
   {
      curLine += ch;
      GetNextChar();
   }
   
   GetNextChar();
   
   return;
}

/*******************************************************************************
 ***  FUNCTION code_init()
 *******************************************************************************
 ***  DESCRIPTION  :  sets up the .code segment for ASM
 ******************************************************************************/
void CodeGen::code_init()
{
   string code;
   
   code += "\t.code\n";
   code += "\tinclude io.asm";
   
   emit(code);
}

/*******************************************************************************
 ***  FUNCTION proc_init()
 *******************************************************************************
 ***  DESCRIPTION  :  sets up a PROC for ASM
 ******************************************************************************/
void CodeGen::proc_init()
{
   string code;
   
   string procname = curLine.substr(5);
   
   int size = symtab->lookupT(procname).function.SizeOfLocal;
   
   code +=  procname + " PROC\n";
   code += "\tpush BP\n";
   code += "\tmov bp,sp\n";
   code += "\tsub sp, " + num2str(size);
   emit(code);
   
   GetNextLine();
   
   size_t found = curLine.find("endp");
   while (found == std::string::npos)
   {
      if (curLine.find(" + ") != string::npos)
         addop();

      else if (curLine.find("push ") != string::npos)
         pushParam();

      else if (curLine.find("call ") != string::npos)
         fnCall();

      else if (curLine.find(" - ") != string::npos);
         subop();

      else if (curLine.find(" * ") != string::npos)
         mulop();

      else if (curLine.find(" / ") != string::npos)
         divop();

      else if (curLine[0] == 'r')
         readop();

      else if (curLine[0] == 'w')
         printop();

      else if (curLine.find("_AX") != string::npos)
         retop();

      else if (curLine.find(" = ") != string::npos)
         assign();

      else
         cout << "Unknown Statement: " << curLine << endl;
      
      GetNextLine();
      found = curLine.find("endp");
   }
   
   code = "\tadd sp," + num2str(size);
   code += "\n\tpop bp";
   code += "\n\tret " + num2str(symtab->lookup(procname)->function.sizeOfParams);
   code += "\n" + procname + " ENDP";
   var_offset = 0;
   emit(code);
   
   GetNextLine();
   GetNextLine();
}

/*******************************************************************************
 ***  FUNCTION assign()
 *******************************************************************************
 ***  DESCRIPTION  :  produces assignment code for ASM
 ******************************************************************************/
void CodeGen::assign()
{
   string code;
   
   size_t space1 = curLine.find(" ");
   size_t space2 = curLine.rfind(" ");
   
   string left = curLine.substr(0, space1);
   string right = curLine.substr(space2 + 1);
   
   code += "\tmov AX," + right;
   code += "\n\tmov " + left + ",AX";
   
   emit(code);
}

/*******************************************************************************
 ***  FUNCTION assign()
 *******************************************************************************
 ***  DESCRIPTION  :  produces assignment code for ASM
 ******************************************************************************/
void CodeGen::pushParam()
{
   string code;
   
   size_t space = curLine.find(" ");
   string right = curLine.substr(space + 1);
   
   code += "\tmov AX, " + right;
   
   code += "\n\tpush AX";
   
   emit(code);
}

/*******************************************************************************
 ***  FUNCTION addop()
 *******************************************************************************
 ***  DESCRIPTION  :  produces addition code for ASM
 ******************************************************************************/
void CodeGen::addop()
{
   string code;
   
   size_t space1 = curLine.find(" = ");
   size_t space2 = curLine.rfind(" + ");
   
   string left = curLine.substr(0, space1);
   string right = curLine.substr(space2 + 3);
   
   int length = curLine.size() - (left.size() + 3 + 3 + right.size());
   string middle = curLine.substr(space1 + 3, length);
   
   code += "\tmov BX," + right;
   code += "\n\tmov AX," + middle;
   code += "\n\tadd AX,BX";
   code += "\n\tmov " + left + ",AX";
   
   emit(code);
}

/*******************************************************************************
 ***  FUNCTION subop()
 *******************************************************************************
 ***  DESCRIPTION  :  produces subtraction code for ASM
 ******************************************************************************/
void CodeGen::subop()
{
   string code;
   
   size_t space1 = curLine.find(" = ");
   size_t space2 = curLine.rfind(" - ");
   
   string left = curLine.substr(0, space1);
   string right = curLine.substr(space2 + 3);
   
   int length = curLine.size() - (left.size() + 3 + 3 + right.size());
   string middle = curLine.substr(space1 + 3, length);
   
   code += "\tmov BX," + right;
   code += "\n\tmov AX," + middle;
   code += "\n\tsub AX,BX";
   code += "\n\tmov " + left + ",AX";
   
   emit(code);
}

/*******************************************************************************
 ***  FUNCTION mulop()
 *******************************************************************************
 ***  DESCRIPTION  :  produces multiplication code for ASM
 ******************************************************************************/
void CodeGen::mulop()
{
   string code;
   
   size_t space1 = curLine.find(" = ");
   size_t space2 = curLine.rfind(" * ");
   
   string left = curLine.substr(0, space1);
   string right = curLine.substr(space2 + 3);
   
   int length = curLine.size() - (left.size() + 3 + 3 + right.size());
   string middle = curLine.substr(space1 + 3, length);
   
   code += "\tmov BX," + right;
   code += "\n\tmov AX," + middle;
   code += "\n\timul BX";
   code += "\n\tmov " + left + ",AX";
   
   emit(code);
}

/*******************************************************************************
 ***  FUNCTION divop()
 *******************************************************************************
 ***  DESCRIPTION  :  produces division code for ASM
 ******************************************************************************/
void CodeGen::divop()
{
   string code;
   
   size_t space1 = curLine.find(" = ");
   size_t space2 = curLine.rfind(" / ");
   
   string left = curLine.substr(0, space1);
   string right = curLine.substr(space2 + 3);
   
   int length = curLine.size() - (left.size() + 3 + 3 + right.size());
   string middle = curLine.substr(space1 + 3, length);
   
   code += "\tmov DX, 0";
   code += "\n\tmov CX," + right;
   code += "\n\tmov AX," + middle;
   code += "\n\tidiv CX";
   code += "\n\tmov " + left + ",AX";
   
   emit(code);
}

/*******************************************************************************
 ***  FUNCTION readop()
 *******************************************************************************
 ***  DESCRIPTION  :  produces input code for ASM
 ******************************************************************************/
void CodeGen::readop()
{
   string code;
   
   size_t space = curLine.find(" ");
   string right = curLine.substr(space + 1);
   
   if (curLine[2] == 'i')
      code += "\tcall readint";
   else
      code += "\tcall readch";
   
   code += "\n\tmov " + right + ",BX";
   
   emit(code);
}

/*******************************************************************************
 ***  FUNCTION printop()
 *******************************************************************************
 ***  DESCRIPTION  :  produces output code for ASM
 ******************************************************************************/
void CodeGen::printop()
{
   string code;
   
   if (curLine[2] == 'i')
   {
      size_t space = curLine.find(" ");
      string right = curLine.substr(space + 1);
      
      code += "\tmov AX," + right;
      code += "\n\tcall writeint";
   }
   else if (curLine[2] == 'c')
   {
      size_t space = curLine.find(" ");
      string right = curLine.substr(space + 1);
      
      code += "\tmov AX," + right;
      code += "\n\tcall writech";
   }
   else if (curLine[2] == 't')
   {
      code += "\tcall writeln";
   }
   else
   {
      size_t space = curLine.find(" ");
      string right = curLine.substr(space + 1);
      
      code += "\tmov DX, offset " + right;
      code += "\n\tcall writestr";
   }
   
   emit(code);
}

/*******************************************************************************
 ***  FUNCTION retop()
 *******************************************************************************
 ***  DESCRIPTION  :  produces return statement code for ASM
 ******************************************************************************/
void CodeGen::retop()
{
   string code;
   
   size_t space2 = curLine.rfind(" ");
   string right = curLine.substr(space2 + 1);
   
   code += "\tmov AX," + right;
   
   emit(code);
}

/*******************************************************************************
 ***  FUNCTION fnCall()
 *******************************************************************************
 ***  DESCRIPTION  :  produces function call code for ASM
 ******************************************************************************/
void CodeGen::fnCall()
{
   string code;
   
   size_t space2 = curLine.rfind(" ");
   string right = curLine.substr(space2 + 1);
   
   code += "\tcall " + right;
   
   emit(code);
}

/*******************************************************************************
 ***  FUNCTION emit()
 *******************************************************************************
 ***  DESCRIPTION  :  emit function for ASM, prints and adds to asm_code vector
 ******************************************************************************/
void CodeGen::emit(string code)
{
   cout << code << endl;
   asm_code.push_back(code);
}

/*******************************************************************************
 ***  FUNCTION writeToFile()
 *******************************************************************************
 ***  DESCRIPTION  :  writes ASM to ASM file
 ******************************************************************************/
void CodeGen::writeToFile()
{
   ofstream asm_file;
   asm_file.open(filename.c_str(), ios::trunc);
   
   for (int i = 0; i < asm_code.size(); i++)
      asm_file << asm_code[i] << endl;
   
   asm_file.close();
}

