/*
 * ReflectionAnalyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ReflectionAnalyzer.h"
#include "ExprEvaluator.h"
#include "AST.h"
#include "Helper.h"
#include "ReportIdents.h"


namespace Xsc
{


ReflectionAnalyzer::ReflectionAnalyzer(Log* log) :
    reportHandler_ { log }
{
}

void ReflectionAnalyzer::Reflect(
    Program& program, const ShaderTarget shaderTarget, Reflection::ReflectionData& reflectionData, bool enableWarnings)
{
    /* Copy parameters */
    shaderTarget_   = shaderTarget;
    program_        = (&program);
    data_           = (&reflectionData);
    enableWarnings_ = enableWarnings;

    /* Visit program AST */
    Visit(program_);
}


/*
 * ======= Private: =======
 */

void ReflectionAnalyzer::Warning(const std::string& msg, const AST* ast)
{
    if (enableWarnings_)
        reportHandler_.Warning(false, msg, program_->sourceCode.get(), (ast ? ast->area : SourceArea::ignore));
}

int ReflectionAnalyzer::GetBindingPoint(const std::vector<RegisterPtr>& slotRegisters) const
{
    if (auto slotRegister = Register::GetForTarget(slotRegisters, shaderTarget_))
        return slotRegister->slot;
    else
        return -1;
}

int ReflectionAnalyzer::EvaluateConstExprInt(Expr& expr)
{
    /* Evaluate expression and return as integer */
    ExprEvaluator exprEvaluator;
    return static_cast<int>(exprEvaluator.EvaluateOrDefault(expr, Variant::IntType(0)).ToInt());
}

