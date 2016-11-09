/*
 * HelpPrinter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HelpPrinter.h"
#include "CommandFactory.h"
#include "Command.h"
#include <algorithm>


namespace Xsc
{

namespace Util
{


void HelpPrinter::AppendCommandHelp(const Command& cmd)
{
    /* Add help entry of command to the list */
    auto help = cmd.Help();
    entries_.push_back({ &cmd, help });

    /* Store length of the longest usage help string */
    maxUsageLen_ = std::max(maxUsageLen_, help.usage.size());

    /* Sort help entries */
    std::sort(
        entries_.begin(), entries_.end(),
        [](const HelpEntry& lhs, const HelpEntry& rhs)
        {
            return (lhs.desc.usage < rhs.desc.usage);
        }
    );
}

void HelpPrinter::PrintAll(std::ostream& output, std::size_t indentSize) const
{
    for (const auto& entry : entries_)
        PrintEntry(output, entry.desc, indentSize);
}

void HelpPrinter::Print(std::ostream& output, const std::string& commandName, std::size_t indentSize) const
{
    auto cmd = CommandFactory::Instance().Get(commandName);
    if (cmd)
    {
        for (const auto& entry : entries_)
        {
            if (entry.cmd == cmd)
            {
                PrintEntry(output, entry.desc, indentSize);
                break;
            }
        }
    }
}


/*
 * ======= Private: =======
 */

void HelpPrinter::PrintEntry(std::ostream& output, const HelpDescriptor& helpDesc, std::size_t indentSize) const
{
    std::string indent(indentSize, ' ');
    
    /* Print usage and brief help */
    output
        << indent << helpDesc.usage << ' '
        << std::string(3u + maxUsageLen_ - helpDesc.usage.size(), '.') << ' '
        << helpDesc.brief << std::endl;

    /* Print optional details */
    std::size_t start = 0;

    std::string detailsIndent(indentSize*2 + 5u + maxUsageLen_, ' ');

    while (start < helpDesc.details.size())
    {
        auto end = helpDesc.details.find('\n', start);
        output << detailsIndent << helpDesc.details.substr(start, end) << std::endl;
        if (end != std::string::npos)
            start = end + 1;
        else
            break;
    }
}


} // /namespace Util

} // /namespace Xsc



// ================================================================================