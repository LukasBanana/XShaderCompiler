/*
 * ReportHandler.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_REPORT_HANDLER_H
#define XSC_REPORT_HANDLER_H


#include "SourcePosition.h"
#include "HLSLErr.h"
#include <Xsc/Report.h>
#include <Xsc/Log.h>
#include <string>


namespace Xsc
{


class SourceCode;

// Error code wrapper for string representation.
class ErrorCode
{

    public:

        ErrorCode() = default;
        ErrorCode(const HLSLErr errorCode);
        //other may follow: e.g. "ErrorCode(const GLSLErr errorCode)"

        // Returns the error code as string.
        inline const std::string& Get() const
        {
            return str_;
        }

    private:

        std::string str_;

};

// Report handler class for simpler error and warning handling.
class ReportHandler
{

    public:

        ReportHandler(const std::string& reportTypeName, Log* log);

        void Error(
            const std::string& msg,
            SourceCode* sourceCode = nullptr,
            const SourceArea& area = SourceArea::ignore,
            const ErrorCode& errorCode = ErrorCode()
        );

        void ErrorBreak(
            const std::string& msg,
            SourceCode* sourceCode = nullptr,
            const SourceArea& area = SourceArea::ignore,
            const ErrorCode& errorCode = ErrorCode()
        );

        void Warning(
            const std::string& msg,
            SourceCode* sourceCode = nullptr,
            const SourceArea& area = SourceArea::ignore,
            const ErrorCode& errorCode = ErrorCode()
        );

        void WarningBreak(
            const std::string& msg,
            SourceCode* sourceCode = nullptr,
            const SourceArea& area = SourceArea::ignore,
            const ErrorCode& errorCode = ErrorCode()
        );

        void SubmitReport(
            bool breakWithExpection,
            const Report::Types type,
            const std::string& typeName,
            const std::string& msg,
            SourceCode* sourceCode = nullptr,
            const SourceArea& area = SourceArea::ignore,
            const ErrorCode& errorCode = ErrorCode()
        );

        // Returns true if any errors have been submitted.
        inline bool HasErros() const
        {
            return hasErrors_;
        }

        // Sets the current filename.
        inline void SetCurrentFilename(const std::string& filename)
        {
            currentFilename_ = filename;
        }

    private:

        Report MakeReport(
            const Report::Types type,
            const std::string& msg,
            SourceCode* sourceCode,
            const SourceArea& area
        );

        std::string reportTypeName_;
        std::string currentFilename_;

        Log*        log_                = nullptr;
        bool        hasErrors_          = false;

};


} // /namespace Xsc


#endif



// ================================================================================