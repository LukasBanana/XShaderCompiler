/*
 * CodeWriter.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_CODE_GENERATOR_H
#define XSC_CODE_GENERATOR_H


#include <Xsc/IndentHandler.h>
#include <ostream>
#include <stack>
#include <vector>
#include <list>


namespace Xsc
{


// Output code writer.
class CodeWriter : public IndentHandler
{
    
    public:
        
        struct Options
        {
            Options() = default;

            inline Options(bool enableNewLine, bool enableIndent) :
                enableNewLine   { enableNewLine },
                enableIndent    { enableIndent  }
            {
            }

            bool enableNewLine  = true;
            bool enableIndent   = true;
        };

        // Throws std::runtime_error If stream is invalid.
        void OutputStream(std::ostream& stream);

        void PushOptions(const Options& options);
        void PopOptions();

        void BeginSeparation();
        void EndSeparation();

        void BeginLine();
        void EndLine();

        void Write(const std::string& text);
        void WriteLine(const std::string& text);

        // Writes either a single space or a formatted separator.
        void Separator();

        Options CurrentOptions() const;

        // Returns the output stream.
        inline std::ostream& Out()
        {
            return (*stream_);
        }

        // Returns true if the code writer is currently in an open line (i.e. BeginLine was called without closing EndLine).
        inline bool IsOpenLine() const
        {
            return openLine_;
        }

    private:
        
        struct SeparatedLine
        {
            std::string                 indent;
            std::vector<std::string>    parts;

            void Tab();

            void Offsets(std::vector<std::size_t>& offsets) const;

            SeparatedLine& operator << (const std::string& text);
        };

        struct SeparatedLineQueue
        {
            std::list<SeparatedLine> lines;

            inline SeparatedLine& Current()
            {
                return lines.back();
            }

            inline const SeparatedLine& Current() const
            {
                return lines.back();
            }
        };

        void FlushSeparatedLines(SeparatedLineQueue& lineQueue);

        std::ostream*       stream_                 = nullptr;

        std::stack<Options> optionsStack_;
        bool                openLine_               = false;

        bool                lineSeparation_         = false;
        SeparatedLineQueue  queuedSeparatedLines_;

};


} // /namespace Xsc


#endif



// ================================================================================