float ReflectionAnalyzer::EvaluateConstExprFloat(Expr& expr)
{
    /* Evaluate expression and return as integer */
    ExprEvaluator exprEvaluator;
    return static_cast<float>(exprEvaluator.EvaluateOrDefault(expr, Variant::RealType(0.0)).ToReal());
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ReflectionAnalyzer::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(Program)
{
    /* Visit both active and disabled code */
    Visit(ast->globalStmnts);
    Visit(ast->disabledAST);

    if (auto entryPoint = ast->entryPointRef)
    {
        /* Reflect input attributes */
        for (auto varDecl : entryPoint->inputSemantics.varDeclRefs)
            data_->inputAttributes.push_back({ varDecl->ident, varDecl->semantic.Index() });
        for (auto varDecl : entryPoint->inputSemantics.varDeclRefsSV)
            data_->inputAttributes.push_back({ varDecl->semantic.ToString(), varDecl->semantic.Index() });

        /* Reflect output attributes */
        for (auto varDecl : entryPoint->outputSemantics.varDeclRefs)
            data_->outputAttributes.push_back({ varDecl->ident, varDecl->semantic.Index() });
        for (auto varDecl : entryPoint->outputSemantics.varDeclRefsSV)
            data_->outputAttributes.push_back({ varDecl->semantic.ToString(), varDecl->semantic.Index() });
        
        if (entryPoint->semantic.IsSystemValue())
            data_->outputAttributes.push_back({ entryPoint->semantic.ToString(), entryPoint->semantic.Index() });
    }
}

/* --- Declarations --- */
Reflection::DataType DataTypeToReflType(DataType dataType)
{
#define CONVERSION_ENTRY(type) case DataType::type: return Reflection::DataType::type;

    switch (dataType)
    {
        CONVERSION_ENTRY(Bool)
            CONVERSION_ENTRY(Int)
            CONVERSION_ENTRY(UInt)
            CONVERSION_ENTRY(Half)
            CONVERSION_ENTRY(Float)
            CONVERSION_ENTRY(Double)
            CONVERSION_ENTRY(Bool2)
            CONVERSION_ENTRY(Bool3)
            CONVERSION_ENTRY(Bool4)
            CONVERSION_ENTRY(Int2)
            CONVERSION_ENTRY(Int3)
            CONVERSION_ENTRY(Int4)
            CONVERSION_ENTRY(UInt2)
            CONVERSION_ENTRY(UInt3)
            CONVERSION_ENTRY(UInt4)
            CONVERSION_ENTRY(Half2)
            CONVERSION_ENTRY(Half3)
            CONVERSION_ENTRY(Half4)
            CONVERSION_ENTRY(Float2)
            CONVERSION_ENTRY(Float3)
            CONVERSION_ENTRY(Float4)
            CONVERSION_ENTRY(Double2)
            CONVERSION_ENTRY(Double3)
            CONVERSION_ENTRY(Double4)
            CONVERSION_ENTRY(Bool2x2)
            CONVERSION_ENTRY(Bool2x3)
            CONVERSION_ENTRY(Bool2x4)
            CONVERSION_ENTRY(Bool3x2)
            CONVERSION_ENTRY(Bool3x3)
            CONVERSION_ENTRY(Bool3x4)
            CONVERSION_ENTRY(Bool4x2)
            CONVERSION_ENTRY(Bool4x3)
            CONVERSION_ENTRY(Bool4x4)
            CONVERSION_ENTRY(Int2x2)
            CONVERSION_ENTRY(Int2x3)
            CONVERSION_ENTRY(Int2x4)
            CONVERSION_ENTRY(Int3x2)
            CONVERSION_ENTRY(Int3x3)
            CONVERSION_ENTRY(Int3x4)
            CONVERSION_ENTRY(Int4x2)
            CONVERSION_ENTRY(Int4x3)
            CONVERSION_ENTRY(Int4x4)
            CONVERSION_ENTRY(UInt2x2)
            CONVERSION_ENTRY(UInt2x3)
            CONVERSION_ENTRY(UInt2x4)
            CONVERSION_ENTRY(UInt3x2)
            CONVERSION_ENTRY(UInt3x3)
            CONVERSION_ENTRY(UInt3x4)
            CONVERSION_ENTRY(UInt4x2)
            CONVERSION_ENTRY(UInt4x3)
            CONVERSION_ENTRY(UInt4x4)
            CONVERSION_ENTRY(Half2x2)
            CONVERSION_ENTRY(Half2x3)
            CONVERSION_ENTRY(Half2x4)
            CONVERSION_ENTRY(Half3x2)
            CONVERSION_ENTRY(Half3x3)
            CONVERSION_ENTRY(Half3x4)
            CONVERSION_ENTRY(Half4x2)
            CONVERSION_ENTRY(Half4x3)
            CONVERSION_ENTRY(Half4x4)
            CONVERSION_ENTRY(Float2x2)
            CONVERSION_ENTRY(Float2x3)
            CONVERSION_ENTRY(Float2x4)
            CONVERSION_ENTRY(Float3x2)
            CONVERSION_ENTRY(Float3x3)
            CONVERSION_ENTRY(Float3x4)
            CONVERSION_ENTRY(Float4x2)
            CONVERSION_ENTRY(Float4x3)
            CONVERSION_ENTRY(Float4x4)
            CONVERSION_ENTRY(Double2x2)
            CONVERSION_ENTRY(Double2x3)
            CONVERSION_ENTRY(Double2x4)
            CONVERSION_ENTRY(Double3x2)
            CONVERSION_ENTRY(Double3x3)
            CONVERSION_ENTRY(Double3x4)
            CONVERSION_ENTRY(Double4x2)
            CONVERSION_ENTRY(Double4x3)
            CONVERSION_ENTRY(Double4x4)
    default:
        return Reflection::DataType::Undefined;
    }

#undef CONVERSION_ENTRY
}

Reflection::BufferType BufferTypeToReflType(BufferType bufferType)
{
#define CONVERSION_ENTRY(type) case BufferType::type: return Reflection::BufferType::type;

    switch (bufferType)
    {
        CONVERSION_ENTRY(Buffer)
            CONVERSION_ENTRY(StructuredBuffer)
            CONVERSION_ENTRY(ByteAddressBuffer)
            CONVERSION_ENTRY(RWBuffer)
            CONVERSION_ENTRY(RWStructuredBuffer)
            CONVERSION_ENTRY(RWByteAddressBuffer)
            CONVERSION_ENTRY(AppendStructuredBuffer)
            CONVERSION_ENTRY(ConsumeStructuredBuffer)
            CONVERSION_ENTRY(RWTexture1D)
            CONVERSION_ENTRY(RWTexture1DArray)
            CONVERSION_ENTRY(RWTexture2D)
            CONVERSION_ENTRY(RWTexture2DArray)
            CONVERSION_ENTRY(RWTexture3D)
            CONVERSION_ENTRY(Texture1D)
            CONVERSION_ENTRY(Texture1DArray)
            CONVERSION_ENTRY(Texture2D)
            CONVERSION_ENTRY(Texture2DArray)
            CONVERSION_ENTRY(Texture3D)
            CONVERSION_ENTRY(TextureCube)
            CONVERSION_ENTRY(TextureCubeArray)
            CONVERSION_ENTRY(Texture2DMS)
            CONVERSION_ENTRY(Texture2DMSArray)
    default:
        return Reflection::BufferType::Undefined;
    }

#undef CONVERSION_ENTRY
}

Reflection::VarType DataTypeToVarType(DataType dataType)
{
#define CONVERSION_ENTRY(type) case DataType::type: return Reflection::VarType::type;

    switch (dataType)
    {
        CONVERSION_ENTRY(Bool)
            CONVERSION_ENTRY(Int)
            CONVERSION_ENTRY(UInt)
            CONVERSION_ENTRY(Half)
            CONVERSION_ENTRY(Float)
            CONVERSION_ENTRY(Double)
            CONVERSION_ENTRY(Bool2)
            CONVERSION_ENTRY(Bool3)
            CONVERSION_ENTRY(Bool4)
            CONVERSION_ENTRY(Int2)
            CONVERSION_ENTRY(Int3)
            CONVERSION_ENTRY(Int4)
            CONVERSION_ENTRY(UInt2)
            CONVERSION_ENTRY(UInt3)
            CONVERSION_ENTRY(UInt4)
            CONVERSION_ENTRY(Half2)
            CONVERSION_ENTRY(Half3)
            CONVERSION_ENTRY(Half4)
            CONVERSION_ENTRY(Float2)
            CONVERSION_ENTRY(Float3)
            CONVERSION_ENTRY(Float4)
            CONVERSION_ENTRY(Double2)
            CONVERSION_ENTRY(Double3)
            CONVERSION_ENTRY(Double4)
            CONVERSION_ENTRY(Bool2x2)
            CONVERSION_ENTRY(Bool2x3)
            CONVERSION_ENTRY(Bool2x4)
            CONVERSION_ENTRY(Bool3x2)
            CONVERSION_ENTRY(Bool3x3)
            CONVERSION_ENTRY(Bool3x4)
            CONVERSION_ENTRY(Bool4x2)
            CONVERSION_ENTRY(Bool4x3)
            CONVERSION_ENTRY(Bool4x4)
            CONVERSION_ENTRY(Int2x2)
            CONVERSION_ENTRY(Int2x3)
            CONVERSION_ENTRY(Int2x4)
            CONVERSION_ENTRY(Int3x2)
            CONVERSION_ENTRY(Int3x3)
            CONVERSION_ENTRY(Int3x4)
            CONVERSION_ENTRY(Int4x2)
            CONVERSION_ENTRY(Int4x3)
            CONVERSION_ENTRY(Int4x4)
            CONVERSION_ENTRY(UInt2x2)
            CONVERSION_ENTRY(UInt2x3)
            CONVERSION_ENTRY(UInt2x4)
            CONVERSION_ENTRY(UInt3x2)
            CONVERSION_ENTRY(UInt3x3)
            CONVERSION_ENTRY(UInt3x4)
            CONVERSION_ENTRY(UInt4x2)
            CONVERSION_ENTRY(UInt4x3)
            CONVERSION_ENTRY(UInt4x4)
            CONVERSION_ENTRY(Half2x2)
            CONVERSION_ENTRY(Half2x3)
            CONVERSION_ENTRY(Half2x4)
            CONVERSION_ENTRY(Half3x2)
            CONVERSION_ENTRY(Half3x3)
            CONVERSION_ENTRY(Half3x4)
            CONVERSION_ENTRY(Half4x2)
            CONVERSION_ENTRY(Half4x3)
            CONVERSION_ENTRY(Half4x4)
            CONVERSION_ENTRY(Float2x2)
            CONVERSION_ENTRY(Float2x3)
            CONVERSION_ENTRY(Float2x4)
            CONVERSION_ENTRY(Float3x2)
            CONVERSION_ENTRY(Float3x3)
            CONVERSION_ENTRY(Float3x4)
            CONVERSION_ENTRY(Float4x2)
            CONVERSION_ENTRY(Float4x3)
            CONVERSION_ENTRY(Float4x4)
            CONVERSION_ENTRY(Double2x2)
            CONVERSION_ENTRY(Double2x3)
            CONVERSION_ENTRY(Double2x4)
            CONVERSION_ENTRY(Double3x2)
            CONVERSION_ENTRY(Double3x3)
            CONVERSION_ENTRY(Double3x4)
            CONVERSION_ENTRY(Double4x2)
            CONVERSION_ENTRY(Double4x3)
            CONVERSION_ENTRY(Double4x4)
    default:
        return Reflection::VarType::Undefined;
    }

#undef CONVERSION_ENTRY
}

IMPLEMENT_VISIT_PROC(SamplerDecl)
{
    /* Reflect sampler state */
    Reflection::SamplerState samplerState;
    {
        for (auto& value : ast->samplerValues)
            ReflectSamplerValue(value.get(), samplerState);
    }
    data_->samplerStates[ast->ident] = samplerState;

    //ref uniform
    Reflection::Uniform uniform;
    uniform.ident = ast->ident;
    uniform.type = Reflection::UniformType::Sampler;
    uniform.baseType = 0;
    //push
    data_->uniforms.push_back(uniform);
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    if (ast->flags(FunctionDecl::isEntryPoint))
        ReflectAttributes(ast->declStmntRef->attribs);

    // add function to reflaction structure
    Reflection::Function function;
    function.ident = ast->ident;

    if (ast->returnType)
    {
        if (auto baseTypeDenoter = ast->returnType->typeDenoter->As<BaseTypeDenoter>())
            function.returnValue = DataTypeToVarType(baseTypeDenoter->dataType);
        else
            function.returnValue = Reflection::VarType::Undefined;
    }
    else
        function.returnValue = Reflection::VarType::Void;

    for (auto& entry : ast->parameters)
    {
        if (entry->varDecls.size() == 0)
            continue;

        VarDeclPtr varDecl = entry->varDecls[0];

        Reflection::Parameter param;
        param.ident = varDecl->ident;

        if (entry->typeSpecifier && entry->typeSpecifier->typeDenoter)
        {
            if (auto baseTypeDenoter = entry->typeSpecifier->typeDenoter->As<BaseTypeDenoter>())
                param.type = DataTypeToVarType(baseTypeDenoter->dataType);
            else
                param.type = Reflection::VarType::Undefined;

            param.flags = entry->typeSpecifier->IsInput() ? Reflection::Parameter::Flags::In : 0;
            param.flags |= entry->typeSpecifier->IsOutput() ? Reflection::Parameter::Flags::Out : 0;
        }
        else
            param.type = Reflection::VarType::Undefined;

        function.parameters.push_back(param);
    }

    data_->functions.push_back(function);

    //visit
    Visitor::VisitFunctionDecl(ast, args);
}

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    if (ast->flags(AST::isReachable))
    {
        /* Reflect constant buffer binding */
        data_->constantBuffers.push_back({ ast->ident, GetBindingPoint(ast->slotRegisters) });
        
        // uniform block
        Reflection::Uniform uniform;
        uniform.ident = ast->ident;
        uniform.type = Reflection::UniformType::UniformBuffer;
        uniform.baseType = 0;
        
        data_->uniforms.push_back(uniform);

        //for all Members
        for (auto& stmt : ast->varMembers)
        {
            Reflection::UniformType type;
            DataType baseType = DataType::Undefined;

            BaseTypeDenoter* baseTypeDenoter = nullptr;
            if (stmt->typeSpecifier->typeDenoter->As<StructTypeDenoter>())
                type = Reflection::UniformType::Struct;
            else
            {
                type = Reflection::UniformType::Variable;

                if (baseTypeDenoter = stmt->typeSpecifier->typeDenoter->As<BaseTypeDenoter>())
                    baseType = baseTypeDenoter->dataType;
            }

            int blockIdx = (int)data_->constantBuffers.size() - 1;

            for (auto& decl : stmt->varDecls)
            {
                Reflection::Uniform uniform;
                uniform.ident = decl->ident;
                uniform.type = type;
                uniform.baseType = (int)DataTypeToReflType(baseType);
                uniform.uniformBlock = blockIdx;
                
                data_->uniforms.push_back(uniform);
            }
        }
    }
}

