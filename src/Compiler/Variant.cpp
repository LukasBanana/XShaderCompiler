/*
 * Variant.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
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
    type_ { Types::Bool },
    bool_ { value       }
{
}

Variant::Variant(IntType value) :
    type_ { Types::Int },
    int_  { value      }
{
}

Variant::Variant(RealType value) :
    type_ { Types::Real },
    real_ { value       }
{
}

Variant::Variant(const std::vector<Variant>& subValues) :
    type_  { Types::Array }//,
    //array_ { subValues    }
{
    array_ = subValues;
}

Variant::Variant(std::vector<Variant>&& subValues) :
    type_  { Types::Array         }//,
    //array_ { std::move(subValues) }
{
    array_ = std::move(subValues);
}

#define IMPLEMENT_VARIANT_OP(OP)    \
    switch (type_)                  \
    {                               \
        case Types::Int:            \
            int_ OP rhs.int_;       \
            break;                  \
        case Types::Real:           \
            real_ OP rhs.real_;     \
            break;                  \
        default:                    \
            /* dummy case block */; \
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
        case Types::Int:
            ++int_;
            break;
        case Types::Real:
            ++real_;
            break;
        default:
            // dummy case block
            break;
    }
    return *this;
}

Variant& Variant::operator -- ()
{
    switch (type_)
    {
        case Types::Int:
            --int_;
            break;
        case Types::Real:
            --real_;
            break;
        default:
            // dummy case block
            break;
    }
    return *this;
}

Variant Variant::operator - ()
{
    Variant result = *this;

    switch (type_)
    {
        case Types::Int:
            result.int_ = -int_;
            break;
        case Types::Real:
            result.real_ = -real_;
            break;
        default:
            // dummy case block
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
        default:
            break;
    }

    return result;
}

Variant::BoolType Variant::ToBool()
{
    switch (type_)
    {
        case Types::Int:
            type_ = Types::Bool;
            bool_ = (int_ != 0);
            break;
        case Types::Real:
            type_ = Types::Bool;
            bool_ = (real_ != 0.0f);
            break;
        default:
            // dummy case block
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
        case Types::Real:
            type_ = Types::Int;
            int_ = static_cast<IntType>(real_);
            break;
        default:
            // dummy case block
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
        default:
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

        default:
        break;
    }

    return 0;
}

Variant Variant::ArraySub(std::size_t idx) const
{
    if (Type() == Types::Array)
    {
        if (idx < array_.size())
            return array_[idx];
    }
    return {};
}

Variant Variant::ParseFrom(const std::string& s)
{
    if (s == "true")
        return Variant(true);
    else if (s == "false")
        return Variant(false);
    else if (s.find_first_of(".eE") != std::string::npos)
        return Variant(FromStringOrDefault<double>(s));
    else
        return Variant(FromStringOrDefault<long long>(s));
}

static void CropStringRight(std::string& s, std::size_t pos)
{
    if (pos < s.size())
        s.erase(pos, std::string::npos);
}

static std::string RealToString(Variant::RealType v)
{
    auto s = std::to_string(v);
    auto posFract = s.find('.');
    if (posFract != std::string::npos)
    {
        auto pos = s.find_last_not_of('0');
        if (pos != std::string::npos)
        {
            if (pos == posFract)
                CropStringRight(s, pos + 2);
            else
                CropStringRight(s, pos + 1);
        }
    }
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
        default:
            return "";
    }
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
