/*
 * IndentHandler.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/IndentHandler.h>


namespace Xsc
{


struct IndentHandler::OpaqueData
{
    std::string                         indent;
    std::string                         indentFull;
    std::stack<std::string::size_type>  indentStack;
};

IndentHandler::IndentHandler(const std::string& initialIndent) :
    data_ { new OpaqueData() }
{
    data_->indent = initialIndent;
}

IndentHandler::~IndentHandler()
{
    delete data_;
}

void IndentHandler::SetIndent(const std::string& indent)
{
    data_->indent = indent;
}

void IndentHandler::IncIndent()
{
    /* Append indentation string and store current size */
    data_->indentFull += data_->indent;
    data_->indentStack.push(data_->indent.size());
}

void IndentHandler::DecIndent()
{
    if (!data_->indentStack.empty())
    {
        /* Reduce indentation string by previous size */
        auto size = data_->indentStack.top();
        data_->indentFull.resize(data_->indentFull.size() - size);
        data_->indentStack.pop();
    }
}

const std::string& IndentHandler::FullIndent() const
{
    return data_->indentFull;
}


} // /namespace Xsc



// ================================================================================