IMPLEMENT_VISIT_PROC(BufferDeclStmnt)
{
    if (ast->flags(AST::isReachable))
    {
        for (auto& bufferDecl : ast->bufferDecls)
        {
            if (bufferDecl->flags(AST::isReachable))
            {
                /* Reflect texture or storage-buffer binding */
                Reflection::BindingSlot bindingSlot;
                {
                    bindingSlot.ident       = bufferDecl->ident;
                    bindingSlot.location    = GetBindingPoint(bufferDecl->slotRegisters);
                };

                if (!IsStorageBufferType(ast->typeDenoter->bufferType))
                    data_->textures.push_back(bindingSlot);
                else
                    data_->storageBuffers.push_back(bindingSlot);

                //ref uniform
                Reflection::Uniform uniform;
                uniform.ident = bufferDecl->ident;
                uniform.type = Reflection::UniformType::Buffer;
                uniform.baseType = (int)BufferTypeToReflType(ast->typeDenoter->bufferType);

                //push
                data_->uniforms.push_back(uniform);
            }
        }
    }
}

IMPLEMENT_VISIT_PROC(VarDecl)
{    
    if (ast->flags(AST::isReachable))
    {
        if (auto typeSpecifier = ast->FetchTypeSpecifier())
        {
            if (typeSpecifier->isUniform)
            {
                DataType baseType = DataType::Undefined;
                /* Add variable as uniform */
                Reflection::Uniform uniform;
                uniform.ident = ast->ident;
                uniform.type = Reflection::UniformType::Variable;
                //find type
                if (auto basetype_denoter = typeSpecifier->typeDenoter->As<BaseTypeDenoter>())
                    uniform.baseType = (int)DataTypeToReflType(basetype_denoter->dataType);
                else
                    uniform.baseType = (int)Reflection::DataType::Undefined;
                //push
                data_->uniforms.push_back(uniform);
            }
        }
    }
}

