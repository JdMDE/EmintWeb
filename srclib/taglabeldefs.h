/*
    A class to manage the labels and associated constants.
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


#ifndef TAGLABELDEFS_H
#define TAGLABELDEFS_H

#include <iostream>
#include <string>
#include <cstdlib>

#define HTML_TAGERROR "INVALID_HTML5_TAG"   //!< A string to be returned to indicate that the label is not know or standard in HTML5

#include "tagshtml5.h"

using namespace std;

/*!
    @author Juan Domingo (Juan.Domingo@uv.es)
    @date  December, 2013
*/

class TaglabelDefs
{
 public:
  //! Default constructor
  TaglabelDefs();

  static TaglabelDefs* instance();

  //! Returns a string with the tag corresponding to a given tag number
  /*!
      \param num The tag number
      \return The string with the tag itself
  */
  string toName(unsigned num);

  //! Returns the number assigned to a given tag
  /*!
      \param name The tag as a string
      \return The tag number
  */
  int toNum(string name);

 private:
  void FillTags();
  static TaglabelDefs* pInstance;
  string *tagnames;                 //!< An array of strings containing the allowed tags
  bool **allowed_into;              //!< A matrix of booleans to indicate which tags are allowed into any other tag
};

#endif // TAGLABELDEFS_H
