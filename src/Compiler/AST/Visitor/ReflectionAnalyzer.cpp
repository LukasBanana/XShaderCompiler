/*
 * ReflectionAnalyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
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
    Visit(ast->globalStmts);
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
    if (ast->samplerValues.empty())
    {
        /* Reflect sampler state */
        Reflection::SamplerState samplerState;
        {
            samplerState.referenced = ast->flags(AST::isReachable);
            samplerState.type       = SamplerTypeToResourceType(ast->GetSamplerType());
            samplerState.name       = ast->ident;
            samplerState.slot       = GetBindingPoint(ast->slotRegisters);
        }
        data_->samplerStates.push_back(samplerState);
    }
    else
    {
        /* Reflect static sampler state */
        Reflection::StaticSamplerState samplerState;
        {
            samplerState.type = SamplerTypeToResourceType(ast->GetSamplerType());
            samplerState.name = ast->ident;
            for (auto& value : ast->samplerValues)
                ReflectSamplerValue(value.get(), samplerState.desc);
        }
        data_->staticSamplerStates.push_back(samplerState);
    }
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    Visitor::VisitStructDecl(ast, args);

    /* Reflect record type */
    const auto recordIndex = data_->records.size();

    Reflection::Record record;
    {
        /* Reflect name and base record index */
        record.referenced       = ast->flags(AST::isReachable);
        record.name             = ast->ident;
        record.baseRecordIndex  = FindRecordIndex(ast->baseStructRef);

        /* Reflect record fields */
        record.size     = 0;
        record.padding  = 0;

        for (const auto& member : ast->varMembers)
        {
            for (const auto& var : member->varDecls)
            {
                Reflection::Field field;
                ReflectField(var.get(), field, record.size, record.padding);
                record.fields.push_back(field);
            }
        }
    }
    data_->records.push_back(record);

    /* Store record in output data and in hash-map associated with the structure declaration object */
    recordIndicesMap_[ast] = recordIndex;
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    if (ast->flags(FunctionDecl::isEntryPoint))
        ReflectAttributes(ast->declStmtRef->attribs);

    Visitor::VisitFunctionDecl(ast, args);

    /* Reflect function declaration */
    Reflection::Function func;
    {
        func.name       = ast->ident;
        func.references = ast->numCalls;
    }
    data_->functions.push_back(func);
}

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    /* Reflect constant buffer binding */
    Reflection::ConstantBuffer constantBuffer;
    {
        /* Reflect type, name, and slot index */
        constantBuffer.referenced   = ast->flags(AST::isReachable);
        constantBuffer.type         = UniformBufferTypeToResourceType(ast->bufferType);
        constantBuffer.name         = ast->ident;
        constantBuffer.slot         = GetBindingPoint(ast->slotRegisters);

        /* Reflect constant buffer fields and size */
        constantBuffer.size     = 0;
        constantBuffer.padding  = 0;

        for (const auto& member : ast->varMembers)
        {
            for (const auto& var : member->varDecls)
            {
                Reflection::Field field;
                ReflectField(var.get(), field, constantBuffer.size, constantBuffer.padding);
                constantBuffer.fields.push_back(field);
            }
        }
    }
    data_->constantBuffers.push_back(constantBuffer);
}

IMPLEMENT_VISIT_PROC(BufferDeclStmt)
{
    for (auto& bufferDecl : ast->bufferDecls)
    {
        /* Reflect texture or storage-buffer binding */
        Reflection::Resource resource;
        {
            resource.referenced = bufferDecl->flags(AST::isReachable);
            resource.type       = BufferTypeToResourceType(ast->typeDenoter->bufferType);
            resource.name       = bufferDecl->ident;
            resource.slot       = GetBindingPoint(bufferDecl->slotRegisters);
        };
        data_->resources.push_back(resource);
    }
}

IMPLEMENT_VISIT_PROC(VarDecl)
{
    if (auto typeSpecifier = ast->FetchTypeSpecifier())
    {
        if (typeSpecifier->isUniform)
        {
            /* Add variable as uniform */
            Reflection::Attribute attribute;
            {
                attribute.referenced    = ast->flags(AST::isReachable);
                attribute.name          = ast->ident;
                attribute.slot          = GetBindingPoint(ast->slotRegisters);
            }
            data_->uniforms.push_back(attribute);
        }
    }
}

#undef IMPLEMENT_VISIT_PROC

/* --- Helper functions for code reflection --- */

