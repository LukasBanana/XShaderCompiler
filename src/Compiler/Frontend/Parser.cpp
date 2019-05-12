/*
 * Parser.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Parser.h"
#include "ReportIdents.h"
#include "AST.h"
#include "Exception.h"
#include <algorithm>


namespace Xsc
{


/*
 * ======= Protected: =======
 */

Parser::Parser(Log* log) :
    reportHandler_ { log },
    log_           { log }
{
}

/* ----- Report Handling ----- */

static SourceArea GetTokenArea(const Token* tkn)
{
    return (tkn != nullptr ? tkn->Area() : SourceArea::ignore);
}

void Parser::Error(const std::string& msg, const SourceArea& area, bool breakWithExpection)
{
    /* Report error with the report handler */
    reportHandler_.SubmitReport(
        breakWithExpection, ReportTypes::Error, R_SyntaxError, msg, GetScanner().Source(), area
    );
}

void Parser::Error(const std::string& msg, const Token* tkn, bool breakWithExpection)
{
    /* Always break with an exception when the end of stream has been reached */
    if (tkn != nullptr && tkn->Type() == Tokens::EndOfStream)
        breakWithExpection = true;

    /* Report error from token source area */
    Error(msg, GetTokenArea(tkn), breakWithExpection);
}

void Parser::Error(const std::string& msg, bool prevToken, bool breakWithExpection)
{
    /* Get token and submit error */
    auto tkn = (prevToken ? GetScanner().CurrentToken().get() : GetScanner().LookAheadToken().get());
    Error(msg, tkn, breakWithExpection);
}

void Parser::ErrorUnexpected(const std::string& hint, const Token* tkn, bool breakWithExpection)
{
    if (!tkn)
        tkn = tkn_.get();

    /* Increment unexpected token counter */
    IncUnexpectedTokenCounter();

    /* Submit error */
    Error(R_UnexpectedToken(Token::TypeToString(tkn->Type()), hint), tkn, breakWithExpection);

    /* Ignore unexpected token to produce further reports */
    AcceptIt();
}

void Parser::ErrorUnexpected(const Tokens type, const Token* tkn, bool breakWithExpection)
{
    auto typeName = Token::TypeToString(type);
    if (typeName.empty())
        ErrorUnexpected("", tkn, breakWithExpection);
    else
        ErrorUnexpected(R_Expected(typeName), tkn, breakWithExpection);
}

void Parser::ErrorInternal(const std::string& msg, const std::string& procName)
{
    reportHandler_.SubmitReport(
        true,
        ReportTypes::Error,
        R_InternalError,
        msg + R_InFunction(procName),
        nullptr,
        SourceArea::ignore
    );
}

void Parser::Warning(const std::string& msg, const SourceArea& area)
{
    if (enableWarnings_)
        reportHandler_.Warning(false, msg, GetScanner().Source(), area);
}

void Parser::Warning(const std::string& msg, const Token* tkn)
{
    Warning(msg, GetTokenArea(tkn));
}

void Parser::Warning(const std::string& msg, bool prevToken)
{
    Warning(msg, prevToken ? GetScanner().CurrentToken().get() : GetScanner().LookAheadToken().get());
}

void Parser::EnableWarnings(bool enable)
{
    enableWarnings_ = enable;
}

/* ----- Scanner ----- */

void Parser::PushScannerSource(const SourceCodePtr& source, const std::string& filename)
{
    /* Add current token to previous scanner */
    if (!scannerStack_.empty())
        scannerStack_.top().nextToken = tkn_;

    /* Make a new token scanner */
    auto scanner = MakeScanner();
    if (!scanner)
        RuntimeErr(R_FailedToCreateScanner);

    scannerStack_.push({ scanner, filename, nullptr });

    /* Start scanning */
    if (!scanner->ScanSource(source))
        RuntimeErr(R_FailedToScanSource);

    /* Set initial source origin for scanner */
    scanner->Source()->NextSourceOrigin(filename, 0);

    /* Accept first token */
    AcceptIt();
}

