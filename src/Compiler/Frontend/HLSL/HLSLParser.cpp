/*
 * HLSLParser.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HLSLParser.h"
#include "HLSLKeywords.h"
#include "ConstExprEvaluator.h"
#include "Helper.h"
#include "AST.h"
#include "ASTFactory.h"


namespace Xsc
{


/*
The HLSL parser is not a fully context free parser,
because cast expressions in HLSL are not context free.
Take a look at the following example:

    int X = 0;
    (X) - (1);

Here "(X) - (1)" is a binary expression, but in the following example it is a cast expression:

    typedef int X;
    (X) - (1);

Here "-(1)" is an unary expression. Thus, cast expression can only be parsed, if the parser is aware of all
types, which are valid in the respective scope.
*/

HLSLParser::HLSLParser(Log* log) :
    Parser{ log }
{
}

ProgramPtr HLSLParser::ParseSource(const SourceCodePtr& source)
{
    PushScannerSource(source);

    try
    {
        auto ast = ParseProgram(source);
        return (GetReportHandler().HasErros() ? nullptr : ast);
    }
    catch (const Report& err)
    {
        if (GetLog())
            GetLog()->SumitReport(err);
    }

    return nullptr;
}


/*
 * ======= Private: =======
 */

ScannerPtr HLSLParser::MakeScanner()
{
    return std::make_shared<HLSLScanner>(GetLog());
}

void HLSLParser::Semi()
{
    Accept(Tokens::Semicolon);
}

bool HLSLParser::IsDataType() const
{
    return
    (
        IsBaseDataType() || Is(Tokens::Vector) || Is(Tokens::Matrix) ||
        Is(Tokens::Buffer) || Is(Tokens::Sampler) || Is(Tokens::SamplerState)
    );
}

bool HLSLParser::IsBaseDataType() const
{
    return (Is(Tokens::ScalarType) || Is(Tokens::VectorType) || Is(Tokens::MatrixType) || Is(Tokens::StringType));
}

bool HLSLParser::IsLiteral() const
{
    return (Is(Tokens::BoolLiteral) || Is(Tokens::IntLiteral) || Is(Tokens::FloatLiteral) || Is(Tokens::StringLiteral));
}

bool HLSLParser::IsArithmeticUnaryExpr() const
{
    return (Is(Tokens::BinaryOp, "-") || Is(Tokens::BinaryOp, "+"));
}

bool HLSLParser::IsVarDeclModifier() const
{
    return (Is(Tokens::InputModifier) || Is(Tokens::InterpModifier) || Is(Tokens::TypeModifier) || Is(Tokens::StorageClass));
}

TypeNameExprPtr HLSLParser::MakeToTypeNameIfLhsOfCastExpr(const ExprPtr& expr)
{
    /* Type name expression (float, int3 etc.) is always allowed for a cast expression */
    if (expr->Type() == AST::Types::TypeNameExpr)
        return std::dynamic_pointer_cast<TypeNameExpr>(expr);

    /* Is this a variable identifier? */
    if (auto varAccessExpr = expr->As<VarAccessExpr>())
    {
        /* Check if the identifier refers to a type name */
        if (!varAccessExpr->varIdent->next && IsRegisteredTypeName(varAccessExpr->varIdent->ident))
        {
            /* Convert the variable access into a type name expression */
            auto typeExpr = Make<TypeNameExpr>();
            typeExpr->typeDenoter = std::make_shared<AliasTypeDenoter>(varAccessExpr->varIdent->ident);
            return typeExpr;
        }
    }

    /* No type name expression */
    return nullptr;
}

TokenPtr HLSLParser::AcceptIt()
{
    auto tkn = Parser::AcceptIt();

    /* Post-process directives */
    while (Tkn()->Type() == Tokens::Directive)
        ProcessDirective(AcceptIt()->Spell());

    return tkn;
}

void HLSLParser::ProcessDirective(const std::string& ident)
{
    if (ident == "line")
    {
        int lineNo = 0;
        std::string filename;

        /* Parse '#line'-directive with base class "AcceptIt" functions to avoid recursive calls of this function */
        if (Is(Tokens::IntLiteral))
            lineNo = FromString<int>(Parser::AcceptIt()->Spell());
        else
            ErrorUnexpected(Tokens::IntLiteral);

        if (Is(Tokens::StringLiteral))
            filename = Parser::AcceptIt()->SpellContent();
        else
            ErrorUnexpected(Tokens::StringLiteral);

        /* Set new line number and filename */
        auto currentLine = static_cast<int>(GetScanner().PreviousToken()->Pos().Row());
        GetScanner().Source()->NextSourceOrigin(filename, (lineNo - currentLine - 1));
    }
    else
        Error("only '#line'-directives are allowed after pre-processing");
}

/* ------- Symbol table ------- */

void HLSLParser::OpenScope()
{
    typeNameSymbolTable_.OpenScope();
}

void HLSLParser::CloseScope()
{
    typeNameSymbolTable_.CloseScope();
}

void HLSLParser::RegisterTypeName(const std::string& ident)
{
    typeNameSymbolTable_.Register(ident, true, nullptr, false);
}

bool HLSLParser::IsRegisteredTypeName(const std::string& ident) const
{
    return typeNameSymbolTable_.Fetch(ident);
}

AliasDeclStmntPtr HLSLParser::MakeAndRegisterAliasDeclStmnt(const DataType dataType, const std::string& ident)
{
    auto ast = ASTFactory::MakeBaseTypeAlias(dataType, ident);
    RegisterTypeName(ident);
    return ast;
}

void HLSLParser::GeneratePreDefinedTypeAliases(Program& ast)
{
    static const std::vector<std::pair<DataType, std::string>> preDefinedTypes
    {
        { DataType::Int,          "DWORD"        },
        { DataType::Float,        "FLOAT"        },
        { DataType::Float4,       "VECTOR"       },
        { DataType::Float4x4,     "MATRIX"       },
        { DataType::String,       "STRING"       },
      //{ DataType::Texture,      "TEXTURE"      },
      //{ DataType::PixelShader,  "PIXELSHADER"  },
      //{ DataType::VertexShader, "VERTEXSHADER" },
    };

    for (const auto& type : preDefinedTypes)
    {
        ast.globalStmnts.push_back(
            MakeAndRegisterAliasDeclStmnt(type.first, type.second)
        );
    }
}

/* ------- Parse functions ------- */

ProgramPtr HLSLParser::ParseProgram(const SourceCodePtr& source)
{
    auto ast = Make<Program>();

    OpenScope();

    /* Generate pre-defined typedef-statements */
    GeneratePreDefinedTypeAliases(*ast);

    /* Keep reference to preprocessed source code */
    ast->sourceCode = source;

    while (true)
    {
        /* Ignore all null statements and techniques */
        while (Is(Tokens::Semicolon) || Is(Tokens::Technique))
        {
            if (Is(Tokens::Technique))
                ParseAndIgnoreTechnique();
            else
                AcceptIt();
        }

        /* Check if end of stream has been reached */
        if (Is(Tokens::EndOfStream))
            break;

        /* Parse next global declaration */
        ParseStmntWithOptionalComment(ast->globalStmnts, std::bind(&HLSLParser::ParseGlobalStmnt, this));
    }

    CloseScope();

    return ast;
}

CodeBlockPtr HLSLParser::ParseCodeBlock()
{
    auto ast = Make<CodeBlock>();

    /* Parse statement list */
    Accept(Tokens::LCurly);
    OpenScope();
    {
        ast->stmnts = ParseStmntList();
    }
    CloseScope();
    Accept(Tokens::RCurly);

    return ast;
}

FunctionCallPtr HLSLParser::ParseFunctionCall(VarIdentPtr varIdent)
{
    auto ast = Make<FunctionCall>();

    /* Parse function name (as variable identifier) */
    if (!varIdent)
    {
        if (IsDataType())
        {
            varIdent = Make<VarIdent>();
            varIdent->ident = AcceptIt()->Spell();
        }
        else
            varIdent = ParseVarIdent();
    }

    ast->varIdent = varIdent;

    /* Parse argument list */
    ast->arguments = ParseArgumentList();

    /* Update AST area */
    ast->area = ast->varIdent->area;

    return ast;
}

FunctionCallPtr HLSLParser::ParseFunctionCall(const TypeDenoterPtr& typeDenoter)
{
    auto ast = Make<FunctionCall>();

    /* Take type denoter */
    ast->typeDenoter = typeDenoter;

    /* Parse argument list */
    ast->arguments = ParseArgumentList();

    return UpdateSourceArea(ast);
}

