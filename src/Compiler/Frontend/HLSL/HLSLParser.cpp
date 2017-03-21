/*
 * HLSLParser.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HLSLParser.h"
#include "HLSLKeywords.h"
#include "ConstExprEvaluator.h"
#include "Helper.h"
#include "AST.h"
#include "ASTFactory.h"
#include "ReportIdents.h"


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

ProgramPtr HLSLParser::ParseSource(
    const SourceCodePtr& source, const NameMangling& nameMangling, bool useD3D10Semantics, bool rowMajorAlignment)
{
    useD3D10Semantics_  = useD3D10Semantics;
    rowMajorAlignment_  = rowMajorAlignment;

    GetNameMangling() = nameMangling;

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
    return (Is(Tokens::NullLiteral) || Is(Tokens::BoolLiteral) || Is(Tokens::IntLiteral) || Is(Tokens::FloatLiteral) || Is(Tokens::StringLiteral));
}

bool HLSLParser::IsArithmeticUnaryExpr() const
{
    return (Is(Tokens::BinaryOp, "-") || Is(Tokens::BinaryOp, "+"));
}

bool HLSLParser::IsModifier() const
{
    return (Is(Tokens::InputModifier) || Is(Tokens::InterpModifier) || Is(Tokens::TypeModifier) || Is(Tokens::StorageClass));
}

TypeSpecifierPtr HLSLParser::MakeTypeSpecifierIfLhsOfCastExpr(const ExprPtr& expr)
{
    /* Type specifier expression (float, int3 etc.) is always allowed for a cast expression */
    if (auto typeSpecifierExpr = expr->As<TypeSpecifierExpr>())
        return typeSpecifierExpr->typeSpecifier;

    /* Is this an object expression? */
    if (auto objectExpr = expr->As<ObjectExpr>())
    {
        /* Check if the identifier refers to a type name */
        if (IsRegisteredTypeName(objectExpr->ident))
        {
            /* Convert the variable access into a type specifier */
            return ASTFactory::MakeTypeSpecifier(std::make_shared<AliasTypeDenoter>(objectExpr->ident));
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
    try
    {
        if (ident == "line")
            ProcessDirectiveLine();
        else if (ident == "pragma")
            ProcessDirectivePragma();
        else
            Error(R_InvalidHLSLDirectiveAfterPP);
    }
    catch (const std::exception& e)
    {
        Error(e.what());
    }
}

void HLSLParser::ProcessDirectiveLine()
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

void HLSLParser::ProcessDirectivePragma()
{
    /* Parse 'pack_matrix' pragma */
    if (Is(Tokens::Ident) && Tkn()->Spell() == "pack_matrix")
    {
        Parser::AcceptIt();

        auto AcceptToken = [&](const Tokens type)
        {
            if (!Is(type))
                throw std::runtime_error(R_UnexpectedTokenInPackMatrixPragma);
            return Parser::AcceptIt();
        };

        AcceptToken(Tokens::LBracket);
        auto alignmentTkn = AcceptToken(Tokens::TypeModifier);
        AcceptToken(Tokens::RBracket);

        /* Set matrix pack alignment */
        auto alignment = alignmentTkn->Spell();
        if (alignment == "row_major")
            rowMajorAlignment_ = true;
        else if (alignment == "column_major")
            rowMajorAlignment_ = false;
        else
            Error(R_UnknownMatrixPackAlignment(alignment), alignmentTkn.get());
    }
    else
        Error(R_InvalidHLSLPragmaAfterPP);
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

AliasDeclStmntPtr HLSLParser::MakeAndRegisterBuildinAlias(const DataType dataType, const std::string& ident)
{
    auto ast = ASTFactory::MakeBaseTypeAlias(dataType, ident);
    RegisterTypeName(ident);
    ast->flags << AST::isBuildIn;
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
            MakeAndRegisterBuildinAlias(type.first, type.second)
        );
    }
}

