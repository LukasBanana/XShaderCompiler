/*
 * UniformPacker.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "UniformPacker.h"
#include "AST.h"
#include "ASTFactory.h"


namespace Xsc
{


//TODO: combine two code blocks for global statements and function parameters in a generic way
void UniformPacker::Convert(Program& program, const CbufferAttributes& cbufferAttribs, bool onlyReachableStmts)
{
    if (cbufferAttribs.name.empty())
        return;

    cbufferAttribs_ = cbufferAttribs;

    /* Convert global statements */
    auto& globalStmts = program.globalStmts;

    for (auto it = globalStmts.begin(); it != globalStmts.end();)
    {
        bool isReachable = (*it)->flags(AST::isReachable);
        if ((*it)->Type() == AST::Types::VarDeclStmt && (isReachable || !onlyReachableStmts))
        {
            auto varDeclStmt = std::static_pointer_cast<VarDeclStmt>(*it);

            /* Check if variable declarations have a uniform type that is neither a sampler nor buffer */
            if (varDeclStmt->IsUniform() && CanConvertUniformWithTypeDenoter(*(varDeclStmt->typeSpecifier->typeDenoter)))
            {
                /* Check if uniform buffer has not been created yet */
                if (!declStmt_)
                {
                    /* Make initial uniform buffer */
                    MakeUniformBuffer();

                    /* Append uniform into constant buffer and remove it from global statements */
                    AppendUniform(varDeclStmt);
                    it = globalStmts.erase(it);

                    /* Insert uniform buffer into global statement list */
                    it = globalStmts.insert(it, declStmt_);
                }
                else
                {
                    /* Append uniform into constant buffer and remove it from global statements */
                    AppendUniform(varDeclStmt);
                    it = globalStmts.erase(it);
                    continue;
                }

                /* Mark as reachable, if only a single variable of it is reachable */
                if (isReachable)
                {
                    declStmt_->flags << AST::isReachable;
                    declStmt_->declObject->flags << AST::isReachable;
                }
            }
        }

        ++it;
    }

    /* Convert parameters of main entry point */
    if (auto entryPoint = program.entryPointRef)
    {
        auto& parameters = entryPoint->parameters;
        for (auto it = parameters.begin(); it != parameters.end();)
        {
            auto varDeclStmt = *it;

            /* Check if variable declarations have a uniform type that is neither a sampler nor buffer */
            if (varDeclStmt->IsUniform() && CanConvertUniformWithTypeDenoter(*(varDeclStmt->typeSpecifier->typeDenoter)))
            {
                /* Check if uniform buffer has not been created yet */
                if (!declStmt_)
                {
                    /* Make initial uniform buffer */
                    MakeUniformBuffer();

                    /* Append uniform into constant buffer and remove it from global statements */
                    AppendUniform(varDeclStmt);
                    it = parameters.erase(it);

                    /* Insert uniform buffer into global statement list */
                    globalStmts.insert(globalStmts.begin(), declStmt_);
                }
                else
                {
                    /* Append uniform into constant buffer and remove it from global statements */
                    AppendUniform(varDeclStmt);
                    it = parameters.erase(it);
                }

                /* Mark as reachable, if only a single variable of it is reachable */
                //if (isReachable)
                {
                    declStmt_->flags << AST::isReachable;
                    declStmt_->declObject->flags << AST::isReachable;
                }
            }
            else
                ++it;
        }
    }
}


/*
 * ======= Private: =======
 */

void UniformPacker::MakeUniformBuffer()
{
    /* Make single constant buffer to pack uniforms into */
    declStmt_ = std::make_shared<BasicDeclStmt>(SourcePosition::ignore);
    {
        uniformBufferDecl_ = ASTFactory::MakeUniformBufferDecl(cbufferAttribs_.name, cbufferAttribs_.bindingSlot);
        uniformBufferDecl_->declStmtRef = declStmt_.get();
    }
    declStmt_->declObject = uniformBufferDecl_;
}

void UniformPacker::AppendUniform(const VarDeclStmtPtr& varDeclStmt)
{
    /* Append shared pointer to both local statements (main list), and the variable members (secondary list) */
    uniformBufferDecl_->localStmts.push_back(varDeclStmt);
    uniformBufferDecl_->varMembers.push_back(varDeclStmt);

    /* Remove "uniform" specifier */
    varDeclStmt->typeSpecifier->isUniform = false;

    /* Remove default initializer */
    for (auto& varDecl : varDeclStmt->varDecls)
        varDecl->initializer.reset();
}

bool UniformPacker::CanConvertUniformWithTypeDenoter(const TypeDenoter& typeDen) const
{
    return !(typeDen.IsSampler() || typeDen.IsBuffer());
}


} // /namespace Xsc



// ================================================================================