VarDeclStmntPtr HLSLParser::ParseParameter()
{
    auto ast = Make<VarDeclStmnt>();

    /* Parse parameter as single variable declaration */
    while (IsVarDeclModifier() || Is(Tokens::PrimitiveType))
        ParseVarDeclStmntModifiers(ast.get(), true);

    ast->varType = ParseVarType();
    ast->varDecls.push_back(ParseVarDecl(ast.get()));

    return UpdateSourceArea(ast);
}

SwitchCasePtr HLSLParser::ParseSwitchCase()
{
    auto ast = Make<SwitchCase>();

    /* Parse switch case header */
    if (Is(Tokens::Case))
    {
        Accept(Tokens::Case);
        ast->expr = ParseExpr();
    }
    else
        Accept(Tokens::Default);
    Accept(Tokens::Colon);

    /* Parse switch case statement list */
    while (!Is(Tokens::Case) && !Is(Tokens::Default) && !Is(Tokens::RCurly))
        ParseStmntWithOptionalComment(ast->stmnts, std::bind(&HLSLParser::ParseStmnt, this, true));

    return ast;
}

SamplerValuePtr HLSLParser::ParseSamplerValue()
{
    auto ast = Make<SamplerValue>();

    /* Parse state name */
    ast->name = ParseIdent();

    /* Parse value expression */
    Accept(Tokens::AssignOp, "=");
    ast->value = ParseExpr();
    Semi();

    return ast;
}

/* --- Variables --- */

AttributePtr HLSLParser::ParseAttribute()
{
    Accept(Tokens::LParen);

    auto ast = Make<Attribute>();

    auto attribIdent = ParseIdent();
    ast->attributeType = HLSLKeywordToAttributeType(attribIdent);

    UpdateSourceArea(ast);

    if (ast->attributeType == AttributeType::Undefined)
        Warning("unknown attribute '" + attribIdent + "'");

    if (Is(Tokens::LBracket))
    {
        AcceptIt();

        if (!Is(Tokens::RBracket))
        {
            while (true)
            {
                ast->arguments.push_back(ParseExpr());
                if (Is(Tokens::Comma))
                    AcceptIt();
                else
                    break;
            }
        }

        Accept(Tokens::RBracket);
    }

    Accept(Tokens::RParen);

    return ast;
}

// https://msdn.microsoft.com/en-us/library/windows/desktop/bb509709#Profiles
static ShaderTarget HLSLShaderProfileToTarget(const std::string& s)
{
    if (s.size() >= 2)
    {
        auto p = s.substr(0, 2);
        if (p == "vs")
            return ShaderTarget::VertexShader;
        if (p == "hs")
            return ShaderTarget::TessellationControlShader;
        if (p == "ds")
            return ShaderTarget::TessellationEvaluationShader;
        if (p == "gs")
            return ShaderTarget::GeometryShader;
        if (p == "ps")
            return ShaderTarget::FragmentShader;
        if (p == "cs")
            return ShaderTarget::ComputeShader;
    }
    return ShaderTarget::Undefined;
}

// ':' 'register' '(' (IDENT ',')? IDENT ('[' INT_LITERAL ']')? ')'
RegisterPtr HLSLParser::ParseRegister(bool parseColon)
{
    /* Colon is only syntactic sugar, thus not part of the source area */
    if (parseColon)
        Accept(Tokens::Colon);
    
    auto ast = Make<Register>();

    Accept(Tokens::Register);
    Accept(Tokens::LBracket);

    auto typeIdent = ParseIdent();

    /* Pares optional shader profile */
    if (Is(Tokens::Comma))
    {
        ast->shaderTarget = HLSLShaderProfileToTarget(typeIdent);

        //TODO: only report a warning (or rather an error), if all valid profiles are checked correctly
        //if (ast->shaderTarget == ShaderTarget::Undefined)
        //    Warning("unknown shader profile: '" + typeIdent + "'");

        AcceptIt();
        typeIdent = ParseIdent();
    }

    /* Set area offset to register type character */
    ast->area.Offset(GetScanner().PreviousToken()->Pos());

    /* Get register type and slot index from type identifier */
    ast->registerType   = CharToRegisterType(typeIdent.front());
    ast->slot           = FromString<int>(typeIdent.substr(1));

    /* Validate register type and slot index */
    if (ast->registerType == RegisterType::Undefined)
        Warning("unknown slot register: '" + typeIdent.substr(0, 1) + "'");

    /* Parse optional sub component (is only added to slot index) */
    if (Is(Tokens::LParen))
    {
        AcceptIt();
        auto subComponent = Accept(Tokens::IntLiteral)->Spell();
        ast->slot += FromString<int>(subComponent);
        Accept(Tokens::RParen);
    }

    Accept(Tokens::RBracket);

    return UpdateSourceArea(ast);
}

// ':' 'packoffset' '(' IDENT ('.' COMPONENT)? ')'
PackOffsetPtr HLSLParser::ParsePackOffset(bool parseColon)
{
    if (parseColon)
        Accept(Tokens::Colon);
    
    auto ast = Make<PackOffset>();

    Accept(Tokens::PackOffset);
    Accept(Tokens::LBracket);

    ast->registerName = ParseIdent();

    if (Is(Tokens::Dot))
    {
        AcceptIt();
        ast->vectorComponent = ParseIdent();
    }

    Accept(Tokens::RBracket);

    return UpdateSourceArea(ast);
}

ExprPtr HLSLParser::ParseArrayDimension(bool allowDynamicDimension)
{
    ExprPtr ast;

    Accept(Tokens::LParen);
    
    if (Is(Tokens::RParen))
    {
        if (!allowDynamicDimension)
            Error("explicit array dimension expected", false);
        ast = Make<NullExpr>();
    }
    else
        ast = ParseExpr();

    Accept(Tokens::RParen);

    return UpdateSourceArea(ast);
}

ExprPtr HLSLParser::ParseInitializer()
{
    Accept(Tokens::AssignOp, "=");
    return ParseExpr();
}

VarIdentPtr HLSLParser::ParseVarIdent()
{
    auto ast = Make<VarIdent>();

    /* Parse variable single identifier */
    ast->ident          = ParseIdent();
    ast->arrayIndices   = ParseArrayDimensionList();
    
    if (Is(Tokens::Dot))
    {
        /* Parse next variable identifier */
        AcceptIt();
        ast->next = ParseVarIdent();
    }

    return UpdateSourceArea(ast);
}

VarTypePtr HLSLParser::ParseVarType(bool parseVoidType)
{
    auto ast = Make<VarType>();

    /* Parse variable type denoter with optional struct declaration */
    ast->typeDenoter = ParseTypeDenoterWithStructDeclOpt(ast->structDecl);

    return UpdateSourceArea(ast);
}

VarDeclPtr HLSLParser::ParseVarDecl(VarDeclStmnt* declStmntRef, const TokenPtr& identTkn)
{
    auto ast = Make<VarDecl>();

    /* Store reference to parent node */
    ast->declStmntRef   = declStmntRef;

    /* Parse variable declaration */
    ast->ident          = (identTkn ? identTkn->Spell() : ParseIdent());
    ast->arrayDims      = ParseArrayDimensionList(true);

    ParseVarDeclSemantic(*ast);

    ast->annotations    = ParseAnnotationList();

    /* Parse optional initializer expression */
    if (Is(Tokens::AssignOp, "="))
        ast->initializer = ParseInitializer();

    /* Update source area */
    ast->area.Update(ast->ident);

    return ast;
}

BufferDeclPtr HLSLParser::ParseBufferDecl(BufferDeclStmnt* declStmntRef, const TokenPtr& identTkn)
{
    auto ast = Make<BufferDecl>();

    /* Store reference to parent node */
    ast->declStmntRef = declStmntRef;

    /* Parse identifier, optional array dimension list, and optional slot registers */
    ast->ident          = ParseIdent(identTkn);
    ast->arrayDims      = ParseArrayDimensionList();
    ast->slotRegisters  = ParseRegisterList();

    return ast;
}

SamplerDeclPtr HLSLParser::ParseSamplerDecl(SamplerDeclStmnt* declStmntRef, const TokenPtr& identTkn)
{
    auto ast = Make<SamplerDecl>();

    /* Store reference to parent node */
    ast->declStmntRef = declStmntRef;

    /* Parse identifier, optional array dimension list, and optional slot registers */
    ast->ident          = ParseIdent(identTkn);
    ast->arrayDims      = ParseArrayDimensionList();
    ast->slotRegisters  = ParseRegisterList();

    /* Parse optional static sampler state (either for D3D9 or D3D10+ shaders) */
    if (Is(Tokens::AssignOp, "="))
    {
        /* Parse sampler state ("sampler_state" in DX9 only) */
        AcceptIt();
        Accept(Tokens::SamplerState, "sampler_state");
        Accept(Tokens::LCurly);
        
        ast->textureIdent = ParseSamplerStateTextureIdent();
        ast->samplerValues = ParseSamplerValueList();

        Accept(Tokens::RCurly);
    }
    else if (Is(Tokens::LCurly))
    {
        AcceptIt();
        ast->samplerValues = ParseSamplerValueList();
        Accept(Tokens::RCurly);
    }

    return ast;
}

