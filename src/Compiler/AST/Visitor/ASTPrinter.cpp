/*
 * ASTPrinter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ASTPrinter.h"
#include "AST.h"
#include "ReportIdents.h"
#include <stdexcept>
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

template <typename T>
inline std::string MemberToString(const T& member)
{
    return "";
}

template <>
inline std::string MemberToString<Identifier>(const Identifier& member)
{
    return member.Original();
}

template <>
inline std::string MemberToString<std::string>(const std::string& member)
{
    return member;
}

template <>
inline std::string MemberToString<bool>(const bool& member)
{
    if (member)
        return "true";
    else
        return "false";
}

template <>
inline std::string MemberToString<IndexedSemantic>(const IndexedSemantic& member)
{
    return member.ToString();
}

#define PRINT_AST(AST_NAME)                         \
    if (!ast->flags(AST::isBuiltin))                \
    {                                               \
        PushPrintable(ast, WriteLabel(#AST_NAME));  \
        VISIT_DEFAULT(AST_NAME);                    \
        PopPrintable();                             \
    }

#define PRINT_AST_EXT(AST_NAME, INFO)                       \
    if (!ast->flags(AST::isBuiltin))                        \
    {                                                       \
        PushPrintable(ast, WriteLabel(#AST_NAME, INFO));    \
        VISIT_DEFAULT(AST_NAME);                            \
        PopPrintable();                                     \
    }

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ASTPrinter::Visit##AST_NAME(AST_NAME* ast, void* args)

#define IMPLEMENT_VISIT_PROC_DEFAULT(AST_NAME)  \
    IMPLEMENT_VISIT_PROC(AST_NAME)              \
    {                                           \
        PRINT_AST(AST_NAME);                    \
    }

#define VISIT_MEMBER(MEMBER) \
    VisitMember(ast->MEMBER, #MEMBER)

#define ADD_PRINTABLE_MEMBER(MEMBER) \
    Printable(ast, std::string(#MEMBER) + " : " + MemberToString(ast->MEMBER))

/* ------- Visit functions ------- */

