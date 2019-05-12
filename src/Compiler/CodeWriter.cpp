/*
 * CodeWriter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CodeWriter.h"
#include "ReportIdents.h"
#include <algorithm>


namespace Xsc
{


void CodeWriter::OutputStream(std::ostream& stream)
{
    stream_ = &stream;
    if (!stream_->good())
        throw std::runtime_error(R_InvalidOutputStream);
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
    if (lineSeparationLevel_ > 0)
        FlushSeparatedLines(queuedSeparatedLines_);
    ++lineSeparationLevel_;
}

void CodeWriter::EndSeparation()
{
    if (lineSeparationLevel_ > 0)
    {
        FlushSeparatedLines(queuedSeparatedLines_);
        --lineSeparationLevel_;
    }
}

void CodeWriter::BeginLine()
{
    if (scopeState_.endLineQueued)
        EndLine();

    /* Check if previous line has ended */
    if (!openLine_)
    {
        /* Begin a new line */
        openLine_ = true;
        scopeState_.beginLineQueued = false;

        /* Write new line in queue */
        if (lineSeparationLevel_ > 0)
        {
            auto& lines = queuedSeparatedLines_.lines;
            lines.resize(lines.size() + 1);
        }

        /* Append indentation */
        if (CurrentOptions().enableIndent)
        {
            if (lineSeparationLevel_ > 0)
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
        scopeState_.endLineQueued = false;

        /* Append new-line character */
        if (lineSeparationLevel_ == 0)
            Out() << '\n';
    }
}

void CodeWriter::Write(const std::string& text)
{
    if (scopeState_.beginLineQueued)
        BeginLine();

    /*if (scopeState_.endLineQueued)
    {
        EndLine();
        BeginLine();
    }*/

    if (lineSeparationLevel_ > 0)
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

void CodeWriter::BeginScope(bool compact, bool endWithSemicolon, bool useBraces)
{
    if (compact)
    {
        /* Write new scope into same line */
        if (useBraces)
            Write(" { ");
        else
            Write(" ");
    }
    else if (newLineOpenScope)
    {
        /* Write new scope into new line, and increment indentation */
        if (IsOpenLine())
        {
            EndLine();
            if (useBraces)
                WriteLine("{");
            IncIndent();
            BeginLine();
        }
        else
        {
            if (useBraces)
                WriteLine("{");
            IncIndent();
        }
    }
    else
    {
        /* Write new scope into same line, and increment indentation */
        if (IsOpenLine())
        {
            if (useBraces)
                Write(" {");
            EndLine();
            IncIndent();
            BeginLine();
        }
        else
        {
            if (useBraces)
                WriteLine("{");
            IncIndent();
        }
    }

    scopeOptionStack_.push({ compact, endWithSemicolon, useBraces });
}

void CodeWriter::EndScope()
{
    auto opt = scopeOptionStack_.top();
    scopeOptionStack_.pop();

    if (opt.compact)
    {
        /* Write scope ending into same line */
        if (opt.useBraces)
            Write(" }");
        if (opt.endWithSemicolon)
            Write(";");
    }
    else if (newLineOpenScope)
    {
        if (IsOpenLine())
            EndLine();

        /* Decrement indentation, and write scope ending into new line */
        DecIndent();

        if (opt.useBraces || opt.endWithSemicolon)
        {
            BeginLine();
            {
                if (opt.useBraces)
                    Write("}");
                if (opt.endWithSemicolon)
                    Write(";");
            }
            EndLine();
        }
    }
    else
    {
        if (IsOpenLine())
            EndLine();

        /* Decrement indentation, and write scope ending into new line */
        DecIndent();
        scopeState_.beginLineQueued = true;

        if (opt.useBraces)
            Write("}");

        if (opt.endWithSemicolon)
        {
            Write(";");
            EndLine();
        }
        else
        {
            scopeState_.scopeCanContinue = true;
            scopeState_.endLineQueued = true;
        }

        scopeState_.scopeUsedBraces = opt.useBraces;
    }
}

void CodeWriter::ContinueScope()
{
    if (scopeState_.scopeCanContinue)
    {
        scopeState_.scopeCanContinue = false;
        scopeState_.endLineQueued = false;
        if (scopeState_.scopeUsedBraces)
            Write(" ");
    }
    else
        BeginLine();
}

void CodeWriter::Separator()
{
    if (lineSeparationLevel_ > 0)
    {
        /* Dummy call to "Write" function, to guarantee correct separator output */
        Write("");

        /* Insert a new separator */
        queuedSeparatedLines_.Current().Tab();
    }
}


/*
 * ======= Private: =======
 */

CodeWriter::Options CodeWriter::CurrentOptions() const
{
    return (!optionsStack_.empty() ? optionsStack_.top() : Options{});
}

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
                /* Write tabbed spaces (limit this to avoid bad_alloc exception on failure) */
                static const std::size_t tabLimit = 128;
                auto len = (offsets[i + 1] - offsets[i] - s.size());
                if (len > 0 && len <= tabLimit)
                    Out() << std::string(len, ' ');
            }
        }

        /* Append new-line if there are any parts, otherwise the line was not ended */
        if (!line.parts.empty())
            Out() << '\n';
    }

    /* Clear queue */
    lineQueue.lines.clear();
}


/*
 * SeparatedLine
 */

void CodeWriter::SeparatedLine::Tab()
{
    parts.resize(parts.size() + 1);
}

void CodeWriter::SeparatedLine::Offsets(std::vector<std::size_t>& offsets) const
{
    offsets.resize(std::max(offsets.size(), parts.size()));

    std::size_t shift = 0, i = 0;

    for (std::size_t pos = 0; i < parts.size(); ++i)
    {
        /* Remember last shift between previous and new offset */
        shift = pos - offsets[i];

        /* Set new offset */
        offsets[i] = pos;

        if (i + 1 < parts.size())
        {
            /* Set next offset by max{ previous_pos + part_size, next_offset } */
            pos = std::max(pos + parts[i].size(), offsets[i + 1] + shift);
        }
    }

    /* Shift all remaining offsets */
    for (; i < offsets.size(); ++i)
        offsets[i] += shift;
}

CodeWriter::SeparatedLine& CodeWriter::SeparatedLine::operator << (const std::string& text)
{
    if (parts.empty())
        Tab();
    parts.back() += text;
    return *this;
}


/*
 * SeparatedLineQueue
 */

CodeWriter::SeparatedLine& CodeWriter::SeparatedLineQueue::Current()
{
    if (lines.empty())
        lines.resize(lines.size() + 1);
    return lines.back();
}


} // /namespace Xsc



// ================================================================================