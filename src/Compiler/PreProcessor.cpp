/*
 * PreProcessor.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "PreProcessor.h"
#include "AST.h"
#include "ConstExprEvaluator.h"
#include <sstream>
#include <algorithm>


namespace Xsc
{


PreProcessor::PreProcessor(IncludeHandler& includeHandler, Log* log) :
    Parser          { log            },
    includeHandler_ { includeHandler }
{
}

std::shared_ptr<std::iostream> PreProcessor::Process(const SourceCodePtr& input)
{
    PushScannerSource(input);

    output_ = std::make_shared<std::stringstream>();

    try
    {
        ParseProgram();

        if (!GetReportHandler().HasErros())
            return output_;
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

ScannerPtr PreProcessor::MakeScanner()
{
    return std::make_shared<PreProcessorScanner>(GetLog());
}

bool PreProcessor::IsDefined(const std::string& ident) const
{
    return (macros_.find(ident) != macros_.end());
}

void PreProcessor::PushIfBlock(const TokenPtr& directiveToken, bool active, bool expectEndif)
{
    IfBlock ifBlock;
    {
        ifBlock.directiveToken  = directiveToken;
        ifBlock.directiveSource = GetScanner().GetSharedSource();
        ifBlock.active          = (TopIfBlock().active && active);
        ifBlock.expectEndif     = expectEndif;
    }
    ifBlockStack_.push(ifBlock);
}

void PreProcessor::PopIfBlock()
{
    if (ifBlockStack_.empty())
        Error("missing '#if'-directive to closing '#endif', '#else', or '#elif'", true, HLSLErr::ERR_ENDIF);
    else
        ifBlockStack_.pop();
}

PreProcessor::IfBlock PreProcessor::TopIfBlock() const
{
    return (ifBlockStack_.empty() ? IfBlock() : ifBlockStack_.top());
}

TokenPtrString PreProcessor::ExpandMacro(const Macro& macro, const std::vector<TokenPtrString>& arguments)
{
    TokenPtrString expandedString;

    if (macro.parameters.size() != arguments.size())
        return expandedString;
    
    auto ExpandTokenString = [&](const Token& tkn) -> bool
    {
        /* Check if current token is an identifier which matches one of the parameters of the macro */
        if (tkn.Type() == Tokens::Ident)
        {
            auto ident = tkn.Spell();
            for (std::size_t i = 0; i < macro.parameters.size(); ++i)
            {
                if (ident == macro.parameters[i])
                {
                    /* Expand identifier by argument token string */
                    expandedString.PushBack(arguments[i]);
                    return true;
                }
            }
        }
        else if (tkn.Type() == Tokens::Directive)
        {
            auto ident = tkn.Spell();
            for (std::size_t i = 0; i < macro.parameters.size(); ++i)
            {
                if (ident == macro.parameters[i])
                {
                    /* Expand identifier by converting argument token string to string literal */
                    std::stringstream stringLiteral;
                    stringLiteral << arguments[i];
                    expandedString.PushBack(Make<Token>(Tokens::StringLiteral, stringLiteral.str()));
                    return true;
                }
            }
        }
        return false;
    };

    for (const auto& tkn : macro.tokenString.GetTokens())
    {
        if (!ExpandTokenString(*tkn))
            expandedString.PushBack(tkn);
    }

    return expandedString;
}

ExprPtr PreProcessor::BuildBinaryExprTree(std::vector<ExprPtr>& exprs, std::vector<BinaryOp>& ops)
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

/* === Parse functions === */

void PreProcessor::ParseProgram()
{
    /* Parse entire program */
    do
    {
        while (!Is(Tokens::EndOfStream))
        {
            if (TopIfBlock().active)
            {
                /* Parse active block */
                switch (TknType())
                {
                    case Tokens::Directive:
                        ParseDirective();
                        break;
                    case Tokens::Comment:
                        ParesComment();
                        break;
                    case Tokens::Ident:
                        ParseIdent();
                        break;
                    default:
                        ParseMisc();
                        break;
                }
            }
            else
            {
                /* On an inactive if-block: parse only '#if'-directives or skip to next line */
                if (TknType() == Tokens::Directive)
                    ParseAnyIfDirectiveAndSkipValidation();
                else
                    AcceptIt();
            }
        }
    }
    while (PopScannerSource());

    /* Check for incomplete '#if'-scopes */
    while (!ifBlockStack_.empty())
    {
        const auto& ifBlock = ifBlockStack_.top();
        GetReportHandler().Error(
            "missing '#endif'-directive for open '#if', '#ifdef', or '#ifndef'",
            ifBlock.directiveSource.get(),
            ifBlock.directiveToken->Area()
        );
        ifBlockStack_.pop();
    }
}

