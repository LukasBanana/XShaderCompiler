/*
 * IndentHandler.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/IndentHandler.h>


namespace Xsc
{


IndentHandler::IndentHandler(const std::string& initialIndent) :
    indent_{ initialIndent }
{
}

IndentHandler::~IndentHandler()
{
    // dummy
}

void IndentHandler::SetIndent(const std::string& indent)
{
    indent_ = indent;
}

void IndentHandler::IncIndent()
{
    /* Appemnd indentation string and store current size */
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