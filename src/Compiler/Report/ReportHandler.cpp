/*
 * ReportHandler.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ReportHandler.h"
#include "ReportIdents.h"
#include "SourceCode.h"
#include "Helper.h"
#include <vector>


namespace Xsc
{


thread_local static std::vector<std::string> g_hintQueue;

ReportHandler::ReportHandler(Log* log) :
    log_ { log }
{
}

void ReportHandler::Warning(
    bool breakWithExpection, const std::string& msg, SourceCode* sourceCode, const SourceArea& area)
{
    SubmitReport(breakWithExpection, ReportTypes::Warning, R_Warning, msg, sourceCode, area);
}

void ReportHandler::SubmitReport(
    bool                            breakWithExpection,
    const ReportTypes               type,
    const std::string&              typeName,
    const std::string&              msg,
    SourceCode*                     sourceCode,
    const SourceArea&               area,
    const std::vector<SourceArea>&  secondaryAreas)
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

    if (type == ReportTypes::Error)
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
    auto report = MakeReport(type, outputMsg, sourceCode, area, secondaryAreas);

    /* Move hint queue into report */
    report.TakeHints(std::move(g_hintQueue));

    /* Either throw or submit report */
    if (breakWithExpection)
        throw report;
    else if (log_)
        log_->SubmitReport(report);
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
    const ReportTypes               type,
    const std::string&              msg,
    SourceCode*                     sourceCode,
    const SourceArea&               area,
    const std::vector<SourceArea>&  secondaryAreas)
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
        /* Construct line with marker */
        std::string line, marker;
        sourceCode->FetchLineMarker(area, line, marker);

        for (const auto& nextArea : secondaryAreas)
        {
            if (nextArea.Pos().GetOrigin() == area.Pos().GetOrigin() && nextArea.Pos().Row() == area.Pos().Row())
            {
                /* Fetch new line marker */
                std::string nextLine, nextMarker;
                if (sourceCode->FetchLineMarker(nextArea, nextLine, nextMarker))
                {
                    /* Merge into current line marker */
                    MergeString(marker, nextMarker, '^', ' ');
                }
            }
        }

        /* Return report */
        if (!line.empty())
            return Report { type, msg, line, marker, contextDesc };
        else
            return Report { type, msg, contextDesc };
    }
    else
        return Report { type, msg, contextDesc };
}


} // /namespace Xsc



// ================================================================================