#undef IMPLEMENT_VISIT_PROC

/* --- Helper functions for code reflection --- */

void ReflectionAnalyzer::ReflectSamplerValue(SamplerValue* ast, Reflection::SamplerState& samplerState)
{
    const auto& name = ast->name;

    /* Assign value to sampler state */
    if (auto literalExpr = ast->value->As<LiteralExpr>())
    {
        const auto& value = literalExpr->value;

        if (name == "MipLODBias")
            samplerState.mipLODBias = FromStringOrDefault<float>(value);
        else if (name == "MaxAnisotropy")
            samplerState.maxAnisotropy = static_cast<unsigned int>(FromStringOrDefault<unsigned long>(value));
        else if (name == "MinLOD")
            samplerState.minLOD = FromStringOrDefault<float>(value);
        else if (name == "MaxLOD")
            samplerState.maxLOD = FromStringOrDefault<float>(value);
    }
    else if (auto objectExpr = ast->value->As<ObjectExpr>())
    {
        const auto& value = objectExpr->ident;

        if (name == "Filter")
            ReflectSamplerValueFilter(value, samplerState.filter, ast);
        else if (name == "AddressU")
            ReflectSamplerValueTextureAddressMode(value, samplerState.addressU, ast);
        else if (name == "AddressV")
            ReflectSamplerValueTextureAddressMode(value, samplerState.addressV, ast);
        else if (name == "AddressW")
            ReflectSamplerValueTextureAddressMode(value, samplerState.addressW, ast);
        else if (name == "ComparisonFunc")
            ReflectSamplerValueComparisonFunc(value, samplerState.comparisonFunc, ast);
    }
    else if (name == "BorderColor")
    {
        try
        {
            if (auto callExpr = ast->value->As<CallExpr>())
            {
                if (callExpr->typeDenoter && callExpr->typeDenoter->IsVector() && callExpr->arguments.size() == 4)
                {
                    /* Evaluate sub expressions to constant floats */
                    for (std::size_t i = 0; i < 4; ++i)
                        samplerState.borderColor[i] = EvaluateConstExprFloat(*callExpr->arguments[i]);
                }
                else
                    throw std::string(R_InvalidTypeOrArgCount);
            }
            else if (auto castExpr = ast->value->As<CastExpr>())
            {
                /* Evaluate sub expression to constant float and copy into four sub values */
                auto subValueSrc = EvaluateConstExprFloat(*castExpr->expr);
                for (std::size_t i = 0; i < 4; ++i)
                    samplerState.borderColor[i] = subValueSrc;
            }
            else if (auto initExpr = ast->value->As<InitializerExpr>())
            {
                if (initExpr->exprs.size() == 4)
                {
                    /* Evaluate sub expressions to constant floats */
                    for (std::size_t i = 0; i < 4; ++i)
                        samplerState.borderColor[i] = EvaluateConstExprFloat(*initExpr->exprs[i]);
                }
                else
                    throw std::string(R_InvalidArgCount);
            }
        }
        catch (const std::string& s)
        {
            Warning(R_FailedToInitializeSamplerValue(s, "BorderColor"), ast->value.get());
        }
    }
}