TypeSpecifierPtr HLSLParser::MakeTypeSpecifierWithPackAlignment()
{
    auto ast = Make<TypeSpecifier>();

    if (rowMajorAlignment_)
        ast->SetTypeModifier(TypeModifier::RowMajor);

    return ast;
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
        ParseAndIgnoreTechniquesAndNullStmnts();

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

VarDeclStmntPtr HLSLParser::ParseParameter()
{
    auto ast = Make<VarDeclStmnt>();

    /* Parse parameter as single variable declaration */
    ast->typeSpecifier = ParseTypeSpecifier();
    ast->varDecls.push_back(ParseVarDecl(ast.get()));

    /* Mark with 'parameter' flag */
    ast->flags << VarDeclStmnt::isParameter;

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
        Warning(R_UnknownAttribute(attribIdent));

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
        Warning(R_UnknownSlotRegister(typeIdent.substr(0, 1)));

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

ArrayDimensionPtr HLSLParser::ParseArrayDimension(bool allowDynamicDimension)
{
    auto ast = Make<ArrayDimension>();

    Accept(Tokens::LParen);
    
    if (Is(Tokens::RParen))
    {
        if (!allowDynamicDimension)
            Error(R_ExpectedExplicitArrayDim, false);
        ast->expr = Make<NullExpr>();
    }
    else
        ast->expr = ParseExpr();

    Accept(Tokens::RParen);

    return UpdateSourceArea(ast);
}

ExprPtr HLSLParser::ParseArrayIndex()
{
    auto area = Tkn()->Area();

    Accept(Tokens::LParen);
    
    auto ast = ParseExpr();
    ast->area = area;
    
    Accept(Tokens::RParen);

    return UpdateSourceArea(ast);
}

ExprPtr HLSLParser::ParseInitializer()
{
    Accept(Tokens::AssignOp, "=");
    return ParseExpr();
}

TypeSpecifierPtr HLSLParser::ParseTypeSpecifier(bool parseVoidType)
{
    auto ast = MakeTypeSpecifierWithPackAlignment();

    /* Parse modifiers and primitive types */
    while (IsModifier() || Is(Tokens::PrimitiveType))
        ParseModifiers(ast.get(), true);

    /* Parse variable type denoter with optional struct declaration */
    ast->typeDenoter = ParseTypeDenoterWithStructDeclOpt(ast->structDecl);

    return UpdateSourceArea(ast);
}

VarDeclPtr HLSLParser::ParseVarDecl(VarDeclStmnt* declStmntRef, const TokenPtr& identTkn)
{
    auto ast = Make<VarDecl>();

    /* Store reference to parent node */
    ast->declStmntRef = declStmntRef;

    /* Parse variable declaration */
    ast->ident = ParseIdentWithNamespaceOpt(ast->namespaceExpr, identTkn, &ast->area);

    /* Parse optional array dimension, semantic, and annocations */
    ast->arrayDims = ParseArrayDimensionList(true);

    ParseVarDeclSemantic(*ast);

    ast->annotations = ParseAnnotationList();

    /* Parse optional initializer expression */
    if (Is(Tokens::AssignOp, "="))
        ast->initializer = ParseInitializer();

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
                Error(R_IllegalRecursiveInheritance);

            if (Is(Tokens::Comma))
                Error(R_IllegalMultipleInheritance, false);
        }
    }

    GetReportHandler().PushContextDesc(ast->ToString());
    {
        /* Parse member variable declarations */
        ast->localStmnts = ParseLocalStmntList();
        
        for (auto& stmnt : ast->localStmnts)
        {
            switch (stmnt->Type())
            {
                case AST::Types::VarDeclStmnt:
                    ast->varMembers.push_back(std::static_pointer_cast<VarDeclStmnt>(stmnt));
                    break;
                case AST::Types::FunctionDecl:
                    ast->funcMembers.push_back(std::static_pointer_cast<FunctionDecl>(stmnt));
                    break;
                default:
                    Error(R_IllegalDeclStmntInsideDeclOf(ast->ToString()), stmnt->area, false);
                    break;
            }
        }

        /* Decorate all member variables with a reference to this structure declaration */
        for (auto& varDeclStmnt : ast->varMembers)
        {
            for (auto& varDecl : varDeclStmnt->varDecls)
                varDecl->structDeclRef = ast.get();
        }

        /* Decorate all member functions with a reference to this structure declaration */
        for (auto& funcDecl : ast->funcMembers)
            funcDecl->structDeclRef = ast.get();
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
        /* Make array type denoter and use input as sub type denoter */
        typeDenoter = std::make_shared<ArrayTypeDenoter>(typeDenoter, ParseArrayDimensionList());
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
        case Tokens::LParen:
        case Tokens::Void:
        case Tokens::Inline:
            return ParseFunctionDecl();
        default:
            return ParseGlobalStmntWithTypeSpecifier();
    }
}

