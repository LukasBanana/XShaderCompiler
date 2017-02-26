/*
 * HelpPrinter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
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

    /* Sort help entries */
    std::sort(
        entries_.begin(), entries_.end(),
        [](const HelpEntry& lhs, const HelpEntry& rhs)
        {
            return (lhs.desc.usage < rhs.desc.usage);
        }
    );
}

void HelpPrinter::PrintHelpReference(std::ostream& output, std::size_t indentSize, bool printCompact, bool entireReference) const
{
    auto PrintHeader = [&](const std::string& label)
    {
        if (!printCompact)
            output << std::endl;
        output << label << ':' << std::endl;
    };

    /* Print usage */
    output << "Usage:" << std::endl;
    output << "  xsc (OPTION+ FILE)+" << std::endl;

    /* Print all options */
    PrintHeader("Main Options");
    PrintAll(output, indentSize, printCompact, HelpCategory::Main, (entireReference ? HelpCategory::All : HelpCategory::Main));

    if (entireReference)
    {
        PrintHeader("Common Options");
        PrintAll(output, indentSize, printCompact, HelpCategory::Common);

        PrintHeader("Formatting Options");
        PrintAll(output, indentSize, printCompact, HelpCategory::Formatting);

        PrintHeader("Name Mangling Options");
        PrintAll(output, indentSize, printCompact, HelpCategory::NameMangling);
    }

    /* Print usage example */
    PrintHeader("Example");
    output << "  xsc -E VS -T vert Example.hlsl -E PS -T frag Example.hlsl" << std::endl;
    output << "   -> Output files: 'Example.VS.vert', and 'Example.PS.frag'" << std::endl;

    if (!entireReference)
    {
        PrintHeader("Hint");
        output << "  Use 'xsc --help' for more details" << std::endl;
    }
}

void HelpPrinter::PrintAll(std::ostream& output, std::size_t indentSize, bool printCompact, long categories, long categoriesSet) const
{
    const auto maxUsageLen = GetMaxUsageLen(categoriesSet);
    for (const auto& entry : entries_)
    {
        if ((entry.desc.category & categories) != 0)
        {
            if (printCompact)
                PrintEntryCompact(output, entry.desc, indentSize, maxUsageLen);
            else
                PrintEntry(output, entry.desc, indentSize);
        }
    }
}

void HelpPrinter::Print(std::ostream& output, const std::string& commandName, std::size_t indentSize, bool printCompact) const
{
    auto cmd = CommandFactory::Instance().Get(commandName);
    if (cmd)
    {
        const auto maxUsageLen = GetMaxUsageLen();
        for (const auto& entry : entries_)
        {
            if (entry.cmd == cmd)
            {
                if (printCompact)
                    PrintEntryCompact(output, entry.desc, indentSize, maxUsageLen);
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

std::size_t HelpPrinter::GetMaxUsageLen(long categories) const
{
    std::size_t len = 0;

    for (const auto& entry : entries_)
    {
        if ((entry.desc.category & categories) != 0)
            len = std::max(len, entry.desc.usage.size());
    }

    return len;
}

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

void HelpPrinter::PrintEntryCompact(std::ostream& output, const HelpDescriptor& helpDesc, std::size_t indentSize, std::size_t maxUsageLen) const
{
    std::string indent(indentSize, ' ');
    
    /* Print usage and brief help */
    output
        << indent << helpDesc.usage << ' '
        << std::string(3u + maxUsageLen - helpDesc.usage.size(), '.') << ' '
        << helpDesc.brief << std::endl;

    /* Print optional details */
    PrintHelpDetails(output, helpDesc.details, (indentSize*2 + 5u + maxUsageLen));
}

void HelpPrinter::PrintHelpDetails(std::ostream& output, const std::string& details, std::size_t indentSize) const
{
    std::size_t start = 0;
    std::string indent(indentSize, ' ');

    while (start < details.size())
    {
        auto end = details.find('\n', start);
        output << indent << (end == std::string::npos ? details.substr(start) : details.substr(start, end - start)) << std::endl;
        if (end != std::string::npos)
            start = end + 1;
        else
            break;
    }
}


} // /namespace Util

} // /namespace Xsc



// ================================================================================
