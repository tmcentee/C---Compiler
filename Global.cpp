//
//  Global.cpp
//  446_A7
//
//  Created by Tyler McEntee on 2/18/14.
//  Copyright (c) 2014 Tyler McEntee. All rights reserved.
//

#include "Global.h"

/*******************************************************************************
 ***  FUNCTION EnumToString()
 *******************************************************************************
 ***  DESCRIPTION  :  Large switch statement to turn enum values to strings for
 ***                  printing purpose.
 ******************************************************************************/
string EnumToString(Global::Symbol token)
{
   switch(token)
   {
      case Global::ift:
         return string("ift");
      case Global::elset:
         return string("elset");
      case Global::whilet:
         return string("whilet");
      case Global::floatt:
         return string("floatt");
      case Global::intt:
         return string("intt");
      case Global::chart:
         return string("chart");
      case Global::breakt:
         return string("breakt");
      case Global::continuet:
         return string("continuet");
      case Global::voidt:
         return string("voidt");
      case Global::assignopt:
         return string("assignopt");
      case Global::relopt:
         return string("relopt");
      case Global::mulopt:
         return string("mulopt");
      case Global::lparent:
         return string("lparent");
      case Global::rparent:
         return string("rparent");
      case Global::lbracet:
         return string("lbracet");
      case Global::rbracet:
         return string("rbracet");
      case Global::rbrackett:
         return string("rbrackett");
      case Global::lbrackett:
         return string("lbrackett");
      case Global::commat:
         return string("commat");
      case Global::periodt:
         return string("periodt");
      case Global::addopt:
         return string("addopt");
      case Global::numt:
         return string("numt");
      case Global::wordt:
         return string("wordt");
      case Global::idt:
         return string("idt");
      case Global::semicolont:
         return string("semicolont");
      case Global::literalt:
         return string("literalt");
      case Global::nott:
         return string("nott");
      case Global::returnt:
         return string("returnt");
      case Global::eoft:
         return string("eoft");
      case Global::cint:
         return string("cint");
      case Global::coutt:
         return string("coutt");
      case Global::endlt:
         return string("endlt");
      default:
         return string("unknownt");
   }
}