StmntPtr HLSLParser::ParseGlobalStmntWithTypeSpecifier()
{
    /* Parse type specifier */
    auto typeSpecifier = ParseTypeSpecifier();

    /* Is this only a struct declaration? */
    if (typeSpecifier->structDecl && Is(Tokens::Semicolon))
    {
        /* Convert type specifier into struct declaration statement */
        auto ast = Make<StructDeclStmnt>();

        ast->structDecl = typeSpecifier->structDecl;
        ast->structDecl->declStmntRef = ast.get();

        Semi();

        return ast;
    }

    /* Parse identifier */
    auto identTkn = Accept(Tokens::Ident);

    /* Is this a function declaration? */
    if (Is(Tokens::LBracket))
    {
        /* Parse function declaration statement */
        return ParseFunctionDecl(typeSpecifier, identTkn);
    }
    else
    {
        /* Parse variable declaration statement */
        auto ast = Make<VarDeclStmnt>();

        ast->typeSpecifier  = typeSpecifier;
        ast->varDecls       = ParseVarDeclList(ast.get(), identTkn);

        Semi();

        return UpdateSourceArea(ast, ast->typeSpecifier.get());
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
        return ParseFunctionDecl(ASTFactory::MakeTypeSpecifier(typeDenoter), identTkn);
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
        return ParseFunctionDecl(ASTFactory::MakeTypeSpecifier(typeDenoter), identTkn);
    }
    else
    {
        /* Parse buffer declaration statement with sampler type denoter */
        return ParseBufferDeclStmnt(typeDenoter, identTkn);
    }
}

FunctionDeclPtr HLSLParser::ParseFunctionDecl(const TypeSpecifierPtr& returnType, const TokenPtr& identTkn)
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
        ast->returnType = ParseTypeSpecifier(true);
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
        GetReportHandler().PushContextDesc(ast->ToString(false));
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
        ast->localStmnts = ParseLocalStmntList();

        /* Copy variable declarations into separated list */
        for (auto& stmnt : ast->localStmnts)
        {
            if (stmnt->Type() == AST::Types::VarDeclStmnt)
                ast->varMembers.push_back(std::static_pointer_cast<VarDeclStmnt>(stmnt));
        }

        /* Decorate all member variables with a reference to this buffer declaration */
        for (auto& varDeclStmnt : ast->varMembers)
        {
            for (auto& varDecl : varDeclStmnt->varDecls)
                varDecl->bufferDeclRef = ast.get();
        }

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
    ast->bufferDecls = ParseBufferDeclList(ast.get(), identTkn);

    Semi();

    if (identTkn)
        ast->area = identTkn->Area();
    else
        UpdateSourceArea(ast);

    return ast;
}

SamplerDeclStmntPtr HLSLParser::ParseSamplerDeclStmnt(const SamplerTypeDenoterPtr& typeDenoter, const TokenPtr& identTkn)
{
    auto ast = Make<SamplerDeclStmnt>();

    ast->typeDenoter = (typeDenoter ? typeDenoter : ParseSamplerTypeDenoter());
    ast->samplerDecls = ParseSamplerDeclList(ast.get(), identTkn);

    Semi();

    if (identTkn)
        ast->area = identTkn->Area();
    else
        UpdateSourceArea(ast);

    return ast;
}

