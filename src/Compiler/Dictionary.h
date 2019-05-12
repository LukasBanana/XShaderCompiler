/*
 * Dictionary.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_DICTIONARY_H
#define XSC_DICTIONARY_H


#include <map>
#include <string>
#include <vector>
#include <initializer_list>
#include <algorithm>


namespace Xsc
{


// Bidirectional map template class, where Key = string, Value = T. 'T' must be an asceding enumerable type.
template <typename T>
class Dictionary
{

    public:

        Dictionary() = default;
        Dictionary(const Dictionary&) = default;

        Dictionary(const std::initializer_list<std::pair<std::string, T>>& stringToEnumPairs) :
            stringToEnum_ { stringToEnumPairs.begin(), stringToEnumPairs.end() }
        {
            /* Reserve container memory in advance */
            std::size_t maxIndex = 0;

            for (const auto& pair : stringToEnumPairs)
                maxIndex = std::max(maxIndex, static_cast<std::size_t>(pair.second));

            enumToString_.resize(maxIndex + 1, nullptr);

            /* Insert references to strings in map */
            for (const auto& pair : stringToEnumPairs)
            {
                const auto idx = static_cast<std::size_t>(pair.second);
                if (enumToString_[idx] == nullptr)
                {
                    auto it = stringToEnum_.find(pair.first);
                    if (it != stringToEnum_.end())
                        enumToString_[idx] = &(it->first);
                }
            }
        }

        // Returns a pointer to the enumeration entry which is associated to the specified string, or null on failure.
        const T* StringToEnum(const std::string& s) const
        {
            auto it = stringToEnum_.find(s);
            if (it != stringToEnum_.end())
                return &(it->second);
            else
                return nullptr;
        }

        // Returns the enumeration entry which is associated to the specified string, or the default value on failure.
        T StringToEnumOrDefault(const std::string& s, const T& defaultValue) const
        {
            auto it = stringToEnum_.find(s);
            if (it != stringToEnum_.end())
                return it->second;
            else
                return defaultValue;
        }

        // Returns a pointer to the first string which is associated to the specified enumeration entry, or null on failure.
        const std::string* EnumToString(const T& e) const
        {
            const auto idx = static_cast<std::size_t>(e);
            if (idx < enumToString_.size())
                return enumToString_[idx];
            else
                return nullptr;
        }

        // Returns the first string which is associated to the specified enumeration entry, or the default string on failure.
        std::string EnumToStringOrDefault(const T& e, const std::string& defaultString) const
        {
            const auto idx = static_cast<std::size_t>(e);
            if (idx < enumToString_.size())
            {
                if (auto s = enumToString_[idx])
                    return *s;
            }
            return defaultString;
        }

    private:

        std::map<std::string, T>        stringToEnum_;
        std::vector<const std::string*> enumToString_;

};


} // /namespace Xsc


#endif



// ================================================================================