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


void ASTPrinter::PrintAST(Program* program, Log& log)
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

#define IMPLEMENT_VISIT_PROC_DEFAULT(AST_NAME)  \
    IMPLEMENT_VISIT_PROC(AST_NAME)              \
    {                                           \
        Print(ast, #AST_NAME);                  \
        DEFAULT_VISITOR(AST_NAME);              \
    }

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ASTPrinter::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC_DEFAULT(Program)

IMPLEMENT_VISIT_PROC_DEFAULT(CodeBlock)

IMPLEMENT_VISIT_PROC_DEFAULT(FunctionCall)

IMPLEMENT_VISIT_PROC(Attribute)
{
    Print(ast, "Attribute", ast->ident);
    DEFAULT_VISITOR(Attribute);
}

IMPLEMENT_VISIT_PROC_DEFAULT(SwitchCase)

IMPLEMENT_VISIT_PROC(SamplerValue)
{
    Print(ast, "SamplerValue", ast->name/* + " = " + ast->value->ToString()*/);
    DEFAULT_VISITOR(SamplerValue);
}

IMPLEMENT_VISIT_PROC(Register)
{
    Print(ast, "Register", ast->ToString());
    DEFAULT_VISITOR(Register);
}

IMPLEMENT_VISIT_PROC(PackOffset)
{ 
    Print(ast, "PackOffset", ast->ToString());
    DEFAULT_VISITOR(PackOffset);
}

IMPLEMENT_VISIT_PROC(VarSemantic)
{
    Print(ast, "VarSemantic", ast->ToString());
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
    Print(ast, "VarDecl", ast->ident);
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
    auto info = ast->ident;
    if (!ast->baseStructName.empty())
        info += " : " + ast->baseStructName;

    Print(ast, "StructDecl", info);
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
    Print(ast, "FunctionDecl", ast->ident);
    DEFAULT_VISITOR(FunctionDecl);
}

IMPLEMENT_VISIT_PROC_DEFAULT(VarDeclStmnt)

IMPLEMENT_VISIT_PROC(BufferDeclStmnt)
{
    Print(ast, "BufferDeclStmnt", ast->ToString());
    DEFAULT_VISITOR(BufferDeclStmnt);
}

IMPLEMENT_VISIT_PROC_DEFAULT(TextureDeclStmnt)

IMPLEMENT_VISIT_PROC_DEFAULT(SamplerDeclStmnt)

IMPLEMENT_VISIT_PROC_DEFAULT(StructDeclStmnt)

IMPLEMENT_VISIT_PROC_DEFAULT(AliasDeclStmnt)

/* --- Statements --- */

IMPLEMENT_VISIT_PROC_DEFAULT(NullStmnt)

IMPLEMENT_VISIT_PROC_DEFAULT(CodeBlockStmnt)

IMPLEMENT_VISIT_PROC_DEFAULT(ForLoopStmnt)

IMPLEMENT_VISIT_PROC_DEFAULT(WhileLoopStmnt)

IMPLEMENT_VISIT_PROC_DEFAULT(DoWhileLoopStmnt)

IMPLEMENT_VISIT_PROC_DEFAULT(IfStmnt)

IMPLEMENT_VISIT_PROC_DEFAULT(ElseStmnt)

IMPLEMENT_VISIT_PROC_DEFAULT(SwitchStmnt)

IMPLEMENT_VISIT_PROC_DEFAULT(ExprStmnt)

IMPLEMENT_VISIT_PROC_DEFAULT(ReturnStmnt)

IMPLEMENT_VISIT_PROC(CtrlTransferStmnt)
{
    Print(ast, "CtrlTransferStmnt", CtrlTransformToString(ast->transfer));
    DEFAULT_VISITOR(CtrlTransferStmnt);
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC_DEFAULT(NullExpr)

IMPLEMENT_VISIT_PROC_DEFAULT(ListExpr)

IMPLEMENT_VISIT_PROC(LiteralExpr)
{
    Print(ast, "LiteralExpr", ast->value);
    DEFAULT_VISITOR(LiteralExpr);
}

IMPLEMENT_VISIT_PROC(TypeNameExpr)
{
    Print(ast, "TypeNameExpr", ast->typeDenoter->ToString());
    DEFAULT_VISITOR(TypeNameExpr);
}

IMPLEMENT_VISIT_PROC_DEFAULT(TernaryExpr)

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

IMPLEMENT_VISIT_PROC_DEFAULT(FunctionCallExpr)

IMPLEMENT_VISIT_PROC_DEFAULT(BracketExpr)

IMPLEMENT_VISIT_PROC_DEFAULT(SuffixExpr)

IMPLEMENT_VISIT_PROC_DEFAULT(ArrayAccessExpr)

IMPLEMENT_VISIT_PROC_DEFAULT(CastExpr)

IMPLEMENT_VISIT_PROC_DEFAULT(VarAccessExpr)

IMPLEMENT_VISIT_PROC_DEFAULT(InitializerExpr)

#undef IMPLEMENT_VISIT_PROC_DEFAULT
#undef IMPLEMENT_VISIT_PROC
#undef DEFAULT_VISITOR
#undef SCOPED_INDENT

/* --- Helper functions --- */

void ASTPrinter::Print(AST* ast, const std::string& astName, const std::string& info)
{
    if (ast->area.Pos().IsValid())
    {
        std::string msg = astName + " (" + ast->area.Pos().ToString() + ")";

        if (!info.empty())
            msg += " \"" + info + "\"";

        log_->SumitReport(Report(Report::Types::Info, msg));
    }
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