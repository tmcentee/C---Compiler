//
//  Global.h
//
//  Created by Tyler McEntee on 1/26/14.
//  Copyright (c) 2014 Tyler McEntee. All rights reserved.
//

#ifndef _Global_h
#define _Global_h

#include <string>
using namespace std;


/*******************************************************************************
 ***  CLASS : Global
 *******************************************************************************
 ***  DESCRIPTION  :  Contains global variables as used by the LexicalAnalyzer
 ******************************************************************************/
class Global
{
public:
    enum Symbol {
       ift, elset, whilet, floatt, intt, chart,
       breakt, continuet, constt, voidt, cint, coutt, endlt, returnt, assignopt, relopt, mulopt, lparent, rparent, lbracet, rbracet, lbrackett, rbrackett,
       commat, periodt, addopt, numt, wordt, idt, semicolont, literalt, instreamt, outstreamt, commentt, nott, eoft, unknownt
    };
    
   Global() { }
   
    string Lexeme;
    Symbol Token;
    int Value;
    float ValueR;
    string Literal;
};

string EnumToString(Global::Symbol);





#endif
