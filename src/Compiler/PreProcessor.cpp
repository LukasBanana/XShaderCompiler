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


namespace Xsc
{


PreProcessor::PreProcessor(IncludeHandler& includeHandler, Log* log) :
    Parser          { log            },
    includeHandler_ { includeHandler }
{
}

std::shared_ptr<std::iostream> PreProcessor::Process(const SourceCodePtr& input, const std::string& filename)
{
    PushScannerSource(input, filename);

    output_ = std::make_shared<std::stringstream>();

    try
    {
        ParseProgram();
        return (GetReportHandler().HasErros() ? nullptr : output_);
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

    if (macro.parameters.size() > arguments.size())
        return expandedString;
    
    auto ExpandTokenString = [&](TokenPtrString::Container::const_iterator& tknIt, const TokenPtrString::Container::const_iterator& tknItEnd) -> bool
    {
        const auto& tkn = **tknIt;

        /* Check if current token is an identifier which matches one of the parameters of the macro */
        switch (tkn.Type())
        {
            case Tokens::Ident:
            {
                auto ident = tkn.Spell();
                if (ident == "__VA_ARGS__")
                {
                    /* Replace '__VA_ARGS__' identifier with all variadic arguments (i.e. all after the number of parameters) */
                    for (std::size_t i = macro.parameters.size(); i < arguments.size(); ++i)
                    {
                        expandedString.PushBack(arguments[i]);
                        if (i + 1 < arguments.size())
                            expandedString.PushBack(Make<Token>(Tokens::Comma, ","));
                    }
                    return true;
                }
                else
                {
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
            }
            break;

            case Tokens::Directive:
            {
                auto ident = tkn.Spell();
                for (std::size_t i = 0; i < macro.parameters.size(); ++i)
                {
                    if (ident == macro.parameters[i])
                    {
                        /* Expand identifier by converting argument token string to string literal */
                        std::stringstream stringLiteral;
                        stringLiteral << '\"' << arguments[i] << '\"';
                        expandedString.PushBack(Make<Token>(Tokens::StringLiteral, stringLiteral.str()));
                        return true;
                    }
                }
            }
            break;

            case Tokens::DirectiveConcat:
            {
                /* Rremove previous white spaces and comments */
                expandedString.TrimBack();

                /* Ignore concatenation token */
                ++tknIt;

                /* Ignore following white spaces and comments */
                while (tknIt != tknItEnd && !PreProcessorTokenOfInterestFunctor::IsOfInterest(*tknIt))
                    ++tknIt;
                
                /* Iterate one token back since it will be incremented in the outer for-loop */
                --tknIt;
                return true;
            }
            break;
        }

        /* Return false -> meaning this function has not added any tokens to the expanded string */
        return false;
    };

    const auto& tokens = macro.tokenString.GetTokens();
    for (auto it = tokens.begin(); it != tokens.end(); ++it)
    {
        if (!ExpandTokenString(it, tokens.end()))
            expandedString.PushBack(*it);
    }

    return expandedString;
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
            false,
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
        /*
        Interpret the macro usage only as plain identifier,
        if the macro has parameters, but the macro usage has no arguments
        */
        return identToken;
    }

    AcceptIt();
    IgnoreWhiteSpaces();

    /* Parse all arguments */
    std::vector<TokenPtrString> arguments;

    while (!Is(Tokens::RBracket))
    {
        auto arg = ParseArgumentTokenString();
        
        /* Remove white spaces and comments from argument */
        arg.TrimBack();
        arg.TrimFront();

        arguments.push_back(arg);

        if (Is(Tokens::Comma))
            AcceptIt();
    }

    AcceptIt();

    /* Check compatability of parameter count to macro */
    if ( ( !macro.varArgs && arguments.size() != macro.parameters.size() ) ||
         ( macro.varArgs && arguments.size() < macro.parameters.size() ) )
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
    {
        /* Ignore unknown preprocessor directives */
        Warning("unknown preprocessor directive: \"" + directive + "\"");
        ParseDirectiveTokenString();
    }
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
    IgnoreWhiteSpaces();

    auto identTkn = Accept(Tokens::Ident);
    auto ident = identTkn->Spell();

    /* Check if identifier is already defined */
    MacroPtr previousMacro;
    auto previousMacroIt = macros_.find(ident);
    if (previousMacroIt != macros_.end())
        previousMacro = previousMacroIt->second;

    /* Make new macro symbol */
    auto macro = std::make_shared<Macro>();

    /* Parse optional parameters */
    if (Is(Tokens::LBracket))
    {
        AcceptIt();
        IgnoreWhiteSpaces();

        if (!Is(Tokens::RBracket))
        {
            while (true)
            {
                /* Parse next parameter identifier or variadic argument specifier (i.e. IDENT or '...') */
                IgnoreWhiteSpaces();
                
                if (Is(Tokens::VarArg))
                {
                    AcceptIt();
                    macro->varArgs = true;
                    IgnoreWhiteSpaces();
                    break;
                }
                else
                {
                    /* Parse next parameter identifier */
                    auto paramIdent = Accept(Tokens::Ident)->Spell();
                    IgnoreWhiteSpaces();

                    macro->parameters.push_back(paramIdent);

                    /* Check if parameter list is finished */
                    if (!Is(Tokens::Comma))
                        break;

                    AcceptIt();
                }
            }
        }
        
        Accept(Tokens::RBracket);
    }

    /* Ignore white spaces and check for end of line */
    IgnoreWhiteSpaces();
    if (!Is(Tokens::NewLines))
    {
        /* Parse optional value */
        macro->tokenString = ParseDirectiveTokenString(false, true);

        /* Now compare previous and new definition */
        if (previousMacro)
        {
            /* Compare parameters and body */
            auto mismatchParam  = (previousMacro->parameters != macro->parameters || previousMacro->varArgs != macro->varArgs);
            auto mismatchBody   = (previousMacro->tokenString != macro->tokenString);

            /* Construct warning message */
            std::string warnMsg = "redefinition of macro \"" + ident + "\"";

            if (mismatchParam && mismatchBody)
                warnMsg += " with mismatch in parameter list and body definition";
            else if (mismatchParam)
                warnMsg += " with mismatch in parameter list";
            else if (mismatchBody)
                warnMsg += " with mismatch in body definition";

            Warning(warnMsg, identTkn.get());
        }
    }

    /* Register symbol as new macro */
    macros_[ident] = macro;
}

void PreProcessor::ParseDirectiveUndef()
{
    /* Parse identifier */
    IgnoreWhiteSpaces();
    auto ident = Accept(Tokens::Ident)->Spell();

    /* Remove macro */
    auto it = macros_.find(ident);
    if (it != macros_.end())
        macros_.erase(it);
    else
        Warning("failed to undefine macro \"" + ident + "\"");
}

// '#' 'include' ('<' TOKEN-STRING '>' | STRING-LITERAL)
void PreProcessor::ParseDirectiveInclude()
{
    bool useSearchPaths = false;
    std::string filename;

    /* Parse filename */
    IgnoreWhiteSpaces();

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
        filename = Accept(Tokens::StringLiteral)->SpellContent();
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
    /* Parse condExpr */
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
        IgnoreWhiteSpaces();
        auto ident = Accept(Tokens::Ident)->Spell();

        /* Push new if-block activation (with 'defined' condExpr) */
        PushIfBlock(tkn, IsDefined(ident));
    }
}

// '#' 'ifndef' IDENT
void PreProcessor::ParseDirectiveIfndef(bool skipEvaluation)
{
    auto tkn = GetScanner().PreviousToken();

    /* Parse identifier */
    IgnoreWhiteSpaces();
    auto ident = Accept(Tokens::Ident)->Spell();
    
    /* Push new if-block activation (with 'not defined' condExpr) */
    PushIfBlock(tkn, !IsDefined(ident));
}

// '#' 'elif CONSTANT-EXPRESSION'
void PreProcessor::ParseDirectiveElif(bool skipEvaluation)
{
    /* Check if '#endif'-directive is expected */
    if (TopIfBlock().expectEndif)
        Error("expected '#endif'-directive after previous '#else', but got '#elif'", true, HLSLErr::ERR_ELIF_ELSE);

    /* Pop if-block and push new if-block in the condExpr-parse function */
    PopIfBlock();

    /* Parse condExpr */
    ParseDirectiveIfOrElifCondition(skipEvaluation);
}

void PreProcessor::ParseDirectiveIfOrElifCondition(bool skipEvaluation)
{
    auto tkn = GetScanner().PreviousToken();

    /*
    Parse condExpr token string, and wrap it inside a bracket expression
    to easier find the legal end of the expression during parsing.
    TODO: this is a work around to detect an illegal end of a constant expression.
    */
    TokenPtrString tokenString;
    tokenString.PushBack(Make<Token>(Tokens::LBracket, "("));
    tokenString.PushBack(ParseDirectiveTokenString(true));
    tokenString.PushBack(Make<Token>(Tokens::RBracket, ")"));

    /* Evalutate condExpr */
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

    /* Pop if-block and push new if-block with negated condExpr */
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
    IgnoreWhiteSpaces();

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
                    GetReportHandler().SubmitReport(false, Report::Types::Info, "message", (*tokenIt)->SpellContent(), nullptr, (*tokenIt)->Area());
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
    IgnoreWhiteSpaces();
    auto lineNumber = Accept(Tokens::IntLiteral)->Spell();

    /* Parse optional filename */
    IgnoreWhiteSpaces();

    if (Is(Tokens::StringLiteral))
    {
        auto filename = AcceptIt()->SpellContent();

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
        errorMsg += tkn->Spell();

    GetReportHandler().SubmitReport(true, Report::Types::Error, "error", errorMsg, GetScanner().Source(), tkn->Area());
}

ExprPtr PreProcessor::ParseExpr()
{
    return ParseGenericExpr();
}

ExprPtr PreProcessor::ParsePrimaryExpr()
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
            auto ast = ParseGenericExpr();
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

TokenPtrString PreProcessor::ParseDirectiveTokenString(bool expandDefinedDirective, bool ignoreComments)
{
    TokenPtrString tokenString;

    IgnoreWhiteSpaces();

    while (!Is(Tokens::NewLines))
    {
        switch (TknType())
        {
            case Tokens::LineBreak:
            {
                AcceptIt();
                IgnoreWhiteSpaces();
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
                    IgnoreWhiteSpaces();

                    /* Parse macro identifier */
                    std::string macroIdent;
                    if (Is(Tokens::LBracket))
                    {
                        AcceptIt();
                        IgnoreWhiteSpaces();
                        macroIdent = Accept(Tokens::Ident)->Spell();
                        IgnoreWhiteSpaces();
                        Accept(Tokens::RBracket);
                    }
                    else
                        macroIdent = Accept(Tokens::Ident)->Spell();

                    /* Determine value of integer literal ('1' if macro is defined, '0' otherwise */
                    std::string intLiteral = (IsDefined(macroIdent) ? "1" : "0");

                    /* Generate new token for boolean literal (which is the replacement of the 'defined IDENT' directive) */
                    tokenString.PushBack(Make<Token>(Tokens::IntLiteral, std::move(intLiteral)));
                }
                else
                {
                    /* Append identifier with macro expansion */
                    tokenString.PushBack(ParseIdentAsTokenString());
                }
            }
            break;

            case Tokens::Comment:
            {
                if (ignoreComments)
                    AcceptIt();
                else
                    tokenString.PushBack(AcceptIt());
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

// Parse next argument as token string
// --> Parse until the closing ')' token or until the next ',' token for the next argument appears
TokenPtrString PreProcessor::ParseArgumentTokenString()
{
    TokenPtrString tokenString;

    int bracketLevel = 0;

    /* Parse tokens until the closing bracket ')' appears */
    while ( bracketLevel > 0 || ( !Is(Tokens::RBracket) && !Is(Tokens::Comma) ) )
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