void PreProcessor::ParesComment()
{
    *output_ << Accept(Tokens::Comment)->Spell();
}

void PreProcessor::ParseIdent()
{
    *output_ << ParseIdentAsTokenString();
}

TokenPtrString PreProcessor::ParseIdentAsTokenString()
{
    TokenPtrString tokenString;

    /* Parse identifier */
    auto identTkn = Accept(Tokens::Ident);
    auto ident = identTkn->Spell();

    /* Search for defined macro */
    auto it = macros_.find(ident);
    if (it != macros_.end())
    {
        /* Perform macro expansion */
        auto& macro = *it->second;
        if (!macro.parameters.empty())
            tokenString.PushBack(ParseIdentArgumentsForMacro(identTkn, macro));
        else
            tokenString.PushBack(macro.tokenString);
    }
    else
        tokenString.PushBack(identTkn);

    return tokenString;
}

TokenPtrString PreProcessor::ParseIdentArgumentsForMacro(const TokenPtr& identToken, const Macro& macro)
{
    /* Parse argument list begin */
    IgnoreWhiteSpaces();

    if (!Is(Tokens::LBracket))
    {
        Error(
            ( "macro \"" + identToken->Spell() + "\" requires an argument list of " +
              std::to_string(macro.parameters.size()) + " parameter(s)" ),
            identToken.get()
        );
    }

    AcceptIt();
    IgnoreWhiteSpaces();

    /* Parse all arguments */
    std::vector<TokenPtrString> arguments;

    while (!Is(Tokens::RBracket))
    {
        arguments.push_back(ParseArgumentTokenString());
        if (Is(Tokens::Comma))
            AcceptIt();
    }

    AcceptIt();

    /* Check compatability of parameter count to macro */
    if (arguments.size() != macro.parameters.size())
    {
        std::string errorMsg;

        if (arguments.size() > macro.parameters.size())
            errorMsg = "too many";
        if (arguments.size() < macro.parameters.size())
            errorMsg = "too few";

        errorMsg += " arguments for macro \"" + identToken->Spell() + "\"";
        errorMsg += " (expected " + std::to_string(macro.parameters.size()) + " but got " + std::to_string(arguments.size()) + ")";

        Error(errorMsg, identToken.get());
    }

    /* Perform macro expansion */
    return ExpandMacro(macro, arguments);
}

void PreProcessor::ParseMisc()
{
    *output_ << AcceptIt()->Spell();
}

void PreProcessor::ParseDirective()
{
    /* Parse pre-processor directive */
    auto directive = Accept(Tokens::Directive)->Spell();

    if (directive == "define")
        ParseDirectiveDefine();
    else if (directive == "undef")
        ParseDirectiveUndef();
    else if (directive == "include")
        ParseDirectiveInclude();
    else if (directive == "if")
        ParseDirectiveIf();
    else if (directive == "ifdef")
        ParseDirectiveIfdef();
    else if (directive == "ifndef")
        ParseDirectiveIfndef();
    else if (directive == "elif")
        ParseDirectiveElif();
    else if (directive == "else")
        ParseDirectiveElse();
    else if (directive == "endif")
        ParseDirectiveEndif();
    else if (directive == "pragma")
        ParseDirectivePragma();
    else if (directive == "line")
        ParseDirectiveLine();
    else if (directive == "error")
        ParseDirectiveError();
    else
        Error("unknown preprocessor directive: \"" + directive + "\"");
}

void PreProcessor::ParseAnyIfDirectiveAndSkipValidation()
{
    /* Parse pre-processor directive */
    auto tkn = Accept(Tokens::Directive);
    auto directive = tkn->Spell();

    if (directive == "if")
        ParseDirectiveIf(true);
    else if (directive == "ifdef")
        ParseDirectiveIfdef(true);
    else if (directive == "ifndef")
        ParseDirectiveIfndef(true);
    else if (directive == "elif")
        ParseDirectiveElif(true);
    else if (directive == "else")
        ParseDirectiveElse();
    else if (directive == "endif")
        ParseDirectiveEndif();
}

