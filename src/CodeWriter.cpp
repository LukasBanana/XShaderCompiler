/*
 * CodeWriter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CodeWriter.h"


namespace Xsc
{


CodeWriter::CodeWriter(const std::string& indentTab) :
    indentTab_{ indentTab }
{
}

void CodeWriter::OutputStream(std::ostream& stream)
{
    stream_ = &stream;
    if (!stream_->good())
        throw std::runtime_error("invalid output stream");
}

void CodeWriter::PushIndent()
{
    indent_ += indentTab_;
}

void CodeWriter::PopIndent()
{
    if (!indent_.empty())
        indent_.resize(indent_.size() - indentTab_.size());
}

void CodeWriter::PushOptions(const Options& options)
{
    optionsStack_.push(options);
}

void CodeWriter::PopOptions()
{
    if (!optionsStack_.empty())
        optionsStack_.pop();
}

void CodeWriter::BeginLine()
{
    if (CurrentOptions().enableTabs)
        stream_->write(indent_.c_str(), indent_.size());
}

void CodeWriter::EndLine()
{
    if (CurrentOptions().enableNewLine)
        (*stream_) << '\n';
}

void CodeWriter::Write(const std::string& text)
{
    stream_->write(text.c_str(), text.size());
}

void CodeWriter::WriteLine(const std::string& text)
{
    BeginLine();
    Write(text);
    EndLine();
}

CodeWriter::Options CodeWriter::CurrentOptions() const
{
    return !optionsStack_.empty() ? optionsStack_.top() : Options();
}


} // /namespace Xsc



// ================================================================================