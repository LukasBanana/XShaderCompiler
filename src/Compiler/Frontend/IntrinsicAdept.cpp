/*
 * IntrinsicAdept.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "IntrinsicAdept.h"
#include "Exception.h"


namespace Xsc
{


IntrinsicAdept::~IntrinsicAdept()
{
}

const std::string& IntrinsicAdept::GetIntrinsicIdent(const Intrinsic intrinsic) const
{
    static const std::string unknwonIntrinsic = "<undefined>";
    const auto idx = INTRINSIC_IDX(intrinsic);
    return (idx < intrinsicIdents_.size() ? intrinsicIdents_[idx] : unknwonIntrinsic);
}


/*
 * ======= Protected: =======
 */

[[noreturn]]
void IntrinsicAdept::ThrowAmbiguousIntrinsicCall(const Intrinsic intrinsic, const std::vector<ExprPtr>& args)
{
    std::string s = "ambiguous intrinsic call";

    //TODO: add more detailed message with argument types ...

    RuntimeErr(s);
}


} // /namespace Xsc



// ================================================================================