bool Parser::PopScannerSource()
{
    /* Get previous scanner */
    if (scannerStack_.empty())
        return false;

    scannerStack_.pop();

    if (scannerStack_.empty())
        return false;

    /* Reset previous 'next token' */
    tkn_ = scannerStack_.top().nextToken;

    return (tkn_ != nullptr);
}

Scanner& Parser::GetScanner()
{
    if (scannerStack_.empty())
        RuntimeErr(R_MissingScanner);
    return *(scannerStack_.top().scanner);
}

std::string Parser::GetCurrentFilename() const
{
    return (!scannerStack_.empty() ? scannerStack_.top().filename : "");
}

TokenPtr Parser::Accept(const Tokens type)
{
    /* Check if token is unexpected, otherwise reset counter */
    AssertTokenType(type);
    unexpectedTokenCounter_ = 0;
    return AcceptIt();
}

TokenPtr Parser::Accept(const Tokens type, const std::string& spell)
{
    /* Check if token is unexpected, otherwise reset counter */
    AssertTokenType(type);
    AssertTokenSpell(spell);
    unexpectedTokenCounter_ = 0;
    return AcceptIt();
}

TokenPtr Parser::AcceptIt()
{
    /* Check if end-of-stream has already reached */
    if (tkn_ && tkn_->Type() == Tokens::EndOfStream)
        Error(R_UnexpectedEndOfStream, tkn_.get());

    /* Return previous token */
    auto prevTkn = tkn_;

    /* Try to scan next token from stack of token strings */
    if (!tokenStringItStack_.empty())
    {
        for (auto it = tokenStringItStack_.rbegin(); it != tokenStringItStack_.rend(); ++it)
        {
            if (!it->ReachedEnd())
            {
                /* Scan next token from token string */
                auto& tokenStringIt = tokenStringItStack_.back();
                tkn_ = *(tokenStringIt++);
                return prevTkn;
            }
        }

        tkn_ = Make<Token>(Tokens::EndOfStream);
        return prevTkn;
    }

    /* Scan next token from scanner */
    tkn_ = GetScanner().Next();
    return prevTkn;
}

void Parser::PushTokenString(const TokenPtrString& tokenString, bool acceptFirst)
{
    /* Cache current token that will come next (after the token strings) */
    if (tokenStringItStack_.empty())
        cachedTkn_ = tkn_;

    /* Push token string into stack */
    tokenStringItStack_.push_back(tokenString.Begin());

    if (acceptFirst)
        AcceptIt();
}

void Parser::PopTokenString()
{
    tokenStringItStack_.pop_back();

    /* Restore cached token before token strings where added */
    if (tokenStringItStack_.empty())
        tkn_ = cachedTkn_;
}

void Parser::IgnoreWhiteSpaces(bool includeNewLines, bool includeComments)
{
    while ( Is(Tokens::WhiteSpace) || ( includeNewLines && Is(Tokens::NewLine) ) || ( includeComments && Is(Tokens::Comment) ) )
        AcceptIt();
}

void Parser::IgnoreNewLines()
{
    while (Is(Tokens::NewLine))
        AcceptIt();
}

/* ----- Parsing ----- */

void Parser::PushParsingState(const ParsingState& state)
{
    parsingStateStack_.push(state);
}

void Parser::PopParsingState()
{
    parsingStateStack_.pop();
}

void Parser::PushPreParsedAST(const ASTPtr& ast)
{
    preParsedASTStack_.push(ast);
}

ASTPtr Parser::PopPreParsedAST()
{
    if (!preParsedASTStack_.empty())
    {
        auto ast = preParsedASTStack_.top();
        preParsedASTStack_.pop();
        return ast;
    }
    return nullptr;
}

Parser::ParsingState Parser::ActiveParsingState() const
{
    return (parsingStateStack_.empty() ? ParsingState{ false } : parsingStateStack_.top());
}

