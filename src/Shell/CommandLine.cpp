/*
 * ArgumentList.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CommandLine.h"
#include "ReportIdents.h"
#include "Helper.h"
#include <stdexcept>
#include <algorithm>


namespace Xsc
{

namespace Util
{


CommandLine::CommandLine(int argc, char** argv)
{
    for (int i = 0; i < argc; ++i)
        args_.push_back(std::string(argv[i]));
}

CommandLine::CommandLine(const std::vector<std::string>& args)
{
    args_.insert(args_.end(), args.begin(), args.end());
}

CommandLine::CommandLine(const std::string& line)
{
    /* Parse command line arguments */
    std::size_t start = 0, end = 0;
    while (start < line.size())
    {
        /* Find begin of next argument */
        start = line.find_first_not_of(' ', end);
        if (start >= line.size())
            break;

        if (line[start] == '\"')
        {
            /* Append argument inside quotation marks */
            end = line.find('\"', start + 1);
            if (end != std::string::npos)
            {
                args_.push_back(line.substr(start + 1, end - start - 1));
                ++end;
            }
        }
        else
        {
            /* Append argument at the end of the next space */
            end = line.find(' ', start + 1);
            if (end != std::string::npos)
                args_.push_back(line.substr(start, end - start));
            else
                args_.push_back(line.substr(start));
        }
    }
}

std::string CommandLine::Get() const
{
    return (args_.empty() ? "" : args_.front());
}

std::string CommandLine::Accept()
{
    if (args_.empty())
        throw std::invalid_argument(R_UnexpectedEndOfCmdLine);

    auto arg = Get();
    args_.pop_front();

    return arg;
}

bool CommandLine::AcceptBoolean()
{
    /* Accept booleans with case insensitive check */
    auto arg = Accept();
    ToUpper(arg);
    
    if (arg == CommandLine::GetBooleanTrue())
        return true;
    if (arg == CommandLine::GetBooleanFalse())
        return false;

    throw std::invalid_argument(R_ExpectedCmdLineBoolean(CommandLine::GetBooleanTrue(), CommandLine::GetBooleanFalse(), arg));
}

bool CommandLine::AcceptBoolean(bool defaultValue)
{
    /* Accept booleans with case insensitive check */
    auto arg = Get();
    ToUpper(arg);

    if (arg == CommandLine::GetBooleanTrue())
    {
        Accept();
        return true;
    }
    if (arg == CommandLine::GetBooleanFalse())
    {
        Accept();
        return false;
    }

    return defaultValue;
}

void CommandLine::Insert(const std::string& argument)
{
    args_.push_front(argument);
}

bool CommandLine::ReachedEnd() const
{
    return args_.empty();
}

const std::string& CommandLine::GetBooleanTrue()
{
    static const std::string value("ON");
    return value;
}

const std::string& CommandLine::GetBooleanFalse()
{
    static const std::string value("OFF");
    return value;
}

const std::string CommandLine::GetBooleanOption()
{
    return (GetBooleanTrue() + '|' + GetBooleanFalse());
}



} // /namespace Util

} // /namespace Xsc



// ================================================================================
