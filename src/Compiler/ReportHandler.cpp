/*
 * ReportHandler.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ReportHandler.h"
#include "SourceCode.h"


namespace Xsc
{


/*
 * ErrorCode class
 */

ErrorCode::ErrorCode(const HLSLErr errorCode)
{
    if (errorCode != HLSLErr::Unknown)
        str_ = "X" + std::to_string(static_cast<int>(errorCode)) + "=" + ErrToString(errorCode);
}


/*
 * ReportHandler class
 */

ReportHandler::ReportHandler(const std::string& reportTypeName, Log* log) :
    reportTypeName_ { reportTypeName },
    log_            { log           }
{
}

void ReportHandler::Error(const std::string& msg, SourceCode* sourceCode, const SourceArea& area, const ErrorCode& errorCode)
{
    SubmitReport(false, Report::Types::Error, (reportTypeName_ + " error"), msg, sourceCode, area, errorCode);
}

void ReportHandler::ErrorBreak(const std::string& msg, SourceCode* sourceCode, const SourceArea& area, const ErrorCode& errorCode)
{
    SubmitReport(true, Report::Types::Error, (reportTypeName_ + " error"), msg, sourceCode, area, errorCode);
}

void ReportHandler::Warning(const std::string& msg, SourceCode* sourceCode, const SourceArea& area, const ErrorCode& errorCode)
{
    SubmitReport(false, Report::Types::Warning, "warning", msg, sourceCode, area, errorCode);
}

void ReportHandler::WarningBreak(const std::string& msg, SourceCode* sourceCode, const SourceArea& area, const ErrorCode& errorCode)
{
    SubmitReport(true, Report::Types::Warning, "warning", msg, sourceCode, area, errorCode);
}

void ReportHandler::SubmitReport(
    bool breakWithExpection, const Report::Types type, const std::string& typeName,
    const std::string& msg, SourceCode* sourceCode, const SourceArea& area, const ErrorCode& errorCode)
{
    /* Initialize output message */
    auto outputMsg = typeName;
    
    if (type == Report::Types::Error)
        hasErrors_ = true;

    /* Add source position */
    outputMsg += " (";

    if (!currentFilename_.empty())
    {
        outputMsg += currentFilename_;
        outputMsg += ":";
    }

    outputMsg += area.pos.ToString();
    outputMsg += ") ";

    /* Add error code */
    if (!errorCode.Get().empty())
        outputMsg += "[" + errorCode.Get() + "] ";

    outputMsg += ": ";

    /* Add actual report message */
    outputMsg += msg;

    /* Either throw or submit report */
    auto report = MakeReport(type, outputMsg, sourceCode, area);

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
        contextDesc += "in '";
        contextDesc += contextDescStack_.top();
        contextDesc += "':";
    }

    /* Make report with parameters */
    if (sourceCode != nullptr && area.length > 0)
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