// expr: logic_or_expr | ternary_expr;
ExprPtr Parser::ParseGenericExpr()
{
    auto ast = ParseLogicOrExpr();

    /* Parse optional ternary expression */
    if (Is(Tokens::TernaryOp))
        return ParseTernaryExpr(ast);

    return ast;
}

// ternary_expr: expr '?' expr ':' expr;
TernaryExprPtr Parser::ParseTernaryExpr(const ExprPtr& condExpr)
{
    auto ast = Make<TernaryExpr>();

    /* Take condExpr expression and use its source position */
    ast->condExpr   = condExpr;
    ast->area       = condExpr->area;

    /* Parse begin of ternary expression */
    Accept(Tokens::TernaryOp);

    /* Update source area */
    UpdateSourceAreaOffset(ast);

    /* Parse 'then' and 'else' branch expressions */
    ast->thenExpr = ParseGenericExpr();
    Accept(Tokens::Colon);
    ast->elseExpr = ParseGenericExpr();

    return UpdateSourceArea(ast);
}

ExprPtr Parser::ParseLogicOrExpr()
{
    return ParseLTRBinaryExpr(std::bind(&Parser::ParseLogicAndExpr, this), { BinaryOp::LogicalOr });
}

ExprPtr Parser::ParseLogicAndExpr()
{
    return ParseLTRBinaryExpr(std::bind(&Parser::ParseBitwiseOrExpr, this), { BinaryOp::LogicalAnd });
}

ExprPtr Parser::ParseBitwiseOrExpr()
{
    return ParseLTRBinaryExpr(std::bind(&Parser::ParseBitwiseXOrExpr, this), { BinaryOp::Or });
}

ExprPtr Parser::ParseBitwiseXOrExpr()
{
    return ParseLTRBinaryExpr(std::bind(&Parser::ParseBitwiseAndExpr, this), { BinaryOp::Xor });
}

ExprPtr Parser::ParseBitwiseAndExpr()
{
    return ParseLTRBinaryExpr(std::bind(&Parser::ParseEqualityExpr, this), { BinaryOp::And });
}

ExprPtr Parser::ParseEqualityExpr()
{
    return ParseLTRBinaryExpr(std::bind(&Parser::ParseRelationExpr, this), { BinaryOp::Equal, BinaryOp::NotEqual });
}

ExprPtr Parser::ParseRelationExpr()
{
    /* Do not parse '<' and '>' as binary operator while a template is actively being parsed */
    if (ActiveParsingState().activeTemplate)
        return ParseLTRBinaryExpr(std::bind(&Parser::ParseShiftExpr, this), { BinaryOp::LessEqual, BinaryOp::GreaterEqual });
    else
        return ParseLTRBinaryExpr(std::bind(&Parser::ParseShiftExpr, this), { BinaryOp::Less, BinaryOp::LessEqual, BinaryOp::Greater, BinaryOp::GreaterEqual });
}

ExprPtr Parser::ParseShiftExpr()
{
    return ParseLTRBinaryExpr(std::bind(&Parser::ParseAddExpr, this), { BinaryOp::LShift, BinaryOp::RShift });
}

ExprPtr Parser::ParseAddExpr()
{
    return ParseLTRBinaryExpr(std::bind(&Parser::ParseSubExpr, this), { BinaryOp::Add });
}

ExprPtr Parser::ParseSubExpr()
{
    return ParseLTRBinaryExpr(std::bind(&Parser::ParseMulExpr, this), { BinaryOp::Sub });
}

ExprPtr Parser::ParseMulExpr()
{
    return ParseLTRBinaryExpr(std::bind(&Parser::ParseDivExpr, this), { BinaryOp::Mul });
}

ExprPtr Parser::ParseDivExpr()
{
    return ParseLTRBinaryExpr(std::bind(&Parser::ParseValueExpr, this), { BinaryOp::Div, BinaryOp::Mod });
}

