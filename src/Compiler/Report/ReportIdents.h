/*
 * ReportIdents.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_REPORT_IDENTS_H
#define XSC_REPORT_IDENTS_H


#include <string>
#include <vector>


namespace Xsc
{


/* ----- Localized global report strings ------ */

#define DECL_REPORT(NAME, VALUE) static const char* R_##NAME = #VALUE

#include "ReportIdentsEN.h"

#undef DECL_REPORT


/* ----- Global functions ----- */

#if 0
class ReportIdent
{

    public:
        
        ReportIdent(const char* s);

        std::string operator () (const std::vector<std::string>& values) const;

    private:
        
        const char* s_;

};
#endif

/*
Joins the specified report string with its values.
Special characters for the string 's' are: '{', '}', '[', and ']'.

"{0}" will be replaced by the first value from the array 'values',
"{1}" will be replaced by the second value and so froth.
Everything inside squared brackets (e.g. "[optional {0}]") will only be joined to the output string,
if all values inside these brackets are specified and non-empty.

These character will only be treated as direct output,
if the escape character '\\' is written in front of it (e.g. "\\[...\\]"),
to write the escape character itself use "\\\\".

Examples:
  JoinString("undeclared identifier {0}", { "foo_bar" })            --> "undeclared identifier foo_bar"
  JoinString("always {0}[, sometimes {1}]", { "first", "second" })  --> "always first, sometimes second"
  JoinString("always {0}[, sometimes {1}]", { "first", "" })        --> "always first"
  JoinString("always {0}[, sometimes {1}]", { "first" })            --> "always first"
  JoinString("one {0}[, two {1}[, three {2}]]", { "1", "2", "3" })  --> "one 1, two 2, three 3"
  JoinString("one {0}[, two {1}[, three {2}]]", { "1", "", "3" })   --> "one 1"
  JoinString("one {0}[, two {1}][, three {2}]", { "1", "", "3" })   --> "one 1, three 3"

Throw:
  std::invalid_argument If there is an incomplete escape character (e.g. "\\")
  std::invalid_argument If there is an incomplete optional part, i.e. a missing closing ']' (e.g. "[")
*/
std::string JoinString(const std::string& s, const std::vector<std::string>& values);


} // /namespace Xsc


#endif



// ================================================================================