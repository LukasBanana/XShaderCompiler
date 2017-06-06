/*
 * IndentHandler.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/IndentHandler.h>


namespace Xsc
{


IndentHandler::IndentHandler(const std::string& initialIndent) :
    indent_ { initialIndent }
{
}

void IndentHandler::SetIndent(const std::string& indent)
{
    indent_ = indent;
}

void IndentHandler::IncIndent()
{
    /* Append indentation string and store current size */
    indentFull_ += indent_;
    indentStack_.push(indent_.size());
}

void IndentHandler::DecIndent()
{
    if (!indentStack_.empty())
    {
        /* Reduce indentation string by previous size */
        auto size = indentStack_.top();
        indentFull_.resize(indentFull_.size() - size);
        indentStack_.pop();
    }
}


} // /namespace Xsc



// ================================================================================