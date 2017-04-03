/*
 * ReportHandler.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ReportHandler.h"
#include "ReportIdents.h"
#include "SourceCode.h"
#include <vector>


namespace Xsc
{


static std::vector<std::string> g_hintQueue;

ReportHandler::ReportHandler(const std::string& reportTypeName, Log* log) :
    reportTypeName_ { reportTypeName },
    log_            { log            }
{
}

void ReportHandler::Error(
    bool breakWithExpection, const std::string& msg, SourceCode* sourceCode, const SourceArea& area)
{
    SubmitReport(breakWithExpection, Report::Types::Error, (reportTypeName_ + " " + R_Error), msg, sourceCode, area);
}

void ReportHandler::Warning(
    bool breakWithExpection, const std::string& msg, SourceCode* sourceCode, const SourceArea& area)
{
    SubmitReport(breakWithExpection, Report::Types::Warning, R_Warning, msg, sourceCode, area);
}

void ReportHandler::SubmitReport(
    bool breakWithExpection, const Report::Types type, const std::string& typeName,
    const std::string& msg, SourceCode* sourceCode, const SourceArea& area)
{
    /* Check if error location has already been reported */
    if (!breakWithExpection && area.Pos().IsValid())
    {
        if (errorPositions_.find(area.Pos()) == errorPositions_.end())
            errorPositions_.insert(area.Pos());
        else
            return;
    }

    /* Initialize output message */
    auto outputMsg = typeName;
    
    if (type == Report::Types::Error)
        hasErrors_ = true;

    /* Add source position */
    if (area.Pos().IsValid())
    {
        outputMsg += " (";
        outputMsg += area.Pos().ToString();
        outputMsg += ") ";
    }
    else
        outputMsg += " ";

    outputMsg += ": ";

    /* Add actual report message */
    outputMsg += msg;

    /* Make report object */
    auto report = MakeReport(type, outputMsg, sourceCode, area);

    /* Move hint queue into report */
    report.TakeHints(std::move(g_hintQueue));

    /* Either throw or submit report */
    if (breakWithExpection)
        throw report;
    else if (log_)
        log_->SumitReport(report);
}

void ReportHandler::PushContextDesc(const std::string& contextDesc)
{
    contextDescStack_.push(contextDesc);
}

void ReportHandler::PopContextDesc()
{
    contextDescStack_.pop();
}

void ReportHandler::HintForNextReport(const std::string& hint)
{
    g_hintQueue.push_back(hint);
}


/*
 * ======= Private: =======
 */

Report ReportHandler::MakeReport(
    const Report::Types type, const std::string& msg, SourceCode* sourceCode, const SourceArea& area)
{
    /* Get current context description */
    std::string contextDesc;
    if (!contextDescStack_.empty())
    {
        contextDesc += R_In + " '";
        contextDesc += contextDescStack_.top();
        contextDesc += "':";
    }

    /* Make report with parameters */
    if (sourceCode != nullptr && area.Length() > 0)
    {
        std::string line, marker;
        if (sourceCode->FetchLineMarker(area, line, marker))
            return Report(type, msg, line, marker, contextDesc);
        else
            return Report(type, msg, contextDesc);
    }
    else
        return Report(type, msg, contextDesc);
}


} // /namespace Xsc



// ================================================================================