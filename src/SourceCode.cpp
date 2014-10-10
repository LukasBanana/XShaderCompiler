/*
 * SourceCode.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SourceCode.h"


namespace HTLib
{


SourceCode::SourceCode(std::istream& stream) :
    stream_(&stream)
{
}

char SourceCode::Next()
{
    if (!stream_ || !stream_->good())
        return 0;

    /* Check if reader is at end-of-line */
    while (pos_.Column() >= line_.size())
    {
        /* Read new line in source file */
        std::getline(*stream_, line_);
        line_ += '\n';
        pos_.IncRow();

        /* Check if end-of-file is reached */
        if (stream_->eof())
            return 0;
    }

    /* Increment column and return current character */
    auto chr = line_[pos_.Column()];
    pos_.IncColumn();

    return chr;
}


} // /namespace HTLib



// ================================================================================