VarDeclStmntPtr HLSLParser::ParseVarDeclStmnt()
{
    auto ast = Make<VarDeclStmnt>();

    /* Parse type specifier and all variable declarations */
    ast->typeSpecifier  = ParseTypeSpecifier();
    ast->varDecls       = ParseVarDeclList(ast.get());

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

    return UpdateSourceArea(ast);
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
        /* Check for illegal attributes */
        if (Is(Tokens::LParen))
        {
            /* Print error, but parse and ignore attributes */
            Error(R_NotAllowedInThisContext(R_Attributes), false, false);
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
            return ParseStmntWithIdent();
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
    ast->structDecl->declStmntRef = ast.get();

    if (!Is(Tokens::Semicolon))
    {
        /* Parse variable declaration with previous structure type */
        auto varDeclStmnt = Make<VarDeclStmnt>();

        varDeclStmnt->typeSpecifier = ASTFactory::MakeTypeSpecifier(ast->structDecl);
        
        /* Parse variable declarations */
        varDeclStmnt->varDecls = ParseVarDeclList(varDeclStmnt.get());
        Semi();

        return UpdateSourceArea(varDeclStmnt);
    }
    else
        Semi();

    return ast;
}

#if 1//TODO: clean this up!!!

// ~~~~~~~~~~~~ MIGHT BE INCOMPLETE ~~~~~~~~~~~~~~~

StmntPtr HLSLParser::ParseStmntWithIdent()
{
    /* Parse the identifier as object expression (can be converted later) */
    auto objectExpr = ParseObjectExpr();

    auto expr = ParseExprWithSuffixOpt(objectExpr);

    if (Is(Tokens::LBracket) || Is(Tokens::UnaryOp) || Is(Tokens::BinaryOp) || Is(Tokens::TernaryOp))
    {
        /* Parse expression statement (function call, variable access, etc.) */
        PushPreParsedAST(expr);
        return ParseExprStmnt();
    }
    else if (Is(Tokens::Semicolon))
    {
        /* Return immediatly with expression statement */
        return ParseExprStmnt(expr);
    }
    else if (expr == objectExpr)
    {
        /* Convert variable identifier to alias type denoter */
        auto ast = Make<VarDeclStmnt>();

        ast->typeSpecifier              = MakeTypeSpecifierWithPackAlignment();
        ast->typeSpecifier->typeDenoter = ParseTypeDenoterWithArrayOpt(ParseAliasTypeDenoter(objectExpr->ident));

        UpdateSourceArea(ast->typeSpecifier, objectExpr.get());

        ast->varDecls = ParseVarDeclList(ast.get());
        Semi();

        return UpdateSourceArea(ast, objectExpr.get());
    }
    else
        return ParseExprStmnt(expr);

    ErrorUnexpected(R_ExpectedVarOrAssignOrFuncCall, nullptr, true);

    return nullptr;
}

#endif

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

    ast->initStmnt = ParseStmnt(false);

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

    UpdateSourceArea(ast);

    Semi();

    return ast;
}

ReturnStmntPtr HLSLParser::ParseReturnStmnt()
{
    auto ast = Make<ReturnStmnt>();

    Accept(Tokens::Return);

    if (!Is(Tokens::Semicolon))
        ast->expr = ParseExpr(true);

    UpdateSourceArea(ast);

    Semi();

    return ast;
}

ExprStmntPtr HLSLParser::ParseExprStmnt(const ExprPtr& expr)
{
    /* Parse expression statement */
    auto ast = Make<ExprStmnt>();

    if (expr)
    {
        ast->expr = expr;
        ast->area = expr->area;
    }
    else
        ast->expr = ParseExpr(true);

    Semi();

    return UpdateSourceArea(ast);
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
        
        UpdateSourceArea(unaryExpr, ast.get());
        UpdateSourceAreaOffset(unaryExpr);

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
    /* Primary prefix of primary expression */
    return ParseExprWithSuffixOpt(ParsePrimaryExprPrefix());
}

ExprPtr HLSLParser::ParsePrimaryExprPrefix()
{
    /* Check if a pre-parsed AST node is available */
    if (auto preParsedAST = PopPreParsedAST())
    {
        if (preParsedAST->Type() == AST::Types::ObjectExpr)
        {
            auto objectExpr = std::static_pointer_cast<ObjectExpr>(preParsedAST);
            if (Is(Tokens::LBracket))
                return ParseCallExpr(objectExpr);
            else
                return objectExpr;
        }
        else
            ErrorInternal(R_UnexpectedPreParsedAST, __FUNCTION__);
    }

    /* Determine which kind of expression the next one is */
    if (IsLiteral())
        return ParseLiteralExpr();
    if (IsModifier())
        return ParseTypeSpecifierExpr();
    if (IsDataType() || Is(Tokens::Struct))
        return ParseTypeSpecifierOrCallExpr();
    if (Is(Tokens::UnaryOp) || IsArithmeticUnaryExpr())
        return ParseUnaryExpr();
    if (Is(Tokens::LBracket))
        return ParseBracketOrCastExpr();
    if (Is(Tokens::LCurly))
        return ParseInitializerExpr();
    if (Is(Tokens::Ident))
        return ParseObjectOrCallExpr();

    ErrorUnexpected(R_ExpectedPrimaryExpr, nullptr, true);

    return nullptr;
}

ExprPtr HLSLParser::ParseExprWithSuffixOpt(ExprPtr expr)
{
    /* Parse optional suffix expressions */
    while (true)
    {
        if (Is(Tokens::LParen))
            expr = ParseArrayExpr(expr);
        else if (Is(Tokens::Dot) || Is(Tokens::DColon))
            expr = ParseObjectOrCallExpr(expr);
        else if (Is(Tokens::AssignOp))
            expr = ParseAssignExpr(expr);
        else
            break;
    }

    return UpdateSourceArea(expr);
}