// '#' 'define' IDENT ( '(' IDENT+ ')' )? (TOKEN-STRING)?
void PreProcessor::ParseDirectiveDefine()
{
    /* Parse identifier */
    IgnoreWhiteSpaces(false);

    auto identTkn = Accept(Tokens::Ident);
    auto ident = identTkn->Spell();

    /* Check if identifier is already defined */
    MacroPtr previousMacro;
    auto previousMacroIt = macros_.find(ident);
    if (previousMacroIt != macros_.end())
        previousMacro = previousMacroIt->second;

    /* Make new macro symbol */
    auto symbol = std::make_shared<Macro>();

    /* Parse optional parameters */
    if (Is(Tokens::LBracket))
    {
        AcceptIt();
        IgnoreWhiteSpaces(false);

        if (!Is(Tokens::RBracket))
        {
            while (true)
            {
                /* Parse next parameter identifier */
                IgnoreWhiteSpaces(false);
                auto paramIdent = Accept(Tokens::Ident)->Spell();
                IgnoreWhiteSpaces(false);

                symbol->parameters.push_back(paramIdent);

                /* Check if parameter list is finished */
                if (!Is(Tokens::Comma))
                    break;

                AcceptIt();
            }
        }
        
        Accept(Tokens::RBracket);
    }

    /* Ignore white spaces and check for end of line */
    IgnoreWhiteSpaces(false);
    if (Is(Tokens::NewLines))
        return;

    /* Parse optional value */
    symbol->tokenString = ParseDirectiveTokenString();

    /* Now compare previous and new definition */
    if (previousMacro)
    {
        /* Compare parameters */
        //TODO...

        /* Compare values */
        if (previousMacro->tokenString == symbol->tokenString)
            Warning("redefinition of symbol \"" + ident + "\"", identTkn.get());
        else
            Error("redefinition of symbol \"" + ident + "\" with mismatch", identTkn.get());
    }

    /* Register symbol as new macro */
    macros_[ident] = symbol;
}

void PreProcessor::ParseDirectiveUndef()
{
    /* Parse identifier */
    IgnoreWhiteSpaces(false);
    auto ident = Accept(Tokens::Ident)->Spell();

    /* Remove symbol */
    auto it = macros_.find(ident);
    if (it != macros_.end())
        macros_.erase(it);
    else
        Warning("failed to undefine symbol \"" + ident + "\"");
}

// '#' 'include' ('<' TOKEN-STRING '>' | STRING-LITERAL)
void PreProcessor::ParseDirectiveInclude()
{
    bool useSearchPaths = false;
    std::string filename;

    /* Parse filename */
    IgnoreWhiteSpaces(false);

    if (Is(Tokens::BinaryOp, "<"))
    {
        /* Parse filename from token string */
        AcceptIt();

        useSearchPaths = true;
        while (!Is(Tokens::BinaryOp, ">"))
            filename += AcceptIt()->Spell();

        AcceptIt();
    }
    else
    {
        /* Parse filename from string literal */
        filename = Accept(Tokens::StringLiteral)->Spell();
    }

    /* Check if filename has already been marked as 'once included' */
    if (onceIncluded_.find(filename) == onceIncluded_.end())
    {
        /* Open source code */
        std::unique_ptr<std::istream> includeStream;

        try
        {
            includeStream = includeHandler_.Include(filename, useSearchPaths);
        }
        catch (const std::exception& e)
        {
            Error(e.what());
        }

        /* Push scanner soruce for include file */
        auto sourceCode = std::make_shared<SourceCode>(std::move(includeStream));
        PushScannerSource(sourceCode, filename);
    }
}

// '#' 'if' CONSTANT-EXPRESSION
void PreProcessor::ParseDirectiveIf(bool skipEvaluation)
{
    /* Parse condition */
    ParseDirectiveIfOrElifCondition(skipEvaluation);
}

