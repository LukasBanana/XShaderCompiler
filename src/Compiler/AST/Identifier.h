/*
 * Identifier.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_IDENTIFIER_H
#define XSC_IDENTIFIER_H


#include <string>


namespace Xsc
{


/*
Class to manage identifiers that can be renamed (maybe several times),
to keep track of the original identifier (e.g. for error reports).
*/
class Identifier
{

    public:

        Identifier() = default;
        Identifier(const Identifier&) = default;

        // Renames this identifier by the final of the specified identifier.
        Identifier& operator = (const Identifier& rhs);

        // Renames this identifier by the specified string.
        Identifier& operator = (const std::string& s);

        // Renames this identifier by appending the specified prefix to the front (if the identifier does not already have this prefix).
        Identifier& AppendPrefix(const std::string& prefix);

        // Renames this identifier by removing the specified prefix.
        Identifier& RemovePrefix(const std::string& prefix);

        // Returns the final identifier (i.e. renamed identifier if set, otherwise original).
        const std::string& Final() const;

        // Returns true if the final of this identifier is empty.
        inline bool Empty() const
        {
            return Final().empty();
        }

        // Operator shortcut for 'Final()'.
        inline operator const std::string& () const
        {
            return Final();
        }

        // Returns the original identifier.
        inline const std::string& Original() const
        {
            return original_;
        }

        // Returns true if this identifier is renamed.
        inline bool IsRenamed() const
        {
            return !renamed_.empty();
        }

    private:

        bool        originalSet_    = false;
        std::string original_;

        bool        renamedSet_     = false;
        std::string renamed_;

        int         counter_        = 0;

};


inline bool operator == (const Identifier& lhs, const Identifier& rhs)
{
    return (std::string(lhs) == std::string(rhs));
}

inline bool operator == (const std::string& lhs, const Identifier& rhs)
{
    return (lhs == std::string(rhs));
}

inline bool operator == (const Identifier& lhs, const std::string& rhs)
{
    return (std::string(lhs) == rhs);
}


inline bool operator != (const Identifier& lhs, const Identifier& rhs)
{
    return (std::string(lhs) != std::string(rhs));
}

inline bool operator != (const std::string& lhs, const Identifier& rhs)
{
    return (lhs != std::string(rhs));
}

inline bool operator != (const Identifier& lhs, const std::string& rhs)
{
    return (std::string(lhs) != rhs);
}


inline std::string operator + (const Identifier& lhs, const Identifier& rhs)
{
    return (std::string(lhs) + std::string(rhs));
}

inline std::string operator + (const std::string& lhs, const Identifier& rhs)
{
    return (lhs + std::string(rhs));
}

inline std::string operator + (const Identifier& lhs, const std::string& rhs)
{
    return (std::string(lhs) + rhs);
}

inline std::string operator + (char lhs, const Identifier& rhs)
{
    return (lhs + std::string(rhs));
}

inline std::string operator + (const Identifier& lhs, char rhs)
{
    return (std::string(lhs) + rhs);
}


} // /namespace Xsc


#endif



// ================================================================================