LiteralExprPtr HLSLParser::ParseLiteralExpr()
{
    if (!IsLiteral())
        ErrorUnexpected(R_ExpectedLiteralExpr);

    /* Parse literal */
    auto ast = Make<LiteralExpr>();

    if (!Is(Tokens::NullLiteral))
        ast->dataType = TokenToDataType(*Tkn());

    ast->value = AcceptIt()->Spell();

    return UpdateSourceArea(ast);
}

ExprPtr HLSLParser::ParseTypeSpecifierOrCallExpr()
{
    /* Parse type denoter with optional structure delcaration */
    if (!IsDataType() && !Is(Tokens::Struct))
        ErrorUnexpected(R_ExpectedTypeNameOrFuncCall);

    StructDeclPtr structDecl;
    auto typeDenoter = ParseTypeDenoter(true, &structDecl);

    /* Determine which kind of expression this is */
    if (Is(Tokens::LBracket) && !structDecl)
    {
        /* Return function call expression */
        return ParseCallExpr(nullptr, typeDenoter);
    }

    /* Return type name expression */
    auto ast = Make<TypeSpecifierExpr>();
    {
        ast->typeSpecifier               = ASTFactory::MakeTypeSpecifier(typeDenoter);
        ast->typeSpecifier->structDecl   = structDecl;
    }
    UpdateSourceArea(ast->typeSpecifier, structDecl.get());

    return UpdateSourceArea(ast, structDecl.get());
}

TypeSpecifierExprPtr HLSLParser::ParseTypeSpecifierExpr()
{
    auto ast = Make<TypeSpecifierExpr>();

    /* Parse type specifier */
    ast->typeSpecifier = ParseTypeSpecifier();

    return UpdateSourceArea(ast);
}

UnaryExprPtr HLSLParser::ParseUnaryExpr()
{
    if (!Is(Tokens::UnaryOp) && !IsArithmeticUnaryExpr())
        ErrorUnexpected(R_ExpectedUnaryExprOp);

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
    if (auto typeSpecifier = MakeTypeSpecifierIfLhsOfCastExpr(expr))
    {
        /* Return cast expression */
        auto ast = Make<CastExpr>();

        /* Take type specifier for cast expression */
        ast->area           = area;
        ast->typeSpecifier  = typeSpecifier;

        /* Parse sub expression */
        ast->expr           = ParsePrimaryExpr();

        return UpdateSourceArea(ast);
    }

    /* Return bracket expression */
    auto ast = Make<BracketExpr>();

    ast->area = area;
    ast->expr = expr;

    return UpdateSourceArea(ast);
}

ObjectExprPtr HLSLParser::ParseObjectExpr(const ExprPtr& expr)
{
    /* Parse prefix token if prefix expression is specified  */
    bool isStatic = false;

    if (expr != nullptr)
    {
        /* Parse '::' or '.' prefix */
        if (Is(Tokens::DColon))
        {
            AcceptIt();
            isStatic = true;
        }
        else if (Is(Tokens::Dot))
            AcceptIt();
        else
            ErrorUnexpected(R_ExpectedIdentPrefix);
    }

    auto ast = Make<ObjectExpr>();

    if (expr)
        ast->area = expr->area;

    /* Take sub expression and parse identifier */
    ast->prefixExpr = expr;
    ast->isStatic   = isStatic;
    ast->ident      = ParseIdent();

    return UpdateSourceArea(ast);
}

AssignExprPtr HLSLParser::ParseAssignExpr(const ExprPtr& expr)
{
    auto ast = Make<AssignExpr>();

    /* Take sub expression and parse assignment */
    ast->lvalueExpr = expr;

    /* Parse assign expression */
    if (Is(Tokens::AssignOp))
    {
        UpdateSourceAreaOffset(ast);
        ast->op         = StringToAssignOp(AcceptIt()->Spell());
        ast->rvalueExpr = ParseExpr();
    }
    else
        ErrorUnexpected(Tokens::AssignOp);

    return UpdateSourceArea(ast);
}

ExprPtr HLSLParser::ParseObjectOrCallExpr(const ExprPtr& expr)
{
    /* Parse variable identifier first (for variables and functions) */
    auto objectExpr = ParseObjectExpr(expr);
    
    if (Is(Tokens::LBracket))
        return ParseCallExpr(objectExpr);

    return objectExpr;
}

