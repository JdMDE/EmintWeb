/*
    The class to contain a single HTML document (page) in HTML5
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


#ifndef HTML5DOCUMENT_H
#define HTML5DOCUMENT_H

#include "tag.h"
#include "tagshtml5.h"
#include "tostring"

#define HTML5_DOCTYPE "<!DOCTYPE html>"   //!< The doctype that the W3C standard dictates for HTML5

using namespace std;

//! A class to contain a document in HTML5. 
/*! 
    An HTML5 document is created and later filled with any contents wich are references to tags (instances of the Tag class) that may be ifself contain
    any other tags. The HTMLDocument in principle will contain two Tag elements, head and body, plus any number of processing instructions.
*/
class HTML5Document
{
 public:
  //! Default constructor. Initializes the document to an empty, but legal, HTML5 document.
  HTML5Document();

  //! Copy constructor. This is implemented in a clever way to allow very fast copying. See documentation to the Tag class.
  HTML5Document(const HTML5Document& other);

  //! Destructor. The internal nodes, and their memory, are deallocated, as long as no one else needs them.
  ~HTML5Document();

  //! Operator equivalent to the copy constructor also implemented in a clever way
  HTML5Document& operator=(const HTML5Document& other);

  //! Equality test. It tests the internal pointer
  bool operator==(const HTML5Document& other) const;

  //! Attach any desired attributes to a node
  /*!
      \param attrs The string with the attributes to be attached, in the form of several attr=value items chained.
  */
  void SetAttributes( string attrs ) { htattrs = attrs; };
  
  //! Function to generate the HTML code of this document
  /*!
      \param ident The unit of indentation added for each new nested level
      \return A string with the HTML content, normally to be saved to a file
  */
  string Generate(string ident);

  //! Addition of a HTML element to the current document. It just calls to the add function of the document tag
  /*!
      \param element An element (passed by reference) to be added to the document.
  */
  void add(Tag &element) { HTML5TagP->add(element); };

 private:
  string htattrs;       //!< The attributes of the document
  Tag *HTML5TagP;       //!< A pointer to the 'document' tag, the root of the DOM tree
};

#endif // HTML5DOCUMENT_H
