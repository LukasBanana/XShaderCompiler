/*
 * ArgumentList.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CommandLine.h"


namespace HTLib
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

static const std::string booleanArgTrue = "ON";
static const std::string booleanArgFalse = "OFF";

bool CommandLine::AcceptBoolean()
{
    auto arg = Accept();
    if (arg == booleanArgTrue)
        return true;
    if (arg == booleanArgFalse)
        return false;
    throw std::invalid_argument("expected 'on' or 'off', but got '" + arg + "'");
}

bool CommandLine::AcceptBoolean(bool defaultValue)
{
    auto arg = Get();
    if (arg == booleanArgTrue)
    {
        Accept();
        return true;
    }
    if (arg == booleanArgFalse)
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


} // /namespace Util

} // /namespace HTLib



// ================================================================================