StructDeclPtr HLSLParser::ParseStructDecl(bool parseStructTkn, const TokenPtr& identTkn)
{
    auto tkn = Tkn();
    auto ast = Make<StructDecl>();

    /* Parse structure declaration */
    if (parseStructTkn)
    {
        Accept(Tokens::Struct);
        UpdateSourceArea(ast);
    }

    if (Is(Tokens::Ident) || identTkn)
    {
        /* Parse structure name */
        tkn = Tkn();
        ast->ident = (identTkn ? identTkn->Spell() : ParseIdent());
        UpdateSourceArea(ast);

        /* Register type name in symbol table */
        RegisterTypeName(ast->ident);

        /* Parse optional inheritance (not documented in HLSL but supported; only single inheritance) */
        if (Is(Tokens::Colon))
        {
            AcceptIt();

            ast->baseStructName = ParseIdent();
            if (ast->baseStructName == ast->ident)
                Error("recursive inheritance is not allowed");

            if (Is(Tokens::Comma))
                Error("multiple inheritance is not allowed", false);
        }
    }

    GetReportHandler().PushContextDesc(ast->SignatureToString());
    {
        /* Parse member variable declarations */
        auto members = ParseVarDeclStmntList();
        ast->members.insert(ast->members.end(), members.begin(), members.end());
    }
    GetReportHandler().PopContextDesc();

    return ast;
}

AliasDeclPtr HLSLParser::ParseAliasDecl(TypeDenoterPtr typeDenoter)
{
    auto ast = Make<AliasDecl>();

    /* Parse alias identifier */
    ast->ident = ParseIdent();

    /* Register type name in symbol table */
    RegisterTypeName(ast->ident);

    /* Parse optional array dimensions */
    if (Is(Tokens::LParen))
    {
        /* Make array type denoter and use input as base type denoter */
        auto arrayTypeDenoter = std::make_shared<ArrayTypeDenoter>();
        {
            arrayTypeDenoter->arrayDims         = ParseArrayDimensionList();
            arrayTypeDenoter->baseTypeDenoter   = typeDenoter;
        }
        typeDenoter = arrayTypeDenoter;
    }

    /* Store final type denoter in alias declaration */
    ast->typeDenoter = typeDenoter;

    return UpdateSourceArea(ast);
}

/* --- Declaration statements --- */

StmntPtr HLSLParser::ParseGlobalStmnt()
{
    switch (TknType())
    {
        case Tokens::Sampler:
        case Tokens::SamplerState:
            return ParseGlobalStmntWithSamplerTypeDenoter();
        case Tokens::Buffer:
            return ParseGlobalStmntWithBufferTypeDenoter();
        case Tokens::UniformBuffer:
            return ParseUniformBufferDecl();
        case Tokens::Typedef:
            return ParseAliasDeclStmnt();
        case Tokens::InputModifier:
        case Tokens::InterpModifier:
        case Tokens::TypeModifier:
        case Tokens::StorageClass:
            return ParseVarDeclStmnt();
        case Tokens::LParen:
        case Tokens::Void:
        case Tokens::Inline:
            return ParseFunctionDecl();
        default:
            return ParseGlobalStmntWithVarType();
    }
}

StmntPtr HLSLParser::ParseGlobalStmntWithVarType()
{
    auto varType = ParseVarType();

    if (varType->structDecl && Is(Tokens::Semicolon))
    {
        auto ast = Make<StructDeclStmnt>();

        ast->structDecl = varType->structDecl;
        Semi();

        return ast;
    }

    auto identTkn = Accept(Tokens::Ident);

    if (Is(Tokens::LBracket))
    {
        /* Parse function declaration statement */
        return ParseFunctionDecl(varType, identTkn);
    }
    else
    {
        /* Parse variable declaration statement */
        auto ast = Make<VarDeclStmnt>();

        ast->varType    = varType;
        ast->varDecls   = ParseVarDeclList(ast.get(), identTkn);

        Semi();

        return UpdateSourceArea(ast, ast->varType.get());
    }
}

StmntPtr HLSLParser::ParseGlobalStmntWithSamplerTypeDenoter()
{
    /* Parse sampler type denoter and identifier */
    auto typeDenoter = ParseSamplerTypeDenoter();
    auto identTkn = Accept(Tokens::Ident);

    if (Is(Tokens::LBracket))
    {
        /* Make variable type from type denoter, then parse function declaration */
        return ParseFunctionDecl(ASTFactory::MakeVarType(typeDenoter), identTkn);
    }
    else
    {
        /* Parse sampler declaration statement with sampler type denoter */
        return ParseSamplerDeclStmnt(typeDenoter, identTkn);
    }
}

StmntPtr HLSLParser::ParseGlobalStmntWithBufferTypeDenoter()
{
    /* Parse buffer type denoter and identifier */
    auto typeDenoter = ParseBufferTypeDenoter();
    auto identTkn = Accept(Tokens::Ident);

    if (Is(Tokens::LBracket))
    {
        /* Make variable type from type denoter, then parse function declaration */
        return ParseFunctionDecl(ASTFactory::MakeVarType(typeDenoter), identTkn);
    }
    else
    {
        /* Parse buffer declaration statement with sampler type denoter */
        return ParseBufferDeclStmnt(typeDenoter, identTkn);
    }
}

FunctionDeclPtr HLSLParser::ParseFunctionDecl(const VarTypePtr& returnType, const TokenPtr& identTkn)
{
    auto ast = Make<FunctionDecl>();

    if (returnType)
    {
        /* Take previously parsed return type */
        ast->returnType = returnType;
    }
    else
    {
        /* Parse function attributes */
        ast->attribs = ParseAttributeList();

        /* Parse (and ignore) optional 'inline' keyword */
        if (Is(Tokens::Inline))
            AcceptIt();

        /* Parse return type */
        ast->returnType = ParseVarType(true);
    }

    /* Parse function identifier */
    if (identTkn)
    {
        ast->area   = identTkn->Area();
        ast->ident  = identTkn->Spell();
    }
    else
    {
        ast->area   = GetScanner().ActiveToken()->Area();
        ast->ident  = ParseIdent();
    }

    /* Parse parameters */
    ast->parameters = ParseParameterList();
    
    ParseFunctionDeclSemantic(*ast);

    ast->annotations = ParseAnnotationList();

    /* Parse optional function body */
    if (Is(Tokens::Semicolon))
        AcceptIt();
    else
    {
        GetReportHandler().PushContextDesc(ast->SignatureToString(false));
        {
            localScope_ = true;
            ast->codeBlock = ParseCodeBlock();
            localScope_ = false;
        }
        GetReportHandler().PopContextDesc();
    }

    return ast;
}

UniformBufferDeclPtr HLSLParser::ParseUniformBufferDecl()
{
    auto ast = Make<UniformBufferDecl>();

    /* Parse buffer header */
    ast->bufferType = ParseUniformBufferType();
    ast->ident      = ParseIdent();

    UpdateSourceArea(ast);

    /* Parse optional registers */
    ast->slotRegisters = ParseRegisterList();

    GetReportHandler().PushContextDesc(ast->ToString());
    {
        /* Parse buffer body */
        ast->members = ParseVarDeclStmntList();

        /* Parse optional semicolon (this seems to be optional for cbuffer, and tbuffer) */
        if (Is(Tokens::Semicolon))
            Semi();
    }
    GetReportHandler().PopContextDesc();

    return ast;
}

BufferDeclStmntPtr HLSLParser::ParseBufferDeclStmnt(const BufferTypeDenoterPtr& typeDenoter, const TokenPtr& identTkn)
{
    auto ast = Make<BufferDeclStmnt>();

    ast->typeDenoter = (typeDenoter ? typeDenoter : ParseBufferTypeDenoter());

    UpdateSourceArea(ast);

    ast->bufferDecls = ParseBufferDeclList(ast.get(), identTkn);

    Semi();

    return ast;
}

SamplerDeclStmntPtr HLSLParser::ParseSamplerDeclStmnt(const SamplerTypeDenoterPtr& typeDenoter, const TokenPtr& identTkn)
{
    auto ast = Make<SamplerDeclStmnt>();

    ast->typeDenoter    = (typeDenoter ? typeDenoter : ParseSamplerTypeDenoter());
    ast->samplerDecls   = ParseSamplerDeclList(ast.get(), identTkn);

    Semi();

    return ast;
}

