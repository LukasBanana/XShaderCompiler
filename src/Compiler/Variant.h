/*
 * Variant.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_VARIANT_H
#define XSC_VARIANT_H


#include "Visitor.h"


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
            Bool,
            Int,
            Real,
        };

        Variant() = default;
        Variant(const Variant&) = default;
        Variant(BoolType value);
        Variant(IntType value);
        Variant(RealType value);

        Variant& operator = (const Variant& rhs) = default;
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

        BoolType ToBool();
        IntType ToInt();
        RealType ToReal();

        // Returns -1 if this variant is less than 'rhs', 0 if they are equal, and 1 if this variant is greater than 'rhs'.
        int CompareWith(const Variant& rhs) const;

        inline BoolType Bool() const
        {
            return bool_;
        }

        inline IntType Int() const
        {
            return int_;
        }

        inline RealType Real() const
        {
            return real_;
        }

        // Returns the current internal type of this variant.
        inline Types Type() const
        {
            return type_;
        }

        static Variant ParseFrom(const std::string& s);

        std::string ToString() const;

    private:

        Types       type_   = Types::Int;
        BoolType    bool_   = false;
        IntType     int_    = 0;
        RealType    real_   = 0.0;
    
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