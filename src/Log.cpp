/*
 * Log.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <HT/Log.h>
#include <iostream>


namespace HTLib
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

void StdLog::PrintReport(const IndentReport& r)
{
    /* Print report with optional line and line-marker */
    std::cout << r.indent << r.report.Message() << std::endl;
    if (r.report.HasLine())
    {
        std::cout << r.indent << r.report.Line() << std::endl;
        std::cout << r.indent << r.report.Marker() << std::endl;
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


} // /namespace HTLib



// ================================================================================