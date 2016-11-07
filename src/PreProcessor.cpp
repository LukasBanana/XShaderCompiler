/*
 * PreProcessor.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "PreProcessor.h"
#include <sstream>


namespace HTLib
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

PreProcessor::MacroPtr PreProcessor::MakeMacro(const std::string& ident)
{
    /* Make new symbol */
    auto symbol = std::make_shared<Macro>();
    macros_[ident] = symbol;
    return symbol;
}

bool PreProcessor::IsDefined(const std::string& ident) const
{
    return (macros_.find(ident) != macros_.end());
}

bool PreProcessor::CompareTokenStrings(const TokenString& lhs, const TokenString& rhs) const
{
    auto IsTokenOfInteres = [](const std::vector<TokenPtr>::const_iterator& it)
    {
        auto type = (*it)->Type();
        return (type != Tokens::Comment && type != Tokens::WhiteSpaces && type != Tokens::NewLines);
    };

    auto NextStringToken = [&](const TokenString& tokenString, std::vector<TokenPtr>::const_iterator& it)
    {
        do
        {
            ++it;
        }
        while (it != tokenString.end() && !IsTokenOfInteres(it));
    };

    /* Get first tokens */
    auto lhsIt = lhs.begin(), rhsIt = rhs.begin();

    if (lhsIt != lhs.end() && !IsTokenOfInteres(lhsIt))
        NextStringToken(lhs, lhsIt);
    if (rhsIt != rhs.end() && !IsTokenOfInteres(rhsIt))
        NextStringToken(rhs, rhsIt);

    /* Check if all tokens of interest are equal in both strings */
    for (; (lhsIt != lhs.end() && rhsIt != rhs.end()); NextStringToken(lhs, lhsIt), NextStringToken(rhs, rhsIt))
    {
        auto lhsTkn = lhsIt->get();
        auto rhsTkn = rhsIt->get();

        /* Compare types */
        if (lhsTkn->Type() != rhsTkn->Type())
            return false;

        /* Compare values */
        if (lhsTkn->Spell() != rhsTkn->Spell())
            return false;
    }

    /* Check if both strings reached the end */
    return (lhsIt == lhs.end() && rhsIt == rhs.end());
}

void PreProcessor::OutputTokenString(const TokenString& tokenString)
{
    for (const auto& tkn : tokenString)
        *output_ << tkn->Spell();
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

//UNUSED
void PreProcessor::SkipToNextLine()
{
    while (!Is(Tokens::NewLines))
        AcceptIt();
    AcceptIt();
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
                switch (Type())
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
                if (Type() == Tokens::Directive)
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
    auto ident = Accept(Tokens::Ident)->Spell();

    /* Search for defined symbol */
    auto it = macros_.find(ident);
    if (it != macros_.end())
    {
        auto& symbol = *it->second;

        if (!symbol.parameters.empty())
        {
            //TODO...
        }
        else
            OutputTokenString(symbol.tokenString);
    }
    else
        *output_ << ident;
}

void PreProcessor::ParseMisc()
{
    *output_ << AcceptIt()->Spell();
}

void PreProcessor::ParseDirective()
{
    /* Parse pre-processor directive */
    auto tkn = Accept(Tokens::Directive);
    auto directive = tkn->Spell();

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
        ParseDirectiveError(tkn);
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

// '#define' IDENT ( '(' IDENT+ ')' )? (TOKEN-STRING)?
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

    /* Make new defined symbol */
    auto symbol = MakeMacro(ident);

    /* Ignore white spaces and check for end of line */
    IgnoreWhiteSpaces(false);
    if (Is(Tokens::NewLines))
        return;

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

    /* Parse optional value */
    symbol->tokenString = ParseTokenString();

    /* Now compare previous and new definition */
    if (previousMacro)
    {
        /* Compare parameters */
        //TODO...

        /* Compare values */
        if (CompareTokenStrings(previousMacro->tokenString, symbol->tokenString))
            Warning("redefinition of symbol \"" + ident + "\"", identTkn.get());
        else
            Error("redefinition of symbol \"" + ident + "\" with mismatch", identTkn.get());
    }
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

void PreProcessor::ParseDirectiveInclude()
{
    /* Parse filename string literal */
    IgnoreWhiteSpaces(false);
    auto filename = Accept(Tokens::StringLiteral)->Spell();
    
    /* Check if filename has already been marked as 'once included' */
    if (onceIncluded_.find(filename) == onceIncluded_.end())
    {
        /* Open source code */
        std::unique_ptr<std::istream> includeStream;

        try
        {
            includeStream = includeHandler_.Include(filename);
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

// '#if' CONSTANT-EXPRESSION
void PreProcessor::ParseDirectiveIf(bool skipEvaluation)
{
    //todo...
}

// '#ifdef' IDENT
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

// '#ifndef' IDENT
void PreProcessor::ParseDirectiveIfndef(bool skipEvaluation)
{
    auto tkn = GetScanner().PreviousToken();

    /* Parse identifier */
    IgnoreWhiteSpaces(false);
    auto ident = Accept(Tokens::Ident)->Spell();
    
    /* Push new if-block activation (with 'not defined' condition) */
    PushIfBlock(tkn, !IsDefined(ident));
}

// '#elif CONSTANT-EXPRESSION'
void PreProcessor::ParseDirectiveElif(bool skipEvaluation)
{
    /* Check if '#endif'-directive is expected */
    if (TopIfBlock().expectEndif)
        Error("expected '#endif'-directive after previous '#else', but got '#elif'", true, HLSLErr::ERR_ELIF_ELSE);

    //todo...
}

// '#else'
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

// '#endif'
void PreProcessor::ParseDirectiveEndif()
{
    /* Only pop if-block from top of the stack */
    PopIfBlock();
}

// '#pragma' TOKEN-STRING
// see https://msdn.microsoft.com/de-de/library/windows/desktop/dd607351(v=vs.85).aspx
void PreProcessor::ParseDirectivePragma()
{
    /* Parse pragma command identifier */
    IgnoreWhiteSpaces(false);
    auto command = Accept(Tokens::Ident)->Spell();

    if (command == "once")
    {
        /* Mark current filename as 'once included' (but not for the main file) */
        auto filename = GetCurrentFilename();
        if (!filename.empty())
            onceIncluded_.insert(std::move(filename));
    }
    else
        Warning("unknown pragma command: \"" + command + "\"");
}

// '#line' NUMBER STRING-LITERAL?
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

// '#error' TOKEN-STRING
void PreProcessor::ParseDirectiveError(const TokenPtr& directiveToken)
{
    auto pos = directiveToken->Pos();

    /* Parse token string */
    auto tokenString = ParseTokenString();
    
    /* Convert token string into error message */
    std::string errorMsg;

    for (const auto& tkn : tokenString)
    {
        if (tkn->Type() == Tokens::StringLiteral)
            errorMsg += '\"' + tkn->Spell() + '\"';
        else
            errorMsg += tkn->Spell();
    }

    throw Report(Report::Types::Error, "error (" + pos.ToString() + ") : " + errorMsg);
}

PreProcessor::TokenString PreProcessor::ParseTokenString()
{
    TokenString tokenString;

    IgnoreWhiteSpaces(false);

    while (!Is(Tokens::NewLines))
    {
        switch (Type())
        {
            case Tokens::LineBreak:
                AcceptIt();
                IgnoreWhiteSpaces(false);
                while (Is(Tokens::NewLines))
                    tokenString.push_back(AcceptIt());
                break;

            default:
                tokenString.push_back(AcceptIt());
                break;
        }
    }

    return tokenString;
}


} // /namespace HTLib



// ================================================================================