// '#' 'ifdef' IDENT
void PreProcessor::ParseDirectiveIfdef(bool skipEvaluation)
{
    auto tkn = GetScanner().PreviousToken();

    if (skipEvaluation)
    {
        /* Push new if-block activation (and skip evaluation, due to currently inactive block) */
        PushIfBlock(tkn);
    }
    else
    {
        /* Parse identifier */
        IgnoreWhiteSpaces(false);
        auto ident = Accept(Tokens::Ident)->Spell();

        /* Push new if-block activation (with 'defined' condition) */
        PushIfBlock(tkn, IsDefined(ident));
    }
}

// '#' 'ifndef' IDENT
void PreProcessor::ParseDirectiveIfndef(bool skipEvaluation)
{
    auto tkn = GetScanner().PreviousToken();

    /* Parse identifier */
    IgnoreWhiteSpaces(false);
    auto ident = Accept(Tokens::Ident)->Spell();
    
    /* Push new if-block activation (with 'not defined' condition) */
    PushIfBlock(tkn, !IsDefined(ident));
}

// '#' 'elif CONSTANT-EXPRESSION'
void PreProcessor::ParseDirectiveElif(bool skipEvaluation)
{
    /* Check if '#endif'-directive is expected */
    if (TopIfBlock().expectEndif)
        Error("expected '#endif'-directive after previous '#else', but got '#elif'", true, HLSLErr::ERR_ELIF_ELSE);

    /* Pop if-block and push new if-block in the condition-parse function */
    PopIfBlock();

    /* Parse condition */
    ParseDirectiveIfOrElifCondition(skipEvaluation);
}

void PreProcessor::ParseDirectiveIfOrElifCondition(bool skipEvaluation)
{
    auto tkn = GetScanner().PreviousToken();

    /*
    Parse condition token string, and wrap it inside a bracket expression
    to easier find the legal end of the expression during parsing.
    TODO: this is a work around to detect an illegal end of a constant expression.
    */
    TokenPtrString tokenString;
    tokenString.PushBack(Make<Token>(Tokens::LBracket, "("));
    tokenString.PushBack(ParseDirectiveTokenString(true));
    tokenString.PushBack(Make<Token>(Tokens::RBracket, ")"));

    /* Evalutate condition */
    Variant condition;

    PushTokenString(tokenString);
    {
        /* Build binary expression tree from token string */
        auto conditionExpr = ParseExpr();

        try
        {
            ConstExprEvaluator exprEval;
            condition = exprEval.EvaluateExpr(*conditionExpr);
        }
        catch (const std::exception& e)
        {
            Error(e.what(), tkn.get());
        }

        #if 0
        /* Check if token string has reached the end */
        auto tokenStringIt = GetScanner().TopTokenStringIterator();
        if (!tokenStringIt.ReachedEnd())
            Error("illegal end of constant expression", tokenStringIt->get());
        #endif
    }
    PopTokenString();

    /* Push new if-block */
    PushIfBlock(tkn, condition.ToBool());
}

// '#' 'else'
void PreProcessor::ParseDirectiveElse()
{
    auto tkn = TopIfBlock().directiveToken;

    /* Check if '#endif'-directive is expected */
    if (TopIfBlock().expectEndif)
        Error("expected '#endif'-directive after previous '#else', but got another '#else'", true, HLSLErr::ERR_ELSE_ELSE);

    /* Pop if-block and push new if-block with negated condition */
    auto elseCondition = !TopIfBlock().active;
    PopIfBlock();
    PushIfBlock(tkn, elseCondition, true);
}

// '#' 'endif'
void PreProcessor::ParseDirectiveEndif()
{
    /* Only pop if-block from top of the stack */
    PopIfBlock();
}

