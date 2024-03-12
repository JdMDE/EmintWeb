/*
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


#include "taglabeldefs.h"

extern bool DEBLIB;

TaglabelDefs* TaglabelDefs::pInstance = 0;

TaglabelDefs::TaglabelDefs()
{
 pInstance=0;

 tagnames = NULL;
 tagnames = new string[NUMTAGS_HTML5];
 if (tagnames==NULL)
 {
  cerr << "Error booking memory for the singleton in tagnames.\n";
  exit(1);
 }
 tagnames[HTML5_HTML]="html";
 tagnames[HTML5_HEAD]="head";
 tagnames[HTML5_TITLE]="title";
 tagnames[HTML5_BASE]="base";
 tagnames[HTML5_LINK]="link";
 tagnames[HTML5_META]="meta";
 tagnames[HTML5_STYLE]="style";
 tagnames[HTML5_SCRIPT]="script";
 tagnames[HTML5_NOSCRIPT]="noscript";
 tagnames[HTML5_TEMPLATE]="template";
 tagnames[HTML5_BODY]="body";
 tagnames[HTML5_ARTICLE]="article";
 tagnames[HTML5_SECTION]="section";
 tagnames[HTML5_NAV]="nav";
 tagnames[HTML5_ASIDE]="aside";
 tagnames[HTML5_H1]="h1";
 tagnames[HTML5_H2]="h2";
 tagnames[HTML5_H3]="h3";
 tagnames[HTML5_H4]="h4";
 tagnames[HTML5_H5]="h5";
 tagnames[HTML5_H6]="h6";
 tagnames[HTML5_HGROUP]="hgroup";
 tagnames[HTML5_HEADER]="header";
 tagnames[HTML5_ADDRESS]="address";
 tagnames[HTML5_P]="p";
 tagnames[HTML5_HR]="hr";
 tagnames[HTML5_PRE]="pre";
 tagnames[HTML5_BLOCKQUOTE]="blockquote";
 tagnames[HTML5_OL]="ol";
 tagnames[HTML5_UL]="ul";
 tagnames[HTML5_LI]="li";
 tagnames[HTML5_DL]="dl";
 tagnames[HTML5_DT]="dt";
 tagnames[HTML5_DD]="dd";
 tagnames[HTML5_FIGURE]="figure";
 tagnames[HTML5_FIGCAPTION]="figcaption";
 tagnames[HTML5_MAIN]="main";
 tagnames[HTML5_DIV]="div";
 tagnames[HTML5_A]="a";
 tagnames[HTML5_EM]="em";
 tagnames[HTML5_STRONG]="strong";
 tagnames[HTML5_SMALL]="small";
 tagnames[HTML5_S]="s";
 tagnames[HTML5_CITE]="cite";
 tagnames[HTML5_Q]="q";
 tagnames[HTML5_DFN]="dfn";
 tagnames[HTML5_ABBR]="abbr";
 tagnames[HTML5_DATA]="data";
 tagnames[HTML5_TIME]="time";
 tagnames[HTML5_CODE]="code";
 tagnames[HTML5_VAR]="var";
 tagnames[HTML5_SAMP]="samp";
 tagnames[HTML5_KBD]="kbd";
 tagnames[HTML5_SUB]="sub";
 tagnames[HTML5_SUP]="sup";
 tagnames[HTML5_I]="i";
 tagnames[HTML5_B]="b";
 tagnames[HTML5_U]="u";
 tagnames[HTML5_MARK]="mark";
 tagnames[HTML5_RUBY]="ruby";
 tagnames[HTML5_RT]="rt";
 tagnames[HTML5_RP]="rp";
 tagnames[HTML5_BDI]="bdi";
 tagnames[HTML5_BDO]="bdo";
 tagnames[HTML5_SPAN]="span";
 tagnames[HTML5_BR]="br";
 tagnames[HTML5_WBR]="wbr";
 tagnames[HTML5_INS]="ins";
 tagnames[HTML5_DEL]="del";
 tagnames[HTML5_IMAGE]="image";
 tagnames[HTML5_IMG]="img";
 tagnames[HTML5_IFRAME]="iframe";
 tagnames[HTML5_EMBED]="embed";
 tagnames[HTML5_OBJECT]="object";
 tagnames[HTML5_PARAM]="param";
 tagnames[HTML5_VIDEO]="video";
 tagnames[HTML5_AUDIO]="audio";
 tagnames[HTML5_SOURCE]="source";
 tagnames[HTML5_TRACK]="track";
 tagnames[HTML5_CANVAS]="canvas";
 tagnames[HTML5_MAP]="map";
 tagnames[HTML5_AREA]="area";
 tagnames[HTML5_MATH]="math";
 tagnames[HTML5_MI]="mi";
 tagnames[HTML5_MO]="mo";
 tagnames[HTML5_MN]="mn";
 tagnames[HTML5_MS]="ms";
 tagnames[HTML5_MTEXT]="mtext";
 tagnames[HTML5_MGLYPH]="mglyph";
 tagnames[HTML5_MALIGNMARK]="malignmark";
 tagnames[HTML5_ANNOTATION_XML]="annotation_xml";
 tagnames[HTML5_SVG]="svg";
 tagnames[HTML5_FOREIGNOBJECT]="foreignobject";
 tagnames[HTML5_DESC]="desc";
 tagnames[HTML5_TABLE]="table";
 tagnames[HTML5_CAPTION]="caption";
 tagnames[HTML5_COLGROUP]="colgroup";
 tagnames[HTML5_COL]="col";
 tagnames[HTML5_TBODY]="tbody";
 tagnames[HTML5_THEAD]="thead";
 tagnames[HTML5_TFOOT]="tfoot";
 tagnames[HTML5_TR]="tr";
 tagnames[HTML5_TD]="td";
 tagnames[HTML5_TH]="th";
 tagnames[HTML5_FORM]="form";
 tagnames[HTML5_FIELDSET]="fieldset";
 tagnames[HTML5_LEGEND]="legend";
 tagnames[HTML5_INPUT]="input";
 tagnames[HTML5_BUTTON]="button";
 tagnames[HTML5_SELECT]="select";
 tagnames[HTML5_DATALIST]="datalist";
 tagnames[HTML5_OPTGROUP]="optgroup";
 tagnames[HTML5_OPTION]="option";
 tagnames[HTML5_TEXTAREA]="textarea";
 tagnames[HTML5_KEYGEN]="keygen";
 tagnames[HTML5_OUTPUT]="output";
 tagnames[HTML5_PROGRESS]="progress";
 tagnames[HTML5_METER]="meter";
 tagnames[HTML5_DETAILS]="details";
 tagnames[HTML5_SUMMARY]="summary";
 tagnames[HTML5_MENU]="menu";
 tagnames[HTML5_APPLET]="applet";
 tagnames[HTML5_ACRONYM]="acronym";
 tagnames[HTML5_BGSOUND]="bgsound";
 tagnames[HTML5_DIR]="dir";
 tagnames[HTML5_FRAME]="frame";
 tagnames[HTML5_FRAMESET]="frameset";
 tagnames[HTML5_ISINDEX]="isindex";
 tagnames[HTML5_LISTING]="listing";
 tagnames[HTML5_XMP]="xmp";
 tagnames[HTML5_NEXTID]="nextid";
 tagnames[HTML5_NOEMBED]="noembed";
 tagnames[HTML5_PLAINTEXT]="plaintext";
 tagnames[HTML5_RB]="rb";
 tagnames[HTML5_STRIKE]="strike";
 tagnames[HTML5_BASEFONT]="basefont";
 tagnames[HTML5_BIG]="big";
 tagnames[HTML5_BLINK]="blink";
 tagnames[HTML5_CENTER]="center";
 tagnames[HTML5_FONT]="font";
 tagnames[HTML5_MARQUEE]="marquee";
 tagnames[HTML5_MULTICOL]="multicol";
 tagnames[HTML5_NOBR]="nobr";
 tagnames[HTML5_SPACER]="spacer";
 tagnames[HTML5_TT]="t";
 tagnames[HTML5_UNKNOWN]="unknown";
 tagnames[HTML5_LAST]="last";

 allowed_into=NULL;
 allowed_into = new bool*[NUMTAGS_HTML5];
 if (allowed_into==NULL)
 {
  cerr << "Error booking memory into singleton in allowed_into.\n";
  exit(1);
 }
 for (unsigned i=0;i<NUMTAGS_HTML5;i++)
 {
  allowed_into[i]=NULL;
  allowed_into[i] = new bool[NUMTAGS_HTML5];
  if (allowed_into[i]==NULL)
  {
   cerr << "Error booking memory into singleton in allowed_into[" << i << "].\n";
   exit(1);
  }
  for (unsigned j=0;j<NUMTAGS_HTML5;j++)
   allowed_into[i][j]=true;
 }
}

TaglabelDefs *TaglabelDefs::instance()
{
 if (pInstance == 0)
 {
  pInstance = new TaglabelDefs;
  if (DEBLIB)
   cout << "First and (supposingly) only instance of TaglabelDefs initialized.\n";
 }

 return pInstance;
}

string TaglabelDefs::toName(unsigned num)
{
 if (num<NUMTAGS_HTML5)
  return tagnames[num];
 else
  return HTML_TAGERROR;
}

int TaglabelDefs::toNum(string name)
{
 unsigned i=0;
 while ((i<NUMTAGS_HTML5) && (tagnames[i]!=name))
  i++;
 return (i<NUMTAGS_HTML5) ? i : HTML_NO_TAG;
}

