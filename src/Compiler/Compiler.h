/*
 * Compiler.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_COMPILER_H
#define XSC_COMPILER_H


#include <Xsc/Xsc.h>
#include <chrono>
#include <array>


namespace Xsc
{


// Compiler driver class.
class Compiler
{

    public:

        using Time      = std::chrono::system_clock;
        using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

        // Time points of all compiler stages.
        struct StageTimePoints
        {
            TimePoint preprocessor;
            TimePoint parser;
            TimePoint analyzer;
            TimePoint optimizer;
            TimePoint generation;
            TimePoint reflection;
        };

        Compiler(Log* log = nullptr);

        bool CompileShader(
            const ShaderInput&          inputDesc,
            const ShaderOutput&         outputDesc,
            Reflection::ReflectionData* reflectionData  = nullptr,
            StageTimePoints*            stageTimePoints = nullptr
        );

    private:

        /* === Functions === */

        bool ReturnWithError(const std::string& msg);
        void Warning(const std::string& msg);

        void ValidateArguments(const ShaderInput& inputDesc, const ShaderOutput& outputDesc);

        bool CompileShaderPrimary(
            const ShaderInput&          inputDesc,
            const ShaderOutput&         outputDesc,
            Reflection::ReflectionData* reflectionData
        );

        /* === Members === */

        Log*            log_        = nullptr;

        StageTimePoints timePoints_;

};


} // /namespace Xsc


#endif



// ================================================================================