/*
 * ASTPrinter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ASTPrinter.h"
#include "AST.h"


namespace Xsc
{


void ASTPrinter::DumpAST(Program* program, Log& log)
{
    log_ = &log;
    Visit(program);
}


/*
 * ======= Private: =======
 */

class ScopedIndent
{
    public:
        
        inline ScopedIndent(Log& log) :
            log_{ log }
        {
            log_.IncIndent();
        }

        inline ~ScopedIndent()
        {
            log_.DecIndent();
        }

    private:

        Log& log_;

};

#define SCOPED_INDENT ScopedIndent indent(*log_)

#define DEFAULT_VISITOR(AST_NAME)               \
    {                                           \
        SCOPED_INDENT;                          \
        Visitor::Visit##AST_NAME(ast, args);    \
    }

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ASTPrinter::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(Program)
{
    Print(ast, "Program");
    DEFAULT_VISITOR(Program);
}

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    Print(ast, "CodeBlock");
    DEFAULT_VISITOR(CodeBlock);
}

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    Print(ast, "FunctionCall");
    DEFAULT_VISITOR(FunctionCall);
}

IMPLEMENT_VISIT_PROC(SwitchCase)
{
    Print(ast, "SwitchCase");
    DEFAULT_VISITOR(SwitchCase);
}

IMPLEMENT_VISIT_PROC(SamplerValue)
{
    Print(ast, "SamplerValue", ast->name/* + " = " + ast->value->ToString()*/);
    DEFAULT_VISITOR(SamplerValue);
}

IMPLEMENT_VISIT_PROC(PackOffset)
{
    auto info = ast->registerName;

    if (!ast->vectorComponent.empty())
        info += " (" + ast->vectorComponent + ")";

    Print(ast, "PackOffset", info);

    DEFAULT_VISITOR(PackOffset);
}

IMPLEMENT_VISIT_PROC(VarSemantic)
{
    auto info = ast->semantic;

    if (!ast->registerName.empty())
        info += " (" + ast->registerName + ")";

    Print(ast, "VarSemantic", info);

    DEFAULT_VISITOR(VarSemantic);
}

IMPLEMENT_VISIT_PROC(VarType)
{
    Print(ast, "VarType", (ast->typeDenoter ? ast->typeDenoter->ToString() : ""));
    DEFAULT_VISITOR(VarType);
}

IMPLEMENT_VISIT_PROC(VarIdent)
{
    Print(ast, "VarIdent", ast->ident);
    DEFAULT_VISITOR(VarIdent);
}

