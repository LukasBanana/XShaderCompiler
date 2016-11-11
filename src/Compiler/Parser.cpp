/*
 * Parser.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Parser.h"
#include "AST.h"
#include <algorithm>


namespace Xsc
{


Parser::~Parser()
{
}


/*
 * ======= Protected: =======
 */

Parser::Parser(Log* log) :
    reportHandler_  { "syntax", log },
    log_            { log           }
{
}

/* ----- Report Handling ----- */

static SourceArea GetTokenArea(Token* tkn)
{
    return (tkn != nullptr ? tkn->Area() : SourceArea::ignore);
}

void Parser::Error(const std::string& msg, Token* tkn, const HLSLErr errorCode, bool breakWithExpection)
{
    /* Always break with an exception when the end of stream has been reached */
    if (tkn->Type() == Tokens::EndOfStream)
        breakWithExpection = true;

    /* Report error with the report handler */
    reportHandler_.Error(breakWithExpection, msg, GetScanner().Source(), GetTokenArea(tkn), errorCode);
}

void Parser::Error(const std::string& msg, bool prevToken, const HLSLErr errorCode, bool breakWithExpection)
{
    /* Get token and submit error */
    auto tkn = (prevToken ? GetScanner().PreviousToken().get() : GetScanner().ActiveToken().get());
    Error(msg, tkn, errorCode, breakWithExpection);
}

void Parser::ErrorUnexpected(const std::string& hint, Token* tkn, bool breakWithExpection)
{
    /* Construct error message */
    if (!tkn)
        tkn = tkn_.get();

    std::string msg = "unexpected token: " + Token::TypeToString(tkn->Type());

    if (!hint.empty())
        msg += " (" + hint + ")";

    Error(msg, tkn, HLSLErr::Unknown, breakWithExpection);
}

void Parser::ErrorUnexpected(const Tokens type, Token* tkn, bool breakWithExpection)
{
    auto typeName = Token::TypeToString(type);
    if (typeName.empty())
        ErrorUnexpected("", tkn, breakWithExpection);
    else
        ErrorUnexpected("expected: " + typeName, tkn, breakWithExpection);
}

void Parser::ErrorInternal(const std::string& msg, const std::string& procName)
{
    reportHandler_.Error(true, msg + " (in function: " + procName + ")");
}

void Parser::Warning(const std::string& msg, Token* tkn)
{
    reportHandler_.Warning(false, msg, GetScanner().Source(), GetTokenArea(tkn));
}

void Parser::Warning(const std::string& msg, bool prevToken)
{
    Warning(msg, prevToken ? GetScanner().PreviousToken().get() : GetScanner().ActiveToken().get());
}

/* ----- Scanner ----- */

void Parser::PushScannerSource(const SourceCodePtr& source, const std::string& filename)
{
    /* Set new filename to report handler */
    GetReportHandler().SetCurrentFilename(filename);

    /* Add current token to previous scanner */
    if (!scannerStack_.empty())
        scannerStack_.top().nextToken = tkn_;

    /* Make a new token scanner */
    auto scanner = MakeScanner();
    if (!scanner)
        throw std::runtime_error("failed to create token scanner");

    scannerStack_.push({ scanner, filename, nullptr });

    /* Start scanning */
    if (!scanner->ScanSource(source))
        throw std::runtime_error("failed to scan source code");

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

    /* Reset previous filename to report handler */
    GetReportHandler().SetCurrentFilename(scannerStack_.top().filename);

    /* Reset previous 'next token' */
    tkn_ = scannerStack_.top().nextToken;

    return (tkn_ != nullptr);
}

Scanner& Parser::GetScanner()
{
    if (scannerStack_.empty())
        throw std::runtime_error("missing token scanner");
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
    auto prevTkn = tkn_;
    tkn_ = GetScanner().Next();
    return prevTkn;
}

void Parser::PushTokenString(const TokenPtrString& tokenString)
{
    /* Push token string onto stack in the scanner and accept first token */
    GetScanner().PushTokenString(tokenString);
    AcceptIt();
}

void Parser::PopTokenString()
{
    /* Pop token string from the stack in the scanner */
    GetScanner().PopTokenString();
}

void Parser::IgnoreWhiteSpaces(bool includeNewLines)//, bool includeComments)
{
    while ( Is(Tokens::WhiteSpaces) || ( includeNewLines && Is(Tokens::NewLines) ) /*|| ( includeComments && Is(Tokens::Comment) )*/ )
        AcceptIt();
}

void Parser::IgnoreNewLines()
{
    while (Is(Tokens::NewLines))
        AcceptIt();
}

/* ----- Parsing ----- */

