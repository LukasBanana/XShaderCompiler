/*
 * Variant.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Variant.h"
#include "Helper.h"
#include <sstream>


namespace Xsc
{


/*
 * Variant class
 */

Variant::Variant(BoolType value) :
    type_{ Types::Bool },
    bool_{ value       }
{
}

Variant::Variant(IntType value) :
    type_   { Types::Int },
    int_    { value      }
{
}

Variant::Variant(RealType value) :
    type_{ Types::Real },
    real_{ value       }
{
}

#define IMPLEMENT_VARIANT_OP(OP)    \
    switch (type_)                  \
    {                               \
        case Types::Bool:           \
            /* dummy case block */; \
            break;                  \
        case Types::Int:            \
            int_ OP rhs.int_;       \
            break;                  \
        case Types::Real:           \
            real_ OP rhs.real_;     \
            break;                  \
    }                               \
    return *this                    \

#define IMPLEMENT_VARIANT_BITWISE_OP(OP)    \
    switch (type_)                          \
    {                                       \
        case Types::Int:                    \
            int_ OP rhs.int_;               \
            break;                          \
        default:                            \
            /* dummy case block */          \
            break;                          \
    }                                       \
    return *this                            \

Variant& Variant::operator += (const Variant& rhs)
{
    IMPLEMENT_VARIANT_OP(+=);
}

Variant& Variant::operator -= (const Variant& rhs)
{
    IMPLEMENT_VARIANT_OP(-=);
}

Variant& Variant::operator *= (const Variant& rhs)
{
    IMPLEMENT_VARIANT_OP(*=);
}

Variant& Variant::operator /= (const Variant& rhs)
{
    IMPLEMENT_VARIANT_OP(/=);
}

Variant& Variant::operator %= (const Variant& rhs)
{
    IMPLEMENT_VARIANT_BITWISE_OP(%=);
}

Variant& Variant::operator |= (const Variant& rhs)
{
    IMPLEMENT_VARIANT_BITWISE_OP(|=);
}

Variant& Variant::operator &= (const Variant& rhs)
{
    IMPLEMENT_VARIANT_BITWISE_OP(&=);
}

Variant& Variant::operator ^= (const Variant& rhs)
{
    IMPLEMENT_VARIANT_BITWISE_OP(^=);
}

Variant& Variant::operator <<= (const Variant& rhs)
{
    IMPLEMENT_VARIANT_BITWISE_OP(<<=);
}

Variant& Variant::operator >>= (const Variant& rhs)
{
    IMPLEMENT_VARIANT_BITWISE_OP(>>=);
}

#undef IMPLEMENT_VARIANT_OP
#undef IMPLEMENT_VARIANT_BITWISE_OP

Variant& Variant::operator ++ ()
{
    switch (type_)
    {
        case Types::Bool:
            // dummy case block
            break;
        case Types::Int:
            ++int_;
            break;
        case Types::Real:
            ++real_;
            break;
    }
    return *this;
}

Variant& Variant::operator -- ()
{
    switch (type_)
    {
        case Types::Bool:
            // dummy case block
            break;
        case Types::Int:
            --int_;
            break;
        case Types::Real:
            --real_;
            break;
    }
    return *this;
}

Variant Variant::operator - ()
{
    Variant result = *this;

    switch (type_)
    {
        case Types::Bool:
            // dummy case block
            break;
        case Types::Int:
            result.int_ = -int_;
            break;
        case Types::Real:
            result.real_ = -real_;
            break;
    }

    return result;
}

Variant Variant::operator ~ ()
{
    Variant result = *this;

    switch (type_)
    {
        case Types::Int:
            result.int_ = ~int_;
            break;
        default:
            // dummy case block
            break;
    }

    return result;
}

Variant Variant::operator ! ()
{
    Variant result = *this;

    switch (type_)
    {
        case Types::Bool:
            result.bool_ = !bool_;
            break;
        case Types::Int:
            result.int_ = !int_;
            break;
        case Types::Real:
            result.real_ = !real_;
            break;
    }

    return result;
}

Variant::BoolType Variant::ToBool()
{
    switch (type_)
    {
        case Types::Bool:
            // dummy case block
            break;
        case Types::Int:
            type_ = Types::Bool;
            bool_ = (int_ != 0);
            break;
        case Types::Real:
            type_ = Types::Bool;
            bool_ = (real_ != 0.0f);
            break;
    }
    return bool_;
}

