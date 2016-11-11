/*
 * IndentHandler.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_INDENT_HANDLER_H
#define XSC_INDENT_HANDLER_H


#include "Export.h"
#include <string>
#include <stack>


namespace Xsc
{


//! Indentation handler base class.
class XSC_EXPORT IndentHandler
{
    
    public:
        
        virtual ~IndentHandler();

        //! Sets the next indentation string. By default two spaces.
        void SetIndent(const std::string& indent);

        //! Increments the indentation.
        void IncIndent();

        //! Decrements the indentation.
        void DecIndent();

    protected:

        IndentHandler(const std::string& initialIndent);

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


} // /namespace Xsc


#endif



// ================================================================================