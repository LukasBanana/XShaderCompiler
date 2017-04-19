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

// Converts the specified strin to lower case.
std::string ToLower(const std::string& s);

// Converts the specified strin to upper case.
std::string ToUpper(const std::string& s);

// Replaces all occurances of 'from' in the string 's' by 'to'.
void Replace(std::string& s, const std::string& from, const std::string& to);


} // /namespace Xsc


#endif



// ================================================================================