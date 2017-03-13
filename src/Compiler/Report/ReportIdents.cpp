/*
 * ReportIdents.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ReportIdents.h"
#include <cstdlib>


namespace Xsc
{



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
  std::invalid_argument If there is an incomplete escape character (e.g. "\\").
  std::invalid_argument If there is an incomplete optional part, i.e. a missing closing ']' (e.g. "[")
*/

/*
Returns true, if all values have been joined to the output string,
i.e. they are set for the repsective index {N} and they are non-empty.
*/
static bool JoinStringSub(
    const std::string& in, std::size_t& pos, std::string& out, const std::vector<std::string>& values)
{
    bool escapeChar         = false;
    bool replacedAllValues  = true;

    for (auto num = in.size(); pos < num;)
    {
        /* Get next character */
        auto c = in[pos++];

        if (escapeChar)
        {
            /* Add character without transformation to output string */
            out.push_back(c);
            escapeChar = false;
        }
        else
        {
            if (c == '\\')
            {
                /* Next character will be added without transformation */
                escapeChar = true;
            }
            else if (c == '{')
            {
                /* Parse index N in '{N}' */
                std::string idxStr;
                while (pos < num)
                {
                    /* Get next character */
                    c = in[pos++];
                    if (c == '}')
                        break;
                    else
                        idxStr.push_back(c);
                }

                /* Get value by index from array */
                const auto idx = static_cast<std::size_t>(std::atoi(idxStr.c_str()));
                if (idx < values.size())
                {
                    const auto& val = values[idx];
                    if (val.empty())
                        replacedAllValues = false;
                    else
                        out.append(val);
                }
                else
                    replacedAllValues = false;
            }
            else if (c == '[')
            {
                /* Parse optional part with recursive call */
                std::string outOpt;
                if (JoinStringSub(in, pos, outOpt, values))
                    out.append(outOpt);
            }
            else if (c == ']')
            {
                /* Close optional part and return from recursive call */
                break;
            }
            else
            {
                /* Add current character to output string */
                out.push_back(c);
            }
        }
    }

    if (escapeChar)
        throw std::invalid_argument("incomplete escape character in report string");

    return replacedAllValues;
}

std::string JoinString(const std::string& s, const std::vector<std::string>& values)
{
    std::string out;
    std::size_t pos = 0;
    
    /* Join sub string */
    JoinStringSub(s, pos, out, values);

    /* Check if position has been reached the end of the input string */
    if (pos != s.size())
        throw std::invalid_argument("incomplete optional part in report string");

    return out;
}


} // /namespace Xsc



// ================================================================================