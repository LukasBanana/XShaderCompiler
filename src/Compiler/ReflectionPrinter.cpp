/*
 * ReflectionPrinter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ReflectionPrinter.h"
#include "ReportIdents.h"
#include <algorithm>


namespace Xsc
{


ReflectionPrinter::ReflectionPrinter(std::ostream& output) :
    output_ { output }
{
}

void ReflectionPrinter::PrintReflection(const Reflection::ReflectionData& reflectionData)
{
    output_ << R_CodeReflection() << ':' << std::endl;
    indentHandler_.IncIndent();
    {
        //vector<uniform> -> vector<string>
        std::vector<std::string> reflectionData_uniforms;
        for (const auto& uname : reflectionData.uniforms)
            reflectionData_uniforms.push_back(uname.ident.c_str());
        //vector<function> -> vector<string>
        std::vector<std::string> reflectionData_functions;
        for (const auto& fname : reflectionData.functions)
            reflectionData_functions.push_back(fname.ident.c_str());
        //print
        PrintReflectionObjects  ( reflectionData.macros,           "Macros"            );
        PrintReflectionObjects  ( reflectionData_uniforms,         "Uniforms"          );
        PrintReflectionObjects  ( reflectionData_functions,        "Functions"         );
        PrintReflectionObjects  ( reflectionData.textures,         "Textures"          );
        PrintReflectionObjects  ( reflectionData.storageBuffers,   "Storage Buffers"   );
        PrintReflectionObjects  ( reflectionData.constantBuffers,  "Constant Buffers"  );
        PrintReflectionObjects  ( reflectionData.inputAttributes,  "Input Attributes"  );
        PrintReflectionObjects  ( reflectionData.outputAttributes, "Output Attributes" );
        PrintReflectionObjects  ( reflectionData.samplerStates,    "Sampler States"    );
        PrintReflectionAttribute( reflectionData.numThreads,       "Number of Threads" );
    }
    indentHandler_.DecIndent();
}


/*
 * ======= Private: =======
 */

std::ostream& ReflectionPrinter::IndentOut()
{
    output_ << indentHandler_.FullIndent();
    return output_;
}

void ReflectionPrinter::PrintReflectionObjects(const std::vector<Reflection::BindingSlot>& objects, const std::string& title)
{
    IndentOut() << title << ':' << std::endl;
    ScopedIndent indent(indentHandler_);

    if (!objects.empty())
    {
        /* Determine offset for right-aligned location index */
        int maxLocation = -1;
        for (const auto& obj : objects)
            maxLocation = std::max(maxLocation, obj.location);

        std::size_t maxLocationLen = std::to_string(maxLocation).size();

        /* Print binding points */
        for (const auto& obj : objects)
        {
            output_ << indentHandler_.FullIndent();
            if (maxLocation >= 0)
            {
                if (obj.location >= 0)
                    output_ << std::string(maxLocationLen - std::to_string(obj.location).size(), ' ') << obj.location << ": ";
                else
                    output_ << std::string(maxLocationLen, ' ') << "  ";
            }
            output_ << obj.ident << std::endl;
        }
    }
    else
        IndentOut() << "< none >" << std::endl;
}

void ReflectionPrinter::PrintReflectionObjects(const std::vector<std::string>& idents, const std::string& title)
{
    IndentOut() << title << ':' << std::endl;
    ScopedIndent indent(indentHandler_);

    if (!idents.empty())
    {
        for (const auto& i : idents)
            IndentOut() << i << std::endl;
    }
    else
        IndentOut() << "< none >" << std::endl;
}

void ReflectionPrinter::PrintReflectionObjects(const std::map<std::string, Reflection::SamplerState>& samplerStates, const std::string& title)
{
    IndentOut() << title << ':' << std::endl;
    ScopedIndent indent(indentHandler_);
    
    if (!samplerStates.empty())
    {
        for (const auto& it : samplerStates)
        {
            IndentOut() << it.first << std::endl;
            indentHandler_.IncIndent();
            {
                const auto& smpl = it.second;
                const auto& brdCol = smpl.borderColor;
                IndentOut() << "AddressU       = " << ToString(smpl.addressU) << std::endl;
                IndentOut() << "AddressV       = " << ToString(smpl.addressV) << std::endl;
                IndentOut() << "AddressW       = " << ToString(smpl.addressW) << std::endl;
                IndentOut() << "BorderColor    = { " << brdCol[0] << ", " << brdCol[1] << ", " << brdCol[2] << ", " << brdCol[3] << " }" << std::endl;
                IndentOut() << "ComparisonFunc = " << ToString(smpl.comparisonFunc) << std::endl;
                IndentOut() << "Filter         = " << ToString(smpl.filter) << std::endl;
                IndentOut() << "MaxAnisotropy  = " << smpl.maxAnisotropy << std::endl;
                IndentOut() << "MaxLOD         = " << smpl.maxLOD << std::endl;
                IndentOut() << "MinLOD         = " << smpl.minLOD << std::endl;
                IndentOut() << "MipLODBias     = " << smpl.mipLODBias << std::endl;
            }
            indentHandler_.DecIndent();
        }
    }
    else
        IndentOut() << "< none >" << std::endl;
}

void ReflectionPrinter::PrintReflectionAttribute(const Reflection::NumThreads& numThreads, const std::string& title)
{
    IndentOut() << title << ':' << std::endl;
    ScopedIndent indent(indentHandler_);

    IndentOut() << "X = " << numThreads.x << std::endl;
    IndentOut() << "Y = " << numThreads.y << std::endl;
    IndentOut() << "Z = " << numThreads.z << std::endl;
}


} // /namespace Xsc



// ================================================================================
