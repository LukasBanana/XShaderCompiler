/*
 * CFGBuilder.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CFGBuilder.h"
#include "AST.h"


namespace Xsc
{


/*CFGBuilder::CFGBuilder(Log* log) :
    Generator { log }
{
}*/


/*
 * ======= Private: =======
 */

CFGBuilder::CFG CFGBuilder::MakeCFG(const std::string& name)
{
    return
    {
        moduleFunc_->MakeBlock(name),
        moduleFunc_->MakeBlock("end" + name)
    };
}

void CFGBuilder::PushCFG(const CFG& cfg)
{
    cfgStack_.push(cfg);
}

CFGBuilder::CFG CFGBuilder::PopCFG()
{
    if (!cfgStack_.empty())
    {
        auto cfg = cfgStack_.top();
        cfgStack_.pop();
        return cfg;
    }
    return { nullptr, nullptr };
}

void CFGBuilder::PushBreak(BasicBlock* bb)
{
    breakBlockStack_.push(bb);
}

void CFGBuilder::PopBreak()
{
    if (!breakBlockStack_.empty())
        breakBlockStack_.pop();
}

BasicBlock* CFGBuilder::TopBreak() const
{
    return (breakBlockStack_.empty() ? nullptr : breakBlockStack_.top());
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void CFGBuilder::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(Program)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(SwitchCase)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(ArrayDimension)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(TypeSpecifier)
{
    //TODO
}

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(SamplerDecl)
{
    //TODO
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    moduleFunc_ = module_.MakeFunction(ast->ident);



    Visit(ast->codeBlock);
}

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(BufferDeclStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(SamplerDeclStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(AliasDeclStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(BasicDeclStmnt)
{
    //TODO
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(NullStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(CodeBlockStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(ForLoopStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(WhileLoopStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmnt)
{
    //TODO
}

/*
    if             if
   /  \           /  \
then  else  or  then  |
   \  /           \  /
   endif          endif
*/
IMPLEMENT_VISIT_PROC(IfStmnt)
{
    /* Create start and end blocks */
    auto cfg    = MakeCFG("if");
    auto bbElse = (ast->elseStmnt ? moduleFunc_->MakeBlock("elseif") : cfg.out);

    /* Create condition CFG */
    PushBreak(bbElse);
    
    Visit(ast->condition);
    auto cfgCond = PopCFG();
    
    PopBreak();

    cfg.in->AddSucc(*cfgCond.in, "condition");

    /* Create then branch CFG */
    Visit(ast->bodyStmnt);
    auto cfgThen = PopCFG();
    
    cfgCond.out->AddSucc(*cfgThen.in);
    cfgThen.out->AddSucc(*cfg.out);

    if (ast->elseStmnt)
    {
        /* Create else branch CFG */
        Visit(ast->elseStmnt);
        auto cfgElse = PopCFG();

        bbElse->AddSucc(*cfgElse.in);
        cfgElse.out->AddSucc(*cfg.out);
    }

    /* Push output block */
    PushCFG(cfg);
}

IMPLEMENT_VISIT_PROC(ElseStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(SwitchStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(ExprStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(CtrlTransferStmnt)
{
    //TODO
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(SequenceExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(LiteralExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(TypeSpecifierExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(TernaryExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(PostUnaryExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(CallExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(BracketExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(ObjectExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(AssignExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(ArrayExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(InitializerExpr)
{
    //TODO
}

#undef IMPLEMENT_VISIT_PROC

/* --- Helper functions for code generation --- */


} // /namespace Xsc



// ================================================================================
