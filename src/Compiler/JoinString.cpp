/*
 * JoinString.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "JoinString.h"
#include <cstdlib>
#include <stdexcept>


namespace Xsc
{


/* ----- Templates ----- */

template <>
void ToStringListPrimary<std::size_t>(std::vector<std::string>& list, const std::size_t& value)
{
    list.push_back(std::to_string(value));
}

template <>
void ToStringListPrimary<int>(std::vector<std::string>& list, const int& value)
{
    list.push_back(std::to_string(value));
}

template <>
void ToStringList(std::vector<std::string>& list)
{
    // dummy
}


/* ----- Classes ----- */

JoinableString::JoinableString(const char* s) :
    s_ { s }
{
    /*
    Check if there is any special character inside the string,
    which can be used to join the string (see JoinString function).
    */
    while (*s != '\0')
    {
        if (*s == '\\' || *s == '{' || *s == '}' || *s == '[' || *s == ']')
        {
            canJoin_ = true;
            break;
        }
        ++s;
    }
}

std::string JoinableString::Join(const std::vector<std::string>& values) const
{
    if (canJoin_)
        return JoinString(s_, values);
    else
        return std::string(s_);
}


/* ----- Functions ----- */

/*
Returns true, if all values have been joined to the output string,
i.e. they are set for the repsective index {N} and they are non-empty.
*/
static bool JoinStringSub(
    const std::string& in, std::size_t& pos, std::string& out, const std::vector<std::string>& values, bool optional)
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
                const auto idx = static_cast<std::size_t>(std::stoul(idxStr));
                if (idx < values.size())
                {
                    /* Append value to output string */
                    const auto& val = values[idx];
                    if (val.empty())
                        replacedAllValues = false;
                    else
                        out.append(val);
                }
                else if (optional)
                {
                    /* This sub string will not be added to the final output string */
                    replacedAllValues = false;
                }
                else
                {
                    /* If this value replacement was not optional -> error */
                    throw std::out_of_range(
                        "index (" + std::to_string(idx) + ") out of range [0, " +
                        std::to_string(values.size()) + ") in joinable string: " + in
                    );
                }
            }
            else if (c == '[')
            {
                /* Parse optional part with recursive call */
                std::string outOpt;
                if (JoinStringSub(in, pos, outOpt, values, true))
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
    JoinStringSub(s, pos, out, values, false);

    /* Check if position has been reached the end of the input string */
    if (pos != s.size())
        throw std::invalid_argument("incomplete optional part in report string");

    return out;
}


} // /namespace Xsc



// ================================================================================