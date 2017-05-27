/*
 * Helper.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_HELPER_H
#define XSC_HELPER_H


#include <string>
#include <sstream>
#include <cmath>
#include <type_traits>
#include <memory>
#include <algorithm>
#include <iterator>
#include <functional>
#include <iomanip>
#include <cctype>


namespace Xsc
{


// Alternative to std::make_unique for strict C++11 support.
template <typename T, typename... Args>
std::unique_ptr<T> MakeUnique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// Removes all entries from the specified container which are equal to the specified type.
template <typename TCollection, typename TValue>
void EraseAll(TCollection& collection, const TValue& value)
{
    collection.erase(
        std::remove(std::begin(collection), std::end(collection), value),
        std::end(collection)
    );
}

// Removes all entries from the specified container for which the specified predicate is true.
template <typename TCollection, typename TPredicate>
void EraseAllIf(TCollection& collection, TPredicate pred)
{
    collection.erase(
        std::remove_if(std::begin(collection), std::end(collection), pred),
        std::end(collection)
    );
}

// Moves all entries from the source into the destination.
template <typename TCollectionSrc, typename TCollectionDest>
void MoveAll(TCollectionSrc& src, TCollectionDest& dst)
{
    std::move(std::begin(src), std::end(src), std::back_inserter(dst));
    src.clear();
}

// Moves all entries from the source into the destination for which the specified predicate is true.
template <typename TCollectionSrc, typename TCollectionDest, typename TPredicate>
void MoveAllIf(TCollectionSrc& src, TCollectionDest& dst, TPredicate pred)
{
    for (auto it = src.begin(); it != src.end();)
    {
        if (pred(*it))
        {
            dst.push_back(*it);
            it = src.erase(it);
        }
        else
            ++it;
    }
}

// Replaces all occurances of 'from' in the string 's' by 'to'.
template <
    class CharT,
    class Traits = std::char_traits<CharT>,
    class Allocator = std::allocator<CharT>
>
void Replace(
    std::basic_string<CharT, Traits, Allocator>& s,
    const std::basic_string<CharT, Traits, Allocator>& from,
    const std::basic_string<CharT, Traits, Allocator>& to)
{
    using T = std::basic_string<CharT, Traits, Allocator>;
    for (typename T::size_type pos = 0; ( pos = s.find(from, pos) ) != T::npos; pos += to.size())
        s.replace(pos, from.size(), to);
}

// Replaces all occurances of 'from' in the string 's' by 'to'.
template <
    class CharT,
    class Traits = std::char_traits<CharT>,
    class Allocator = std::allocator<CharT>
>
void Replace(
    std::basic_string<CharT, Traits, Allocator>& s,
    const char* from,
    const char* to)
{
    Replace(s, std::basic_string<CharT, Traits, Allocator>(from), std::basic_string<CharT, Traits, Allocator>(to));
}

// Replaces all occurances of 'from' in the string 's' by 'to'.
template <
    class CharT,
    class Traits = std::char_traits<CharT>,
    class Allocator = std::allocator<CharT>
>
void Replace(
    std::basic_string<CharT, Traits, Allocator>& s,
    const std::basic_string<CharT, Traits, Allocator>& from,
    const char* to)
{
    Replace(s, from, std::basic_string<CharT, Traits, Allocator>(to));
}

// Replaces all occurances of 'from' in the string 's' by 'to'.
template <
    class CharT,
    class Traits = std::char_traits<CharT>,
    class Allocator = std::allocator<CharT>
>
void Replace(
    std::basic_string<CharT, Traits, Allocator>& s,
    const char* from,
    const std::basic_string<CharT, Traits, Allocator>& to)
{
    Replace(s, std::basic_string<CharT, Traits, Allocator>(from), to);
}

// Parses a number from the specified string with std::stoi, std::stoll, std::stof, or std::stod
template <typename T>
inline T FromStringOrDefault(const std::string& s)
{
    throw std::runtime_error("default template of FromStringOrDefault<T> not implemented");
}

// Returns true if the specified string contains a hexadecimal number.
inline bool IsHexLiteral(const std::string& s)
{
    return (s.size() >= 3 && s[0] == '0' && ::toupper(s[1]) == 'X');
}

template <>
inline int FromStringOrDefault<int>(const std::string& s)
{
    try
    {
        return std::stoi(s, nullptr, (IsHexLiteral(s) ? 16 : 10));
    }
    catch (const std::exception&)
    {
        return 0;
    }
}

template <>
inline long long FromStringOrDefault<long long>(const std::string& s)
{
    try
    {
        return std::stoll(s, nullptr, (IsHexLiteral(s) ? 16 : 10));
    }
    catch (const std::exception&)
    {
        return 0ll;
    }
}

template <>
inline unsigned long FromStringOrDefault<unsigned long>(const std::string& s)
{
    try
    {
        return std::stoul(s, nullptr, (IsHexLiteral(s) ? 16 : 10));
    }
    catch (const std::exception&)
    {
        return 0ul;
    }
}

template <>
inline float FromStringOrDefault<float>(const std::string& s)
{
    try
    {
        return std::stof(s);
    }
    catch (const std::exception&)
    {
        return 0.0f;
    }
}

template <>
inline double FromStringOrDefault<double>(const std::string& s)
{
    try
    {
        return std::stod(s);
    }
    catch (const std::exception&)
    {
        return 0.0;
    }
}

// Transforms the specified string to upper case.
template <typename T>
void ToUpper(T& s)
{
    std::transform(std::begin(s), std::end(s), std::begin(s), ::toupper);
}

/*
Merges the source string 'src' into the destination string 'dst',
keeps the destination characters specified by 'keepDst',
and ignores the source characters specified by 'ignoreChar'.
*/
template <typename T>
void MergeString(std::basic_string<T>& dst, const std::basic_string<T>& src, const T& keepDst, const T& ignoreSrc)
{
    const auto nDst = dst.size();
    const auto nSrc = src.size();

    /* Replace characters in the destination string */
    typename std::basic_string<T>::size_type i = 0;

    for (; i < nDst; ++i)
    {
        if (i < nSrc)
        {
            if (dst[i] != keepDst && src[i] != ignoreSrc)
                dst[i] = src[i];
        }
        else
            return;
    }

    /* Append remaining characters to destination string */
    if (i < nSrc)
        dst.append(src.substr(i));
}

// Returns a hexa-decimal string of the specified integral value.
template <typename T>
std::string ToHexString(const T& i, const std::string& prefix = "0x")
{
    std::stringstream s;
    s << prefix << std::setfill('0') << std::setw(sizeof(T)*2) << std::hex << i;
    return s.str();
}

// Returns the number of digits of the specified integral type.
template <typename T>
int NumDigits(T n)
{
    static_assert(std::is_integral<T>::value, "NumDigits template only allows integral types");
    
    int digits = (n < 0 ? 1 : 0);

    while (n != 0)
    {
        n /= 10;
        ++digits;
    }

    return digits;
}


} // /namespace Xsc


#endif



// ================================================================================
