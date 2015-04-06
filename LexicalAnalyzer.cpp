//
//  LexicalAnalyzer.cpp
//
//  Created by Tyler McEntee on 1/26/14.
//  Copyright (c) 2014 Tyler McEntee. All rights reserved.
//

#include "LexicalAnalyzer.h"
#include "Global.h"
#include <stdlib.h>
#include <iomanip>
#include <iostream>
#include <fstream>
using namespace std;


/*******************************************************************************
 ***  FUNCTION LexicalAnalyzer()
 *******************************************************************************
 ***  DESCRIPTION  :  Constructor for Lexical Analyzer class
 ******************************************************************************/
LexicalAnalyzer::LexicalAnalyzer(string source_filename)
{
   global.Lexeme = string();
   filename = source_filename;
   
   stream_pos = 0;
   line_num = 1;
   
   done = false;
   
   isFloat = false;
    
   ifstream source_file;
   source_file.open(filename.c_str());
   
   if (!source_file)
      cerr << "File not found: " << filename << endl;
   else
   {
      //read file into buffer and close file.
      //stringstreams are easier to work with than basic file streams.
      file_stream << source_file.rdbuf();
      source_file.close();
       
      buildResWords();
   }
}

/*******************************************************************************
 ***  FUNCTION GetNextToken()
 *******************************************************************************
 ***  DESCRIPTION  :  Gets the next token in the file stream
 ******************************************************************************/
void LexicalAnalyzer::GetNextToken()
{
    global.Lexeme = string();
   
    if (file_stream.str().length() == stream_pos)
       done = true;
    
    while (ch <= ' ' && !done)
    {
       GetNextChar();

       if (file_stream.str().length() <= stream_pos)
          done = true;
    }
   
   if (!done)
   {
      ProcessToken();
   }
   else
   {
      global.Token = Global::eoft;
      done = true;
   }
    
   return;
}

/*******************************************************************************
 ***  FUNCTION GetNextChar()
 *******************************************************************************
 ***  DESCRIPTION  :  Gets a single char from the file stream
 ******************************************************************************/
void LexicalAnalyzer::GetNextChar()
{
   ch = file_stream.get();
   if (ch == '\n') { line_num++; }
   stream_pos++;
}

/*******************************************************************************
 ***  FUNCTION ProcessToken()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes a single token.
 ******************************************************************************/
void LexicalAnalyzer::ProcessToken()
{
   global.Lexeme += ch;
   
   if (isalpha(global.Lexeme[0]))
      ProcessWordToken();
   
   else if (isdigit(global.Lexeme[0]))
      ProcessNumToken();
   
   else if (global.Lexeme[0] == '/' && LookAhead() == '*')
      ProcessComment();
   
   else if (global.Lexeme[0] == '\'' || global.Lexeme[0] == '\"')
      ProcessLiteralToken();
   
   else if (global.Lexeme[0] == '<' || global.Lexeme[0] == '>' || global.Lexeme[0] == '=' || global.Lexeme[0] == '!' ||
           global.Lexeme[0] == '&' || global.Lexeme[0] == '|')
   {
      char ch_t = global.Lexeme[0];
      
      if (ch_t == '<' && LookAhead() == '<')
         ProcessStreamOp();

      else if (ch_t == '>' && LookAhead() == '>')
         ProcessStreamOp();

      else if ((ch_t == '<' || ch_t == '>' || ch_t == '=' || ch_t == '!') && LookAhead() == '=')
         ProcessDoubleCharToken();

      else if ((ch_t == '&' && LookAhead() == '&') || (ch_t == '|' && LookAhead() == '|'))
         ProcessDoubleCharToken();

      else
         ProcessSingleCharToken();
   }
   
   else
      ProcessSingleCharToken();
}

/*******************************************************************************
 ***  FUNCTION ProcessStreamOp()
 *******************************************************************************
 ***  DESCRIPTION  :  processes stream operators
 ******************************************************************************/
void LexicalAnalyzer::ProcessStreamOp()
{
   GetNextChar();
   
   if (global.Lexeme[0] == '>')
      global.Token = Global::instreamt;

   else if (global.Lexeme[0] == '<')
      global.Token = Global::outstreamt;

   else
      global.Token = Global::unknownt;
   
   GetNextChar();
}

/*******************************************************************************
 ***  FUNCTION ProcessComment()
 *******************************************************************************
 ***  DESCRIPTION  :  Skips over comments in a source file
 ******************************************************************************/
void LexicalAnalyzer::ProcessComment()
{
   while (!(ch == '*' && LookAhead() == '/'))
   {
      GetNextChar();

      if (file_stream.str().length() < stream_pos)
      {
         global.Token = Global::unknownt;
         return;
      }
   }
   
   GetNextChar();
   GetNextChar();
   global.Lexeme = string();
   global.Token = Global::commentt;
}

/*******************************************************************************
 ***  FUNCTION LookAhead()
 *******************************************************************************
 ***  DESCRIPTION  :  Looks one char ahead in the stream to help process token
 ******************************************************************************/
char LexicalAnalyzer::LookAhead()
{
   char peek = file_stream.get();
   file_stream.putback(peek);
   
   return peek;
}

/*******************************************************************************
 ***  FUNCTION ProcessWordToken()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes a word token.
 ******************************************************************************/
void LexicalAnalyzer::ProcessWordToken()
{
   GetNextChar();
   
   while (isdigit(ch) || isalpha(ch) || ch == '_')
   {
      global.Lexeme += ch;
      GetNextChar();
   }
   
   if (global.Lexeme.length() > 31)
   {
      global.Lexeme = string("ERROR: LEXEME GREATER THAN 31 CHARACTERS.");
      global.Token = Global::unknownt;
   }
   
   else
   {
      for (int i = Global::ift; i < Global::returnt + 2; i++)
      {
         if (reswords[i] == global.Lexeme)
         {
            global.Token = static_cast<Global::Symbol>(i - 1);
            return;
         }
      }
      
      global.Token = Global::idt;
   }
}

