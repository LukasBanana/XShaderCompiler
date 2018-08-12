/*
 * Variant.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_VARIANT_H
#define XSC_VARIANT_H


#include "Visitor.h"
#include <string>
#include <vector>


namespace Xsc
{


// Helper class to simply cast expressions between boolean, float, and integral types.
class Variant
{

    public:

        using BoolType = bool;
        using IntType = long long;
        using RealType = double;

        enum class Types
        {
            Undefined,
            Bool,
            Int,
            Real,
            Array,
        };

        Variant() = default;
        Variant(const Variant&) = default;
        Variant(Variant&&) = default;
        Variant(BoolType value);
        Variant(IntType value);
        Variant(RealType value);
        Variant(const std::vector<Variant>& subValues);
        Variant(std::vector<Variant>&& subValues);

        Variant& operator = (const Variant&) = default;
        Variant& operator = (Variant&&) = default;

        Variant& operator += (const Variant& rhs);
        Variant& operator -= (const Variant& rhs);
        Variant& operator *= (const Variant& rhs);
        Variant& operator /= (const Variant& rhs);
        Variant& operator %= (const Variant& rhs);
        Variant& operator |= (const Variant& rhs);
        Variant& operator &= (const Variant& rhs);
        Variant& operator ^= (const Variant& rhs);
        Variant& operator <<= (const Variant& rhs);
        Variant& operator >>= (const Variant& rhs);
        Variant& operator ++ ();
        Variant& operator -- ();

        Variant operator - ();
        Variant operator ~ ();
        Variant operator ! ();

        // Converts this variant to a boolean type and returns its value.
        BoolType ToBool();

        // Converts this variant to an integral type and returns its value.
        IntType ToInt();

        // Converts this variant to a real type and returns its value.
        RealType ToReal();

        // Returns -1 if this variant is less than 'rhs', 0 if they are equal, and 1 if this variant is greater than 'rhs'.
        int CompareWith(const Variant& rhs) const;

        // Returns the sub variant of the array value, or the default variant if this is not an array or the index is out of bounds.
        Variant ArraySub(std::size_t idx) const;

        // Returns true if this variant is a boolean type.
        inline bool IsBool() const
        {
            return (Type() == Types::Bool);
        }

        // Returns true if this variant is an integral type.
        inline bool IsInt() const
        {
            return (Type() == Types::Int);
        }

        // Returns true if this variant is a real type.
        inline bool IsReal() const
        {
            return (Type() == Types::Real);
        }

        // Returns true if this variant is an array type.
        inline bool IsArray() const
        {
            return (Type() == Types::Array);
        }

        // Returns the boolean value.
        inline BoolType Bool() const
        {
            return bool_;
        }

        // Returns the integral value.
        inline IntType Int() const
        {
            return int_;
        }

        // Returns the real value.
        inline RealType Real() const
        {
            return real_;
        }

        // Returns the array value.
        inline const std::vector<Variant>& Array() const
        {
            return array_;
        }

        // Returns the current internal type of this variant.
        inline Types Type() const
        {
            return type_;
        }

        // Returns true if this variant is not undefined.
        inline operator bool () const
        {
            return (Type() != Types::Undefined);
        }

        // Returns a variant, parsed from the specified string (e.g. "true" for a boolean type, or "1.5" for a real type).
        static Variant ParseFrom(const std::string& s);

        // Returns this variant as string.
        std::string ToString() const;

    private:

        Types                   type_   = Types::Undefined;
        BoolType                bool_   = false;
        IntType                 int_    = 0;
        RealType                real_   = 0.0;
        std::vector<Variant>    array_;

};


/* ----- Global operators ----- */

Variant operator == (const Variant& lhs, const Variant& rhs);
Variant operator != (const Variant& lhs, const Variant& rhs);
Variant operator < (const Variant& lhs, const Variant& rhs);
Variant operator <= (const Variant& lhs, const Variant& rhs);
Variant operator > (const Variant& lhs, const Variant& rhs);
Variant operator >= (const Variant& lhs, const Variant& rhs);
Variant operator + (const Variant& lhs, const Variant& rhs);
Variant operator - (const Variant& lhs, const Variant& rhs);
Variant operator * (const Variant& lhs, const Variant& rhs);
Variant operator / (const Variant& lhs, const Variant& rhs);
Variant operator % (const Variant& lhs, const Variant& rhs);
Variant operator | (const Variant& lhs, const Variant& rhs);
Variant operator & (const Variant& lhs, const Variant& rhs);
Variant operator ^ (const Variant& lhs, const Variant& rhs);
Variant operator << (const Variant& lhs, const Variant& rhs);
Variant operator >> (const Variant& lhs, const Variant& rhs);


} // /namespace Xsc


#endif



// ================================================================================