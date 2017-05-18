/*
 * Generator.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_GENERATOR_H
#define XSC_GENERATOR_H


#include <Xsc/Xsc.h>
#include "CodeWriter.h"
#include "VisitorTracker.h"
#include "ReportHandler.h"
#include "Flags.h"


namespace Xsc
{


// Output code generator base class.
class Generator : protected VisitorTracker
{
    
    public:
        
        Generator(Log* log);

        bool GenerateCode(
            Program& program,
            const ShaderInput& inputDesc,
            const ShaderOutput& outputDesc,
            Log* log = nullptr
        );

    protected:
        
        virtual void GenerateCodePrimary(
            Program& program,
            const ShaderInput& inputDesc,
            const ShaderOutput& outputDesc
        ) = 0;

        void Error(const std::string& msg, const AST* ast = nullptr, bool breakWithExpection = true);
        void Warning(const std::string& msg, const AST* ast = nullptr);

        void BeginLn();
        void EndLn();

        void BeginSep();
        void EndSep();

        void Separator();
        
        void WriteScopeOpen(bool compact = false, bool endWithSemicolon = false, bool useBraces = true);
        void WriteScopeClose();
        void WriteScopeContinue();

        bool IsOpenLine() const;
        
        void Write(const std::string& text);
        void WriteLn(const std::string& text);

        void IncIndent();
        void DecIndent();
        
        void PushOptions(const CodeWriter::Options& options);
        void PopOptions();

        // Push the specified text to the write-prefix which will be written in front of the text of the next "Write"/"WriteLn" call.
        void PushWritePrefix(const std::string& text);
        void PopWritePrefix(const std::string& text = "");

        // Returns true, if the current (top most) write prefix was written out.
        bool TopWritePrefix() const;

        void Blank();

        // Returns the current date and time point (can be used in a headline comment).
        std::string TimePoint() const;

        // Returns the AST root node.
        inline Program* GetProgram() const
        {
            return program_;
        }

        // Returns the shader target.
        inline ShaderTarget GetShaderTarget() const
        {
            return shaderTarget_;
        }

        // Returns true if the specified warnings flags are enabled.
        bool WarnEnabled(unsigned int flags) const;

        bool IsVertexShader() const;
        bool IsTessControlShader() const;
        bool IsTessEvaluationShader() const;
        bool IsGeometryShader() const;
        bool IsFragmentShader() const;
        bool IsComputeShader() const;

    private:

        /*
        Prefix text that can be written in front of the text of the next "Write"/"WriteLn" call.
        This can be used to insert an optional output text before it is clear, if this text is need.
        E.g. to write "layout(std140)", the prefix "layout(" can be used and only written if "std140" will be written afterwards,
        otherwise the entire "layout(...)" expression can be omitted.
        */
        struct WritePrefix
        {
            std::string text;       // Specifies the prefix text.
            bool        written;    // Specifies whether this prefix has already been written out.
        };

        // Writes all prefixes that have not already been written.
        void FlushWritePrefixes();

        CodeWriter                  writer_;
        ReportHandler               reportHandler_;

        Program*                    program_                = nullptr;

        ShaderTarget                shaderTarget_           = ShaderTarget::VertexShader;
        Flags                       warnings_;

        bool                        allowBlanks_            = true;
        bool                        allowLineSeparation_    = true;

        std::vector<WritePrefix>    writePrefixStack_;

};


} // /namespace Xsc


#endif



// ================================================================================