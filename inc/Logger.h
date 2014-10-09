/*
 * Logger.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __HT_LOGGER_H__
#define __HT_LOGGER_H__


#include <string>


namespace HLSLTrans
{


//! Logger interface.
class Logger
{
    
    public:
        
        virtual ~Logger()
        {
        }

        virtual void Info(const std::string& message)
        {
        }

        virtual void Warning(const std::string& message)
        {
        }

        virtual void Error(const std::string& message)
        {
        }

};


} // /namespace HLSLTrans


#endif



// ================================================================================