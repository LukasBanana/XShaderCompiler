/*
 * JoinString.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_JOIN_STRING_H
#define XSC_JOIN_STRING_H


#include <string>
#include <vector>


namespace Xsc
{


/* ----- Functions ----- */

/*
Joins the specified string with its values.
Special characters for the string 's' are: '{', '}', '[', and ']'.

"{0}" will be replaced by the first value from the array 'values',
"{1}" will be replaced by the second value and so forth.
Everything inside squared brackets (e.g. "[optional {0}]") will only be joined to the output string,
if all values inside these brackets are specified and non-empty.

These characters will only be treated as direct output,
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
  std::out_of_range If a value index, that is not optional, is out of range
  std::invalid_argument If there is an incomplete escape character (e.g. "\\")
  std::invalid_argument If there is an incomplete optional part, i.e. a missing closing ']' (e.g. "[")
*/
std::string JoinString(const std::string& s, const std::vector<std::string>& values);


/* ----- Templates ----- */

template <typename T>
void ToStringListPrimary(std::vector<std::string>& list, const T& value)
{
    list.push_back(value);
}

template <>
void ToStringListPrimary<std::size_t>(std::vector<std::string>& list, const std::size_t& value);

template <>
void ToStringListPrimary<int>(std::vector<std::string>& list, const int& value);

// Forward declaration (required for GCC and clang)
template <typename... Args>
void ToStringList(std::vector<std::string>& list, Args&&... args);

template <typename Arg0, typename... ArgsN>
void ToStringListSecondary(std::vector<std::string>& list, Arg0&& arg0, ArgsN&&... argsN)
{
    ToStringListPrimary(list, std::forward<Arg0>(arg0));
    ToStringList(list, std::forward<ArgsN>(argsN)...);
}

template <typename... Args>
void ToStringList(std::vector<std::string>& list, Args&&... args)
{
    ToStringListSecondary(list, std::forward<Args>(args)...);
}

template <>
void ToStringList(std::vector<std::string>& list);

template <typename Arg0>
void ToStringListSecondary(std::vector<std::string>& list, Arg0&& arg0)
{
    ToStringListPrimary(list, std::forward<Arg0>(arg0));
}


/* ----- Classes ----- */

class JoinableString
{

    public:
        
        JoinableString() = default;
        JoinableString(const JoinableString&) = default;
        JoinableString& operator = (const JoinableString&) = default;

        JoinableString(const char* s);

        // see JoinString
        std::string Join(const std::vector<std::string>& values = {}) const;

        template <typename... Args>
        std::string operator () (Args&&... args) const
        {
            std::vector<std::string> values;
            ToStringList(values, std::forward<Args>(args)...);
            return Join(values);
        }

        // Returns only the string without any transformation.
        inline operator std::string () const
        {
            return Join({});
        }

    private:
        
        const char* s_          = nullptr;
        bool        canJoin_    = false;

};


// Returns lhs.Join() + rhs
inline std::string operator + (const JoinableString& lhs, const std::string& rhs)
{
    return lhs.Join() + rhs;
}

// Returns lhs + rhs.Join()
inline std::string operator + (const std::string& lhs, const JoinableString& rhs)
{
    return lhs + rhs.Join();
}


} // /namespace Xsc


#endif



// ================================================================================