void ReflectionAnalyzer::ReflectSamplerValue(SamplerValue* ast, Reflection::SamplerStateDesc& desc)
{
    const auto& name = ast->name;

    /* Assign value to sampler state */
    if (auto literalExpr = ast->value->As<LiteralExpr>())
    {
        const auto& value = literalExpr->value;

        if (name == "MipLODBias")
            desc.mipLODBias = FromStringOrDefault<float>(value);
        else if (name == "MaxAnisotropy")
            desc.maxAnisotropy = static_cast<unsigned int>(FromStringOrDefault<unsigned long>(value));
        else if (name == "MinLOD")
            desc.minLOD = FromStringOrDefault<float>(value);
        else if (name == "MaxLOD")
            desc.maxLOD = FromStringOrDefault<float>(value);
    }
    else if (auto identExpr = ast->value->As<IdentExpr>())
    {
        const auto& value = identExpr->ident;

        if (name == "Filter")
            ReflectSamplerValueFilter(value, desc.filter, ast);
        else if (name == "AddressU")
            ReflectSamplerValueTextureAddressMode(value, desc.addressU, ast);
        else if (name == "AddressV")
            ReflectSamplerValueTextureAddressMode(value, desc.addressV, ast);
        else if (name == "AddressW")
            ReflectSamplerValueTextureAddressMode(value, desc.addressW, ast);
        else if (name == "ComparisonFunc")
            ReflectSamplerValueComparisonFunc(value, desc.comparisonFunc, ast);
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
                        desc.borderColor[i] = EvaluateConstExprFloat(*callExpr->arguments[i]);
                }
                else
                    throw std::string(R_InvalidTypeOrArgCount);
            }
            else if (auto castExpr = ast->value->As<CastExpr>())
            {
                /* Evaluate sub expression to constant float and copy into four sub values */
                auto subValueSrc = EvaluateConstExprFloat(*castExpr->expr);
                for (std::size_t i = 0; i < 4; ++i)
                    desc.borderColor[i] = subValueSrc;
            }
            else if (auto initExpr = ast->value->As<InitializerExpr>())
            {
                if (initExpr->exprs.size() == 4)
                {
                    /* Evaluate sub expressions to constant floats */
                    for (std::size_t i = 0; i < 4; ++i)
                        desc.borderColor[i] = EvaluateConstExprFloat(*initExpr->exprs[i]);
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

static Reflection::FieldType ToFieldType(const DataType t)
{
    using T = Reflection::FieldType;
    switch (BaseDataType(t))
    {
        case DataType::Bool:    return T::Bool;
        case DataType::Int:     return T::Int;
        case DataType::UInt:    return T::UInt;
        case DataType::Half:    return T::Half;
        case DataType::Float:   return T::Float;
        case DataType::Double:  return T::Double;
        default:                return T::Undefined;
    }
}

static void ReflectFieldBaseType(const DataType dataType, Reflection::Field& field)
{
    /* Determine base type */
    field.type = ToFieldType(dataType);

    /* Determine matrix dimensions */
    auto typeDim = MatrixTypeDim(dataType);
    field.dimensions[0] = typeDim.first;
    field.dimensions[1] = typeDim.second;
}

void ReflectionAnalyzer::ReflectField(VarDecl* ast, Reflection::Field& field, unsigned int& accumSize, unsigned int& accumPadding)
{
    /* Reflect name and reachability */
    field.referenced    = ast->flags(AST::isReachable);
    field.name          = ast->ident;

    /* Reflect field type */
    ReflectFieldType(field, ast->GetTypeDenoter()->GetAliased());

    /* Determine size and byte offset */
    const auto currentSize      = accumSize;
    const auto currentPadding   = accumPadding;

    if (ast->AccumAlignedVectorSize(accumSize, accumPadding, &(field.offset)))
    {
        const auto localPadding = (accumPadding - currentPadding);
        field.size = (accumSize - currentSize - localPadding);
    }
    else
        field.size = ~0;
}

void ReflectionAnalyzer::ReflectFieldType(Reflection::Field& field, const TypeDenoter& typeDen)
{
    if (auto baseTypeDen = typeDen.As<BaseTypeDenoter>())
    {
        /* Determine base data type and dimensions */
        ReflectFieldBaseType(baseTypeDen->dataType, field);
    }
    /* Reflect structure type */
    else if (auto structTypeDen = typeDen.As<StructTypeDenoter>())
    {
        /* Determine record type index */
        field.type              = Reflection::FieldType::Record;
        field.dimensions[0]     = 0;
        field.dimensions[1]     = 0;
        field.typeRecordIndex   = FindRecordIndex(structTypeDen->structDeclRef);
    }
    /* Reflect array type */
    else if (auto arrayTypeDen = typeDen.As<ArrayTypeDenoter>())
    {
        /* Determine base field type */
        ReflectFieldType(field, arrayTypeDen->subTypeDenoter->GetAliased());

        /* Determine array dimensions */
        auto dimSizes = arrayTypeDen->GetDimensionSizes();
        field.arrayElements.reserve(dimSizes.size());

        for (auto size : dimSizes)
        {
            if (size >= 0)
                field.arrayElements.push_back(static_cast<unsigned int>(size));
            else
                field.arrayElements.push_back(0);
        }
    }
}

int ReflectionAnalyzer::FindRecordIndex(const StructDecl* structDecl) const
{
    auto it = recordIndicesMap_.find(structDecl);
    if (it != recordIndicesMap_.end())
        return static_cast<int>(it->second);
    else
        return -1;
}


} // /namespace Xsc



// ================================================================================