VarDeclStmntPtr HLSLParser::ParseVarDeclStmnt()
{
    auto ast = Make<VarDeclStmnt>();

    while (true)
    {
        if (IsVarDeclModifier())
        {
            /* Parse variable declaration modifiers */
            ParseVarDeclStmntModifiers(ast.get());
        }
        else if (Is(Tokens::Ident) || IsDataType())
        {
            /* Parse type denoter */
            ast->varType = Make<VarType>();
            ast->varType->typeDenoter = ParseTypeDenoter();
            UpdateSourceArea(ast->varType);
            break;
        }
        else if (Is(Tokens::Struct))
        {
            /* Parse structure variable type */
            ast->varType = ASTFactory::MakeVarType(ParseStructDecl());
            break;
        }
        else
            ErrorUnexpected();
    }

    /* Parse variable declarations */
    ast->varDecls = ParseVarDeclList(ast.get());
    Semi();

    return UpdateSourceArea(ast);
}

// 'typedef' type_denoter IDENT;
AliasDeclStmntPtr HLSLParser::ParseAliasDeclStmnt()
{
    auto ast = Make<AliasDeclStmnt>();

    /* Parse type alias declaration */
    Accept(Tokens::Typedef);

    /* Parse type denoter with optional struct declaration */
    auto typeDenoter = ParseTypeDenoterWithStructDeclOpt(ast->structDecl);

    /* Parse type aliases */
    ast->aliasDecls = ParseAliasDeclList(typeDenoter);

    Semi();

    /* Store references in decls to this statement */
    for (auto& decl : ast->aliasDecls)
        decl->declStmntRef = ast.get();

    return ast;
}

/* --- Statements --- */

StmntPtr HLSLParser::ParseStmnt(bool allowAttributes)
{
    if (allowAttributes)
    {
        /* Parse attributes and statement */
        auto attribs = ParseAttributeList();
        auto ast = ParseStmntPrimary();
        ast->attribs = std::move(attribs);
        return ast;
    }
    else
    {
        /* Check if illegal attributes */
        if (Is(Tokens::LParen))
        {
            /* Print error, but parse and ignore attributes */
            Error("attributes not allowed in this context", false, HLSLErr::Unknown, false);
            ParseAttributeList();
        }

        /* Parse statement only */
        return ParseStmntPrimary();
    }
}

StmntPtr HLSLParser::ParseStmntPrimary()
{
    /* Determine which kind of statement the next one is */
    switch (TknType())
    {
        case Tokens::Semicolon:
            return ParseNullStmnt();
        case Tokens::LCurly:
            return ParseCodeBlockStmnt();
        case Tokens::Return:
            return ParseReturnStmnt();
        case Tokens::Ident:
            return ParseStmntWithVarIdent();
        case Tokens::For:
            return ParseForLoopStmnt();
        case Tokens::While:
            return ParseWhileLoopStmnt();
        case Tokens::Do:
            return ParseDoWhileLoopStmnt();
        case Tokens::If:
            return ParseIfStmnt();
        case Tokens::Switch:
            return ParseSwitchStmnt();
        case Tokens::CtrlTransfer:
            return ParseCtrlTransferStmnt();
        case Tokens::Struct:
            return ParseStmntWithStructDecl();
        case Tokens::Typedef:
            return ParseAliasDeclStmnt();
        case Tokens::Sampler:
        case Tokens::SamplerState:
            return ParseSamplerDeclStmnt();
        case Tokens::StorageClass:
        case Tokens::InterpModifier:
        case Tokens::TypeModifier:
            return ParseVarDeclStmnt();
        default:
            break;
    }

    if (IsDataType())
        return ParseVarDeclStmnt();

    /* Parse statement of arbitrary expression */
    return ParseExprStmnt();
}

StmntPtr HLSLParser::ParseStmntWithStructDecl()
{
    /* Parse structure declaration statement */
    auto ast = Make<StructDeclStmnt>();
    
    ast->structDecl = ParseStructDecl();

    if (!Is(Tokens::Semicolon))
    {
        /* Parse variable declaration with previous structure type */
        auto varDeclStmnt = Make<VarDeclStmnt>();

        varDeclStmnt->varType = ASTFactory::MakeVarType(ast->structDecl);
        
        /* Parse variable declarations */
        varDeclStmnt->varDecls = ParseVarDeclList(varDeclStmnt.get());
        Semi();

        return UpdateSourceArea(varDeclStmnt);
    }
    else
        Semi();

    return ast;
}

StmntPtr HLSLParser::ParseStmntWithVarIdent()
{
    /*
    Parse variable identifier first [ ident ( '.' ident )* ],
    then check if only a single identifier is required
    */
    auto varIdent = ParseVarIdent();
    
    if (Is(Tokens::LBracket) || Is(Tokens::UnaryOp) || Is(Tokens::BinaryOp) || Is(Tokens::TernaryOp))
    {
        /* Parse expression statement (function call, variable access, etc.) */
        PushPreParsedAST(varIdent);
        return ParseExprStmnt();
    }
    else if (Is(Tokens::AssignOp))
    {
        /* Parse assignment statement */
        auto ast = Make<ExprStmnt>();
        {
            auto expr = Make<VarAccessExpr>();
            {
                expr->area          = varIdent->area;
                expr->varIdent      = varIdent;
                expr->assignOp      = StringToAssignOp(AcceptIt()->Spell());
                UpdateSourceAreaOffset(expr);
                expr->assignExpr    = ParseExpr(true);
            }
            ast->expr = UpdateSourceArea(expr);

            Semi();
        }
        return ast;
    }

    if (!varIdent->next)
    {
        /* Convert variable identifier to alias type denoter */
        auto ast = Make<VarDeclStmnt>();

        ast->varType = Make<VarType>();
        ast->varType->typeDenoter = ParseAliasTypeDenoter(varIdent->ident);
        UpdateSourceArea(ast->varType, varIdent.get());

        if (!varIdent->arrayIndices.empty())
        {
            /* Convert variable identifier to array of alias type denoter */
            ast->varType->typeDenoter = MakeShared<ArrayTypeDenoter>(ast->varType->typeDenoter, varIdent->arrayIndices);
        }

        ast->varDecls = ParseVarDeclList(ast.get());
        Semi();

        return UpdateSourceArea(ast, varIdent.get());
    }

    ErrorUnexpected("expected variable declaration, assignment, or function call statement");
    
    return nullptr;
}

NullStmntPtr HLSLParser::ParseNullStmnt()
{
    /* Parse null statement */
    auto ast = Make<NullStmnt>();
    Semi();
    return ast;
}

CodeBlockStmntPtr HLSLParser::ParseCodeBlockStmnt()
{
    /* Parse code block statement */
    auto ast = Make<CodeBlockStmnt>();
    ast->codeBlock = ParseCodeBlock();
    return ast;
}

ForLoopStmntPtr HLSLParser::ParseForLoopStmnt()
{
    auto ast = Make<ForLoopStmnt>();

    /* Parse loop initializer statement (attributes not allowed here) */
    Accept(Tokens::For);
    Accept(Tokens::LBracket);

    ast->initSmnt = ParseStmnt(false);

    /* Parse loop condExpr */
    if (!Is(Tokens::Semicolon))
        ast->condition = ParseExpr(true);
    Semi();

    /* Parse loop iteration */
    if (!Is(Tokens::RBracket))
        ast->iteration = ParseExpr(true);
    Accept(Tokens::RBracket);

    /* Parse loop body */
    ast->bodyStmnt = ParseStmnt();

    return ast;
}

WhileLoopStmntPtr HLSLParser::ParseWhileLoopStmnt()
{
    auto ast = Make<WhileLoopStmnt>();

    /* Parse loop condExpr */
    Accept(Tokens::While);

    Accept(Tokens::LBracket);
    ast->condition = ParseExpr(true);
    Accept(Tokens::RBracket);

    /* Parse loop body */
    ast->bodyStmnt = ParseStmnt();

    return ast;
}

DoWhileLoopStmntPtr HLSLParser::ParseDoWhileLoopStmnt()
{
    auto ast = Make<DoWhileLoopStmnt>();

    /* Parse loop body */
    Accept(Tokens::Do);
    ast->bodyStmnt = ParseStmnt();

    /* Parse loop condExpr */
    Accept(Tokens::While);

    Accept(Tokens::LBracket);
    ast->condition = ParseExpr(true);
    Accept(Tokens::RBracket);

    Semi();

    return ast;
}

