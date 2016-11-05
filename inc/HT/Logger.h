/*
 * Log.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef HTLIB_LOGGER_H
#define HTLIB_LOGGER_H


#include "Export.h"

#include <string>


namespace HTLib
{


// Log interface.
class HTLIB_EXPORT Log
{
    
    public:
        
        virtual ~Log()
        {
        }

        // Prints an information.
        virtual void Info(const std::string& message)
        {
            // dummy
        }

        // Prints a warning.
        virtual void Warning(const std::string& message)
        {
            // dummy
        }

        // Prints an error.
        virtual void Error(const std::string& message)
        {
            // dummy
        }

        // Increments the indentation.
        virtual void IncIndent()
        {
            // dummy
        }

        // Decrements the indentation.
        virtual void DecIndent()
        {
            // dummy
        }

};


} // /namespace HTLib


#endif



// ================================================================================