// '#' 'pragma' TOKEN-STRING
// see https://msdn.microsoft.com/de-de/library/windows/desktop/dd607351(v=vs.85).aspx
void PreProcessor::ParseDirectivePragma()
{
    auto tkn = GetScanner().PreviousToken();

    /* Parse pragma command identifier */
    IgnoreWhiteSpaces(false);

    /* Parse token string */
    auto tokenString = ParseDirectiveTokenString();

    if (!tokenString.Empty())
    {
        /* Check if first token in the token string is an identifier */
        auto tokenIt = tokenString.Begin();
        if ((*tokenIt)->Type() == Tokens::Ident)
        {
            auto command = (*tokenIt)->Spell();
            if (command == "once")
            {
                /* Mark current filename as 'once included' (but not for the main file) */
                auto filename = GetCurrentFilename();
                if (!filename.empty())
                    onceIncluded_.insert(std::move(filename));
            }
            else if (command == "message")
            {
                /* Parse message string */
                if ((*++tokenIt)->Type() == Tokens::StringLiteral)
                    GetReportHandler().SubmitReport(false, Report::Types::Info, "message", (*tokenIt)->Spell(), nullptr, (*tokenIt)->Area());
                else
                    ErrorUnexpected(Tokens::StringLiteral, tokenIt->get());
            }
            else if (command == "def" || command == "pack_matrix" || command == "warning")
            {
                Warning("pragma \"" + command + "\" can currently not be handled", tokenIt->get());
                return;
            }
            else
                Warning("unknown pragma: \"" + command + "\"", tokenIt->get());
        }
        else
            Warning("unexpected token in '#pragam'-directive", tokenIt->get());

        /* Check if there are remaining unused tokens in the token string */
        if (!(++tokenIt).ReachedEnd())
            Warning("remaining unhandled tokens in '#pragma'-directive", tokenIt->get());
    }
    else
        Warning("empty '#pragma'-directive", tkn.get());
}

// '#' 'line' NUMBER STRING-LITERAL?
void PreProcessor::ParseDirectiveLine()
{
    /* Parse line number */
    IgnoreWhiteSpaces(false);
    auto lineNumber = Accept(Tokens::IntLiteral)->Spell();

    /* Parse optional filename */
    IgnoreWhiteSpaces(false);

    if (Is(Tokens::StringLiteral))
    {
        auto filename = AcceptIt()->Spell();

        //TODO...
    }
}

// '#' 'error' TOKEN-STRING
void PreProcessor::ParseDirectiveError()
{
    auto tkn = GetScanner().PreviousToken();

    /* Parse token string */
    auto tokenString = ParseDirectiveTokenString();
    
    /* Convert token string into error message */
    std::string errorMsg;

    for (const auto& tkn : tokenString.GetTokens())
    {
        if (tkn->Type() == Tokens::StringLiteral)
            errorMsg += '\"' + tkn->Spell() + '\"';
        else
            errorMsg += tkn->Spell();
    }

    GetReportHandler().SubmitReport(true, Report::Types::Error, "error", errorMsg, GetScanner().Source(), tkn->Area());
}

ExprPtr PreProcessor::ParseExpr()
{
    return ParseLogicOrExpr();
}

/*
EXPR: EXPR (OPERATOR EXPR)*;
*/
ExprPtr PreProcessor::ParseAbstractBinaryExpr(const std::function<ExprPtr()>& parseFunc, const BinaryOpList& binaryOps)
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

ExprPtr PreProcessor::ParseLogicOrExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&PreProcessor::ParseLogicAndExpr, this), { BinaryOp::LogicalOr });
}

ExprPtr PreProcessor::ParseLogicAndExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&PreProcessor::ParseBitwiseOrExpr, this), { BinaryOp::LogicalAnd });
}

ExprPtr PreProcessor::ParseBitwiseOrExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&PreProcessor::ParseBitwiseXOrExpr, this), { BinaryOp::Or });
}

ExprPtr PreProcessor::ParseBitwiseXOrExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&PreProcessor::ParseBitwiseAndExpr, this), { BinaryOp::Xor });
}

ExprPtr PreProcessor::ParseBitwiseAndExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&PreProcessor::ParseEqualityExpr, this), { BinaryOp::And });
}

ExprPtr PreProcessor::ParseEqualityExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&PreProcessor::ParseRelationExpr, this), { BinaryOp::Equal, BinaryOp::NotEqual });
}

ExprPtr PreProcessor::ParseRelationExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&PreProcessor::ParseShiftExpr, this), { BinaryOp::Less, BinaryOp::LessEqual, BinaryOp::Greater, BinaryOp::GreaterEqual });
}

ExprPtr PreProcessor::ParseShiftExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&PreProcessor::ParseAddExpr, this), { BinaryOp::LShift, BinaryOp::RShift });
}

ExprPtr PreProcessor::ParseAddExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&PreProcessor::ParseSubExpr, this), { BinaryOp::Add });
}

ExprPtr PreProcessor::ParseSubExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&PreProcessor::ParseMulExpr, this), { BinaryOp::Sub });
}

ExprPtr PreProcessor::ParseMulExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&PreProcessor::ParseDivExpr, this), { BinaryOp::Mul });
}

