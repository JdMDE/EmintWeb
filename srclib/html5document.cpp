/*
    See comments to htmldocument.h
    Copyright (C) 2013  Juan Domingo Esteve <Juan.Domingo@uv.es>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "html5document.h"

bool DEBLIB=false;

HTML5Document::HTML5Document()
{
 htattrs="";
 HTML5TagP = new Tag("html",htattrs);
}

HTML5Document::HTML5Document(const HTML5Document &other)
{
 htattrs=other.htattrs;
 HTML5TagP=other.HTML5TagP;
}

HTML5Document::~HTML5Document()
{
 HTML5TagP->~Tag();
}

HTML5Document& HTML5Document::operator=(const HTML5Document& other)
{
 if ( &other == this )
  return *this;

 htattrs=other.htattrs;
 HTML5TagP=other.HTML5TagP;
 
 return *this;
}

bool HTML5Document::operator==(const HTML5Document& other) const
{
 return ((htattrs==other.htattrs) &&
         (HTML5TagP==other.HTML5TagP));
}

string HTML5Document::Generate(string ident)
{
 string ret="";
 
 ret += string(HTML5_DOCTYPE)+"\n";
 
 if ( htattrs != "" )
  htattrs = " " + htattrs;

 ret += "<html" + htattrs.substr(0,htattrs.find_last_not_of(" ")+1) + ">\n";
 ret += HTML5TagP->Generate(ident);
 ret += "</html>\n";
 
 return ret;
}
