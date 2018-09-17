/*
 * FuncNameConverter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "FuncNameConverter.h"
#include "AST.h"


namespace Xsc
{


void FuncNameConverter::Convert(
    Program&                        program,
    const NameMangling&             nameMangling,
    const OnFuncSigantureCompare&   onFuncSignatureCompare,
    const Flags&                    conversionFlags)
{
    /* Copy parameters */
    nameMangling_           = nameMangling;
    onFuncSignatureCompare_ = onFuncSignatureCompare;
    conversionFlags_        = conversionFlags;

    /* Visit program AST */
    Visit(&program);

    /* Convert equal function signatures */
    for (auto& it : funcDeclMap_)
         ConvertEqualFunctionSignatures(it.second);
}


/*
 * ======= Private: =======
 */

void FuncNameConverter::ConvertEqualFunctionSignatures(FuncList& funcList)
{
    /* Check if name mangling is required (if any function declaration is equal to another) */
    if (onFuncSignatureCompare_)
    {
        /* Compare all functions with all other functions (number of comparisons: (n-1) + (n-2) + ... + 1) */
        unsigned int nameIndex = 0;

        for (std::size_t i = 0, n = funcList.size(); i + 1 < n; ++i)
        {
            if (auto funcLhs = funcList[i])
            {
                for (std::size_t j = i + 1; j < n; ++j)
                {
                    if (auto funcRhs = funcList[j])
                    {
                        if (onFuncSignatureCompare_(*funcLhs, *funcRhs))
                        {
                            /* Rename first function (if not already done) */
                            if (nameIndex == 0)
                                ConvertMemberFunctionName(*funcLhs, nameIndex);

                            /* Rename second function */
                            ConvertMemberFunctionName(*funcRhs, nameIndex);

                            /* Remove second function from the list, to avoid another renaming */
                            funcList[j] = nullptr;
                        }
                    }
                }
            }
        }
    }
}

void FuncNameConverter::ConvertMemberFunctionName(FunctionDecl& funcDecl, unsigned int& nameIndex)
{
    /* Rename function to "{Prefix}{FunctionName}_{Index}" */
    funcDecl.ident.AppendPrefix(nameMangling_.namespacePrefix);
    funcDecl.ident = (funcDecl.ident + "_" + std::to_string(nameIndex));

    /* Increase index for next function name mangling */
    ++nameIndex;
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void FuncNameConverter::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    // ignore sub nodes here
}

IMPLEMENT_VISIT_PROC(BufferDeclStmnt)
{
    // ignore sub nodes here
}

IMPLEMENT_VISIT_PROC(SamplerDeclStmnt)
{
    // ignore sub nodes here
}

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    /* Rename member functions (if flag enabled) */
    if (conversionFlags_(RenameMemberFunctions))
    {
        if (auto structDecl = ast->structDeclRef)
        {
            /* Rename function to "{TempPrefix}{StructName}_{FuncName}" */
            ast->ident.RemovePrefix(nameMangling_.namespacePrefix);
            ast->ident = structDecl->ident + "_" + ast->ident;
            ast->ident.AppendPrefix(nameMangling_.namespacePrefix);
        }
    }

    /* Add function declaration to map for later renamging of equal function signatures (if flag enabled) */
    if (conversionFlags_(RenameFunctionSignatures) && !ast->IsForwardDecl())
        funcDeclMap_[ast->ident].push_back(ast);
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================