ExprPtr PreProcessor::ParseDivExpr()
{
    return ParseAbstractBinaryExpr(std::bind(&PreProcessor::ParseValueExpr, this), { BinaryOp::Div, BinaryOp::Mod });
}

ExprPtr PreProcessor::ParseValueExpr()
{
    switch (TknType())
    {
        case Tokens::Ident:
        {
            /* Parse identifier without macro expansion (this already happend at this point) */
            auto ast = Make<VarAccessExpr>();
            ast->varIdent = Make<VarIdent>();
            ast->varIdent->ident = AcceptIt()->Spell();
            return ast;
        }
        break;

        case Tokens::UnaryOp:
        {
            /* Parse unary expression */
            auto ast = Make<UnaryExpr>();
            ast->op     = StringToUnaryOp(AcceptIt()->Spell());
            ast->expr   = ParseValueExpr();
            return ast;
        }
        break;

        case Tokens::BoolLiteral:
        case Tokens::IntLiteral:
        case Tokens::FloatLiteral:
        {
            /* Parse literal */
            auto ast = Make<LiteralExpr>();
            ast->type   = TknType();
            ast->value  = AcceptIt()->Spell();
            return ast;
        }
        break;

        case Tokens::LBracket:
        {
            /* Parse bracket expression */
            AcceptIt();
            auto ast = ParseExpr();
            Accept(Tokens::RBracket);
            return ast;
        }
        break;

        default:
        {
            ErrorUnexpected("expected constant expression");
        }
        break;
    }
    return nullptr;
}

TokenPtrString PreProcessor::ParseDirectiveTokenString(bool expandDefinedDirective)
{
    TokenPtrString tokenString;

    IgnoreWhiteSpaces(false);

    while (!Is(Tokens::NewLines))
    {
        switch (TknType())
        {
            case Tokens::LineBreak:
            {
                AcceptIt();
                IgnoreWhiteSpaces(false);
                while (Is(Tokens::NewLines))
                    tokenString.PushBack(AcceptIt());
            }
            break;

            case Tokens::Ident:
            {
                if (expandDefinedDirective && Tkn()->Spell() == "defined")
                {
                    /* Parse 'defined IDENT' or 'defined (IDENT)' */
                    AcceptIt();
                    IgnoreWhiteSpaces(false);

                    /* Parse macro identifier */
                    std::string macroIdent;
                    if (Is(Tokens::LBracket))
                    {
                        AcceptIt();
                        IgnoreWhiteSpaces(false);
                        macroIdent = Accept(Tokens::Ident)->Spell();
                        IgnoreWhiteSpaces(false);
                        Accept(Tokens::RBracket);
                    }
                    else
                        macroIdent = Accept(Tokens::Ident)->Spell();

                    /* Determine value of boolean literal ('true' if macro is defined, 'false' otherwise */
                    std::string booleanLiteral = (IsDefined(macroIdent) ? "true" : "false");

                    /* Generate new token for boolean literal (which is the replacement of the 'defined IDENT' directive) */
                    tokenString.PushBack(Make<Token>(Tokens::BoolLiteral, std::move(booleanLiteral)));
                }
                else
                {
                    /* Append identifier with macro expansion */
                    tokenString.PushBack(ParseIdentAsTokenString());
                }
            }
            break;

            default:
            {
                tokenString.PushBack(AcceptIt());
            }
            break;
        }
    }

    return tokenString;
}

TokenPtrString PreProcessor::ParseArgumentTokenString()
{
    TokenPtrString tokenString;

    int bracketLevel = 0;

    /* Parse tokens until the closing bracket ')' appears */
    while ( ( bracketLevel > 0 || !Is(Tokens::RBracket) ) && !Is(Tokens::Comma) )
    {
        /* Do not exit loop if a closing bracket ')' appears, which belongs to an inner opening bracket '(' */
        if (Is(Tokens::LBracket))
            ++bracketLevel;
        else if (bracketLevel > 0 && Is(Tokens::RBracket))
            --bracketLevel;

        /* Add token to token string */
        if (Is(Tokens::Ident))
            tokenString.PushBack(ParseIdentAsTokenString());
        else
            tokenString.PushBack(AcceptIt());
    }

    return tokenString;
}


} // /namespace Xsc



// ================================================================================
