//
//  LexicalAnalyzer.h
//
//  Created by Tyler McEntee on 1/26/14.
//  Copyright (c) 2014 Tyler McEntee. All rights reserved.
//

#ifndef _LexicalAnalyzer_h
#define _LexicalAnalyzer_h

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "Global.h"
using namespace std;


class LexicalAnalyzer
{
public:
   LexicalAnalyzer(string);
    
   void GetNextToken();
   void GetNextChar();
   void ProcessToken();
   void ProcessWordToken();
   void ProcessNumToken();
   void ProcessLiteralToken();
   void ProcessSingleCharToken();
   void ProcessDoubleCharToken();
   void ProcessStreamOp();
   void ProcessComment();
   char LookAhead();
   void Print();
    
private:
    void buildResWords();
    
public:
   char ch;
   string filename;
   vector<string> reswords;
   Global global;
   stringstream file_stream;
   int stream_pos;
   int line_num;
   
   bool done;
   bool isFloat;
};

#endif
