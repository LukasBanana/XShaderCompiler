/*
 * GLSLGenerator.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __HT_GLSL_GENERATOR_H__
#define __HT_GLSL_GENERATOR_H__


#include "CodeWriter.h"
#include "HT/Translator.h"
#include "Visitor.h"
#include "Token.h"


namespace HTLib
{


//! GLSL output code generator.
class GLSLGenerator : private Visitor
{
    
    public:
        
        GLSLGenerator(
            Logger* log = nullptr,
            IncludeHandler* includeHandler = nullptr,
            const Options& options = {}
        );

        bool GenerateCode(
            std::ostream& output,
            const std::string& entryPoint,
            const ShaderTargets shaderTarget,
            const ShaderVersions shaderVersion
        );

    private:
        
        /* === Functions === */

        void BeginLn();
        void EndLn();
        
        void Write(const std::string& text);
        void WriteLn(const std::string& text);

        void IncTab();
        void DecTab();
        
        //! Writes a new single line comment.
        void Comment(const std::string& text);
        //! Writes a "#version" directive.
        void Version(int versionNumber);
        //! Writes a "#line" directive.
        void Line(int lineNumber);
        void Line(const TokenPtr& tkn);

        //! Opens a new scope with '{'.
        void OpenScope();
        //! Closes the current scope with '}'.
        void CloseScope();

        /* === Visitor implementation === */

        //...

        /* === Members === */

        CodeWriter      writer_;
        IncludeHandler* includeHandler_ = nullptr;
        Logger*         log_            = nullptr;

};


} // /namespace HTLib


#endif



// ================================================================================