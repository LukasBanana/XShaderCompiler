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

void ReflectionPrinter::PrintReflection(const Reflection::ReflectionData& reflectionData, bool referencedOnly)
{
    output_ << R_CodeReflection() << ':' << std::endl;
    indentHandler_.IncIndent();
    {
        PrintReflectionObjects  ( reflectionData.macros,                "Macros"                               );
        PrintReflectionObjects  ( reflectionData.records,               "Structures",           referencedOnly );
        PrintReflectionObjects  ( reflectionData.inputAttributes,       "Input Attributes",     referencedOnly );
        PrintReflectionObjects  ( reflectionData.outputAttributes,      "Output Attributes",    referencedOnly );
        PrintReflectionObjects  ( reflectionData.uniforms,              "Uniforms",             referencedOnly );
        PrintReflectionObjects  ( reflectionData.resources,             "Resources",            referencedOnly );
        PrintReflectionObjects  ( reflectionData.constantBuffers,       "Constant Buffers",     referencedOnly );
        PrintReflectionObjects  ( reflectionData.samplerStates,         "Sampler States",       referencedOnly );
        PrintReflectionObjects  ( reflectionData.staticSamplerStates,   "Static Sampler States"                );
        PrintReflectionAttribute( reflectionData.numThreads,            "Number of Threads"                    );
    }
    indentHandler_.DecIndent();
}


/*
 * ======= Private: =======
 */

// Template to determine if the 'referenced' member in any object of the list is true.
template <typename T>
bool HasAnyReferencedObjects(const std::vector<T>& list)
{
    auto it = std::find_if(
        list.begin(), list.end(),
        [](const T& entry) -> bool
        {
            return entry.referenced;
        }
    );
    return (it != list.end());
}

template <typename T>
std::size_t GetMaxSlotLength(const std::vector<T>& container, int& maxSlot)
{
    for (const auto& entry : container)
        maxSlot = std::max(maxSlot, entry.slot);
    return std::to_string(maxSlot).size();
}

template <typename T>
std::size_t GetMaxSlotLength(const std::vector<T>& container, int& maxSlot, bool referencedOnly)
{
    for (const auto& entry : container)
    {
        if (!referencedOnly || entry.referenced)
            maxSlot = std::max(maxSlot, entry.slot);
    }
    return std::to_string(maxSlot).size();
}

std::ostream& ReflectionPrinter::IndentOut()
{
    output_ << indentHandler_.FullIndent();
    return output_;
}

void ReflectionPrinter::PrintReflectionObjects(const std::vector<std::string>& idents, const char* title)
{
    IndentOut() << title << ':' << std::endl;
    ScopedIndent indent { indentHandler_ };

    if (!idents.empty())
    {
        for (const auto& i : idents)
            IndentOut() << i << std::endl;
    }
    else
        IndentOut() << "< none >" << std::endl;
}

void ReflectionPrinter::PrintFields(const std::vector<Reflection::Field>& objects, bool referencedOnly)
{
    if (!objects.empty() && (!referencedOnly || HasAnyReferencedObjects(objects)))
    {
        /* Print fields */
        for (const auto& obj : objects)
        {
            if (!referencedOnly || obj.referenced)
            {
                output_ << indentHandler_.FullIndent();
                output_ << obj.name << " <Field";
                if (obj.size != ~0)
                    output_ << "(offset: " << obj.offset << ", size: " << obj.size << ')';
                output_ << '>' << std::endl;
            }
        }
    }
}

void ReflectionPrinter::PrintReflectionObjects(const std::vector<Reflection::Record>& objects, const char* title, bool referencedOnly)
{
    IndentOut() << title << ':' << std::endl;
    ScopedIndent indent { indentHandler_ };

    if (!objects.empty() && (!referencedOnly || HasAnyReferencedObjects(objects)))
    {
        /* Print records */
        for (const auto& obj : objects)
        {
            if (!referencedOnly || obj.referenced)
            {
                /* Print record identity */
                output_ << indentHandler_.FullIndent();
                output_ << obj.name << " <Structure";
                if (obj.size != ~0)
                    output_ << "(size: " << obj.size << ", padding: " << obj.padding << ')';
                output_ << '>' << std::endl;

                /* Print fields */
                ScopedIndent indent { indentHandler_ };
                PrintFields(obj.fields, referencedOnly);
            }
        }
    }
    else
        IndentOut() << "< none >" << std::endl;
}