/*******************************************************************************
 ***  FUNCTION ProcessNumToken()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes a number token. Can be int or float.
 ******************************************************************************/
void LexicalAnalyzer::ProcessNumToken()
{
   GetNextChar();
   
   isFloat = false;
   
   while (isdigit(ch))
   {
      global.Lexeme += ch;
      GetNextChar();
   }
   
   if (ch == '.')
   {
      global.Lexeme += ch;
      
      GetNextChar();
      
      if (isdigit(ch))
      {
         while (isdigit(ch))
         {
            if (ch != '\n')
            {
               global.Lexeme += ch;
               GetNextChar();
            }
            else
               break;
         }
      }
      else
      {
         global.Lexeme = string("ERROR: INVALID NUMBER FORMAT");
         global.Token = Global::unknownt;
         return;
      }
   }
   
   if (global.Lexeme.length() > 31)
   {
      global.Lexeme = string("ERROR: LEXEME GREATER THAN 31 CHARACTERS.");
      global.Token = Global::unknownt;
      return;
   }
   else
   {
      size_t found  = global.Lexeme.find(".");
      
      if (found < global.Lexeme.length())
      {
         global.ValueR = atof(global.Lexeme.c_str());
         isFloat = true;
      }
      else
      {
         global.Value = atoi(global.Lexeme.c_str());
         isFloat = false;
      }
   }
   
   global.Token = Global::numt;
}

/*******************************************************************************
 ***  FUNCTION ProcessLiteralToken()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes a string literal token.
 ******************************************************************************/
void LexicalAnalyzer::ProcessLiteralToken()
{
   bool illegal = false;
   
   GetNextChar();
   global.Lexeme += ch;
   
   while (ch != global.Lexeme[0])
   {
      GetNextChar();

      if (ch == '\n')
      {
         illegal = true;
         break;
      }
      
      global.Lexeme += ch;
   }
   
   if (!illegal)
   {
      global.Token = Global::literalt;
      global.Literal = global.Lexeme;
   }
   else
   {
      global.Token = Global::unknownt;
      global.Lexeme = string("ERROR: INVALID LITERAL");
   }
   
   GetNextChar();
}

/*******************************************************************************
 ***  FUNCTION ProcessSingleCharToken()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes a single char token.
 ******************************************************************************/
void LexicalAnalyzer::ProcessSingleCharToken()
{
   switch(global.Lexeme[0])
   {
      case '=':
         global.Token = Global::assignopt;
         break;
         
      case '<':
      case '>':
         global.Token = Global::relopt;
         break;
         
      case '+':
      case '-':
         global.Token = Global::addopt;
         break;
         
      case '*':
      case '/':
      case '%':
         global.Token = Global::mulopt;
         break;
      
      case '(':
         global.Token = Global::lparent;
         break;
      case ')':
         global.Token = Global::rparent;
         break;
      case '[':
         global.Token = Global::lbrackett;
         break;
      case ']':
         global.Token = Global::rbrackett;
         break;
      case '{':
         global.Token = Global::lbracet;
         break;
      case '}':
         global.Token = Global::rbracet;
         break;
      case ',':
         global.Token = Global::commat;
         break;
      case '.':
         global.Token = Global::periodt;
         break;
      case ';':
         global.Token = Global::semicolont;
         break;
      case '!':
         global.Token = Global::nott;
         break;
         
      default:
         global.Token = Global::unknownt;
      break;
   }
   
   GetNextChar();
}

/*******************************************************************************
 ***  FUNCTION ProcessDoubleCharToken()
 *******************************************************************************
 ***  DESCRIPTION  :  Processes double char tokens like '&&', '||', and '<='
 ******************************************************************************/
void LexicalAnalyzer::ProcessDoubleCharToken()
{
   GetNextChar();
   global.Lexeme += ch;
   global.Token = Global::relopt;
   GetNextChar();
}

/*******************************************************************************
 ***  FUNCTION Print()
 *******************************************************************************
 ***  DESCRIPTION  :
 ******************************************************************************/
void LexicalAnalyzer::Print()
{
   if (global.Token != Global::commentt)
   {
      cout << setw(15) << left << EnumToString(global.Token);
      cout << setw(35) << global.Lexeme;

      if (global.Token == Global::numt)
      {
         if (isFloat)
            cout << setw(35) << global.ValueR;
         else
            cout << setw(35) << global.Value;
      }

      if (global.Token == Global::literalt)
         cout << setw(35) << global.Literal;
     
      cout << endl;
   }
}

/*******************************************************************************
 ***  FUNCTION buildResWords()
 *******************************************************************************
 ***  DESCRIPTION  :  Builds the list of reserved words to search through
 ******************************************************************************/
void LexicalAnalyzer::buildResWords()
{
      //enums start at 1 so put null string at zeroth spot
      reswords.push_back(string());

      reswords.push_back(string("if"));
      reswords.push_back(string("else"));
      reswords.push_back(string("while"));
      reswords.push_back(string("float"));
      reswords.push_back(string("int"));
      reswords.push_back(string("char"));
      reswords.push_back(string("break"));
      reswords.push_back(string("continue"));
      reswords.push_back(string("const"));
      reswords.push_back(string("void"));
      reswords.push_back(string("cin"));
      reswords.push_back(string("cout"));
      reswords.push_back(string("endl"));
      reswords.push_back(string("return"));
   
    
    return;
}