ArrayExprPtr HLSLParser::ParseArrayExpr(const ExprPtr& expr)
{
    auto ast = Make<ArrayExpr>();

    /* Take sub expression and parse array dimensions */
    ast->prefixExpr     = expr;
    ast->arrayIndices   = ParseArrayIndexList();

    return UpdateSourceArea(ast, expr.get());
}

CallExprPtr HLSLParser::ParseCallExpr(const ObjectExprPtr& objectExpr, const TypeDenoterPtr& typeDenoter)
{
    if (objectExpr)
    {
        /* Make new identifier token with source position form input */
        auto identTkn = std::make_shared<Token>(objectExpr->area.Pos(), Tokens::Ident, objectExpr->ident);

        /* Parse call expression and take prefix expression from input */
        return ParseCallExprWithPrefixOpt(objectExpr->prefixExpr, objectExpr->isStatic, identTkn);
    }
    else if (typeDenoter)
    {
        /* Parse call expression with type denoter */
        return ParseCallExprAsTypeCtor(typeDenoter);
    }
    else
    {
        /* Parse completely new call expression */
        return ParseCallExprWithPrefixOpt();
    }
}

CallExprPtr HLSLParser::ParseCallExprWithPrefixOpt(const ExprPtr& prefixExpr, bool isStatic, const TokenPtr& identTkn)
{
    auto ast = Make<CallExpr>();

    /* Take prefix expression */
    ast->prefixExpr = prefixExpr;
    ast->isStatic   = isStatic;

    /* Parse function name */
    if (identTkn)
    {
        /* Take identifier token */
        ast->ident  = identTkn->Spell();
        ast->area   = identTkn->Area();
    }
    else
    {
        /* Parse identifier token */
        ast->ident = ParseIdent();
        UpdateSourceArea(ast);
    }

    /* Parse argument list */
    ast->arguments = ParseArgumentList();

    return ast;
}

// Parse function call as a type constructor (e.g. "float4(...)")
CallExprPtr HLSLParser::ParseCallExprAsTypeCtor(const TypeDenoterPtr& typeDenoter)
{
    auto ast = Make<CallExpr>();

    /* Take type denoter */
    ast->typeDenoter = typeDenoter;

    /* Parse argument list */
    ast->arguments = ParseArgumentList();

    return UpdateSourceArea(ast);
}

InitializerExprPtr HLSLParser::ParseInitializerExpr()
{
    /* Parse initializer list expression */
    auto ast = Make<InitializerExpr>();
    ast->exprs = ParseInitializerList();
    return UpdateSourceArea(ast);
}

/* --- Lists --- */

std::vector<StmntPtr> HLSLParser::ParseLocalStmntList()
{
    std::vector<StmntPtr> stmnts;

    Accept(Tokens::LCurly);

    /* Parse all variable declaration statements */
    while (!Is(Tokens::RCurly))
    {
        /* Parse next global declaration (ignore techniques and null statements) */
        ParseAndIgnoreTechniquesAndNullStmnts();
        ParseStmntWithOptionalComment(stmnts, std::bind(&HLSLParser::ParseGlobalStmnt, this));
    }

    AcceptIt();

    return stmnts;
}

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

std::vector<ArrayDimensionPtr> HLSLParser::ParseArrayDimensionList(bool allowDynamicDimension)
{
    std::vector<ArrayDimensionPtr> arrayDims;

    while (Is(Tokens::LParen))
        arrayDims.push_back(ParseArrayDimension(allowDynamicDimension));

    return arrayDims;
}

