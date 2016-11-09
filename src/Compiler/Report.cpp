/*
 * Report.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/Report.h>


namespace Xsc
{


Report::Report(const Types type, const std::string& message) :
    type_   { type    },
    message_{ message }
{
}

Report::Report(const Types type, const std::string& message, const std::string& line, const std::string& marker) :
    type_   { type    },
    message_{ message },
    line_   { line    },
    marker_ { marker  }
{
    /* Remove new-line characters from end of source line */
    while ( !line_.empty() && ( line_.back() == '\n' || line_.back() == '\r' ) )
        line_.pop_back();
}

const char* Report::what() const throw()
{
    return message_.c_str();
}


} // /namespace Xsc



// ================================================================================