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

        void BeginLine();
        void EndLine();

        void Write(const std::string& text);
        void WriteLine(const std::string& text);

        Options CurrentOptions() const;

        // Returns the output stream.
        inline std::ostream& Out()
        {
            return (*stream_);
        }

    private:
        
        std::ostream*       stream_         = nullptr;

        std::stack<Options> optionsStack_;

};


} // /namespace Xsc


#endif



// ================================================================================