std::vector<ExprPtr> HLSLParser::ParseArrayIndexList()
{
    std::vector<ExprPtr> exprs;

    while (Is(Tokens::LParen))
        exprs.push_back(ParseArrayIndex());

    return exprs;
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

std::string HLSLParser::ParseIdent(TokenPtr identTkn, SourceArea* area)
{
    /* Parse identifier */
    if (!identTkn)
        identTkn = Accept(Tokens::Ident);

    auto ident = identTkn->Spell();

    /* Return source area of identifier */
    if (area)
        *area = identTkn->Area();

    /* Check overlapping of reserved prefixes for name mangling */
    if (auto prefix = FindNameManglingPrefix(ident))
        Error(R_IdentNameManglingConflict(ident, *prefix), identTkn.get(), false);

    return ident;
}

std::string HLSLParser::ParseIdentWithNamespaceOpt(ObjectExprPtr& namespaceExpr, TokenPtr identTkn, SourceArea* area)
{
    /* Parse first identifier */
    SourceArea identArea;
    auto ident = ParseIdent(identTkn, &identArea);

    /* Check if the current identifier is a static namespace */
    if (Is(Tokens::DColon))
    {
        AcceptIt();
        
        /* Take first identifier as namespace prefix */
        namespaceExpr           = Make<ObjectExpr>();
        namespaceExpr->ident    = ident;
        namespaceExpr->area     = identArea;

        /* Parse next identifier */
        ObjectExprPtr subObjectExpr;
        ident = ParseIdentWithNamespaceOpt(subObjectExpr, nullptr, area);
        namespaceExpr->prefixExpr = subObjectExpr;

        return ident;
    }

    /* Return identifier and its source area */
    *area = identArea;

    return ident;
}

TypeDenoterPtr HLSLParser::ParseTypeDenoter(bool allowVoidType, StructDeclPtr* structDecl)
{
    if (Is(Tokens::Void))
    {
        /* Parse void type denoter */
        if (allowVoidType)
            return ParseVoidTypeDenoter();

        Error(R_NotAllowedInThisContext(R_VoidTypeDen));
        return nullptr;
    }
    else
    {
        /* Parse primary type denoter and optional array dimensions */
        auto typeDenoter = ParseTypeDenoterPrimary(structDecl);

        if (Is(Tokens::LParen))
        {
            /* Make array type denoter */
            typeDenoter = std::make_shared<ArrayTypeDenoter>(typeDenoter, ParseArrayDimensionList());
        }

        return typeDenoter;
    }
}

TypeDenoterPtr HLSLParser::ParseTypeDenoterPrimary(StructDeclPtr* structDecl)
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
    {
        if (structDecl)
            return ParseStructTypeDenoterWithStructDeclOpt(*structDecl);
        else
            return ParseStructTypeDenoter();
    }
    else if (Is(Tokens::Buffer))
        return ParseBufferTypeDenoter();
    else if (Is(Tokens::Sampler) || Is(Tokens::SamplerState))
        return ParseSamplerTypeDenoter();

    ErrorUnexpected(R_ExpectedTypeDen, GetScanner().ActiveToken().get(), true);
    return nullptr;
}

TypeDenoterPtr HLSLParser::ParseTypeDenoterWithStructDeclOpt(StructDeclPtr& structDecl, bool allowVoidType)
{
    if (Is(Tokens::Struct))
        return ParseStructTypeDenoterWithStructDeclOpt(structDecl);
    else
        return ParseTypeDenoter(allowVoidType);
}

TypeDenoterPtr HLSLParser::ParseTypeDenoterWithArrayOpt(const TypeDenoterPtr& baseTypeDenoter)
{
    if (Is(Tokens::LParen))
    {
        auto arrayTypeDenoter = std::make_shared<ArrayTypeDenoter>(baseTypeDenoter);

        /* Parse array dimension list */
        arrayTypeDenoter->arrayDims = ParseArrayDimensionList();

        return arrayTypeDenoter;
    }
    return baseTypeDenoter;
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
    ErrorUnexpected(R_ExpectedBaseTypeDen, nullptr, true);
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
                        Warning(R_TextureSampleCountLimitIs128(genSize), bufferTypeTkn.get());
                }
                else if (IsPatchBufferType(typeDenoter->bufferType))
                {
                    if (genSize < 1 || genSize > 64)
                        Warning(R_PatchCtrlPointLimitIs64(genSize), bufferTypeTkn.get());
                }
                else
                    Error(R_IllegalBufferTypeGenericSize);

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

StructTypeDenoterPtr HLSLParser::ParseStructTypeDenoterWithStructDeclOpt(StructDeclPtr& structDecl)
{
    Accept(Tokens::Struct);

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
        /* Evaluate expression and throw error on object access */
        ConstExprEvaluator exprEvaluator;
        return exprEvaluator.EvaluateExpr(*expr, [](ObjectExpr* expr) -> Variant { throw expr; });
    }
    catch (const std::exception& e)
    {
        Error(e.what(), tkn.get());
    }
    catch (const ObjectExpr* expr)
    {
        GetReportHandler().Error(true, R_ExpectedConstExpr, GetScanner().Source(), expr->area);
    }

    return Variant();
}