Variant::IntType Variant::ToInt()
{
    switch (type_)
    {
        case Types::Bool:
            type_ = Types::Int;
            int_ = static_cast<IntType>(bool_);
            break;
        case Types::Int:
            // dummy case block
            break;
        case Types::Real:
            type_ = Types::Int;
            int_ = static_cast<IntType>(real_);
            break;
    }
    return int_;
}

Variant::RealType Variant::ToReal()
{
    switch (type_)
    {
        case Types::Bool:
            type_ = Types::Real;
            real_ = static_cast<RealType>(bool_);
            break;
        case Types::Int:
            type_ = Types::Real;
            real_ = static_cast<RealType>(int_);
            break;
        case Types::Real:
            // dummy case block
            break;
    }
    return real_;
}

int Variant::CompareWith(const Variant& rhs) const
{
    auto cmp = rhs;

    switch (type_)
    {
        case Types::Bool:
        {
            cmp.ToBool();
            if (Bool() && !cmp.Bool())
                return 1;
            if (!Bool() && cmp.Bool())
                return -1;
        }
        break;

        case Types::Int:
        {
            cmp.ToInt();
            if (Int() < cmp.Int())
                return -1;
            if (Int() > cmp.Int())
                return 1;
        }
        break;

        case Types::Real:
        {
            cmp.ToReal();
            if (Real() < cmp.Real())
                return -1;
            if (Real() > cmp.Real())
                return 1;
        }
        break;
    }

    return 0;
}

Variant Variant::ParseFrom(const std::string& s)
{
    if (s == "true")
        return Variant(true);
    else if (s == "false")
        return Variant(false);
    else if (s.find_first_of(".eE") != std::string::npos)
        return Variant(FromString<Variant::RealType>(s));
    else
        return Variant(FromString<Variant::IntType>(s));
}

static std::string RealToString(Variant::RealType v)
{
    auto s = std::to_string(v);
    s.erase(s.find_last_not_of('0') + 2, std::string::npos);
    return s;
}

std::string Variant::ToString() const
{
    switch (Type())
    {
        case Types::Bool:
            return (Bool() ? "true" : "false");
        case Types::Int:
            return std::to_string(Int());
        case Types::Real:
            return RealToString(Real());
    }
    return "";
}


/*
 * Global functions
 */

Variant operator == (const Variant& lhs, const Variant& rhs)
{
    return (lhs.CompareWith(rhs) == 0);
}

Variant operator != (const Variant& lhs, const Variant& rhs)
{
    return (lhs.CompareWith(rhs) != 0);
}

Variant operator < (const Variant& lhs, const Variant& rhs)
{
    return (lhs.CompareWith(rhs) < 0);
}

Variant operator <= (const Variant& lhs, const Variant& rhs)
{
    return (lhs.CompareWith(rhs) <= 0);
}

Variant operator > (const Variant& lhs, const Variant& rhs)
{
    return (lhs.CompareWith(rhs) > 0);
}

Variant operator >= (const Variant& lhs, const Variant& rhs)
{
    return (lhs.CompareWith(rhs) >= 0);
}

Variant operator + (const Variant& lhs, const Variant& rhs)
{
    Variant result = lhs;
    result += rhs;
    return result;
}

Variant operator - (const Variant& lhs, const Variant& rhs)
{
    Variant result = lhs;
    result -= rhs;
    return result;
}

Variant operator * (const Variant& lhs, const Variant& rhs)
{
    Variant result = lhs;
    result *= rhs;
    return result;
}

Variant operator / (const Variant& lhs, const Variant& rhs)
{
    Variant result = lhs;
    result /= rhs;
    return result;
}

Variant operator % (const Variant& lhs, const Variant& rhs)
{
    Variant result = lhs;
    result %= rhs;
    return result;
}

Variant operator | (const Variant& lhs, const Variant& rhs)
{
    Variant result = lhs;
    result |= rhs;
    return result;
}

Variant operator & (const Variant& lhs, const Variant& rhs)
{
    Variant result = lhs;
    result &= rhs;
    return result;
}

Variant operator ^ (const Variant& lhs, const Variant& rhs)
{
    Variant result = lhs;
    result ^= rhs;
    return result;
}

Variant operator << (const Variant& lhs, const Variant& rhs)
{
    Variant result = lhs;
    result <<= rhs;
    return result;
}

Variant operator >> (const Variant& lhs, const Variant& rhs)
{
    Variant result = lhs;
    result >>= rhs;
    return result;
}


} // /namespace Xsc



// ================================================================================
