/*
 * FuncNameConverter.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_FUNC_NAME_CONVERTER_H
#define XSC_FUNC_NAME_CONVERTER_H


#include "VisitorTracker.h"
#include "Flags.h"
#include <Xsc/Xsc.h>
#include <functional>
#include <map>


namespace Xsc
{


// Function name mangling AST converter.
class FuncNameConverter : public VisitorTracker
{

    public:

        // Conversion flags enumeration.
        enum : unsigned int
        {
            // Rename all member functions to "{Prefix}{OwnerStruct}_{FunctionName}"
            RenameMemberFunctions       = (1 << 0),

            // Rename functions where the signatures are equal, specified by the signature compare callback.
            RenameFunctionSignatures    = (1 << 1),

            All                         = (RenameMemberFunctions | RenameFunctionSignatures),
        };

        // Function signature compare callback interface, which returns true if the signatures are considered to be equal.
        using OnFuncSigantureCompare = std::function<bool(const FunctionDecl& lhs, const FunctionDecl& rhs)>;

        // Converts the function declaration identigiers in the specified AST.
        void Convert(
            Program&                        program,
            const NameMangling&             nameMangling,
            const OnFuncSigantureCompare&   onFuncSignatureCompare,
            const Flags&                    conversionFlags
        );

    private:

        /* ----- Conversion ----- */

        using FuncList = std::vector<FunctionDecl*>;

        void ConvertEqualFunctionSignatures(FuncList& funcList);
        void ConvertMemberFunctionName(FunctionDecl& funcDecl, unsigned int& nameIndex);

        /* ----- Visitor implementation ----- */

        DECL_VISIT_PROC( UniformBufferDecl );
        DECL_VISIT_PROC( BufferDeclStmt    );
        DECL_VISIT_PROC( SamplerDeclStmt   );

        DECL_VISIT_PROC( FunctionDecl      );

    private:

        NameMangling                    nameMangling_;
        OnFuncSigantureCompare          onFuncSignatureCompare_;
        Flags                           conversionFlags_;

        std::map<std::string, FuncList> funcDeclMap_;

};


} // /namespace Xsc


#endif



// ================================================================================