ExprPtr Parser::ParseValueExpr()
{
    return ParsePrimaryExpr();
}

int Parser::ParseIntLiteral(TokenPtr tkn)
{
    /* Parse value string of integer literal token */
    if (!tkn)
        tkn = Accept(Tokens::IntLiteral);

    /* Parse value string of token */
    return ParseIntLiteral(tkn->Spell(), tkn.get());
}

int Parser::ParseIntLiteral(const std::string& valueStr, const Token* tkn)
{
    /* Convert literal string into integer */
    try
    {
        return std::stoi(valueStr);
    }
    catch (const std::invalid_argument&)
    {
        Error(R_ExpectedIntLiteral(valueStr), tkn, false);
    }
    catch (const std::out_of_range&)
    {
        Error(R_IntLiteralOutOfRange(valueStr), tkn, false);
    }
    return 0;
}

/* ----- Common ----- */

const std::string* Parser::FindNameManglingPrefix(const std::string& ident) const
{
    auto FindPrefix = [&ident](const std::string& prefix) -> const std::string*
    {
        if (ident.compare(0, prefix.size(), prefix) == 0)
            return &prefix;
        else
            return nullptr;
    };

    if (auto prefix = FindPrefix(nameMangling_.inputPrefix))
        return prefix;

    if (auto prefix = FindPrefix(nameMangling_.outputPrefix))
        return prefix;

    if (auto prefix = FindPrefix(nameMangling_.reservedWordPrefix))
        return prefix;

    if (auto prefix = FindPrefix(nameMangling_.temporaryPrefix))
        return prefix;

    return nullptr;
}


/*
 * ======= Private: =======
 */

// binary_expr: (binary_expr OP)? sub_expr;
ExprPtr Parser::ParseLTRBinaryExpr(const std::function<ExprPtr()>& parseSubExprFunc, const BinaryOpList& binaryOps)
{
    /* Parse primary expression */
    ExprPtr ast = parseSubExprFunc();

    while (Is(Tokens::BinaryOp))
    {
        /* Parse binary operator */
        auto op = StringToBinaryOp(Tkn()->Spell());

        if (std::find(binaryOps.begin(), binaryOps.end(), op) == binaryOps.end())
            break;

        AcceptIt();

        /* Create left-to-right associative binary expression and parse next primary expression */
        auto binaryExpr = Make<BinaryExpr>();

        binaryExpr->lhsExpr = ast;
        binaryExpr->op      = op;
        binaryExpr->rhsExpr = parseSubExprFunc();

        /* New binary expression becomes the active AST node */
        ast = binaryExpr;
    }

    return ast;
}

void Parser::IncUnexpectedTokenCounter()
{
    /* Increment counter */
    ++unexpectedTokenCounter_;

    /* Track how many errors of this kind happend without a single accepted token */
    if (unexpectedTokenCounter_ > unexpectedTokenLimit_)
        reportHandler_.SubmitReport(true, ReportTypes::Error, R_Error, R_TooManySyntaxErrors);
}

void Parser::AssertTokenType(const Tokens type)
{
    /* Check if token type is unexpected */
    while (tkn_->Type() != type)
    {
        /* Increment unexpected token counter */
        IncUnexpectedTokenCounter();

        /* Submit error */
        ErrorUnexpected(type);

        /* Ignore unexpected token to produce further reports */
        AcceptIt();
    }
}

void Parser::AssertTokenSpell(const std::string& spell)
{
    /* Check if token spelling is unexpected */
    while (tkn_->Spell() != spell)
    {
        /* Increment unexpected token counter */
        IncUnexpectedTokenCounter();

        /* Submit error */
        Error(R_UnexpectedTokenSpell(tkn_->Spell(), spell), true, false);

        /* Ignore unexpected token to produce further reports */
        AcceptIt();
    }
}


} // /namespace Xsc



// ================================================================================
