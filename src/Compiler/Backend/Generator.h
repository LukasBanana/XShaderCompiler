/*
 * Generator.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_GENERATOR_H
#define XSC_GENERATOR_H


#include <Xsc/Xsc.h>
#include "CodeWriter.h"
#include "Visitor.h"


namespace Xsc
{


// Output code generator base class.
class Generator : protected Visitor
{
    
    public:
        
        bool GenerateCode(
            Program& program,
            const ShaderInput& inputDesc,
            const ShaderOutput& outputDesc,
            Log* log = nullptr
        );

    protected:
        
        virtual void GeneratePrimaryCode(
            Program& program,
            const ShaderInput& inputDesc,
            const ShaderOutput& outputDesc
        ) = 0;

        void Error(const std::string& msg, const AST* ast = nullptr);
        void ErrorInvalidNumArgs(const std::string& functionName, const AST* ast = nullptr);

        void BeginLn();
        void EndLn();
        
        void Write(const std::string& text);
        void WriteLn(const std::string& text);

        void IncIndent();
        void DecIndent();
        
        void PushOptions(const CodeWriter::Options& options);
        void PopOptions();

        void Blank();

        // Returns the current date and time point (can be used in a headline comment).
        std::string TimePoint() const;

    private:

        CodeWriter  writer_;

        bool        allowBlanks_ = true;

};


} // /namespace Xsc


#endif



// ================================================================================