/*
 * Report.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_REPORT_H
#define XSC_REPORT_H


#include <stdexcept>
#include <string>
#include <vector>


namespace Xsc
{


//! Report types enumeration.
enum class ReportTypes
{
    Info,       //!< Standard information.
    Warning,    //!< Warning message.
    Error       //!< Error message.
};

//! Report exception class which contains a completely constructed message with optional line marker, hints, and context description.
class Report : public std::exception
{

    public:

        Report(const Report&) = default;
        Report& operator = (const Report&) = default;

        inline Report(const ReportTypes type, const std::string& message, const std::string& context = "") :
            type_    { type    },
            context_ { context },
            message_ { message }
        {
        }

        inline Report(const ReportTypes type, const std::string& message, const std::string& line, const std::string& marker, const std::string& context = "") :
            type_    { type    },
            context_ { context },
            message_ { message },
            line_    { line    },
            marker_  { marker  }
        {
            /* Remove new-line characters from end of source line */
            while ( !line_.empty() && ( line_.back() == '\n' || line_.back() == '\r' ) )
                line_.pop_back();
        }

        //! Overrides the 'std::exception::what' function.
        inline const char* what() const throw() override
        {
            return message_.c_str();
        }

        //! Moves the specified hints into this report.
        inline void TakeHints(std::vector<std::string>&& hints)
        {
            hints_ = std::move(hints);
        }

        //! Returns the type of this report.
        inline ReportTypes Type() const
        {
            return type_;
        }

        //! Returns the context description string (e.g. a function name where the report occured). This may also be empty.
        inline const std::string& Context() const
        {
            return context_;
        }

        //! Returns the message string.
        inline const std::string& Message() const
        {
            return message_;
        }

        //! Returns the line string where the report occured. This line never has new-line characters at its end.
        inline const std::string& Line() const
        {
            return line_;
        }

        //! Returns the line marker string to highlight the area where the report occured.
        inline const std::string& Marker() const
        {
            return marker_;
        }

        //! Returns the list of optional hints of the report.
        inline const std::vector<std::string>& GetHints() const
        {
            return hints_;
        }

        /**
        \brief Returns true if this report has a line with line marker.
        \see Line
        \see Marker
        */
        inline bool HasLine() const
        {
            return (!line_.empty());
        }

    private:

        ReportTypes                 type_       = ReportTypes::Info;
        std::string                 context_;
        std::string                 message_;
        std::string                 line_;
        std::string                 marker_;
        std::vector<std::string>    hints_;

};


} // /namespace Xsc


#endif



// ================================================================================