IfStmntPtr HLSLParser::ParseIfStmnt()
{
    auto ast = Make<IfStmnt>();

    /* Parse if condExpr */
    Accept(Tokens::If);

    Accept(Tokens::LBracket);
    ast->condition = ParseExpr(true);
    Accept(Tokens::RBracket);

    /* Parse if body */
    ast->bodyStmnt = ParseStmnt();

    /* Parse optional else statement */
    if (Is(Tokens::Else))
        ast->elseStmnt = ParseElseStmnt();

    return ast;
}

ElseStmntPtr HLSLParser::ParseElseStmnt()
{
    /* Parse else statment */
    auto ast = Make<ElseStmnt>();

    Accept(Tokens::Else);
    ast->bodyStmnt = ParseStmnt();

    return ast;
}

SwitchStmntPtr HLSLParser::ParseSwitchStmnt()
{
    auto ast = Make<SwitchStmnt>();

    /* Parse switch selector */
    Accept(Tokens::Switch);

    Accept(Tokens::LBracket);
    ast->selector = ParseExpr(true);
    Accept(Tokens::RBracket);

    /* Parse switch cases */
    Accept(Tokens::LCurly);
    ast->cases = ParseSwitchCaseList();
    Accept(Tokens::RCurly);

    return ast;
}

CtrlTransferStmntPtr HLSLParser::ParseCtrlTransferStmnt()
{
    /* Parse control transfer statement */
    auto ast = Make<CtrlTransferStmnt>();
    
    auto ctrlTransfer = Accept(Tokens::CtrlTransfer)->Spell();
    ast->transfer = StringToCtrlTransfer(ctrlTransfer);

    Semi();

    return ast;
}

ReturnStmntPtr HLSLParser::ParseReturnStmnt()
{
    auto ast = Make<ReturnStmnt>();

    Accept(Tokens::Return);

    if (!Is(Tokens::Semicolon))
        ast->expr = ParseExpr(true);

    Semi();

    return ast;
}

ExprStmntPtr HLSLParser::ParseExprStmnt()
{
    /* Parse expression statement */
    auto ast = Make<ExprStmnt>();

    ast->expr = ParseExpr(true);

    Semi();

    return ast;
}

/* --- Expressions --- */

ExprPtr HLSLParser::ParseExpr(bool allowComma)
{
    /* Parse generic expression, then post expression */
    auto ast = ParseGenericExpr();

    /* Parse optional post-unary expression (e.g. 'x++', 'x--') */
    if (Is(Tokens::UnaryOp))
    {
        auto unaryExpr = Make<PostUnaryExpr>();
        unaryExpr->expr = ast;
        unaryExpr->op = StringToUnaryOp(AcceptIt()->Spell());
        ast = unaryExpr;
    }

    /* Parse optional list expression */
    if (allowComma && Is(Tokens::Comma))
    {
        AcceptIt();

        auto listExpr = Make<ListExpr>();
        listExpr->firstExpr = ast;
        listExpr->nextExpr = ParseExpr(true);

        return listExpr;
    }

    return ast;
}

ExprPtr HLSLParser::ParsePrimaryExpr()
{
    /* Check if a pre-parsed AST node is available */
    if (auto preParsedAST = PopPreParsedAST())
    {
        if (preParsedAST->Type() == AST::Types::VarIdent)
        {
            auto varIdent = std::static_pointer_cast<VarIdent>(preParsedAST);
            return ParseVarAccessOrFunctionCallExpr(varIdent);
        }
        else
            ErrorInternal("unexpected pre-parsed AST node", __FUNCTION__);
    }

    /* Determine which kind of expression the next one is */
    if (IsLiteral())
        return ParseLiteralOrSuffixExpr();
    if (IsDataType() || Is(Tokens::Struct))
        return ParseTypeNameOrFunctionCallExpr();
    if (Is(Tokens::UnaryOp) || IsArithmeticUnaryExpr())
        return ParseUnaryExpr();
    if (Is(Tokens::LBracket))
        return ParseBracketOrCastExpr();
    if (Is(Tokens::LCurly))
        return ParseInitializerExpr();
    if (Is(Tokens::Ident))
        return ParseVarAccessOrFunctionCallExpr();

    ErrorUnexpected("expected primary expression", nullptr, true);
    return nullptr;
}

ExprPtr HLSLParser::ParseLiteralOrSuffixExpr()
{
    /* Parse literal expression */
    ExprPtr expr = ParseLiteralExpr();

    /* Parse optional suffix expression */
    if (Is(Tokens::Dot))
        expr = ParseSuffixExpr(expr);

    return UpdateSourceArea(expr);
}

LiteralExprPtr HLSLParser::ParseLiteralExpr()
{
    if (!IsLiteral())
        ErrorUnexpected("expected literal expression");

    /* Parse literal */
    auto ast = Make<LiteralExpr>();

    ast->dataType   = TokenToDataType(*Tkn());
    ast->value      = AcceptIt()->Spell();

    return UpdateSourceArea(ast);
}

ExprPtr HLSLParser::ParseTypeNameOrFunctionCallExpr()
{
    /* Parse type name */
    if (!IsDataType() && !Is(Tokens::Struct))
        ErrorUnexpected("expected type name or function call expression");

    auto typeDenoter = ParseTypeDenoter();

    /* Determine which kind of expression this is */
    if (Is(Tokens::LBracket))
    {
        /* Return function call expression */
        return ParseFunctionCallExpr(nullptr, typeDenoter);
    }

    /* Return type name expression */
    auto ast = Make<TypeNameExpr>();
    ast->typeDenoter = typeDenoter;

    return ast;
}

UnaryExprPtr HLSLParser::ParseUnaryExpr()
{
    if (!Is(Tokens::UnaryOp) && !IsArithmeticUnaryExpr())
        ErrorUnexpected("expected unary expression operator");

    /* Parse unary expression */
    auto ast = Make<UnaryExpr>();

    ast->op     = StringToUnaryOp(AcceptIt()->Spell());
    ast->expr   = ParsePrimaryExpr();

    return UpdateSourceArea(ast);
}

/* ----- Parsing ----- */

ExprPtr HLSLParser::ParseBracketOrCastExpr()
{
    ExprPtr expr;
    SourceArea area(GetScanner().Pos(), 1);

    /* Parse expression inside the bracket */
    Accept(Tokens::LBracket);
    {
        if (ActiveParsingState().activeTemplate)
        {
            /* Inside brackets, '<' and '>' are allowed as binary operators (albeit an active template is being parsed) */
            auto parsingState = ActiveParsingState();
            parsingState.activeTemplate = false;
            PushParsingState(parsingState);
            {
                expr = ParseExpr(true);
            }
            PopParsingState();
        }
        else
            expr = ParseExpr(true);
    }
    Accept(Tokens::RBracket);

    /*
    Parse cast expression if the expression inside the bracket is the left-hand-side of a cast expression,
    which is checked by the symbol table, because HLSL cast expressions are not context free.
    */
    if (auto typeNameExpr = MakeToTypeNameIfLhsOfCastExpr(expr))
    {
        /* Return cast expression */
        auto ast = Make<CastExpr>();
        
        ast->area       = area;
        ast->typeExpr   = typeNameExpr;
        ast->expr       = ParsePrimaryExpr();

        return UpdateSourceArea(ast);
    }

    /* Return bracket expression */
    auto ast = Make<BracketExpr>();

    ast->area = area;
    ast->expr = expr;

    expr = ast;

    /* Parse optional array-access expression */
    if (Is(Tokens::LParen))
        expr = ParseArrayAccessExpr(expr);

    /* Parse optional suffix expression */
    if (Is(Tokens::Dot))
        expr = ParseSuffixExpr(expr);

    return UpdateSourceArea(expr);
}

SuffixExprPtr HLSLParser::ParseSuffixExpr(const ExprPtr& expr)
{
    auto ast = Make<SuffixExpr>();

    /* Take sub expression */
    ast->expr = expr;

    /* Parse suffix after dot */
    Accept(Tokens::Dot);
    ast->varIdent = ParseVarIdent();

    return UpdateSourceArea(ast, expr.get());
}

ArrayAccessExprPtr HLSLParser::ParseArrayAccessExpr(const ExprPtr& expr)
{
    auto ast = Make<ArrayAccessExpr>();

    /* Take sub expression and parse array dimensions */
    ast->expr           = expr;
    ast->arrayIndices   = ParseArrayDimensionList();

    return UpdateSourceArea(ast, expr.get());
}

ExprPtr HLSLParser::ParseVarAccessOrFunctionCallExpr(VarIdentPtr varIdent)
{
    /* Parse variable identifier first (for variables and functions) */
    if (!varIdent)
        varIdent = ParseVarIdent();
    
    if (Is(Tokens::LBracket))
        return ParseFunctionCallExpr(varIdent);

    return ParseVarAccessExpr(varIdent);
}