void ReflectionPrinter::PrintReflectionObjects(const std::vector<Reflection::Attribute>& objects, const char* title, bool referencedOnly)
{
    IndentOut() << title << ':' << std::endl;
    ScopedIndent indent { indentHandler_ };

    if (!objects.empty() && (!referencedOnly || HasAnyReferencedObjects(objects)))
    {
        /* Determines the offset for right-aligned location index */
        int maxSlot = -1;
        auto maxSlotLen = GetMaxSlotLength(objects, maxSlot, referencedOnly);

        /* Print binding points */
        for (const auto& obj : objects)
        {
            if (!referencedOnly || obj.referenced)
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
    }
    else
        IndentOut() << "< none >" << std::endl;
}

void ReflectionPrinter::PrintReflectionObjects(const std::vector<Reflection::Resource>& objects, const char* title, bool referencedOnly)
{
    IndentOut() << title << ':' << std::endl;
    ScopedIndent indent { indentHandler_ };

    if (!objects.empty() && (!referencedOnly || HasAnyReferencedObjects(objects)))
    {
        /* Determines the offset for right-aligned location index */
        int maxSlot = -1;
        auto maxSlotLen = GetMaxSlotLength(objects, maxSlot, referencedOnly);

        /* Print binding points */
        for (const auto& obj : objects)
        {
            if (!referencedOnly || obj.referenced)
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
    }
    else
        IndentOut() << "< none >" << std::endl;
}

void ReflectionPrinter::PrintReflectionObjects(const std::vector<Reflection::ConstantBuffer>& objects, const char* title, bool referencedOnly)
{
    IndentOut() << title << ':' << std::endl;
    ScopedIndent indent { indentHandler_ };

    if (!objects.empty() && (!referencedOnly || HasAnyReferencedObjects(objects)))
    {
        /* Determines the offset for right-aligned location index */
        int maxSlot = -1;
        auto maxSlotLen = GetMaxSlotLength(objects, maxSlot, referencedOnly);

        /* Print binding points */
        for (const auto& obj : objects)
        {
            if (!referencedOnly || obj.referenced)
            {
                /* Print constant buffer identity */
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

                /* Print fields */
                ScopedIndent indent { indentHandler_ };
                PrintFields(obj.fields, referencedOnly);
            }
        }
    }
    else
        IndentOut() << "< none >" << std::endl;
}

void ReflectionPrinter::PrintReflectionObjects(const std::vector<Reflection::SamplerState>& objects, const char* title, bool referencedOnly)
{
    IndentOut() << title << ':' << std::endl;
    ScopedIndent indent { indentHandler_ };

    if (!objects.empty() && (!referencedOnly || HasAnyReferencedObjects(objects)))
    {
        /* Determines the offset for right-aligned location index */
        int maxSlot = -1;
        auto maxSlotLen = GetMaxSlotLength(objects, maxSlot, referencedOnly);

        /* Print binding points */
        for (const auto& obj : objects)
        {
            if (!referencedOnly || obj.referenced)
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
    }
    else
        IndentOut() << "< none >" << std::endl;
}

void ReflectionPrinter::PrintReflectionObjects(const std::vector<Reflection::StaticSamplerState>& samplerStates, const char* title)
{
    IndentOut() << title << ':' << std::endl;
    ScopedIndent indent { indentHandler_ };

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
        ScopedIndent indent { indentHandler_ };

        IndentOut() << "X = " << numThreads.x << std::endl;
        IndentOut() << "Y = " << numThreads.y << std::endl;
        IndentOut() << "Z = " << numThreads.z << std::endl;
    }
}


} // /namespace Xsc



// ================================================================================
