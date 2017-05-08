/*
 * ASTPrinter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ASTPrinter.h"
#include "AST.h"
#include <algorithm>


namespace Xsc
{


void ASTPrinter::PrintAST(Program* program, Log& log)
{
    /* Build new printable tree */
    Visit(program);

    /* Print all children of the tree root */
    for (const auto& child : treeRoot_.children)
        Print(log, child);

    /* Clean up (if instance of ASTPrinter is used multiple times) */
    treeRoot_.label.clear();
    treeRoot_.children.clear();
}


/*
 * ======= Private: =======
 */

/* ------- Visit functions ------- */

#define PRINT_AST(AST_NAME)                                         \
    if (!ast->flags(AST::isBuildIn))                                \
    {                                                               \
        PushPrintable(ast->area.Pos(), WriteLabel(ast, #AST_NAME)); \
        VISIT_DEFAULT(AST_NAME);                                    \
        PopPrintable();                                             \
    }

#define PRINT_AST_EXT(AST_NAME, INFO)                                       \
    if (!ast->flags(AST::isBuildIn))                                        \
    {                                                                       \
        PushPrintable(ast->area.Pos(), WriteLabel(ast, #AST_NAME, INFO));   \
        VISIT_DEFAULT(AST_NAME);                                            \
        PopPrintable();                                                     \
    }

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ASTPrinter::Visit##AST_NAME(AST_NAME* ast, void* args)

#define IMPLEMENT_VISIT_PROC_DEFAULT(AST_NAME)  \
    IMPLEMENT_VISIT_PROC(AST_NAME)              \
    {                                           \
        PRINT_AST(AST_NAME);                    \
    }

IMPLEMENT_VISIT_PROC_DEFAULT(Program)

IMPLEMENT_VISIT_PROC_DEFAULT(CodeBlock)

IMPLEMENT_VISIT_PROC_DEFAULT(Attribute)

IMPLEMENT_VISIT_PROC_DEFAULT(SwitchCase)

IMPLEMENT_VISIT_PROC(SamplerValue)
{
    PRINT_AST_EXT(SamplerValue, ast->name);
}

IMPLEMENT_VISIT_PROC(Register)
{
    PRINT_AST_EXT(Register, ast->ToString());
}

IMPLEMENT_VISIT_PROC(PackOffset)
{ 
    PRINT_AST_EXT(PackOffset, ast->ToString());
}

IMPLEMENT_VISIT_PROC(ArrayDimension)
{
    PRINT_AST_EXT(ArrayDimension, ast->ToString());
}

IMPLEMENT_VISIT_PROC(TypeSpecifier)
{
    PRINT_AST_EXT(TypeSpecifier, ast->ToString());
}

/* --- Declaration --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    PRINT_AST_EXT(VarDecl, ast->ident.Original());
}

IMPLEMENT_VISIT_PROC(BufferDecl)
{
    PRINT_AST_EXT(BufferDecl, ast->ident.Original());
}

IMPLEMENT_VISIT_PROC(SamplerDecl)
{
    PRINT_AST_EXT(SamplerDecl, ast->ident.Original());
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    auto info = ast->ident.Original();
    if (!ast->baseStructName.empty())
        info += " : " + ast->baseStructName;

    PRINT_AST_EXT(StructDecl, info);
}

IMPLEMENT_VISIT_PROC(AliasDecl)
{
    PRINT_AST_EXT(AliasDecl, ast->ident.Original());
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    PRINT_AST_EXT(FunctionDecl, ast->ToString());
}

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    PRINT_AST_EXT(UniformBufferDecl, ast->ToString());
}

IMPLEMENT_VISIT_PROC_DEFAULT(BufferDeclStmnt)

IMPLEMENT_VISIT_PROC_DEFAULT(SamplerDeclStmnt)

IMPLEMENT_VISIT_PROC_DEFAULT(StructDeclStmnt)

IMPLEMENT_VISIT_PROC_DEFAULT(VarDeclStmnt)

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
    PRINT_AST_EXT(CtrlTransferStmnt, CtrlTransformToString(ast->transfer));
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC_DEFAULT(NullExpr)

IMPLEMENT_VISIT_PROC_DEFAULT(SequenceExpr)

IMPLEMENT_VISIT_PROC(LiteralExpr)
{
    PRINT_AST_EXT(LiteralExpr, (ast->dataType == DataType::String ? ast->GetStringValue() : ast->value));
}

IMPLEMENT_VISIT_PROC(TypeSpecifierExpr)
{
    PRINT_AST_EXT(TypeSpecifierExpr, ast->GetTypeDenoter()->ToString());
}

IMPLEMENT_VISIT_PROC_DEFAULT(TernaryExpr)

IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    PRINT_AST_EXT(BinaryExpr, BinaryOpToString(ast->op));
}

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    PRINT_AST_EXT(UnaryExpr, UnaryOpToString(ast->op));
}

IMPLEMENT_VISIT_PROC(PostUnaryExpr)
{
    PRINT_AST_EXT(PostUnaryExpr, UnaryOpToString(ast->op));
}

static std::string IdentWithPrefixOpt(bool hasPrefix, bool isStatic, const std::string& ident)
{
    return ((hasPrefix ? (isStatic ? "::" : ".") : "") + ident);
}

IMPLEMENT_VISIT_PROC(CallExpr)
{
    PRINT_AST_EXT(CallExpr, IdentWithPrefixOpt(ast->prefixExpr != nullptr, ast->isStatic, ast->ident));
}

IMPLEMENT_VISIT_PROC_DEFAULT(BracketExpr)

IMPLEMENT_VISIT_PROC(ObjectExpr)
{
    PRINT_AST_EXT(ObjectExpr, IdentWithPrefixOpt(ast->prefixExpr != nullptr, ast->isStatic, ast->ident));
}

IMPLEMENT_VISIT_PROC_DEFAULT(AssignExpr)

IMPLEMENT_VISIT_PROC_DEFAULT(ArrayExpr)

IMPLEMENT_VISIT_PROC_DEFAULT(CastExpr)

IMPLEMENT_VISIT_PROC_DEFAULT(InitializerExpr)

#undef PRINT_AST
#undef PRINT_AST_EXT
#undef IMPLEMENT_VISIT_PROC
#undef IMPLEMENT_VISIT_PROC_DEFAULT

/* --- Helper functions --- */

std::string ASTPrinter::WriteLabel(AST* ast, const std::string& astName, const std::string& info)
{
    std::string s;
        
    /* Append AST name */
    s = astName;

    /* Append brief information */
    if (!info.empty())
        s += " \"" + info + "\"";

    return s;
}

void ASTPrinter::Print(Log& log, const PrintableTree& tree)
{
    std::string s;

    /* Write row of source position */
    s += std::string(maxRowStrLen_ - tree.row.size(), ' ');
    s += tree.row;
    s += ':';

    /* Write column of source position */
    s += std::string(maxColStrLen_ - tree.col.size(), ' ');
    s += tree.col;
    s += "  ";

    /* Write node hierarchy level */
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

    /* Write label */
    s += tree.label;

    /* Submit print as report */
    log.SumitReport(Report(ReportTypes::Info, s));

    /* Print children */
    if (!tree.children.empty())
    {
        lastSubNodeStack_.push_back(false);
        {
            /* Print all children except the last one */
            for (std::size_t i = 0, n = tree.children.size(); i + 1 < n; ++i)
                Print(log, tree.children[i]);

            /* Print last children */
            lastSubNodeStack_.back() = true;
            Print(log, tree.children.back());
        }
        lastSubNodeStack_.pop_back();
    }
}

bool ASTPrinter::PushPrintable(const SourcePosition& pos, const std::string& label)
{
    if (!label.empty())
    {
        /* Store longest source position string */
        const auto rowStr = std::to_string(pos.GetOrigin() ? pos.Row() + pos.GetOrigin()->lineOffset : pos.Row());
        const auto colStr = std::to_string(pos.Column());

        maxRowStrLen_ = std::max(maxRowStrLen_, rowStr.size());
        maxColStrLen_ = std::max(maxColStrLen_, colStr.size());

        /* Add new child node to printable tree */
        auto& children = TopPrintable()->children;
        children.push_back({ rowStr, colStr, label, {} });
        parentNodeStack_.push(&(children.back()));

        return true;
    }
    return false;
}

void ASTPrinter::PopPrintable()
{
    parentNodeStack_.pop();
}

ASTPrinter::PrintableTree* ASTPrinter::TopPrintable()
{
    if (parentNodeStack_.empty())
        return (&treeRoot_);
    else
        return parentNodeStack_.top();
}


} // /namespace Xsc



// ================================================================================