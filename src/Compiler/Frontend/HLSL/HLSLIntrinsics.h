/*
 * HLSLIntrinsics.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_HLSL_INTRINSICS_H
#define XSC_HLSL_INTRINSICS_H


#include "IntrinsicAdept.h"
#include "ASTEnums.h"
#include "ShaderVersion.h"
#include "TypeDenoter.h"
#include <map>


namespace Xsc
{


struct HLSLIntrinsicEntry
{
    inline HLSLIntrinsicEntry(Intrinsic intrinsic, int major, int minor) :
        intrinsic       { intrinsic    },
        minShaderModel  { major, minor }
    {
    }

    Intrinsic       intrinsic;
    ShaderVersion   minShaderModel;
};

using HLSLIntrinsicsMap = std::map<std::string, HLSLIntrinsicEntry>;


// IntrinsicAdept interface implementation for HLSL frontend.
class HLSLIntrinsicAdept : public IntrinsicAdept
{

    public:

        HLSLIntrinsicAdept();

        TypeDenoterPtr GetIntrinsicReturnType(const Intrinsic intrinsic, const std::vector<ExprPtr>& args) const override;

        std::vector<TypeDenoterPtr> GetIntrinsicParameterTypes(const Intrinsic intrinsic, const std::vector<ExprPtr>& args) const override;

        std::vector<std::size_t> GetIntrinsicOutputParameterIndices(const Intrinsic intrinsic) const override;

        // Returns the intrinsics map (Intrinsic name -> Intrinsic ID and minimum HLSL shader model).
        static const HLSLIntrinsicsMap& GetIntrinsicMap();

};


} // /namespace Xsc


#endif



// ================================================================================