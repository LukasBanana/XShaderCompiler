/*
 * ReflectionPrinter.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_REFLECTION_PRINTER_H
#define XSC_REFLECTION_PRINTER_H


#include <Xsc/IndentHandler.h>
#include <Xsc/Reflection.h>
#include <ostream>


namespace Xsc
{


class ReflectionPrinter
{

    public:

        ReflectionPrinter(std::ostream& output);

        void PrintReflection(const Reflection::ReflectionData& reflectionData, bool referencedOnly = false);

    private:

        std::ostream& IndentOut();

        void PrintReflectionObjects(const std::vector<std::string>& idents, const char* title);
        void PrintReflectionObjects(const std::vector<Reflection::Record>& objects, const char* title, bool referencedOnly);
        void PrintReflectionObjects(const std::vector<Reflection::Attribute>& objects, const char* title, bool referencedOnly);
        void PrintReflectionObjects(const std::vector<Reflection::Resource>& objects, const char* title, bool referencedOnly);
        void PrintReflectionObjects(const std::vector<Reflection::ConstantBuffer>& objects, const char* title, bool referencedOnly);
        void PrintReflectionObjects(const std::vector<Reflection::SamplerState>& objects, const char* title, bool referencedOnly);
        void PrintReflectionObjects(const std::vector<Reflection::StaticSamplerState>& samplerStates, const char* title);
        void PrintReflectionAttribute(const Reflection::NumThreads& numThreads, const char* title);

        std::ostream&   output_;
        IndentHandler   indentHandler_;

};


} // /namespace Xsc


#endif



// ================================================================================