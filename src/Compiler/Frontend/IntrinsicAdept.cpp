/*
 * IntrinsicAdept.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "IntrinsicAdept.h"
#include "Exception.h"
#include "AST.h"


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
    std::string s = "ambiguous intrinsic call '";

    s += GetIntrinsicIdent(intrinsic);
    s += '(';

    for (std::size_t i = 0, n = args.size(); i < n; ++i)
    {
        s += args[i]->GetTypeDenoter()->ToString();
        if (i + 1 < n)
            s += ", ";
    }

    s += ")'";

    RuntimeErr(s);
}


} // /namespace Xsc



// ================================================================================
