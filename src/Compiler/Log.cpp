/*
 * Log.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/Log.h>
#include <Xsc/ConsoleManip.h>
#include <iostream>
#include <algorithm>


namespace Xsc
{


/*
 * Internal types
 */

struct IndentReport
{
    std::string indent;
    Report      report;
};

using IndentReportList = std::vector<IndentReport>;


/*
 * Internal functions
 */

static void PrintMultiLineString(const std::string& s, const std::string& indent)
{
    /* Determine at which position the actual text begins (excluding the "error (X:Y) : " or the like) */
    auto textStartPos = s.find(" : ");

    if (textStartPos != std::string::npos)
        textStartPos += 3;
    else
        textStartPos = 0;

    std::string newLineIndent(textStartPos, ' ');

    std::size_t start = 0;
    bool useNewLineIndent = false;

    while (start < s.size())
    {
        /* Print indentation */
        std::cout << indent;

        if (useNewLineIndent)
            std::cout << newLineIndent;

        /* Print next line */
        auto end = s.find('\n', start);

        if (end != std::string::npos)
        {
            std::cout << s.substr(start, end - start);
            start = end + 1;
        }
        else
        {
            std::cout << s.substr(start);
            start = end;
        }

        std::cout << std::endl;

        useNewLineIndent = true;
    }
}

using Colors = ConsoleManip::ColorFlags;

static void PrintReport(const IndentReport& r, bool verbose)
{
    /* Print optional context description */
    if (verbose && !r.report.Context().empty())
        PrintMultiLineString(r.report.Context(), r.indent);

    /* Print report message */
    auto type = r.report.Type();
    const auto& msg = r.report.Message();

    if (type == ReportTypes::Error)
    {
        ConsoleManip::ScopedColor highlight(Colors::Red | Colors::Intens);
        PrintMultiLineString(msg, r.indent);
    }
    else if (type == ReportTypes::Warning)
    {
        ConsoleManip::ScopedColor highlight(Colors::Yellow);
        PrintMultiLineString(msg, r.indent);
    }
    else
        PrintMultiLineString(msg, r.indent);

    if (!verbose)
        return;

    /* Print optional line and line-marker */
    if (r.report.HasLine())
    {
        const auto& line = r.report.Line();
        const auto& mark = r.report.Marker();

        /* Print line with color highlight for the occurrence */
        {
            ConsoleManip::ScopedColor highlight(Colors::Green | Colors::Blue);

            std::cout << r.indent;

            std::size_t start = 0, end = 0;

            while ( end < mark.size() && ( start = mark.find_first_not_of(' ', end) ) != std::string::npos )
            {
                /* Write unhighlighted text */
                std::cout << line.substr(end, start - end);

                /* Write highlighted text */
                {
                    ConsoleManip::ScopedColor highlight(Colors::Cyan);

                    end = mark.find(' ', start);

                    if (end == std::string::npos)
                        end = std::min(line.size(), mark.size());

                    std::cout << line.substr(start, end - start);
                }
            }

            if (end < line.size())
                std::cout << line.substr(end);

            std::cout << std::endl;
        }

        /* Print line marker */
        if (!mark.empty())
        {
            ConsoleManip::ScopedColor highlight(Colors::Cyan);
            std::cout << r.indent << mark << std::endl;
        }
    }

    /* Print optional hints */
    for (const auto& hint : r.report.GetHints())
        std::cout << r.indent << hint << std::endl;
}

static void PrintAndClearReports(IndentReportList& reports, bool verbose, const std::string& headline = "")
{
    if (!reports.empty())
    {
        /* Print headline */
        if (!headline.empty())
        {
            auto s = std::to_string(reports.size()) + " " + headline;
            std::cout << s << std::endl;
            std::cout << std::string(s.size(), '-') << std::endl;
        }

        /* Print and clear reports */
        for (const auto& r : reports)
            PrintReport(r, verbose);

        reports.clear();
    }
}


/*
 * StdLog class
 */

struct StdLog::OpaqueData
{
    IndentReportList infos_;
    IndentReportList warnings_;
    IndentReportList errors_;
};

StdLog::StdLog() :
    data_ { new OpaqueData() }
{
}

StdLog::~StdLog()
{
    delete data_;
}

void StdLog::SubmitReport(const Report& report)
{
    switch (report.Type())
    {
        case ReportTypes::Info:
            data_->infos_.push_back({ FullIndent(), report });
            break;
        case ReportTypes::Warning:
            data_->warnings_.push_back({ FullIndent(), report });
            break;
        case ReportTypes::Error:
            data_->errors_.push_back({ FullIndent(), report });
            break;
    }
}

void StdLog::PrintAll(bool verbose)
{
    PrintAndClearReports(data_->infos_, verbose);
    PrintAndClearReports(data_->warnings_, verbose, (data_->warnings_.size() == 1 ? "WARNING" : "WARNINGS"));
    PrintAndClearReports(data_->errors_, verbose, (data_->errors_.size() == 1 ? "ERROR": "ERRORS"));
}


} // /namespace Xsc



// ================================================================================