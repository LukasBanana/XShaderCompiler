/*
 * UniformPacker.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "UniformPacker.h"
#include "AST.h"
#include "ASTFactory.h"


namespace Xsc
{


//TODO: combine two code blocks for global statements and function parameters in a generic way
void UniformPacker::Convert(Program& program, const CbufferAttributes& cbufferAttribs, bool onlyReachableStmnts)
{
    if (cbufferAttribs.name.empty())
        return;

    cbufferAttribs_ = cbufferAttribs;

    /* Convert global statements */
    auto& globalStmnts = program.globalStmnts;

    for (auto it = globalStmnts.begin(); it != globalStmnts.end();)
    {
        bool isReachable = (*it)->flags(AST::isReachable);
        if ((*it)->Type() == AST::Types::VarDeclStmnt && (isReachable || !onlyReachableStmnts))
        {
            auto varDeclStmnt = std::static_pointer_cast<VarDeclStmnt>(*it);

            /* Check if variable declarations have a uniform type that is neither a sampler nor buffer */
            if (varDeclStmnt->IsUniform() && CanConvertUniformWithTypeDenoter(*(varDeclStmnt->typeSpecifier->typeDenoter)))
            {
                /* Check if uniform buffer has not been created yet */
                if (!declStmnt_)
                {
                    /* Make initial uniform buffer */
                    MakeUniformBuffer();

                    /* Append uniform into constant buffer and remove it from global statements */
                    AppendUniform(varDeclStmnt);
                    it = globalStmnts.erase(it);

                    /* Insert uniform buffer into global statement list */
                    it = globalStmnts.insert(it, declStmnt_);
                }
                else
                {
                    /* Append uniform into constant buffer and remove it from global statements */
                    AppendUniform(varDeclStmnt);
                    it = globalStmnts.erase(it);
                    continue;
                }

                /* Mark as reachable, if only a single variable of it is reachable */
                if (isReachable)
                {
                    declStmnt_->flags << AST::isReachable;
                    declStmnt_->declObject->flags << AST::isReachable;
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
            auto varDeclStmnt = *it;

            /* Check if variable declarations have a uniform type that is neither a sampler nor buffer */
            if (varDeclStmnt->IsUniform() && CanConvertUniformWithTypeDenoter(*(varDeclStmnt->typeSpecifier->typeDenoter)))
            {
                /* Check if uniform buffer has not been created yet */
                if (!declStmnt_)
                {
                    /* Make initial uniform buffer */
                    MakeUniformBuffer();

                    /* Append uniform into constant buffer and remove it from global statements */
                    AppendUniform(varDeclStmnt);
                    it = parameters.erase(it);

                    /* Insert uniform buffer into global statement list */
                    globalStmnts.insert(globalStmnts.begin(), declStmnt_);
                }
                else
                {
                    /* Append uniform into constant buffer and remove it from global statements */
                    AppendUniform(varDeclStmnt);
                    it = parameters.erase(it);
                }

                /* Mark as reachable, if only a single variable of it is reachable */
                //if (isReachable)
                {
                    declStmnt_->flags << AST::isReachable;
                    declStmnt_->declObject->flags << AST::isReachable;
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
    declStmnt_ = std::make_shared<BasicDeclStmnt>(SourcePosition::ignore);
    {
        uniformBufferDecl_ = ASTFactory::MakeUniformBufferDecl(cbufferAttribs_.name, cbufferAttribs_.bindingSlot);
        uniformBufferDecl_->declStmntRef = declStmnt_.get();
    }
    declStmnt_->declObject = uniformBufferDecl_;
}

void UniformPacker::AppendUniform(const VarDeclStmntPtr& varDeclStmnt)
{
    /* Append shared pointer to both local statements (main list), and the variable members (secondary list) */
    uniformBufferDecl_->localStmnts.push_back(varDeclStmnt);
    uniformBufferDecl_->varMembers.push_back(varDeclStmnt);

    /* Remove "uniform" specifier */
    varDeclStmnt->typeSpecifier->isUniform = false;

    /* Remove default initializer */
    for (auto& varDecl : varDeclStmnt->varDecls)
        varDecl->initializer.reset();
}

bool UniformPacker::CanConvertUniformWithTypeDenoter(const TypeDenoter& typeDen) const
{
    return !(typeDen.IsSampler() || typeDen.IsBuffer());
}


} // /namespace Xsc



// ================================================================================