VarAccessExprPtr HLSLParser::ParseVarAccessExpr(const VarIdentPtr& varIdent)
{
    auto ast = Make<VarAccessExpr>();

    if (varIdent)
        ast->varIdent = varIdent;
    else
        ast->varIdent = ParseVarIdent();

    ast->area = ast->varIdent->area;

    /* Parse optional assign expression */
    if (Is(Tokens::AssignOp))
    {
        UpdateSourceAreaOffset(ast);
        ast->assignOp   = StringToAssignOp(AcceptIt()->Spell());
        ast->assignExpr = ParseExpr();
    }

    return UpdateSourceArea(ast);
}

ExprPtr HLSLParser::ParseFunctionCallExpr(const VarIdentPtr& varIdent, const TypeDenoterPtr& typeDenoter)
{
    /* Parse function call expression */
    auto ast = Make<FunctionCallExpr>();

    if (typeDenoter)
        ast->call = ParseFunctionCall(typeDenoter);
    else
        ast->call = ParseFunctionCall(varIdent);

    /* Update source area */
    UpdateSourceArea(ast, ast->call.get());

    /* Parse optional array-access expression */
    ExprPtr expr = ast;

    if (Is(Tokens::LParen))
        expr = ParseArrayAccessExpr(expr);

    /* Parse optional suffix expression */
    if (Is(Tokens::Dot))
        expr = ParseSuffixExpr(expr);

    return expr;
}

InitializerExprPtr HLSLParser::ParseInitializerExpr()
{
    /* Parse initializer list expression */
    auto ast = Make<InitializerExpr>();
    ast->exprs = ParseInitializerList();
    return UpdateSourceArea(ast);
}

/* --- Lists --- */

std::vector<VarDeclPtr> HLSLParser::ParseVarDeclList(VarDeclStmnt* declStmntRef, TokenPtr firstIdentTkn)
{
    std::vector<VarDeclPtr> varDecls;

    /* Parse variable declaration list */
    while (true)
    {
        varDecls.push_back(ParseVarDecl(declStmntRef, firstIdentTkn));
        firstIdentTkn = nullptr;
        if (Is(Tokens::Comma))
            AcceptIt();
        else
            break;
    }

    return varDecls;
}

std::vector<VarDeclStmntPtr> HLSLParser::ParseVarDeclStmntList()
{
    std::vector<VarDeclStmntPtr> members;

    Accept(Tokens::LCurly);

    /* Parse all variable declaration statements */
    while (!Is(Tokens::RCurly))
        members.push_back(ParseVarDeclStmnt());

    AcceptIt();

    return members;
}

std::vector<VarDeclStmntPtr> HLSLParser::ParseParameterList()
{
    std::vector<VarDeclStmntPtr> parameters;

    Accept(Tokens::LBracket);

    /* Parse all variable declaration statements */
    if (!Is(Tokens::RBracket))
    {
        while (true)
        {
            parameters.push_back(ParseParameter());
            if (Is(Tokens::Comma))
                AcceptIt();
            else
                break;
        }
    }

    Accept(Tokens::RBracket);

    return parameters;
}

std::vector<VarDeclStmntPtr> HLSLParser::ParseAnnotationList()
{
    std::vector<VarDeclStmntPtr> annotations;

    if (Is(Tokens::BinaryOp, "<"))
    {
        AcceptIt();

        while (!Is(Tokens::BinaryOp, ">"))
            annotations.push_back(ParseVarDeclStmnt());
        
        AcceptIt();
    }

    return annotations;
}

std::vector<StmntPtr> HLSLParser::ParseStmntList()
{
    std::vector<StmntPtr> stmnts;

    while (!Is(Tokens::RCurly))
        ParseStmntWithOptionalComment(stmnts, std::bind(&HLSLParser::ParseStmnt, this, true));

    return stmnts;
}

std::vector<ExprPtr> HLSLParser::ParseExprList(const Tokens listTerminatorToken, bool allowLastComma)
{
    std::vector<ExprPtr> exprs;

    /* Parse all argument expressions */
    if (!Is(listTerminatorToken))
    {
        while (true)
        {
            exprs.push_back(ParseExpr());
            if (Is(Tokens::Comma))
            {
                AcceptIt();
                if (allowLastComma && Is(listTerminatorToken))
                    break;
            }
            else
                break;
        }
    }

    return exprs;
}

std::vector<ExprPtr> HLSLParser::ParseArrayDimensionList(bool allowDynamicDimension)
{
    std::vector<ExprPtr> arrayDims;

    while (Is(Tokens::LParen))
        arrayDims.push_back(ParseArrayDimension(allowDynamicDimension));

    return arrayDims;
}

std::vector<ExprPtr> HLSLParser::ParseArgumentList()
{
    Accept(Tokens::LBracket);
    auto exprs = ParseExprList(Tokens::RBracket);
    Accept(Tokens::RBracket);
    return exprs;
}

std::vector<ExprPtr> HLSLParser::ParseInitializerList()
{
    Accept(Tokens::LCurly);
    auto exprs = ParseExprList(Tokens::RCurly, true);
    Accept(Tokens::RCurly);
    return exprs;
}

std::vector<RegisterPtr> HLSLParser::ParseRegisterList(bool parseFirstColon)
{
    std::vector<RegisterPtr> registers;

    if (parseFirstColon && Is(Tokens::Register))
        registers.push_back(ParseRegister(false));

    while (Is(Tokens::Colon))
        registers.push_back(ParseRegister());


    return registers;
}

std::vector<AttributePtr> HLSLParser::ParseAttributeList()
{
    std::vector<AttributePtr> attribs;

    while (Is(Tokens::LParen))
        attribs.push_back(ParseAttribute());

    return attribs;
}

std::vector<SwitchCasePtr> HLSLParser::ParseSwitchCaseList()
{
    std::vector<SwitchCasePtr> cases;

    while (Is(Tokens::Case) || Is(Tokens::Default))
        cases.push_back(ParseSwitchCase());

    return cases;
}

std::vector<BufferDeclPtr> HLSLParser::ParseBufferDeclList(BufferDeclStmnt* declStmntRef, const TokenPtr& identTkn)
{
    std::vector<BufferDeclPtr> bufferDecls;

    bufferDecls.push_back(ParseBufferDecl(declStmntRef, identTkn));

    while (Is(Tokens::Comma))
    {
        AcceptIt();
        bufferDecls.push_back(ParseBufferDecl(declStmntRef));
    }

    return bufferDecls;
}

std::vector<SamplerDeclPtr> HLSLParser::ParseSamplerDeclList(SamplerDeclStmnt* declStmntRef, const TokenPtr& identTkn)
{
    std::vector<SamplerDeclPtr> samplerDecls;

    samplerDecls.push_back(ParseSamplerDecl(declStmntRef, identTkn));

    while (Is(Tokens::Comma))
    {
        AcceptIt();
        samplerDecls.push_back(ParseSamplerDecl(declStmntRef));
    }

    return samplerDecls;
}

std::vector<SamplerValuePtr> HLSLParser::ParseSamplerValueList()
{
    std::vector<SamplerValuePtr> samplerValues;

    while (!Is(Tokens::RCurly))
        samplerValues.push_back(ParseSamplerValue());

    return samplerValues;
}

std::vector<AliasDeclPtr> HLSLParser::ParseAliasDeclList(TypeDenoterPtr typeDenoter)
{
    std::vector<AliasDeclPtr> aliasDecls;

    aliasDecls.push_back(ParseAliasDecl(typeDenoter));

    while (Is(Tokens::Comma))
    {
        AcceptIt();
        aliasDecls.push_back(ParseAliasDecl(typeDenoter));
    }

    return aliasDecls;
}

/* --- Others --- */

std::string HLSLParser::ParseIdent(const TokenPtr& identTkn)
{
    return (identTkn ? identTkn->Spell() : Accept(Tokens::Ident)->Spell());
}

std::string HLSLParser::ParseRegister_OBSOLETE(bool parseColon)
{
    if (localScope_)
        Error("semantics are not allowed in local scope", false, HLSLErr::ERR_SEMANTICS, false);

    /* Parse ': register(IDENT)' */
    if (parseColon)
        Accept(Tokens::Colon);
    
    Accept(Tokens::Register);
    Accept(Tokens::LBracket);

    auto registerName = ParseIdent();

    Accept(Tokens::RBracket);

    return registerName;
}

