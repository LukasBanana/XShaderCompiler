/*
 * Log.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_LOG_H
#define XSC_LOG_H


#include "IndentHandler.h"
#include "Report.h"
#include <vector>


namespace Xsc
{


//! Log base class.
class XSC_EXPORT Log : public IndentHandler
{
    
    public:
        
        //! Submits the specified report.
        virtual void SumitReport(const Report& report) = 0;

    protected:

        Log();

};

//! Standard output log (uses std::cout to submit a report).
class XSC_EXPORT StdLog : public Log
{

    public:

        //! Implements the base class interface.
        void SumitReport(const Report& report) override;

        //! Prints all submitted reports to the standard output.
        void PrintAll(bool verbose = true);

    private:

        struct IndentReport
        {
            std::string indent;
            Report      report;
        };

        using IndentReportList = std::vector<IndentReport>;

        void PrintReport(const IndentReport& r, bool verbose);
        void PrintAndClearReports(IndentReportList& reports, bool verbose, const std::string& headline = "");

        IndentReportList infos_;
        IndentReportList warnings_;
        IndentReportList errors_;

};


} // /namespace Xsc


#endif



// ================================================================================