IMPLEMENT_VISIT_PROC(Program)
{
    PushPrintable(ast, WriteLabel("Program"));
    {
        VISIT_MEMBER(globalStmnts);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    PushPrintable(ast, WriteLabel("CodeBlock"));
    {
        VISIT_MEMBER(stmnts);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(Attribute)
{
    PushPrintable(ast, WriteLabel("Attribute"));
    {
        VISIT_MEMBER(arguments);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(SwitchCase)
{
    PushPrintable(ast, WriteLabel("SwitchCase"));
    {
        VISIT_MEMBER(expr);
        VISIT_MEMBER(stmnts);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(SamplerValue)
{
    PushPrintable(ast, WriteLabel("SamplerValue"));
    {
        VISIT_MEMBER(value);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(Register)
{
    PushPrintable(ast, WriteLabel("Register"));
    {
        // do nothing
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(PackOffset)
{
    PushPrintable(ast, WriteLabel("PackOffset"));
    {
        // do nothing
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(ArrayDimension)
{
    PushPrintable(ast, WriteLabel("ArrayDimension"));
    {
        VISIT_MEMBER(expr);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(TypeSpecifier)
{
    PushPrintable(ast, WriteLabel("TypeSpecifier"));
    {
        VISIT_MEMBER(structDecl);
        Printable(ast, "typeDenoter : " + ast->ToString());
    }
    PopPrintable();
}

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    PushPrintable(ast, WriteLabel("VarDecl"));
    {
        ADD_PRINTABLE_MEMBER(ident);
        VISIT_MEMBER(namespaceExpr);
        VISIT_MEMBER(arrayDims);
        VISIT_MEMBER(packOffset);
        VISIT_MEMBER(annotations);
        VISIT_MEMBER(initializer);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(BufferDecl)
{
    PushPrintable(ast, WriteLabel("BufferDecl"));
    {
        ADD_PRINTABLE_MEMBER(ident);
        VISIT_MEMBER(arrayDims);
        VISIT_MEMBER(slotRegisters);
        VISIT_MEMBER(annotations);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(SamplerDecl)
{
    PushPrintable(ast, WriteLabel("SamplerDecl"));
    {
        ADD_PRINTABLE_MEMBER(ident);
        VISIT_MEMBER(arrayDims);
        VISIT_MEMBER(slotRegisters);
        VISIT_MEMBER(samplerValues);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    PushPrintable(ast, WriteLabel("StructDecl"));
    {
        ADD_PRINTABLE_MEMBER(ident);
        VISIT_MEMBER(localStmnts);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(AliasDecl)
{
    if (!ast->flags(AST::isBuiltin))
    {
        ADD_PRINTABLE_MEMBER(ident);
        Printable(ast, WriteLabel("AliasDecl"));
    }
}

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    PushPrintable(ast, WriteLabel("FunctionDecl"));
    {
        ADD_PRINTABLE_MEMBER(ident);
        VISIT_MEMBER(returnType);
        VISIT_MEMBER(parameters);
        if (ast->semantic.IsValid())
            ADD_PRINTABLE_MEMBER(semantic);
        VISIT_MEMBER(annotations);
        VISIT_MEMBER(codeBlock);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    PushPrintable(ast, WriteLabel("UniformBufferDecl"));
    {
        ADD_PRINTABLE_MEMBER(ident);
        Printable(ast, "bufferType : " + std::string(ast->bufferType == UniformBufferType::ConstantBuffer ? "cbuffer" : "tbuffer"));
        VISIT_MEMBER(slotRegisters);
        VISIT_MEMBER(localStmnts);
    }
    PopPrintable();
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(BufferDeclStmnt)
{
    PushPrintable(ast, WriteLabel("BufferDeclStmnt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(bufferDecls);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(SamplerDeclStmnt)
{
    PushPrintable(ast, WriteLabel("SamplerDeclStmnt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(samplerDecls);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    PushPrintable(ast, WriteLabel("VarDeclStmnt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(typeSpecifier);
        VISIT_MEMBER(varDecls);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(AliasDeclStmnt)
{
    if (!ast->flags(AST::isBuiltin))
    {
        PushPrintable(ast, WriteLabel("AliasDeclStmnt"));
        {
            VISIT_MEMBER(attribs);
            VISIT_MEMBER(structDecl);
            VISIT_MEMBER(aliasDecls);
        }
        PopPrintable();
    }
}

IMPLEMENT_VISIT_PROC(BasicDeclStmnt)
{
    PushPrintable(ast, WriteLabel("BasicDeclStmnt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(declObject);
    }
    PopPrintable();
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(NullStmnt)
{
    PushPrintable(ast, WriteLabel("NullStmnt"));
    {
        VISIT_MEMBER(attribs);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(CodeBlockStmnt)
{
    PushPrintable(ast, WriteLabel("CodeBlockStmnt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(codeBlock);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(ForLoopStmnt)
{
    PushPrintable(ast, WriteLabel("ForLoopStmnt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(initStmnt);
        VISIT_MEMBER(condition);
        VISIT_MEMBER(iteration);
        VISIT_MEMBER(bodyStmnt);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(WhileLoopStmnt)
{
    PushPrintable(ast, WriteLabel("WhileLoopStmnt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(condition);
        VISIT_MEMBER(bodyStmnt);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmnt)
{
    PushPrintable(ast, WriteLabel("DoWhileLoopStmnt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(bodyStmnt);
        VISIT_MEMBER(condition);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(IfStmnt)
{
    PushPrintable(ast, WriteLabel("IfStmnt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(condition);
        VISIT_MEMBER(bodyStmnt);
        VISIT_MEMBER(elseStmnt);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(ElseStmnt)
{
    PushPrintable(ast, WriteLabel("ElseStmnt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(bodyStmnt);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(SwitchStmnt)
{
    PushPrintable(ast, WriteLabel("SwitchStmnt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(selector);
        VISIT_MEMBER(cases);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(ExprStmnt)
{
    PushPrintable(ast, WriteLabel("ExprStmnt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(expr);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    PushPrintable(ast, WriteLabel("ReturnStmnt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(expr);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(CtrlTransferStmnt)
{
    PushPrintable(ast, WriteLabel("CtrlTransferStmnt"));
    {
        VISIT_MEMBER(attribs);
        Printable(ast, "transfer : " + CtrlTransformToString(ast->transfer));
    }
    PopPrintable();
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(NullExpr)
{
    Printable(ast, WriteLabel("NullExpr"));
}

IMPLEMENT_VISIT_PROC(SequenceExpr)
{
    PushPrintable(ast, WriteLabel("SequenceExpr"));
    {
        VISIT_MEMBER(exprs);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(LiteralExpr)
{
    PushPrintable(ast, WriteLabel("LiteralExpr"));
    {
        PrintableType(ast);
        ADD_PRINTABLE_MEMBER(value);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(TypeSpecifierExpr)
{
    PushPrintable(ast, WriteLabel("TypeSpecifierExpr"));
    {
        PrintableType(ast);
        VISIT_MEMBER(typeSpecifier);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(TernaryExpr)
{
    PushPrintable(ast, WriteLabel("TernaryExpr"));
    {
        PrintableType(ast);
        VISIT_MEMBER(condExpr);
        VISIT_MEMBER(thenExpr);
        VISIT_MEMBER(elseExpr);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    PushPrintable(ast, WriteLabel("BinaryExpr"));
    {
        PrintableType(ast);
        VISIT_MEMBER(lhsExpr);
        Printable(ast, "op : " + BinaryOpToString(ast->op));
        VISIT_MEMBER(rhsExpr);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    PushPrintable(ast, WriteLabel("UnaryExpr"));
    {
        PrintableType(ast);
        Printable(ast, "op : " + UnaryOpToString(ast->op));
        VISIT_MEMBER(expr);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(PostUnaryExpr)
{
    PushPrintable(ast, WriteLabel("PostUnaryExpr"));
    {
        PrintableType(ast);
        VISIT_MEMBER(expr);
        Printable(ast, "op : " + UnaryOpToString(ast->op));
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(CallExpr)
{
    PushPrintable(ast, WriteLabel("CallExpr"));
    {
        PrintableType(ast);
        VISIT_MEMBER(prefixExpr);
        ADD_PRINTABLE_MEMBER(isStatic);
        ADD_PRINTABLE_MEMBER(ident);
        VISIT_MEMBER(arguments);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(BracketExpr)
{
    PushPrintable(ast, WriteLabel("BracketExpr"));
    {
        PrintableType(ast);
        VISIT_MEMBER(expr);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(AssignExpr)
{
    PushPrintable(ast, WriteLabel("AssignExpr"));
    {
        PrintableType(ast);
        VISIT_MEMBER(lvalueExpr);
        Printable(ast, "op : " + AssignOpToString(ast->op));
        VISIT_MEMBER(rvalueExpr);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(ObjectExpr)
{
    PushPrintable(ast, WriteLabel("ObjectExpr"));
    {
        PrintableType(ast);
        VISIT_MEMBER(prefixExpr);
        ADD_PRINTABLE_MEMBER(isStatic);
        ADD_PRINTABLE_MEMBER(ident);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(ArrayExpr)
{
    PushPrintable(ast, WriteLabel("ArrayExpr"));
    {
        PrintableType(ast);
        VISIT_MEMBER(prefixExpr);
        VISIT_MEMBER(arrayIndices);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    PushPrintable(ast, WriteLabel("CastExpr"));
    {
        PrintableType(ast);
        VISIT_MEMBER(typeSpecifier);
        VISIT_MEMBER(expr);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(InitializerExpr)
{
    PushPrintable(ast, WriteLabel("InitializerExpr"));
    {
        PrintableType(ast);
        VISIT_MEMBER(exprs);
    }
    PopPrintable();
}

#if 0

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

IMPLEMENT_VISIT_PROC_DEFAULT(VarDeclStmnt)

IMPLEMENT_VISIT_PROC_DEFAULT(AliasDeclStmnt)

IMPLEMENT_VISIT_PROC_DEFAULT(BasicDeclStmnt)

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

#endif

#undef PRINT_AST
#undef PRINT_AST_EXT
#undef IMPLEMENT_VISIT_PROC
#undef IMPLEMENT_VISIT_PROC_DEFAULT
#undef VISIT_MEMBER
#undef ADD_PRINTABLE_MEMBER

/* --- Helper functions --- */

std::string ASTPrinter::WriteLabel(const std::string& astName, const std::string& info)
{
    std::string s;

    /* Append AST name */
    const auto& memberName = TopMemberName();
    if (!memberName.empty())
    {
        s += memberName;
        s += " : ";
        s += astName;
    }
    else
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

bool ASTPrinter::PushPrintable(const AST* ast, const std::string& label)
{
    if (!label.empty())
    {
        /* Store longest source position string */
        const auto& pos = ast->area.Pos();

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

void ASTPrinter::Printable(const AST* ast, const std::string& label)
{
    PushPrintable(ast, label);
    PopPrintable();
}

void ASTPrinter::PrintableType(TypedAST* ast)
{
    try
    {
        Printable(ast, "GetTypeDenoter() : " + ast->GetTypeDenoter()->ToString());
    }
    catch (const std::exception&)
    {
        Printable(ast, "GetTypeDenoter() : <" + R_Unspecified + ">");
    }
}

ASTPrinter::PrintableTree* ASTPrinter::TopPrintable()
{
    if (parentNodeStack_.empty())
        return (&treeRoot_);
    else
        return parentNodeStack_.top();
}

void ASTPrinter::PushMemberName(const std::string& name)
{
    memberNameStack_.push(name);
}

void ASTPrinter::PopMemberName()
{
    if (!memberNameStack_.empty())
        memberNameStack_.pop();
}

const std::string& ASTPrinter::TopMemberName() const
{
    if (memberNameStack_.empty())
    {
        static const std::string defaultName;
        return defaultName;
    }
    else
        return memberNameStack_.top();
}


} // /namespace Xsc



// ================================================================================