TypeDenoterPtr HLSLParser::ParseTypeDenoter(bool allowVoidType)
{
    if (Is(Tokens::Void))
    {
        /* Parse void type denoter */
        if (allowVoidType)
            return ParseVoidTypeDenoter();
        else
            Error("'void' type not allowed in this context");
        return nullptr;
    }
    else
    {
        /* Parse primary type denoter and optional array dimensions */
        auto typeDenoter = ParseTypeDenoterPrimary();

        if (Is(Tokens::LParen))
        {
            /* Make array type denoter */
            auto arrayTypeDenoter = std::make_shared<ArrayTypeDenoter>();
            {
                arrayTypeDenoter->arrayDims         = ParseArrayDimensionList();
                arrayTypeDenoter->baseTypeDenoter   = typeDenoter;
            }
            typeDenoter = arrayTypeDenoter;
        }

        return typeDenoter;
    }
}

TypeDenoterPtr HLSLParser::ParseTypeDenoterPrimary()
{
    if (IsBaseDataType())
        return ParseBaseTypeDenoter();
    else if (Is(Tokens::Vector))
        return ParseBaseVectorTypeDenoter();
    else if (Is(Tokens::Matrix))
        return ParseBaseMatrixTypeDenoter();
    else if (Is(Tokens::Ident))
        return ParseAliasTypeDenoter();
    else if (Is(Tokens::Struct))
        return ParseStructTypeDenoter();
    else if (Is(Tokens::Buffer))
        return ParseBufferTypeDenoter();
    else if (Is(Tokens::Sampler) || Is(Tokens::SamplerState))
        return ParseSamplerTypeDenoter();
    else
        ErrorUnexpected("expected type denoter", GetScanner().ActiveToken().get(), true);
    return nullptr;
}

TypeDenoterPtr HLSLParser::ParseTypeDenoterWithStructDeclOpt(StructDeclPtr& structDecl, bool allowVoidType)
{
    if (Is(Tokens::Struct))
    {
        AcceptIt();

        if (Is(Tokens::LCurly))
        {
            /* Parse struct-decl */
            structDecl = ParseStructDecl(false);

            /* Make struct type denoter with reference to the structure of this alias decl */
            return std::make_shared<StructTypeDenoter>(structDecl.get());
        }
        else
        {
            /* Parse struct ident token */
            auto structIdentTkn = Accept(Tokens::Ident);

            if (Is(Tokens::LCurly) || Is(Tokens::Colon))
            {
                /* Parse struct-decl */
                structDecl = ParseStructDecl(false, structIdentTkn);

                /* Make struct type denoter with reference to the structure of this alias decl */
                return std::make_shared<StructTypeDenoter>(structDecl.get());
            }
            else
            {
                /* Make struct type denoter without struct decl */
                return std::make_shared<StructTypeDenoter>(structIdentTkn->Spell());
            }
        }
    }
    else
        return ParseTypeDenoter(allowVoidType);
}

VoidTypeDenoterPtr HLSLParser::ParseVoidTypeDenoter()
{
    Accept(Tokens::Void);
    return std::make_shared<VoidTypeDenoter>();
}

BaseTypeDenoterPtr HLSLParser::ParseBaseTypeDenoter()
{
    if (IsBaseDataType())
    {
        auto keyword = AcceptIt()->Spell();

        /* Make base type denoter by data type keyword */
        auto typeDenoter = std::make_shared<BaseTypeDenoter>();
        typeDenoter->dataType = ParseDataType(keyword);
        return typeDenoter;
    }
    ErrorUnexpected("expected base type denoter", nullptr, true);
    return nullptr;
}

// matrix < ScalarType, '1'-'4', '1'-'4' >;
BaseTypeDenoterPtr HLSLParser::ParseBaseVectorTypeDenoter()
{
    std::string vectorType;

    /* Parse scalar type */
    Accept(Tokens::Vector);

    if (Is(Tokens::BinaryOp, "<"))
    {
        AcceptIt();
    
        PushParsingState({ true });
        {
            vectorType = Accept(Tokens::ScalarType)->Spell();

            /* Parse vector dimension */
            Accept(Tokens::Comma);
            int dim = ParseAndEvaluateVectorDimension();

            /* Build final type denoter */
            vectorType += std::to_string(dim);
        }
        PopParsingState();

        Accept(Tokens::BinaryOp, ">");
    }
    else
        vectorType = "float4";

    /* Make base type denoter by data type keyword */
    auto typeDenoter = std::make_shared<BaseTypeDenoter>();
    typeDenoter->dataType = ParseDataType(vectorType);

    return typeDenoter;
}

// matrix < ScalarType, '1'-'4', '1'-'4' >;
BaseTypeDenoterPtr HLSLParser::ParseBaseMatrixTypeDenoter()
{
    std::string matrixType;

    /* Parse scalar type */
    Accept(Tokens::Matrix);

    if (Is(Tokens::BinaryOp, "<"))
    {
        AcceptIt();
    
        PushParsingState({ true });
        {
            matrixType = Accept(Tokens::ScalarType)->Spell();

            /* Parse matrix dimensions */
            Accept(Tokens::Comma);
            int dimM = ParseAndEvaluateVectorDimension();

            Accept(Tokens::Comma);
            int dimN = ParseAndEvaluateVectorDimension();

            /* Build final type denoter */
            matrixType += std::to_string(dimM) + 'x' + std::to_string(dimN);
        }
        PopParsingState();

        Accept(Tokens::BinaryOp, ">");
    }
    else
        matrixType = "float4x4";

    /* Make base type denoter by data type keyword */
    auto typeDenoter = std::make_shared<BaseTypeDenoter>();
    typeDenoter->dataType = ParseDataType(matrixType);

    return typeDenoter;
}

BufferTypeDenoterPtr HLSLParser::ParseBufferTypeDenoter()
{
    /* Make buffer type denoter */
    auto typeDenoter = std::make_shared<BufferTypeDenoter>();

    /* Parse buffer type */
    auto bufferTypeTkn = Tkn();
    typeDenoter->bufferType = ParseBufferType();

    /* Parse optional template arguments */
    if (Is(Tokens::BinaryOp, "<"))
    {
        PushParsingState({ true });
        {
            AcceptIt();

            /* Parse generic type denoter ('<' TYPE '>') */
            typeDenoter->genericTypeDenoter = ParseTypeDenoter(false);

            /* Parse optional generic size */
            if (Is(Tokens::Comma))
            {
                AcceptIt();
                auto genSize = ParseAndEvaluateConstExprInt();

                if (IsTextureMSBufferType(typeDenoter->bufferType))
                {
                    if (genSize < 1 || genSize >= 128)
                    {
                        Warning(
                            "number of samples in texture must be in the range [1, 128), but got " +
                            std::to_string(genSize), bufferTypeTkn.get()
                        );
                    }
                }
                else if (IsPatchBufferType(typeDenoter->bufferType))
                {
                    if (genSize < 1 || genSize > 64)
                    {
                        Warning(
                            "number of control points in patch must be in the range [1, 64], but got " +
                            std::to_string(genSize), bufferTypeTkn.get()
                        );
                    }
                }
                else
                    Error("illegal usage of generic size in texture, buffer, or stream object");

                typeDenoter->genericSize = genSize;
            }

            Accept(Tokens::BinaryOp, ">");
        }
        PopParsingState();
    }

    return typeDenoter;
}

SamplerTypeDenoterPtr HLSLParser::ParseSamplerTypeDenoter()
{
    /* Make sampler type denoter */
    auto samplerType = ParseSamplerType();
    return std::make_shared<SamplerTypeDenoter>(samplerType);
}

StructTypeDenoterPtr HLSLParser::ParseStructTypeDenoter()
{
    /* Parse optional 'struct' keyword */
    if (Is(Tokens::Struct))
        AcceptIt();

    /* Parse identifier */
    auto ident = ParseIdent();

    /* Make struct type denoter */
    auto typeDenoter = std::make_shared<StructTypeDenoter>(ident);

    return typeDenoter;
}

AliasTypeDenoterPtr HLSLParser::ParseAliasTypeDenoter(std::string ident)
{
    /* Parse identifier */
    if (ident.empty())
        ident = ParseIdent();

    /* Make alias type denoter per default (change this to a struct type later) */
    return std::make_shared<AliasTypeDenoter>(ident);
}

Variant HLSLParser::ParseAndEvaluateConstExpr()
{
    /* Parse expression */
    auto tkn = Tkn();
    auto expr = ParseExpr();

    try
    {
        /* Evaluate expression and throw error on var-access */
        ConstExprEvaluator exprEvaluator;
        return exprEvaluator.EvaluateExpr(*expr, [](VarAccessExpr* ast) -> Variant { throw ast; });
    }
    catch (const std::exception& e)
    {
        Error(e.what(), tkn.get());
    }
    catch (const VarAccessExpr* expr)
    {
        GetReportHandler().Error(true, "expected constant expression", GetScanner().Source(), expr->area);
    }

    return Variant();
}

