/*
 * ASTPrinter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ASTPrinter.h"
#include "AST.h"


namespace Xsc
{


void ASTPrinter::PrintAST(Program* program, Log& log)
{
    log_ = &log;
    Visit(program);
}


/*
 * ======= Private: =======
 */

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ASTPrinter::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(Program)
{
    Print(ast, "Program");
    PushNode();
    {
        VisitAndSignalLast(ast->globalStmnts);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    Print(ast, "CodeBlock");
    PushNode();
    {
        VisitAndSignalLast(ast->stmnts);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    Print(ast, "FunctionCall", (ast->varIdent ? ast->varIdent->Last()->ident : ""));
    PushNode();
    {
        Visit(ast->varIdent);
        VisitAndSignalLast(ast->arguments);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(Attribute)
{
    Print(ast, "Attribute");
    PushNode();
    {
        VisitAndSignalLast(ast->arguments);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(SwitchCase)
{
    Print(ast, "SwitchCase");
    PushNode();
    {
        Visit(ast->expr);
        VisitAndSignalLast(ast->stmnts);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(SamplerValue)
{
    Print(ast, "SamplerValue", ast->name);
    PushNode();
    {
        VisitAndSignalLast(ast->value);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(Register)
{
    Print(ast, "Register", ast->ToString());
}

IMPLEMENT_VISIT_PROC(PackOffset)
{ 
    Print(ast, "PackOffset", ast->ToString());
}

IMPLEMENT_VISIT_PROC(ArrayDimension)
{
    Print(ast, "ArrayDimension", ast->ToString());
    PushNode();
    {
        VisitAndSignalLast(ast->expr);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(TypeName)
{
    Print(ast, "TypeName", ast->ToString());
    PushNode();
    {
        VisitAndSignalLast(ast->structDecl);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(VarIdent)
{
    Print(ast, "VarIdent", ast->ident);
    PushNode();
    {
        if (ast->next)
        {
            Visit(ast->arrayIndices);
            VisitAndSignalLast(ast->next);
        }
        else
            VisitAndSignalLast(ast->arrayIndices);
    }
    PopNode();
}

/* --- Declaration --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    Print(ast, "VarDecl", ast->ident);
    PushNode();
    {
        Visit(ast->arrayDims);
        Visit(ast->packOffset);
        Visit(ast->annotations);
        VisitAndSignalLast(ast->initializer);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(BufferDecl)
{
    Print(ast, "BufferDecl", ast->ident);
    PushNode();
    {
        Visit(ast->arrayDims);
        VisitAndSignalLast(ast->slotRegisters);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(SamplerDecl)
{
    Print(ast, "SamplerDecl", ast->ident);
    PushNode();
    {
        Visit(ast->arrayDims);
        Visit(ast->slotRegisters);
        VisitAndSignalLast(ast->samplerValues);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    auto info = ast->ident;
    if (!ast->baseStructName.empty())
        info += " : " + ast->baseStructName;

    Print(ast, "StructDecl", info);
    PushNode();
    {
        VisitAndSignalLast(ast->members);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(AliasDecl)
{
    Print(ast, "AliasDecl", ast->ident);
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    Print(ast, "FunctionDecl", ast->ident);
    PushNode();
    {
        Visit(ast->attribs);
        if (ast->codeBlock)
        {
            Visit(ast->returnType);
            Visit(ast->parameters);
            Visit(ast->annotations);
            VisitAndSignalLast(ast->codeBlock);
        }
        else if (!ast->annotations.empty())
        {
            Visit(ast->returnType);
            Visit(ast->parameters);
            VisitAndSignalLast(ast->annotations);
        }
        else if (!ast->parameters.empty())
        {
            Visit(ast->returnType);
            VisitAndSignalLast(ast->parameters);
        }
        else
            VisitAndSignalLast(ast->returnType);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    Print(ast, "UniformBufferDecl", ast->ToString());
    PushNode();
    {
        Visit(ast->attribs);
        Visit(ast->members);
        VisitAndSignalLast(ast->slotRegisters);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(BufferDeclStmnt)
{
    Print(ast, "BufferDeclStmnt");
    PushNode();
    {
        Visit(ast->attribs);
        VisitAndSignalLast(ast->bufferDecls);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(SamplerDeclStmnt)
{
    Print(ast, "SamplerDeclStmnt");
    PushNode();
    {
        Visit(ast->attribs);
        VisitAndSignalLast(ast->samplerDecls);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(StructDeclStmnt)
{
    Print(ast, "StructDeclStmnt");
    PushNode();
    {
        Visit(ast->attribs);
        VisitAndSignalLast(ast->structDecl);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    Print(ast, "VarDeclStmnt");
    PushNode();
    {
        Visit(ast->attribs);
        Visit(ast->varType);
        VisitAndSignalLast(ast->varDecls);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(AliasDeclStmnt)
{
    Print(ast, "AliasDeclStmnt");
    PushNode();
    {
        Visit(ast->attribs);
        Visit(ast->structDecl);
        VisitAndSignalLast(ast->aliasDecls);
    }
    PopNode();
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(NullStmnt)
{
    Print(ast, "NullStmnt");
    PushNode();
    {
        VisitAndSignalLast(ast->attribs);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(CodeBlockStmnt)
{
    Print(ast, "CodeBlockStmnt");
    PushNode();
    {
        Visit(ast->attribs);
        VisitAndSignalLast(ast->codeBlock);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(ForLoopStmnt)
{
    Print(ast, "ForLoopStmnt");
    PushNode();
    {
        Visit(ast->attribs);
        Visit(ast->initSmnt);
        Visit(ast->condition);
        Visit(ast->iteration);
        VisitAndSignalLast(ast->bodyStmnt);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(WhileLoopStmnt)
{
    Print(ast, "WhileLoopStmnt");
    PushNode();
    {
        Visit(ast->attribs);
        Visit(ast->condition);
        VisitAndSignalLast(ast->bodyStmnt);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmnt)
{
    Print(ast, "DoWhileLoopStmnt");
    PushNode();
    {
        Visit(ast->attribs);
        Visit(ast->bodyStmnt);
        VisitAndSignalLast(ast->condition);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(IfStmnt)
{
    Print(ast, "IfStmnt");
    PushNode();
    {
        Visit(ast->attribs);
        Visit(ast->condition);
        if (ast->elseStmnt)
        {
            Visit(ast->bodyStmnt);
            VisitAndSignalLast(ast->elseStmnt);
        }
        else
            VisitAndSignalLast(ast->bodyStmnt);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(ElseStmnt)
{
    Print(ast, "ElseStmnt");
    PushNode();
    {
        Visit(ast->attribs);
        VisitAndSignalLast(ast->bodyStmnt);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(SwitchStmnt)
{
    Print(ast, "SwitchStmnt");
    PushNode();
    {
        Visit(ast->attribs);
        Visit(ast->selector);
        VisitAndSignalLast(ast->cases);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(ExprStmnt)
{
    Print(ast, "ExprStmnt");
    PushNode();
    {
        Visit(ast->attribs);
        VisitAndSignalLast(ast->expr);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    Print(ast, "ReturnStmnt");
    PushNode();
    {
        Visit(ast->attribs);
        VisitAndSignalLast(ast->expr);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(CtrlTransferStmnt)
{
    Print(ast, "CtrlTransferStmnt", CtrlTransformToString(ast->transfer));
    PushNode();
    {
        VisitAndSignalLast(ast->attribs);
    }
    PopNode();
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(NullExpr)
{
    Print(ast, "NullExpr");
}

IMPLEMENT_VISIT_PROC(ListExpr)
{
    Print(ast, "ListExpr");
    PushNode();
    {
        Visit(ast->firstExpr);
        VisitAndSignalLast(ast->nextExpr);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(LiteralExpr)
{
    Print(ast, "LiteralExpr", ast->value);
}

IMPLEMENT_VISIT_PROC(TypeNameExpr)
{
    Print(ast, "TypeNameExpr", ast->GetTypeDenoter()->ToString());
    PushNode();
    {
        VisitAndSignalLast(ast->typeName);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(TernaryExpr)
{
    Print(ast, "TernaryExpr");
    PushNode();
    {
        Visit(ast->condExpr);
        Visit(ast->thenExpr);
        VisitAndSignalLast(ast->elseExpr);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    Print(ast, "BinaryExpr", BinaryOpToString(ast->op));
    PushNode();
    {
        Visit(ast->lhsExpr);
        VisitAndSignalLast(ast->rhsExpr);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    Print(ast, "UnaryExpr", UnaryOpToString(ast->op));
    PushNode();
    {
        VisitAndSignalLast(ast->expr);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(PostUnaryExpr)
{
    Print(ast, "PostUnaryExpr", UnaryOpToString(ast->op));
    PushNode();
    {
        VisitAndSignalLast(ast->expr);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(FunctionCallExpr)
{
    Print(ast, "FunctionCallExpr");
    PushNode();
    {
        VisitAndSignalLast(ast->call);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(BracketExpr)
{
    Print(ast, "BracketExpr");
    PushNode();
    {
        VisitAndSignalLast(ast->expr);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(SuffixExpr)
{
    Print(ast, "SuffixExpr");
    PushNode();
    {
        Visit(ast->expr);
        VisitAndSignalLast(ast->varIdent);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(ArrayAccessExpr)
{
    Print(ast, "ArrayAccessExpr");
    PushNode();
    {
        Visit(ast->expr);
        VisitAndSignalLast(ast->arrayIndices);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    Print(ast, "CastExpr");
    PushNode();
    {
        Visit(ast->typeExpr);
        VisitAndSignalLast(ast->expr);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
    Print(ast, "VarAccessExpr");
    PushNode();
    {
        if (ast->assignExpr)
        {
            Visit(ast->varIdent);
            VisitAndSignalLast(ast->assignExpr);
        }
        else
            VisitAndSignalLast(ast->varIdent);
    }
    PopNode();
}

IMPLEMENT_VISIT_PROC(InitializerExpr)
{
    Print(ast, "InitializerExpr");
    PushNode();
    {
        VisitAndSignalLast(ast->exprs);
    }
    PopNode();
}

#undef IMPLEMENT_VISIT_PROC

/* --- Helper functions --- */

void ASTPrinter::Print(AST* ast, const std::string& astName, const std::string& info)
{
    if (ast->area.Pos().IsValid())
    {
        std::string s;
        
        WriteNodeHierarchyLevel(s);
        s += astName + " (" + ast->area.Pos().ToString(false) + ")";

        if (!info.empty())
            s += " \"" + info + "\"";

        log_->SumitReport(Report(Report::Types::Info, s));
    }
}

void ASTPrinter::PushNode(bool signalLastSubNode)
{
    lastSubNodeStack_.push_back(signalLastSubNode);
}

void ASTPrinter::PopNode()
{
    lastSubNodeStack_.pop_back();
}

void ASTPrinter::SignalLastSubNode()
{
    if (!lastSubNodeStack_.empty())
        lastSubNodeStack_.back() = true;
}

void ASTPrinter::WriteNodeHierarchyLevel(std::string& s)
{
    if (!lastSubNodeStack_.empty())
    {
        for (std::size_t i = 0; i + 1 < lastSubNodeStack_.size(); ++i)
        {
            if (lastSubNodeStack_[i])
                s += "  ";
            else
                s += "| ";
        }

        if (lastSubNodeStack_.back())
            s += "`-";
        else
            s += "|-";
    }
}

template <typename T>
void ASTPrinter::VisitAndSignalLast(T ast)
{
    /* Signal and visit last AST node */
    SignalLastSubNode();
    Visit(ast);
}

template <typename T>
void ASTPrinter::VisitAndSignalLast(const std::vector<T>& astList)
{
    if (!astList.empty())
    {
        /* Visit all AST nodes expect the last one */
        for (std::size_t i = 0, n = astList.size(); i + 1 < n; ++i)
            Visit(astList[i]);

        /* Signal and visit last AST node */
        VisitAndSignalLast(astList.back());
    }
}


} // /namespace Xsc



// ================================================================================