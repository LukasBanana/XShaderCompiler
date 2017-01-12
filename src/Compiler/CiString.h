/*
 * CiString.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_CI_STRING_H
#define XSC_CI_STRING_H


#include <string>
#include <cctype>


namespace Xsc
{


/**
Case insensitive character traits structure.
\note This structure overwrites some functions of the 'std::char_traits' STL structure.
*/
template <typename T>
struct CiCharTraits : public std::char_traits<T>
{
    static bool eq(T c1, T c2)
    {
        return (std::toupper(c1) == std::toupper(c2));
    }

    static bool ne(T c1, T c2)
    {
        return (std::toupper(c1) != std::toupper(c2));
    }

    static bool lt(T c1, T c2)
    {
        return (std::toupper(c1) < std::toupper(c2));
    }

    static int compare(const T* s1, const T* s2, size_t n)
    {
        while (n-- != 0)
        {
            if (std::toupper(*s1) < std::toupper(*s2))
                return -1;
            if (std::toupper(*s1) > std::toupper(*s2))
                return 1;
            ++s1;
            ++s2;
        }
        return 0;
    }

    static const T* find(const T* s, int n, T a)
    {
        const auto ua = std::toupper(a);
        while (n-- > 0)
        {
            if (std::toupper(*s) == ua)
                return s;
            s++;
        }
        return nullptr;
    }
};


// Case insensitive UTF-8 string.
using CiString = std::basic_string<char, CiCharTraits<char>>;


// Converts the specified std::string into an CiString.
inline CiString ToCiString(const std::string& s)
{
    return CiString(s.begin(), s.end());
}


} // /namespace Xsc


#endif



// ================================================================================