int HLSLParser::ParseAndEvaluateConstExprInt()
{
    auto tkn = Tkn();
    auto value = ParseAndEvaluateConstExpr();

    if (value.Type() != Variant::Types::Int)
        Error("expected integral constant expression", tkn.get());

    return static_cast<int>(value.Int());
}

int HLSLParser::ParseAndEvaluateVectorDimension()
{
    auto tkn = Tkn();
    auto value = ParseAndEvaluateConstExprInt();

    if (value < 1 || value > 4)
        Error("vector and matrix dimensions must be between 1 and 4", tkn.get());

    return value;
}

void HLSLParser::ParseAndIgnoreTechnique()
{
    /* Only expect 'technique' keyword */
    Accept(Tokens::Technique);

    Warning("techniques are ignored");

    /* Ignore all tokens until the first opening brace */
    std::stack<TokenPtr> braceTknStack;

    while (!Is(Tokens::LCurly))
        AcceptIt();

    braceTknStack.push(Accept(Tokens::LCurly));

    /* Ignore all tokens and count the opening and closing braces */
    while (!braceTknStack.empty())
    {
        if (Is(Tokens::LCurly))
            braceTknStack.push(Tkn());
        else if (Is(Tokens::RCurly))
            braceTknStack.pop();
        else if (Is(Tokens::EndOfStream))
            Error("missing closing brace '}' for open code block", braceTknStack.top().get());
        AcceptIt();
    }
}

void HLSLParser::ParseVarDeclSemantic(VarDecl& varDecl, bool allowPackOffset)
{
    while (Is(Tokens::Colon))
    {
        /* Colon is only syntactic sugar, thus not part of the source area */
        Accept(Tokens::Colon);

        if (Is(Tokens::Register))
        {
            /* Parse and ignore registers for variable declarations */
            Warning("register is ignore for variable declarations");
            ParseRegister(false);
        }
        else if (Is(Tokens::PackOffset))
        {
            /* Parse pack offset (ignore previous pack offset) */
            varDecl.packOffset = ParsePackOffset(false);
            if (!allowPackOffset)
                Error("packoffset is only allowed in a constant buffer", true, HLSLErr::ERR_PACK_OFFSET_IN_INVALID_SCOPE);
        }
        else
        {
            /* Parse semantic (ignore previous semantic) */
            varDecl.semantic = ParseSemantic(false);
        }
    }
}

void HLSLParser::ParseFunctionDeclSemantic(FunctionDecl& funcDecl)
{
    while (Is(Tokens::Colon))
    {
        /* Colon is only syntactic sugar, thus not part of the source area */
        Accept(Tokens::Colon);

        if (Is(Tokens::Register))
        {
            /* Parse and ignore registers for variable declarations */
            Warning("register is ignore for function declarations");
            ParseRegister(false);
        }
        else if (Is(Tokens::PackOffset))
        {
            /* Report error and ignore packoffset */
            Error("packoffset is only allowed in a constant buffer", true, HLSLErr::ERR_PACK_OFFSET_IN_INVALID_SCOPE);
            ParsePackOffset(false);
        }
        else
        {
            /* Parse semantic (ignore previous semantic) */
            funcDecl.semantic = ParseSemantic(false);
        }
    }
}

DataType HLSLParser::ParseDataType(const std::string& keyword)
{
    try
    {
        return HLSLKeywordToDataType(keyword);
    }
    catch (const std::exception& e)
    {
        Error(e.what());
    }
    return DataType::Undefined;
}

PrimitiveType HLSLParser::ParsePrimitiveType()
{
    try
    {
        return HLSLKeywordToPrimitiveType(Accept(Tokens::PrimitiveType)->Spell());
    }
    catch (const std::exception& e)
    {
        Error(e.what());
    }
    return PrimitiveType::Undefined;
}

InterpModifier HLSLParser::ParseInterpModifier()
{
    try
    {
        return HLSLKeywordToInterpModifier(Accept(Tokens::InterpModifier)->Spell());
    }
    catch (const std::exception& e)
    {
        Error(e.what());
    }
    return InterpModifier::Undefined;
}

TypeModifier HLSLParser::ParseTypeModifier()
{
    try
    {
        return HLSLKeywordToTypeModifier(Accept(Tokens::TypeModifier)->Spell());
    }
    catch (const std::exception& e)
    {
        Error(e.what());
    }
    return TypeModifier::Undefined;
}

StorageClass HLSLParser::ParseStorageClass()
{
    try
    {
        return HLSLKeywordToStorageClass(Accept(Tokens::StorageClass)->Spell());
    }
    catch (const std::exception& e)
    {
        Error(e.what());
    }
    return StorageClass::Undefined;
}

UniformBufferType HLSLParser::ParseUniformBufferType()
{
    try
    {
        return HLSLKeywordToUniformBufferType(Accept(Tokens::UniformBuffer)->Spell());
    }
    catch (const std::exception& e)
    {
        Error(e.what());
    }
    return UniformBufferType::Undefined;
}

BufferType HLSLParser::ParseBufferType()
{
    try
    {
        return HLSLKeywordToBufferType(Accept(Tokens::Buffer)->Spell());
    }
    catch (const std::exception& e)
    {
        Error(e.what());
    }
    return BufferType::Undefined;
}

SamplerType HLSLParser::ParseSamplerType()
{
    try
    {
        if (Is(Tokens::Sampler) || Is(Tokens::SamplerState))
            return HLSLKeywordToSamplerType(AcceptIt()->Spell());
        else
            ErrorUnexpected("expected sampler type denoter or sampler state");
    }
    catch (const std::exception& e)
    {
        Error(e.what());
    }
    return SamplerType::Undefined;
}

IndexedSemantic HLSLParser::ParseSemantic(bool parseColon)
{
    if (parseColon)
        Accept(Tokens::Colon);
    return HLSLKeywordToSemantic(ParseIdent());
}

std::string HLSLParser::ParseSamplerStateTextureIdent()
{
    std::string ident;

    Accept(Tokens::Buffer, "texture");
    Accept(Tokens::AssignOp, "=");

    if (Is(Tokens::LBracket))
    {
        AcceptIt();
        ident = ParseIdent();
        Accept(Tokens::RBracket);
    }
    else if (Is(Tokens::BinaryOp, "<"))
    {
        AcceptIt();
        ident = ParseIdent();
        Accept(Tokens::BinaryOp, ">");
    }
    else
        ErrorUnexpected("expected '<' or '('");

    Semi();

    return ident;
}

void HLSLParser::ParseStmntWithOptionalComment(std::vector<StmntPtr>& stmnts, const std::function<StmntPtr()>& parseFunction)
{
    /* Parse next statement with optional commentary */
    auto comment = GetScanner().GetComment();

    auto ast = parseFunction();
    stmnts.push_back(ast);

    ast->comment = std::move(comment);
}

bool HLSLParser::ParseVarDeclStmntModifiers(VarDeclStmnt* ast, bool allowPrimitiveType)
{
    if (Is(Tokens::InputModifier))
    {
        /* Parse input modifier */
        auto modifier = AcceptIt()->Spell();

        if (modifier == "in")
            ast->isInput = true;
        else if (modifier == "out")
            ast->isOutput = true;
        else if (modifier == "inout")
        {
            ast->isInput = true;
            ast->isOutput = true;
        }
        else if (modifier == "uniform")
            ast->isUniform = true;
    }
    else if (Is(Tokens::InterpModifier))
    {
        /* Parse interpolation modifier */
        ast->interpModifiers.insert(ParseInterpModifier());
    }
    else if (Is(Tokens::TypeModifier))
    {
        /* Parse type modifier (const, row_major, column_major) */
        ast->typeModifiers.insert(ParseTypeModifier());
    }
    else if (Is(Tokens::StorageClass))
    {
        /* Parse storage class */
        ast->storageClasses.insert(ParseStorageClass());
    }
    else if (Is(Tokens::PrimitiveType))
    {
        /* Parse primitive type */
        if (!allowPrimitiveType)
            Error("primitive type not allowed in this context", false, HLSLErr::Unknown, false);
        
        auto primitiveType = ParsePrimitiveType();

        if (ast->primitiveType == PrimitiveType::Undefined)
            ast->primitiveType = primitiveType;
        else if (ast->primitiveType == primitiveType)
            Error("duplicate primitive type specified", true, HLSLErr::Unknown, false);
        else if (ast->primitiveType != primitiveType)
            Error("conflicting primitive types", true, HLSLErr::Unknown, false);
    }
    else
        return false;

    return true;
}


} // /namespace Xsc



// ================================================================================