int HLSLParser::ParseAndEvaluateConstExprInt()
{
    auto tkn = Tkn();
    auto value = ParseAndEvaluateConstExpr();

    if (value.Type() != Variant::Types::Int)
        Error(R_ExpectedConstIntExpr, tkn.get());

    return static_cast<int>(value.Int());
}

int HLSLParser::ParseAndEvaluateVectorDimension()
{
    auto tkn = Tkn();
    auto value = ParseAndEvaluateConstExprInt();

    if (value < 1 || value > 4)
        Error(R_VectorAndMatrixDimOutOfRange(value), tkn.get());

    return value;
}

void HLSLParser::ParseAndIgnoreTechniquesAndNullStmnts()
{
    /* Ignore all null statements and techniques */
    while (Is(Tokens::Semicolon) || Is(Tokens::Technique))
    {
        if (Is(Tokens::Technique))
            ParseAndIgnoreTechnique();
        else
            AcceptIt();
    }
}

void HLSLParser::ParseAndIgnoreTechnique()
{
    /* Only expect 'technique' keyword */
    Accept(Tokens::Technique);

    Warning(R_TechniquesAreIgnored);

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
            Error(R_MissingClosingBrace, braceTknStack.top().get());
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
            Warning(R_RegisterIgnoredForVarDecls);
            ParseRegister(false);
        }
        else if (Is(Tokens::PackOffset))
        {
            /* Parse pack offset (ignore previous pack offset) */
            varDecl.packOffset = ParsePackOffset(false);
            if (!allowPackOffset)
                Error(R_IllegalPackOffset, true);
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
            Warning(R_RegisterIgnoredForFuncDecls);
            ParseRegister(false);
        }
        else if (Is(Tokens::PackOffset))
        {
            /* Report error and ignore packoffset */
            Error(R_IllegalPackOffset, true);
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
            ErrorUnexpected(R_ExpectedSamplerOrSamplerState);
    }
    catch (const std::exception& e)
    {
        Error(e.what());
    }
    return SamplerType::Undefined;
}

IndexedSemantic HLSLParser::ParseSemantic(bool parseColon)
{
    try
    {
        if (parseColon)
            Accept(Tokens::Colon);
        return HLSLKeywordToSemantic(ParseIdent(), useD3D10Semantics_);
    }
    catch (const std::exception& e)
    {
        Error(e.what());
    }
    return Semantic::UserDefined;
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
        ErrorUnexpected(R_ExpectedOpenBracketOrAngleBracket);

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

bool HLSLParser::ParseModifiers(TypeSpecifier* typeSpecifier, bool allowPrimitiveType)
{
    if (Is(Tokens::InputModifier))
    {
        /* Parse input modifier */
        auto modifier = AcceptIt()->Spell();

        if (modifier == "in")
            typeSpecifier->isInput = true;
        else if (modifier == "out")
            typeSpecifier->isOutput = true;
        else if (modifier == "inout")
        {
            typeSpecifier->isInput = true;
            typeSpecifier->isOutput = true;
        }
        else if (modifier == "uniform")
            typeSpecifier->isUniform = true;
    }
    else if (Is(Tokens::InterpModifier))
    {
        /* Parse interpolation modifier */
        typeSpecifier->interpModifiers.insert(ParseInterpModifier());
    }
    else if (Is(Tokens::TypeModifier))
    {
        /* Parse type modifier (const, row_major, column_major, snorm, unorm) */
        typeSpecifier->SetTypeModifier(ParseTypeModifier());
    }
    else if (Is(Tokens::StorageClass))
    {
        /* Parse storage class */
        typeSpecifier->storageClasses.insert(ParseStorageClass());
    }
    else if (Is(Tokens::PrimitiveType))
    {
        /* Parse primitive type */
        if (!allowPrimitiveType)
            Error(R_NotAllowedInThisContext(R_PrimitiveType), false, false);
        
        auto primitiveType = ParsePrimitiveType();

        if (typeSpecifier->primitiveType == PrimitiveType::Undefined)
            typeSpecifier->primitiveType = primitiveType;
        else if (typeSpecifier->primitiveType == primitiveType)
            Error(R_DuplicatedPrimitiveType, true, false);
        else if (typeSpecifier->primitiveType != primitiveType)
            Error(R_ConflictingPrimitiveTypes, true, false);
    }
    else
        return false;

    return true;
}


} // /namespace Xsc



// ================================================================================