ExprPtr Parser::BuildBinaryExprTree(std::vector<ExprPtr>& exprs, std::vector<BinaryOp>& ops)
{
    if (exprs.empty())
        ErrorInternal("sub-expressions must not be empty", __FUNCTION__);

    if (exprs.size() > 1)
    {
        if (exprs.size() != ops.size() + 1)
            ErrorInternal("sub-expressions and operators have uncorrelated number of elements", __FUNCTION__);

        auto ast = Make<BinaryExpr>();

        /* Build right hand side */
        ast->rhsExpr    = exprs.back();
        ast->op         = ops.back();

        exprs.pop_back();
        ops.pop_back();

        /* Build left hand side of the tree */
        ast->lhsExpr = BuildBinaryExprTree(exprs, ops);

        return ast;
    }

    return exprs.front();
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
    ast->pos        = condExpr->pos;

    /* Parse expressions for 'then' and 'else' branches */
    Accept(Tokens::TernaryOp);
    ast->thenExpr = ParseGenericExpr();
    Accept(Tokens::Colon);
    ast->elseExpr = ParseGenericExpr();

    return ast;
}

// expr: expr (operator expr)*;
ExprPtr Parser::ParseAbstractBinaryExpr(const std::function<ExprPtr()>& parseFunc, const BinaryOpList& binaryOps)
{
    /* Parse sub expressions */
    std::vector<ExprPtr> exprs;
    std::vector<BinaryOp> ops;

    /* Parse primary expression */
    exprs.push_back(parseFunc());

    while (Is(Tokens::BinaryOp))
    {
        /* Parse binary operator */
        auto op = StringToBinaryOp(Tkn()->Spell());

        if (std::find(binaryOps.begin(), binaryOps.end(), op) == binaryOps.end())
            break;

        AcceptIt();
        ops.push_back(op);

        /* Parse next sub-expression */
        exprs.push_back(parseFunc());
    }

    /* Build (left-to-rigth) binary expression tree */
    return BuildBinaryExprTree(exprs, ops);
}

ExprPtr Parser::ParseLogicOrExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&Parser::ParseLogicAndExpr, this), { BinaryOp::LogicalOr });
}

ExprPtr Parser::ParseLogicAndExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&Parser::ParseBitwiseOrExpr, this), { BinaryOp::LogicalAnd });
}

ExprPtr Parser::ParseBitwiseOrExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&Parser::ParseBitwiseXOrExpr, this), { BinaryOp::Or });
}

ExprPtr Parser::ParseBitwiseXOrExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&Parser::ParseBitwiseAndExpr, this), { BinaryOp::Xor });
}

ExprPtr Parser::ParseBitwiseAndExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&Parser::ParseEqualityExpr, this), { BinaryOp::And });
}

ExprPtr Parser::ParseEqualityExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&Parser::ParseRelationExpr, this), { BinaryOp::Equal, BinaryOp::NotEqual });
}

ExprPtr Parser::ParseRelationExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&Parser::ParseShiftExpr, this), { BinaryOp::Less, BinaryOp::LessEqual, BinaryOp::Greater, BinaryOp::GreaterEqual });
}

ExprPtr Parser::ParseShiftExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&Parser::ParseAddExpr, this), { BinaryOp::LShift, BinaryOp::RShift });
}

ExprPtr Parser::ParseAddExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&Parser::ParseSubExpr, this), { BinaryOp::Add });
}

ExprPtr Parser::ParseSubExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&Parser::ParseMulExpr, this), { BinaryOp::Sub });
}

ExprPtr Parser::ParseMulExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&Parser::ParseDivExpr, this), { BinaryOp::Mul });
}

ExprPtr Parser::ParseDivExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&Parser::ParseValueExpr, this), { BinaryOp::Div, BinaryOp::Mod });
}

ExprPtr Parser::ParseValueExpr()
{
    return ParsePrimaryExpr();
}


/*
 * ======= Private: =======
 */

void Parser::IncUnexpectedTokenCounter(Token* tkn)
{
    /* Increment counter */
    ++unexpectedTokenCounter_;

    /* Track how many errors of this kind happend without a single accepted token */
    if (unexpectedTokenCounter_ > unexpectedTokenLimit_)
        reportHandler_.SubmitReport(true, Report::Types::Error, "error", "too many syntax errors");
}

void Parser::AssertTokenType(const Tokens type)
{
    /* Check if token type is unexpected */
    while (tkn_->Type() != type)
    {
        /* Increment unexpected token counter */
        IncUnexpectedTokenCounter(tkn_.get());

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
        IncUnexpectedTokenCounter(tkn_.get());

        /* Submit error */
        Error("unexpected token spelling '" + tkn_->Spell() + "' (expected '" + spell + "')", true, HLSLErr::Unknown, false);

        /* Ignore unexpected token to produce further reports */
        AcceptIt();
    }
}


} // /namespace Xsc



// ================================================================================
