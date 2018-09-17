/*
 * ReflectionPrinter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
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
        PrintReflectionObjects  ( reflectionData.macros,                "Macros"                );
        PrintReflectionObjects  ( reflectionData.inputAttributes,       "Input Attributes"      );
        PrintReflectionObjects  ( reflectionData.outputAttributes,      "Output Attributes"     );
        PrintReflectionObjects  ( reflectionData.uniforms,              "Uniforms"              );
        PrintReflectionObjects  ( reflectionData.resources,             "Resources"             );
        PrintReflectionObjects  ( reflectionData.constantBuffers,       "Constant Buffers"      );
        PrintReflectionObjects  ( reflectionData.samplerStates,         "Sampler States"        );
        PrintReflectionObjects  ( reflectionData.staticSamplerStates,   "Static Sampler States" );
        PrintReflectionAttribute( reflectionData.numThreads,            "Number of Threads"     );
    }
    indentHandler_.DecIndent();
}


/*
 * ======= Private: =======
 */

template <typename T>
std::size_t GetMaxSlotLength(const std::vector<T>& container, int& maxSlot)
{
    for (const auto& entry : container)
        maxSlot = std::max(maxSlot, entry.slot);
    return std::to_string(maxSlot).size();
}

std::ostream& ReflectionPrinter::IndentOut()
{
    output_ << indentHandler_.FullIndent();
    return output_;
}

void ReflectionPrinter::PrintReflectionObjects(const std::vector<Reflection::Attribute>& objects, const char* title)
{
    IndentOut() << title << ':' << std::endl;
    ScopedIndent indent(indentHandler_);

    if (!objects.empty())
    {
        /* Determines the offset for right-aligned location index */
        int maxSlot = -1;
        auto maxSlotLen = GetMaxSlotLength(objects, maxSlot);

        /* Print binding points */
        for (const auto& obj : objects)
        {
            output_ << indentHandler_.FullIndent();
            if (maxSlot >= 0)
            {
                if (obj.slot >= 0)
                    output_ << std::string(maxSlotLen - std::to_string(obj.slot).size(), ' ') << obj.slot << ": ";
                else
                    output_ << std::string(maxSlotLen, ' ') << "  ";
            }
            output_ << obj.name << std::endl;
        }
    }
    else
        IndentOut() << "< none >" << std::endl;
}

void ReflectionPrinter::PrintReflectionObjects(const std::vector<std::string>& idents, const char* title)
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

void ReflectionPrinter::PrintReflectionObjects(const std::vector<Reflection::Resource>& objects, const char* title)
{
    IndentOut() << title << ':' << std::endl;
    ScopedIndent indent(indentHandler_);

    if (!objects.empty())
    {
        /* Determines the offset for right-aligned location index */
        int maxSlot = -1;
        auto maxSlotLen = GetMaxSlotLength(objects, maxSlot);

        /* Print binding points */
        for (const auto& obj : objects)
        {
            output_ << indentHandler_.FullIndent();
            if (maxSlot >= 0)
            {
                if (obj.slot >= 0)
                    output_ << std::string(maxSlotLen - std::to_string(obj.slot).size(), ' ') << obj.slot << ": ";
                else
                    output_ << std::string(maxSlotLen, ' ') << "  ";
            }
            output_ << obj.name << " <" << ToString(obj.type) << '>' << std::endl;
        }
    }
    else
        IndentOut() << "< none >" << std::endl;
}

void ReflectionPrinter::PrintReflectionObjects(const std::vector<Reflection::ConstantBuffer>& objects, const char* title)
{
    IndentOut() << title << ':' << std::endl;
    ScopedIndent indent(indentHandler_);

    if (!objects.empty())
    {
        /* Determines the offset for right-aligned location index */
        int maxSlot = -1;
        auto maxSlotLen = GetMaxSlotLength(objects, maxSlot);

        /* Print binding points */
        for (const auto& obj : objects)
        {
            output_ << indentHandler_.FullIndent();
            if (maxSlot >= 0)
            {
                if (obj.slot >= 0)
                    output_ << std::string(maxSlotLen - std::to_string(obj.slot).size(), ' ') << obj.slot << ": ";
                else
                    output_ << std::string(maxSlotLen, ' ') << "  ";
            }
            output_ << obj.name << " <" << ToString(obj.type);
            if (obj.size != ~0)
                output_ << "(size: " << obj.size << ", padding: " << obj.padding << ')';
            output_ << '>' << std::endl;
        }
    }
    else
        IndentOut() << "< none >" << std::endl;
}

void ReflectionPrinter::PrintReflectionObjects(const std::vector<Reflection::SamplerState>& objects, const char* title)
{
    IndentOut() << title << ':' << std::endl;
    ScopedIndent indent(indentHandler_);

    if (!objects.empty())
    {
        /* Determines the offset for right-aligned location index */
        int maxSlot = -1;
        auto maxSlotLen = GetMaxSlotLength(objects, maxSlot);

        /* Print binding points */
        for (const auto& obj : objects)
        {
            output_ << indentHandler_.FullIndent();
            if (maxSlot >= 0)
            {
                if (obj.slot >= 0)
                    output_ << std::string(maxSlotLen - std::to_string(obj.slot).size(), ' ') << obj.slot << ": ";
                else
                    output_ << std::string(maxSlotLen, ' ') << "  ";
            }
            output_ << obj.name << std::endl;
        }
    }
    else
        IndentOut() << "< none >" << std::endl;
}

void ReflectionPrinter::PrintReflectionObjects(const std::vector<Reflection::StaticSamplerState>& samplerStates, const char* title)
{
    IndentOut() << title << ':' << std::endl;
    ScopedIndent indent(indentHandler_);

    if (!samplerStates.empty())
    {
        for (const auto& sampler : samplerStates)
        {
            IndentOut() << sampler.name << std::endl;
            indentHandler_.IncIndent();
            {
                const auto& desc = sampler.desc;
                const auto& brdCol = desc.borderColor;
                IndentOut() << "AddressU       = " << ToString(desc.addressU) << std::endl;
                IndentOut() << "AddressV       = " << ToString(desc.addressV) << std::endl;
                IndentOut() << "AddressW       = " << ToString(desc.addressW) << std::endl;
                IndentOut() << "BorderColor    = { " << brdCol[0] << ", " << brdCol[1] << ", " << brdCol[2] << ", " << brdCol[3] << " }" << std::endl;
                IndentOut() << "ComparisonFunc = " << ToString(desc.comparisonFunc) << std::endl;
                IndentOut() << "Filter         = " << ToString(desc.filter) << std::endl;
                IndentOut() << "MaxAnisotropy  = " << desc.maxAnisotropy << std::endl;
                IndentOut() << "MaxLOD         = " << desc.maxLOD << std::endl;
                IndentOut() << "MinLOD         = " << desc.minLOD << std::endl;
                IndentOut() << "MipLODBias     = " << desc.mipLODBias << std::endl;
            }
            indentHandler_.DecIndent();
        }
    }
    else
        IndentOut() << "< none >" << std::endl;
}

void ReflectionPrinter::PrintReflectionAttribute(const Reflection::NumThreads& numThreads, const char* title)
{
    if (numThreads.x > 0 || numThreads.y > 0 || numThreads.z > 0)
    {
        IndentOut() << title << ':' << std::endl;
        ScopedIndent indent(indentHandler_);

        IndentOut() << "X = " << numThreads.x << std::endl;
        IndentOut() << "Y = " << numThreads.y << std::endl;
        IndentOut() << "Z = " << numThreads.z << std::endl;
    }
}


} // /namespace Xsc



// ================================================================================
