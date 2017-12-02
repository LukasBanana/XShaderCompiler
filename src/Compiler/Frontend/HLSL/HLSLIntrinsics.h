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
        intrinsic      { intrinsic    },
        minShaderModel { major, minor }
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

        TypeDenoterPtr GetIntrinsicReturnType(
            const Intrinsic intrinsic,
            const std::vector<ExprPtr>& args,
            const TypeDenoterPtr& prefixTypeDenoter = nullptr
        ) const override;

        std::vector<TypeDenoterPtr> GetIntrinsicParameterTypes(CallExpr& expr) const override;

        std::vector<std::size_t> GetIntrinsicOutputParameterIndices(CallExpr& expr) const override;

        // Returns the intrinsics map (Intrinsic name -> Intrinsic ID and minimum HLSL shader model).
        static const HLSLIntrinsicsMap& GetIntrinsicMap();

    private:

        TypeDenoterPtr DeriveReturnType(const Intrinsic intrinsic, const std::vector<ExprPtr>& args) const;
        TypeDenoterPtr DeriveReturnTypeMul(const std::vector<ExprPtr>& args) const;
        TypeDenoterPtr DeriveReturnTypeMulPrimary(const std::vector<ExprPtr>& args, const TypeDenoterPtr& type0, const TypeDenoterPtr& type1) const;
        TypeDenoterPtr DeriveReturnTypeTranspose(const std::vector<ExprPtr>& args) const;
        TypeDenoterPtr DeriveReturnTypeVectorCompare(const std::vector<ExprPtr>& args) const;
        TypeDenoterPtr DeriveReturnTypeTextureSample(const BaseTypeDenoterPtr& genericTypeDenoter) const;
        TypeDenoterPtr DeriveReturnTypeTextureSampleCmp(const BaseTypeDenoterPtr& genericTypeDenoter) const;
        TypeDenoterPtr DeriveReturnTypeTextureGather(const BaseTypeDenoterPtr& genericTypeDenoter) const;
        TypeDenoterPtr DeriveReturnTypeTextureGatherCmp(const BaseTypeDenoterPtr& genericTypeDenoter) const;

        void DeriveParameterTypes(
            std::vector<TypeDenoterPtr>& paramTypeDenoters,
            const Intrinsic intrinsic,
            const std::vector<ExprPtr>& args,
            bool useMinDimension = false
        ) const;

        void DeriveParameterTypesMul(std::vector<TypeDenoterPtr>& paramTypeDenoters, const std::vector<ExprPtr>& args) const;
        void DeriveParameterTypesTranspose(std::vector<TypeDenoterPtr>& paramTypeDenoters, const std::vector<ExprPtr>& args) const;
        void DeriveParameterTypesFirstBit(std::vector<TypeDenoterPtr>& paramTypeDenoters, const std::vector<ExprPtr>& args, const Intrinsic intrinsic) const;

        BaseTypeDenoterPtr GetGenericTextureTypeFromPrefix(const Intrinsic intrinsic, const TypeDenoterPtr& prefixTypeDenoter) const;

};


} // /namespace Xsc


#endif



// ================================================================================