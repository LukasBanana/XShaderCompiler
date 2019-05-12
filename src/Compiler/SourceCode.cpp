/*
 * SourceCode.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SourceCode.h"
#include <algorithm>


namespace Xsc
{


SourceCode::SourceCode(const std::shared_ptr<std::istream>& stream) :
    stream_ { stream }
{
}

bool SourceCode::IsValid() const
{
    return (stream_ != nullptr && stream_->good());
}

char SourceCode::Next()
{
    /* Check if reader is at end-of-line */
    while (pos_.Column() >= currentLine_.size())
    {
        /* Check if end-of-file is reached */
        if (!IsValid() || stream_->eof())
            return 0;

        /* Read new line in source file */
        std::getline(*stream_, currentLine_);
        currentLine_ += '\n';
        pos_.IncRow();

        /* Store current line for later reports */
        lines_.push_back(currentLine_);
    }

    /* Increment column and return current character */
    auto chr = currentLine_[pos_.Column()];
    pos_.IncColumn();

    return chr;
}

// Builds the line marker for reports (e.g. "^~~~~~~")
static bool BuildLineMarker(
    const SourceArea& area, const std::string& lineIn, std::string& lineOut, std::string& markerOut)
{
    if (area.Pos().Column() >= lineIn.size() || area.Pos().Column() == 0 || area.Length() == 0)
        return false;

    /* Copy input line into output line */
    lineOut = lineIn;

    /* Replace all tabs with blanks after source area to make the line marker always fit ('\t' -> ' ') */
    for (auto i = static_cast<std::size_t>(area.Pos().Column()); i < lineOut.size(); ++i)
    {
        if (lineOut[i] == '\t')
            lineOut[i] = ' ';
    }

    /* Construct the space offset */
    markerOut = std::string(area.Pos().Column() - 1, ' ');

    for (std::size_t i = 0, n = markerOut.size(); i < n; ++i)
    {
        if (lineIn[i] == '\t')
            markerOut[i] = '\t';
    }

    /* Construct the marker */
    auto len = std::min(area.Length(), static_cast<unsigned int>(lineIn.size()) - area.Pos().Column());

    if (len > 0)
    {
        if (area.Offset() < len)
        {
            markerOut += std::string(area.Offset(), '~');
            markerOut += '^';
            markerOut += std::string(len - 1 - area.Offset(), '~');
        }
        else
            markerOut += std::string(len, '~');
        return true;
    }

    return false;
}

bool SourceCode::FetchLineMarker(const SourceArea& area, std::string& line, std::string& marker)
{
    if (area.Length() > 0)
    {
        auto row = area.Pos().Row();
        if (row == pos_.Row())
            return BuildLineMarker(area, Line(), line, marker);
        else if (row > 0)
            return BuildLineMarker(area, GetLine(static_cast<std::size_t>(row - 1)), line, marker);
    }
    return false;
}

void SourceCode::NextSourceOrigin(const std::string& filename, int lineOffset)
{
    auto origin = std::make_shared<SourceOrigin>();
    {
        origin->filename    = filename;
        origin->lineOffset  = lineOffset;
    }
    pos_.SetOrigin(origin);
}

std::string SourceCode::Filename() const
{
    if (auto origin = pos_.GetOrigin())
        return origin->filename;
    else
        return "";
}


/*
 * ======= Private: =======
 */

std::string SourceCode::GetLine(std::size_t lineIndex) const
{
    return (lineIndex < lines_.size() ? lines_[lineIndex] : "");
}


} // /namespace Xsc



// ================================================================================
