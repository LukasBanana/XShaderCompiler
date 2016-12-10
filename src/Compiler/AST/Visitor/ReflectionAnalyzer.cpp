/*
 * ReflectionAnalyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ReflectionAnalyzer.h"
#include "ConstExprEvaluator.h"
#include "AST.h"
#include "Helper.h"


namespace Xsc
{


ReflectionAnalyzer::ReflectionAnalyzer(Log* log) :
    reportHandler_{ "reflection", log }
{
}

void ReflectionAnalyzer::Reflect(Program& program, const ShaderTarget shaderTarget, Reflection::ReflectionData& reflectionData)
{
    shaderTarget_   = shaderTarget;
    program_        = (&program);
    data_           = (&reflectionData);

    Visit(program_);
}


/*
 * ======= Private: =======
 */

void ReflectionAnalyzer::Warning(const std::string& msg, const AST* ast)
{
    reportHandler_.Warning(false, msg, program_->sourceCode.get(), (ast ? ast->area : SourceArea::ignore));
}

int ReflectionAnalyzer::GetBindingPoint(const std::vector<RegisterPtr>& slotRegisters) const
{
    if (auto slotRegister = Register::GetForTarget(slotRegisters, shaderTarget_))
        return slotRegister->slot;
    else
        return -1;
}

Variant ReflectionAnalyzer::EvaluateConstExpr(Expr& expr)
{
    try
    {
        /* Evaluate expression and throw error on var-access */
        ConstExprEvaluator exprEvaluator;
        return exprEvaluator.EvaluateExpr(expr, [](VarAccessExpr* ast) -> Variant { throw ast; });
    }
    catch (const std::exception&)
    {
        return Variant();
    }
    catch (const VarAccessExpr*)
    {
        return Variant();
    }
    return Variant();
}

float ReflectionAnalyzer::EvaluateConstExprFloat(Expr& expr)
{
    return static_cast<float>(EvaluateConstExpr(expr).ToReal());
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

IMPLEMENT_VISIT_PROC(SamplerDecl)
{
    /* Reflect sampler state */
    Reflection::SamplerState samplerState;
    {
        for (auto& value : ast->samplerValues)
            ReflectSamplerValue(value.get(), samplerState);
    }
    data_->samplerStates[ast->ident] = samplerState;
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    if (ast->flags(AST::isReachable))
    {
        /* Reflect constant buffer binding */
        data_->constantBuffers.push_back({ ast->ident, GetBindingPoint(ast->slotRegisters) });
    }
}

IMPLEMENT_VISIT_PROC(TextureDeclStmnt)
{
    if (ast->flags(AST::isReachable))
    {
        for (auto& texDecl : ast->textureDecls)
        {
            if (texDecl->flags(AST::isReachable))
            {
                /* Reflect texture binding */
                data_->textures.push_back({ texDecl->ident, GetBindingPoint(texDecl->slotRegisters) });
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
            samplerState.mipLODBias = FromString<float>(value);
        else if (name == "MaxAnisotropy")
            samplerState.maxAnisotropy = FromString<unsigned int>(value);
        else if (name == "MinLOD")
            samplerState.minLOD = FromString<float>(value);
        else if (name == "MaxLOD")
            samplerState.maxLOD = FromString<float>(value);
    }
    else if (auto varAccessExpr = ast->value->As<VarAccessExpr>())
    {
        const auto& value = varAccessExpr->varIdent->ident;

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
            if (auto funcCallExpr = ast->value->As<FunctionCallExpr>())
            {
                if (funcCallExpr->call->typeDenoter && funcCallExpr->call->typeDenoter->IsVector() && funcCallExpr->call->arguments.size() == 4)
                {
                    /* Evaluate sub expressions to constant floats */
                    for (std::size_t i = 0; i < 4; ++i)
                        samplerState.borderColor[i] = EvaluateConstExprFloat(*funcCallExpr->call->arguments[i]);
                }
                else
                    throw std::string("invalid type or invalid number of arguments");
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
                    throw std::string("invalid number of arguments");
            }
        }
        catch (const std::string& s)
        {
            Warning(s + " to initialize sampler value 'BorderColor'", ast->value.get());
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


} // /namespace Xsc



// ================================================================================