/* --- Declaration --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    Print(ast, "VarDecl", ast->name);
    DEFAULT_VISITOR(VarDecl);
}

IMPLEMENT_VISIT_PROC(TextureDecl)
{
    Print(ast, "TextureDecl", ast->ident);
    DEFAULT_VISITOR(TextureDecl);
}

IMPLEMENT_VISIT_PROC(SamplerDecl)
{
    Print(ast, "SamplerDecl", ast->ident);
    DEFAULT_VISITOR(SamplerDecl);
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    Print(ast, "StructDecl");
    DEFAULT_VISITOR(StructDecl);
}

IMPLEMENT_VISIT_PROC(AliasDecl)
{
    Print(ast, "AliasDecl", ast->ident);
    DEFAULT_VISITOR(AliasDecl);
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    Print(ast, "FunctionDecl", ast->name);
    DEFAULT_VISITOR(FunctionDecl);
}

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    Print(ast, "VarDeclStmnt");
    DEFAULT_VISITOR(VarDeclStmnt);
}

IMPLEMENT_VISIT_PROC(BufferDeclStmnt)
{
    Print(ast, "BufferDeclStmnt", ast->name + " (" + ast->bufferType + ")");
    DEFAULT_VISITOR(BufferDeclStmnt);
}

IMPLEMENT_VISIT_PROC(TextureDeclStmnt)
{
    Print(ast, "TextureDeclStmnt");
    DEFAULT_VISITOR(TextureDeclStmnt);
}

IMPLEMENT_VISIT_PROC(SamplerDeclStmnt)
{
    Print(ast, "SamplerDeclStmnt");
    DEFAULT_VISITOR(SamplerDeclStmnt);
}

IMPLEMENT_VISIT_PROC(StructDeclStmnt)
{
    Print(ast, "StructDeclStmnt");
    DEFAULT_VISITOR(StructDeclStmnt);
}

IMPLEMENT_VISIT_PROC(AliasDeclStmnt)
{
    Print(ast, "AliasDeclStmnt");
    DEFAULT_VISITOR(AliasDeclStmnt);
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(NullStmnt)
{
    Print(ast, "NullStmnt");
    DEFAULT_VISITOR(NullStmnt);
}

IMPLEMENT_VISIT_PROC(CodeBlockStmnt)
{
    Print(ast, "CodeBlockStmnt");
    DEFAULT_VISITOR(CodeBlockStmnt);
}

IMPLEMENT_VISIT_PROC(ForLoopStmnt)
{
    Print(ast, "ForLoopStmnt");
    DEFAULT_VISITOR(ForLoopStmnt);
}

IMPLEMENT_VISIT_PROC(WhileLoopStmnt)
{
    Print(ast, "WhileLoopStmnt");
    DEFAULT_VISITOR(WhileLoopStmnt);
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmnt)
{
    Print(ast, "DoWhileLoopStmnt");
    DEFAULT_VISITOR(DoWhileLoopStmnt);
}

IMPLEMENT_VISIT_PROC(IfStmnt)
{
    Print(ast, "IfStmnt");
    DEFAULT_VISITOR(IfStmnt);
}

IMPLEMENT_VISIT_PROC(ElseStmnt)
{
    Print(ast, "ElseStmnt");
    DEFAULT_VISITOR(ElseStmnt);
}

IMPLEMENT_VISIT_PROC(SwitchStmnt)
{
    Print(ast, "SwitchStmnt");
    DEFAULT_VISITOR(SwitchStmnt);
}

IMPLEMENT_VISIT_PROC(AssignStmnt)
{
    Print(ast, "AssignStmnt", AssignOpToString(ast->op));
    DEFAULT_VISITOR(AssignStmnt);
}

IMPLEMENT_VISIT_PROC(ExprStmnt)
{
    Print(ast, "ExprStmnt");
    DEFAULT_VISITOR(ExprStmnt);
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    Print(ast, "ReturnStmnt");
    DEFAULT_VISITOR(ReturnStmnt);
}

IMPLEMENT_VISIT_PROC(CtrlTransferStmnt)
{
    Print(ast, "CtrlTransferStmnt", CtrlTransformToString(ast->transfer));
    DEFAULT_VISITOR(CtrlTransferStmnt);
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(ListExpr)
{
    Print(ast, "ListExpr");
    DEFAULT_VISITOR(ListExpr);
}

IMPLEMENT_VISIT_PROC(LiteralExpr)
{
    Print(ast, "LiteralExpr", ast->value);
    DEFAULT_VISITOR(LiteralExpr);
}

IMPLEMENT_VISIT_PROC(TypeNameExpr)
{
    Print(ast, "TypeNameExpr");
    DEFAULT_VISITOR(TypeNameExpr);
}

IMPLEMENT_VISIT_PROC(TernaryExpr)
{
    Print(ast, "TernaryExpr");
    DEFAULT_VISITOR(TernaryExpr);
}

IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    Print(ast, "BinaryExpr", BinaryOpToString(ast->op));
    DEFAULT_VISITOR(BinaryExpr);
}

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    Print(ast, "UnaryExpr", UnaryOpToString(ast->op));
    DEFAULT_VISITOR(UnaryExpr);
}

IMPLEMENT_VISIT_PROC(PostUnaryExpr)
{
    Print(ast, "PostUnaryExpr", UnaryOpToString(ast->op));
    DEFAULT_VISITOR(PostUnaryExpr);
}

IMPLEMENT_VISIT_PROC(FunctionCallExpr)
{
    Print(ast, "FunctionCallExpr");
    DEFAULT_VISITOR(FunctionCallExpr);
}

IMPLEMENT_VISIT_PROC(BracketExpr)
{
    Print(ast, "BracketExpr");
    DEFAULT_VISITOR(BracketExpr);
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    Print(ast, "CastExpr");
    DEFAULT_VISITOR(CastExpr);
}

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
    Print(ast, "VarAccessExpr");
    DEFAULT_VISITOR(VarAccessExpr);
}

IMPLEMENT_VISIT_PROC(InitializerExpr)
{
    Print(ast, "InitializerExpr");
    DEFAULT_VISITOR(InitializerExpr);
}

#undef IMPLEMENT_VISIT_PROC
#undef DEFAULT_VISITOR
#undef SCOPED_INDENT

/* --- Helper functions --- */

void ASTPrinter::Print(AST* ast, const std::string& astName, const std::string& info)
{
    std::string msg = astName + " (" + ast->area.pos.ToString() + ")";

    if (!info.empty())
        msg += " \"" + info + "\"";

    log_->SumitReport(Report(Report::Types::Info, msg));
}

void ASTPrinter::IncIndent()
{
    log_->IncIndent();
}

void ASTPrinter::DecIndent()
{
    log_->DecIndent();
}


} // /namespace Xsc



// ================================================================================