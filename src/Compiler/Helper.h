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

template <>
inline int FromStringOrDefault<int>(const std::string& s)
{
    try { return std::stoi(s); } catch (const std::exception&) { return 0; }
}

template <>
inline long long FromStringOrDefault<long long>(const std::string& s)
{
    try { return std::stoll(s); } catch (const std::exception&) { return 0ll; }
}

template <>
inline unsigned long FromStringOrDefault<unsigned long>(const std::string& s)
{
    try { return std::stoul(s); } catch (const std::exception&) { return 0ul; }
}

template <>
inline float FromStringOrDefault<float>(const std::string& s)
{
    try { return std::stof(s); } catch (const std::exception&) { return 0.0f; }
}

template <>
inline double FromStringOrDefault<double>(const std::string& s)
{
    try { return std::stod(s); } catch (const std::exception&) { return 0.0; }
}


} // /namespace Xsc


#endif



// ================================================================================
