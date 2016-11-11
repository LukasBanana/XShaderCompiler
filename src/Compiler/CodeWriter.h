/*
 * CodeWriter.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_CODE_GENERATOR_H
#define XSC_CODE_GENERATOR_H


#include <ostream>
#include <stack>


namespace Xsc
{


// Output code writer.
class CodeWriter
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

        void SetIndent(const std::string& indent);

        void PushIndent();
        void PopIndent();

        void PushOptions(const Options& options);
        void PopOptions();

        void BeginLine();
        void EndLine();

        void Write(const std::string& text);
        void WriteLine(const std::string& text);

        Options CurrentOptions() const;

    private:
        
        std::ostream*       stream_         = nullptr;
        std::string         indent_         = std::string(4, ' ');
        std::string         indentFull_;

        std::stack<Options> optionsStack_;

};


} // /namespace Xsc


#endif



// ================================================================================