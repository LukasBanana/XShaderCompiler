/*
 * HelpPrinter.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_HELP_PRINTER_H
#define XSC_HELP_PRINTER_H


#include <vector>
#include <string>


namespace Xsc
{

namespace Util
{


class Command;

// Shell help entry category enumeration.
struct HelpCategory
{
    enum
    {
        Main        = (1 << 0),
        Common      = (1 << 1),
        Formatting  = (1 << 2),
        All         = ~0,
    };
};

// Shell help entry.
struct HelpDescriptor
{
    HelpDescriptor() = default;

    inline HelpDescriptor(
        const std::string& usage, const std::string& brief,
        const std::string& details = "", long category = HelpCategory::Common) :
            usage   { usage    },
            brief   { brief    },
            details { details  },
            category{ category }
    {
    }

    inline HelpDescriptor(
        const std::string& usage, const std::string& brief, long category) :
            usage   { usage    },
            brief   { brief    },
            category{ category }
    {
    }

    std::string usage;
    std::string brief;
    std::string details;
    long        category = HelpCategory::Common;
};

// Shell help printer class.
class HelpPrinter
{

    public:

        // Appends the help entry for the specified shell command.
        void AppendCommandHelp(const Command& cmd);

        // Prints all previously added help entries to the specified output stream.
        void PrintAll(std::ostream& output, std::size_t indentSize = 0, bool printCompact = true, long categories = HelpCategory::All) const;

        // Prints the help entry only for the specified shell command.
        void Print(std::ostream& output, const std::string& commandName, std::size_t indentSize = 0, bool printCompact = true) const;

    private:

        struct HelpEntry
        {
            const Command*  cmd;
            HelpDescriptor  desc;
        };

        void PrintEntry(std::ostream& output, const HelpDescriptor& helpDesc, std::size_t indentSize) const;
        void PrintEntryCompact(std::ostream& output, const HelpDescriptor& helpDesc, std::size_t indentSize) const;
        void PrintHelpDetails(std::ostream& output, const std::string& details, std::size_t indentSize) const;

        std::vector<HelpEntry>  entries_;
        std::size_t             maxUsageLen_ = 0;

};


} // /namespace Util

} // /namespace Xsc


#endif



// ================================================================================