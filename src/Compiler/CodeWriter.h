/*
 * CodeWriter.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
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
                enableNewLine { enableNewLine },
                enableIndent  { enableIndent  }
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

        // Inserts a separator if line separation formatting is currently enabled.
        void Separator();

        // Begins a new line and inserts the current indentation.
        void BeginLine();

        // Ends the current line and inserts the new-line character to the output stream.
        void EndLine();

        // Writes the specified text to the output.
        void Write(const std::string& text);

        // Shortcut for: BeginLine(), Write(text), EndLine().
        void WriteLine(const std::string& text);

        // Begins a new scope with the '{' character and adds a new line either befor or after this character.
        void BeginScope(bool compact = false, bool endWithSemicolon = false, bool useBraces = true);

        // Ends the current scope with the '}' character. If next command is not "ContinueScope", the line wil be ended first.
        void EndScope();

        // Continues the previously ended scope.
        void ContinueScope();

        // Returns true if the code writer is currently in an open line (i.e. BeginLine was called without closing EndLine).
        inline bool IsOpenLine() const
        {
            return openLine_;
        }

    public:

        // Write new line for each scope.
        bool newLineOpenScope = false;

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

            SeparatedLine& Current();
        };

        struct ScopeState
        {
            bool scopeCanContinue   = false;
            bool scopeUsedBraces    = false;
            bool beginLineQueued    = false;
            bool endLineQueued      = false;
        };

        struct ScopeOptions
        {
            bool compact;
            bool endWithSemicolon;
            bool useBraces;
        };

    private:

        Options CurrentOptions() const;

        void FlushSeparatedLines(SeparatedLineQueue& lineQueue);

        // Returns the output stream.
        inline std::ostream& Out()
        {
            return (*stream_);
        }

    private:

        std::ostream*               stream_                 = nullptr;

        std::stack<Options>         optionsStack_;
        bool                        openLine_               = false;

        unsigned int                lineSeparationLevel_    = 0;
        SeparatedLineQueue          queuedSeparatedLines_;

        ScopeState                  scopeState_;
        std::stack<ScopeOptions>    scopeOptionStack_;

};


} // /namespace Xsc


#endif



// ================================================================================