/*
 * IntrinsicAdept.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "IntrinsicAdept.h"
#include "Exception.h"
#include "AST.h"
#include "ReportIdents.h"


namespace Xsc
{


XSC_THREAD_LOCAL static IntrinsicAdept* g_intrinsicAdeptInstance = nullptr;

IntrinsicAdept::IntrinsicAdept()
{
    g_intrinsicAdeptInstance = this;
}

IntrinsicAdept::~IntrinsicAdept()
{
    g_intrinsicAdeptInstance = nullptr;
}

const IntrinsicAdept& IntrinsicAdept::Get()
{
    return *g_intrinsicAdeptInstance;
}

const std::string& IntrinsicAdept::GetIntrinsicIdent(const Intrinsic intrinsic) const
{
    static const std::string unknwonIntrinsic = R_Undefined();
    const auto idx = INTRINSIC_IDX(intrinsic);
    return (idx < intrinsicIdents_.size() ? intrinsicIdents_[idx] : unknwonIntrinsic);
}


/*
 * ======= Protected: =======
 */

void IntrinsicAdept::SetIntrinsicIdent(const Intrinsic intrinsic, const std::string& ident)
{
    const auto idx = INTRINSIC_IDX(intrinsic);
    if (idx < intrinsicIdents_.size())
        intrinsicIdents_[idx] = ident;
}

void IntrinsicAdept::FillOverloadedIntrinsicIdents()
{
    const std::string* prevIdent = nullptr;

    for (auto& ident : intrinsicIdents_)
    {
        if (ident.empty())
        {
            if (prevIdent)
                ident = *prevIdent;
        }
        else
            prevIdent = (&ident);
    }
}

[[noreturn]]
void IntrinsicAdept::ThrowAmbiguousIntrinsicCall(const Intrinsic intrinsic, const std::vector<ExprPtr>& args)
{
    std::string s;

    s += GetIntrinsicIdent(intrinsic);
    s += '(';

    for (std::size_t i = 0, n = args.size(); i < n; ++i)
    {
        s += args[i]->GetTypeDenoter()->ToString();
        if (i + 1 < n)
            s += ", ";
    }

    s += ')';

    RuntimeErr(R_AmbiguousIntrinsicCall(s));
}


} // /namespace Xsc



// ================================================================================
