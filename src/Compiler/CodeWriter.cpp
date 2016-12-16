/*
 * CodeWriter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CodeWriter.h"
#include <algorithm>


namespace Xsc
{


void CodeWriter::OutputStream(std::ostream& stream)
{
    stream_ = &stream;
    if (!stream_->good())
        throw std::runtime_error("invalid output stream");
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

void CodeWriter::BeginSeparation()
{
    if (lineSeparation_)
        EndSeparation();
    else
        lineSeparation_ = true;
}

void CodeWriter::EndSeparation()
{
    if (lineSeparation_)
    {
        FlushSeparatedLines(queuedSeparatedLines_);
        lineSeparation_ = false;
    }
}

void CodeWriter::BeginLine()
{
    /* Check if previous line has ended */
    if (!openLine_)
    {
        /* Begin a new line */
        openLine_ = true;

        /* Write new line in queue */
        if (lineSeparation_)
        {
            auto& lines = queuedSeparatedLines_.lines;
            lines.resize(lines.size() + 1);
        }

        /* Append indentation */
        if (CurrentOptions().enableIndent)
        {
            if (lineSeparation_)
                queuedSeparatedLines_.Current().indent = FullIndent();
            else
                Out() << FullIndent();
        }
    }
}

void CodeWriter::EndLine()
{
    /* Check if there is no open line */
    if (openLine_ && CurrentOptions().enableNewLine )
    {
        /* End current line */
        openLine_ = false;

        /* Append new-line character */
        if (!lineSeparation_)
            Out() << '\n';
    }
}

void CodeWriter::Write(const std::string& text)
{
    if (lineSeparation_)
    {
        /* Push text into queue */
        queuedSeparatedLines_.Current() << text;
    }
    else
    {
        /* Write text into output stream */
        Out() << text;
    }
}

void CodeWriter::WriteLine(const std::string& text)
{
    BeginLine();
    Write(text);
    EndLine();
}

void CodeWriter::Separator()
{
    if (lineSeparation_)
        queuedSeparatedLines_.Current().Tab();
}

CodeWriter::Options CodeWriter::CurrentOptions() const
{
    return (!optionsStack_.empty() ? optionsStack_.top() : Options());
}


/*
 * ======= Private: =======
 */

void CodeWriter::FlushSeparatedLines(SeparatedLineQueue& lineQueue)
{
    /* Determine all tab offsets */
    std::vector<std::size_t> offsets;
    for (const auto& line : lineQueue.lines)
        line.Offsets(offsets);

    /* Write all lines */
    for (const auto& line : lineQueue.lines)
    {
        Out() << line.indent;

        for (std::size_t i = 0; i < line.parts.size(); ++i)
        {
            /* Write line part */
            const auto& s = line.parts[i];
            Out() << s;

            if (i + 1 < line.parts.size())
            {
                /* Write tabbed spaces */
                auto len = (offsets[i + 1] - offsets[i] - s.size());
                if (len > 0)
                    Out() << std::string(len, ' ');
            }
        }

        Out() << '\n';
    }

    /* Clear queue */
    lineQueue.lines.clear();
}


/*
 * TabSeparatedLine
 */

void CodeWriter::SeparatedLine::Tab()
{
    parts.resize(parts.size() + 1);
}

void CodeWriter::SeparatedLine::Offsets(std::vector<std::size_t>& offsets) const
{
    offsets.resize(std::max(offsets.size(), parts.size()));

    for (std::size_t i = 0, pos = 0; i < parts.size(); ++i)
    {
        /* Set new offset */
        offsets[i] = pos;

        if (i + 1 < parts.size())
        {
            /* Set next offset by max{ previous_pos + part_size, next_offset } */
            pos = std::max(pos + parts[i].size(), offsets[i + 1]);
        }
    }
}

CodeWriter::SeparatedLine& CodeWriter::SeparatedLine::operator << (const std::string& text)
{
    if (parts.empty())
        Tab();
    parts.back() += text;
    return *this;
}


} // /namespace Xsc



// ================================================================================