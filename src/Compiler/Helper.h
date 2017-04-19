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


namespace Xsc
{


//! Alternative to std::make_unique for strict C++11 support.
template <typename T, typename... Args>
std::unique_ptr<T> MakeUnique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T, typename... Args>
std::shared_ptr<T> MakeShared(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

// Removes all entries from the specified container which are equal to the specified type.
template <typename Cont, typename Value>
void EraseAll(Cont& container, Value value)
{
    container.erase(
        std::remove(std::begin(container), std::end(container), value),
        std::end(container)
    );
}

// Removes all entries from the specified container for which the specified predicate is true.
template <typename Cont, typename Pred>
void EraseAllIf(Cont& container, Pred pred)
{
    container.erase(
        std::remove_if(std::begin(container), std::end(container), pred),
        std::end(container)
    );
}

// Moves all entries from the source into the destination.
template <typename Source, typename Destination>
void MoveAll(Source& source, Destination& destination)
{
    std::move(std::begin(source), std::end(source), std::back_inserter(destination));
    source.clear();
}

// Moves all entries from the source into the destination for which the specified predicate is true.
template <typename Source, typename Destination, typename Pred>
void MoveAllIf(Source& source, Destination& destination, Pred pred)
{
    for (auto it = source.begin(); it != source.end();)
    {
        if (pred(*it))
        {
            destination.push_back(*it);
            it = source.erase(it);
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


} // /namespace Xsc


#endif



// ================================================================================