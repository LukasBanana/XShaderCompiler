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
    includeHandler_ { includeHandler },
    log_            { log            },
    scanner_        { log            }
{
}

std::shared_ptr<std::iostream> PreProcessor::Process(const std::shared_ptr<SourceCode>& input)
{
    output_ = std::make_shared<std::stringstream>();

    while (auto tkn = scanner_.Next())
    {
        if (tkn->Type() == Token::Types::Directive)
        {

        }
        else
        {

        }
    }

    return output_;
}


/*
 * ======= Private: =======
 */




} // /namespace HTLib



// ================================================================================