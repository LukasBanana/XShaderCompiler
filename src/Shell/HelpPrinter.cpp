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

void HelpPrinter::PrintAll(std::ostream& output, std::size_t indentSize, bool printCompact) const
{
    for (const auto& entry : entries_)
    {
        if (printCompact)
            PrintEntryCompact(output, entry.desc, indentSize);
        else
            PrintEntry(output, entry.desc, indentSize);
    }
}

void HelpPrinter::Print(std::ostream& output, const std::string& commandName, std::size_t indentSize, bool printCompact) const
{
    auto cmd = CommandFactory::Instance().Get(commandName);
    if (cmd)
    {
        for (const auto& entry : entries_)
        {
            if (entry.cmd == cmd)
            {
                if (printCompact)
                    PrintEntryCompact(output, entry.desc, indentSize);
                else
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
    output << indent << helpDesc.usage << std::endl;
    output << indent << indent << indent << helpDesc.brief << std::endl;

    /* Print optional details */
    PrintHelpDetails(output, helpDesc.details, (indentSize*4));

    output << std::endl;
}

void HelpPrinter::PrintEntryCompact(std::ostream& output, const HelpDescriptor& helpDesc, std::size_t indentSize) const
{
    std::string indent(indentSize, ' ');
    
    /* Print usage and brief help */
    output
        << indent << helpDesc.usage << ' '
        << std::string(3u + maxUsageLen_ - helpDesc.usage.size(), '.') << ' '
        << helpDesc.brief << std::endl;

    /* Print optional details */
    PrintHelpDetails(output, helpDesc.details, (indentSize*2 + 5u + maxUsageLen_));
}

void HelpPrinter::PrintHelpDetails(std::ostream& output, const std::string& details, std::size_t indentSize) const
{
    std::size_t start = 0;
    std::string indent(indentSize, ' ');

    while (start < details.size())
    {
        auto end = details.find('\n', start);
        output << indent << details.substr(start, end) << std::endl;
        if (end != std::string::npos)
            start = end + 1;
        else
            break;
    }
}


} // /namespace Util

} // /namespace Xsc



// ================================================================================