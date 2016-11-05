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
    includeHandler_ { includeHandler },
    scanner_        { log            }
{
}

std::shared_ptr<std::iostream> PreProcessor::Process(const std::shared_ptr<SourceCode>& input)
{
    if (!scanner_.ScanSource(input))
        return nullptr;

    AcceptIt();

    output_ = std::make_shared<std::stringstream>();

    try
    {
        ParseProgram();

        #ifdef _DEBUG
        auto output = output_->str();
        #endif

        return output_;
    }
    catch (const std::exception& err)
    {
        if (GetLog())
            GetLog()->Error(err.what());
    }

    return nullptr;
}


/*
 * ======= Private: =======
 */

Scanner& PreProcessor::GetScanner()
{
    return scanner_;
}

PreProcessor::DefinedSymbolPtr PreProcessor::MakeSymbol(const std::string& ident)
{
    /* Check if identifier is already defined */
    if (definedSymbols_.find(ident) != definedSymbols_.end())
        Warning("redefinition of symbol \"" + ident + "\"");

    /* Make new symbol */
    auto symbol = std::make_shared<DefinedSymbol>();
    definedSymbols_[ident] = symbol;

    return symbol;
}

void PreProcessor::ParseProgram()
{
    while (!Is(Tokens::EndOfStream))
    {
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
}

void PreProcessor::ParesComment()
{
    *output_ << "/* " << Accept(Tokens::Comment)->Spell() << " */";
}

void PreProcessor::ParseIdent()
{
    auto ident = Accept(Tokens::Ident)->Spell();

    /* Search for defined symbol */
    auto it = definedSymbols_.find(ident);
    if (it != definedSymbols_.end())
    {
        auto& symbol = *it->second;

        if (!symbol.parameters.empty())
        {
            //TODO...
        }
        else
            *output_ << symbol.value;
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
    else if (directive == "pragma")
        ParseDirectivePragma();
}

// #define IDENT ( '(' IDENT+ ')' )? (TOKEN-STRING)?
void PreProcessor::ParseDirectiveDefine()
{
    /* Parse identifier */
    IgnoreWhiteSpaces(false);
    auto ident = Accept(Tokens::Ident)->Spell();

    /* Make new defined symbol */
    auto symbol = MakeSymbol(ident);

    /* Ignore white spaces and check for end of line */
    IgnoreWhiteSpaces(false);
    if (Is(Tokens::NewLines))
        return;

    /* Parse optional parameters */
    if (Is(Tokens::LBracket))
    {
        AcceptIt();

        while (!Is(Tokens::RBracket))
        {
            //todo...
        }
        
        AcceptIt();
    }

    /* Parse optional value */
    IgnoreWhiteSpaces(false);

    while (!Is(Tokens::NewLines))
    {
        auto tkn = Tkn();
        switch (Type())
        {
            case Tokens::LineBreak:
                symbol->value += "\n";
                AcceptIt();
                IgnoreWhiteSpaces(false);
                IgnoreNewLines();
                break;

            case Tokens::Comment:
                symbol->value += "/* " + AcceptIt()->Spell() + " */";
                break;

            default:
                symbol->value += AcceptIt()->Spell();
                break;
        }
    }
}

void PreProcessor::ParseDirectiveUndef()
{
    /* Parse identifier */
    IgnoreWhiteSpaces(false);
    auto ident = Accept(Tokens::Ident)->Spell();

    /* Remove symbol */
    auto it = definedSymbols_.find(ident);
    if (it != definedSymbols_.end())
        definedSymbols_.erase(it);
    else
        Warning("failed to undefine symbol \"" + ident + "\"");
}

void PreProcessor::ParseDirectiveInclude()
{
    /* Parse filename string literal */
    IgnoreWhiteSpaces(false);
    auto filename = Accept(Tokens::StringLiteral)->Spell();
    


}

void PreProcessor::ParseDirectiveIf()
{
    //todo...
}

void PreProcessor::ParseDirectiveIfdef()
{
    //todo...
}

void PreProcessor::ParseDirectiveIfndef()
{
    //todo...
}

void PreProcessor::ParseDirectivePragma()
{
    //todo...
}


} // /namespace HTLib



// ================================================================================