void ReflectionAnalyzer::ReflectSamplerValueFilter(const std::string& value, Reflection::Filter& filter, const AST* ast)
{
    try
    {
        filter = StringToFilter(value);
    }
    catch (const std::invalid_argument& e)
    {
        Warning(e.what(), ast);
    }
}

void ReflectionAnalyzer::ReflectSamplerValueTextureAddressMode(const std::string& value, Reflection::TextureAddressMode& addressMode, const AST* ast)
{
    try
    {
        addressMode = StringToTexAddressMode(value);
    }
    catch (const std::invalid_argument& e)
    {
        Warning(e.what(), ast);
    }
}

void ReflectionAnalyzer::ReflectSamplerValueComparisonFunc(const std::string& value, Reflection::ComparisonFunc& comparisonFunc, const AST* ast)
{
    try
    {
        comparisonFunc = StringToCompareFunc(value);
    }
    catch (const std::invalid_argument& e)
    {
        Warning(e.what(), ast);
    }
}

void ReflectionAnalyzer::ReflectAttributes(const std::vector<AttributePtr>& attribs)
{
    for (const auto& attr : attribs)
    {
        switch (attr->attributeType)
        {
            case AttributeType::NumThreads:
                ReflectAttributesNumThreads(attr.get());
                break;
            default:
                break;
        }
    }
}

void ReflectionAnalyzer::ReflectAttributesNumThreads(Attribute* ast)
{
    /* Reflect "numthreads" attribute for compute shader */
    if (shaderTarget_ == ShaderTarget::ComputeShader && ast->arguments.size() == 3)
    {
        /* Evaluate attribute arguments */
        data_->numThreads.x = EvaluateConstExprInt(*ast->arguments[0]);
        data_->numThreads.y = EvaluateConstExprInt(*ast->arguments[1]);
        data_->numThreads.z = EvaluateConstExprInt(*ast->arguments[2]);
    }
}


} // /namespace Xsc



// ================================================================================
