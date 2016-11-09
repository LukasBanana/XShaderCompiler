/*
 * ArgumentList.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CommandLine.h"
#include <stdexcept>


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

std::string CommandLine::Get() const
{
    return (args_.empty() ? "" : args_.front());
}

std::string CommandLine::Accept()
{
    if (args_.empty())
        throw std::invalid_argument("unexpected end of command line arguments");
    auto arg = Get();
    args_.pop_front();
    return arg;
}

bool CommandLine::AcceptBoolean()
{
    auto arg = Accept();
    if (arg == CommandLine::GetBooleanTrue())
        return true;
    if (arg == CommandLine::GetBooleanFalse())
        return false;
    throw std::invalid_argument(
        "expected '" + CommandLine::GetBooleanTrue() + "' or '" +
        CommandLine::GetBooleanFalse() + "', but got '" + arg + "'"
    );
}

bool CommandLine::AcceptBoolean(bool defaultValue)
{
    auto arg = Get();
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
    return (GetBooleanTrue() + '/' + GetBooleanFalse());
}



} // /namespace Util

} // /namespace Xsc



// ================================================================================
