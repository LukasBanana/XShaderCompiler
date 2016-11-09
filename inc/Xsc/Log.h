/*
 * Log.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_LOG_H
#define XSC_LOG_H


#include "Export.h"
#include "Report.h"
#include <string>
#include <stack>
#include <vector>


namespace Xsc
{


//! Log base class.
class XSC_EXPORT Log
{
    
    public:
        
        virtual ~Log();

        //! Submits the specified report.
        virtual void SumitReport(const Report& report) = 0;

        //! Sets the next indentation string. By default two spaces.
        void SetIndent(const std::string& indent);

        //! Increments the indentation.
        void IncIndent();

        //! Decrements the indentation.
        void DecIndent();

    protected:

        Log();

        /**
        \brief Returns the current full indentation string.
        \remarks Add this to the front of each report message.
        \code
        MyLog::SubmitReport(const Report& report)
        {
            std::cout << FullIndent() << report.Message() << std::endl;
        }
        \endcode
        */
        inline const std::string& FullIndent() const
        {
            return indentFull_;
        }

    private:

        std::string                         indent_;
        std::string                         indentFull_;
        std::stack<std::string::size_type>  indentStack_;

};

//! Standard output log (uses std::cout to submit a report).
class XSC_EXPORT StdLog : public Log
{

    public:

        //! Implements the base class interface.
        void SumitReport(const Report& report) override;

        //! Prints all submitted reports to the standard output.
        void PrintAll();

    private:

        struct IndentReport
        {
            std::string indent;
            Report      report;
        };

        using IndentReportList = std::vector<IndentReport>;

        void PrintReport(const IndentReport& r);
        void PrintAndClearReports(IndentReportList& reports, const std::string& headline = "");

        IndentReportList infos_;
        IndentReportList warnings_;
        IndentReportList errors_;

};


} // /namespace Xsc


#endif



// ================================================================================