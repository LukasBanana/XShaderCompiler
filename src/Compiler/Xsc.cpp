/*
 * Xsc.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/Xsc.h>
#include "Compiler.h"
#include "SPIRVDisassembler.h"
#include "ReportIdents.h"


namespace Xsc
{


XSC_EXPORT bool CompileShader(
    const ShaderInput& inputDesc, const ShaderOutput& outputDesc,
    Log* log, Reflection::ReflectionData* reflectionData)
{
    /* Compile shader with compiler driver */
    Compiler::StageTimePoints timePoints;

    Compiler compiler(log);

    auto result = compiler.CompileShader(
        inputDesc,
        outputDesc,
        reflectionData,
        &timePoints
    );

    if (reflectionData)
    {
        /* Sort reflection */
        auto SortStats = [](std::vector<Reflection::BindingSlot>& objects)
        {
            std::sort(
                objects.begin(), objects.end(),
                [](const Reflection::BindingSlot& lhs, const Reflection::BindingSlot& rhs)
                {
                    return (lhs.location < rhs.location);
                }
            );
        };

        SortStats(reflectionData->textures);
        SortStats(reflectionData->constantBuffers);
        SortStats(reflectionData->inputAttributes);
        SortStats(reflectionData->outputAttributes);
    }

    /* Show timings */
    if (outputDesc.options.showTimes && log)
    {
        using TimePoint = Compiler::TimePoint;

        auto PrintTiming = [log](const std::string& processName, const TimePoint startTime, const TimePoint endTime)
        {
            long long duration = 0ll;
            
            if (endTime > startTime)
            {
                duration =
                (
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::duration<float>(endTime - startTime)
                    ).count()
                );
            }

            log->SumitReport(
                Report(
                    ReportTypes::Info,
                    "timing " + processName + std::to_string(duration) + " ms"
                )
            );
        };

        PrintTiming( "pre-processing:   ", timePoints.preprocessor, timePoints.parser     );
        PrintTiming( "parsing:          ", timePoints.parser,       timePoints.analyzer   );
        PrintTiming( "context analysis: ", timePoints.analyzer,     timePoints.optimizer  );
        PrintTiming( "optimization:     ", timePoints.optimizer,    timePoints.generation );
        PrintTiming( "code generation:  ", timePoints.generation,   timePoints.reflection );
    }

    return result;
}

XSC_EXPORT void DisassembleShader(
    std::istream& streamIn, std::ostream& streamOut, const AssemblyDescriptor& desc)
{
    switch (desc.intermediateLanguage)
    {
        case IntermediateLanguage::SPIRV:
        {
            /* Disassemble SPIR-V module */
            SPIRVDisassembler disassembler;
            disassembler.Parse(streamIn);
            disassembler.Print(streamOut, desc.idPrefixChar);
            break;
        }

        default:
        {
            throw std::invalid_argument(R_InvalidILForDisassembling);
        }
        break;
    }
}


} // /namespace Xsc



// ================================================================================
