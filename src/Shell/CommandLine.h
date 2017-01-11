/*
 * CommandLine.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_COMMAND_LINE_H
#define XSC_COMMAND_LINE_H


#include <list>
#include <vector>
#include <string>


namespace Xsc
{

namespace Util
{


class CommandLine
{

    public:

        CommandLine() = default;
        CommandLine(const CommandLine&) = default;
        CommandLine& operator = (const CommandLine&) = default;

        CommandLine(int argc, char** argv);
        CommandLine(const std::vector<std::string>& args);
        explicit CommandLine(const std::string& line);

        // Returns the current argument or an empty string if the end has reached.
        std::string Get() const;

        /*
        Returns the current argument and moves on to the next argument or return an empty string if the end has reached.
        If the return value is an empty string, instead an std::invalid_argument exception is thrown.
        */
        std::string Accept();

        /*
        Returns the current argument as boolean (if it is "on" or "off") and moves on to the next argument.
        If the current argument is not "on" or "off" an std::invalid_argument exception is thrown.
        */
        bool AcceptBoolean();

        /*
        Returns the current argument as boolean (if it is "on" or "off") and moves on to the next argument.
        If the current argument is not "on" or "off" the default value is returned.
        */
        bool AcceptBoolean(bool defaultValue);

        // Inserts the specified argument to the current position (this will be the next value returned by the "Get" function).
        void Insert(const std::string& argument);

        // Returns true if the end of the argument list has reached.
        bool ReachedEnd() const;
    
        // Returns the value command line argument for a boolean true value (e.g. "ON", "YES", or "true").
        static const std::string& GetBooleanTrue();
    
        // Returns the value command line argument for a boolean false value (e.g. "OFF", "NO", or "false").
        static const std::string& GetBooleanFalse();
    
        // Returns the value command line argument for a boolean value (e.g. "ON/OFF").
        static const std::string GetBooleanOption();

    private:

        std::list<std::string> args_;

};


} // /namespace Util

} // /namespace Xsc


#endif



// ================================================================================
