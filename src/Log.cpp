/*
 * Log.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/Log.h>
#include <Xsc/ConsoleManip.h>
#include <iostream>


namespace Xsc
{


/*
 * Log class
 */

Log::Log() :
    indent_{ std::string(2, ' ') }
{
}

Log::~Log()
{
    // dummy
}

void Log::SetIndent(const std::string& indent)
{
    indent_ = indent;
}

void Log::IncIndent()
{
    /* Appemnd indentation string and store current size */
    indentFull_ += indent_;
    indentStack_.push(indent_.size());
}

void Log::DecIndent()
{
    if (!indentStack_.empty())
    {
        /* Reduce indentation string by previous size */
        auto size = indentStack_.top();
        indentFull_.resize(indentFull_.size() - size);
        indentStack_.pop();
    }
}


/*
 * StdLog class
 */

void StdLog::SumitReport(const Report& report)
{
    switch (report.Type())
    {
        case Report::Types::Info:
            infos_.push_back({ FullIndent(), report });
            break;
        case Report::Types::Warning:
            warnings_.push_back({ FullIndent(), report });
            break;
        case Report::Types::Error:
            errors_.push_back({ FullIndent(), report });
            break;
    }
}

void StdLog::PrintAll()
{
    PrintAndClearReports(infos_);
    PrintAndClearReports(warnings_, "WARNING(S)");
    PrintAndClearReports(errors_, "ERROR(S)");
}

using Colors = ConsoleManip::ColorFlags;

void StdLog::PrintReport(const IndentReport& r)
{
    /* Print report message */
    std::cout << r.indent;

    auto type = r.report.Type();
    const auto& msg = r.report.Message();

    if (type == Report::Types::Error)
    {
        ConsoleManip::ScopedColor highlight(std::cout, Colors::Red | Colors::Intens);
        std::cout << msg;
    }
    else if (type == Report::Types::Warning)
    {
        ConsoleManip::ScopedColor highlight(std::cout, Colors::Yellow);
        std::cout << msg;
    }
    else
        std::cout << msg;

    std::cout << std::endl;

    /* Print optional line and line-marker */
    if (r.report.HasLine())
    {
        const auto& line    = r.report.Line();
        const auto& marker  = r.report.Marker();

        /* Print line with color highlight for the occurrence */
        {
            ConsoleManip::ScopedColor highlight(std::cout, Colors::Green | Colors::Blue);

            std::cout << r.indent;

            auto pos = marker.find('^');
            if (pos != std::string::npos && pos < marker.size())
            {
                std::cout << line.substr(0, pos);
                {
                    ConsoleManip::ScopedColor highlight(std::cout, Colors::Cyan);
                    std::cout << line.substr(pos, marker.size() - pos);
                }
                if (marker.size() < line.size())
                    std::cout << line.substr(marker.size());
            }
            else
                std::cout << line;

            std::cout << std::endl;
        }

        /* Print line marker */
        {
            ConsoleManip::ScopedColor highlight(std::cout, Colors::Cyan);
            std::cout << r.indent << marker << std::endl;
        }
    }
}

void StdLog::PrintAndClearReports(IndentReportList& reports, const std::string& headline)
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
            PrintReport(r);

        reports.clear();
    }
}


} // /namespace Xsc



// ================================================================================