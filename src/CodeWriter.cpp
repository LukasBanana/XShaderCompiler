/*
 * CodeWriter.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CodeWriter.h"


namespace HTLib
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

void CodeWriter::BeginLine()
{
    stream_->write(indent_.c_str(), indent_.size());
}

void CodeWriter::EndLine()
{
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


} // /namespace HTLib



// ================================================================================