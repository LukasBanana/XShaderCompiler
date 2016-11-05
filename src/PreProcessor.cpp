/*
 * PreProcessor.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "PreProcessor.h"


namespace HTLib
{


PreProcessor::PreProcessor(IncludeHandler& includeHandler, Log* log) :
    includeHandler_{ includeHandler }
{
}

std::shared_ptr<std::iostream> PreProcessor::Process(const std::shared_ptr<SourceCode>& input)
{


    return output_;
}


/*
 * ======= Private: =======
 */

void PreProcessor::ProcessLine(std::string& line)
{
    /* Search for '#' character */

    



}


} // /namespace HTLib



// ================================================================================