/*
 * ASTPrinter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ASTPrinter.h"
#include "AST.h"
#include "ReportIdents.h"
#include <Xsc/ConsoleManip.h>
#include <stdexcept>
#include <algorithm>


namespace Xsc
{


void ASTPrinter::PrintAST(Program* program, std::ostream& output)
{
    /* Build new printable tree */
    Visit(program);

    /* Print all children of the tree root */
    for (const auto& child : treeRoot_.children)
        Print(child, output);

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
inline std::string MemberToString<int>(const int& member)
{
    return std::to_string(member);
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

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ASTPrinter::Visit##AST_NAME(AST_NAME* ast, void* args)

#define VISIT_MEMBER(MEMBER) \
    VisitMember(ast->MEMBER, #MEMBER)

#define ADD_PRINTABLE_MEMBER(MEMBER) \
    Printable(ast, std::string(#MEMBER), MemberToString(ast->MEMBER))

/* ------- Visit functions ------- */

IMPLEMENT_VISIT_PROC(Program)
{
    PushPrintable(ast, WriteLabel("Program"));
    {
        VISIT_MEMBER(globalStmts);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    PushPrintable(ast, WriteLabel("CodeBlock"));
    {
        VISIT_MEMBER(stmts);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(Attribute)
{
    PushPrintable(ast, WriteLabel("Attribute"));
    {
        Printable(ast, "attributeType", ast->ToString());
        VISIT_MEMBER(arguments);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(SwitchCase)
{
    PushPrintable(ast, WriteLabel("SwitchCase"));
    {
        VISIT_MEMBER(expr);
        VISIT_MEMBER(stmts);
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
        Printable(ast, "registerType", std::string(1, RegisterTypeToChar(ast->registerType)));
        ADD_PRINTABLE_MEMBER(slot);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(PackOffset)
{
    PushPrintable(ast, WriteLabel("PackOffset"));
    {
        ADD_PRINTABLE_MEMBER(registerName);
        ADD_PRINTABLE_MEMBER(vectorComponent);
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
    PushPrintable(ast, WriteLabel("TypeSpecifier", ast));
    {
        VISIT_MEMBER(structDecl);
        Printable(ast, "typeDenoter", ast->ToString());
    }
    PopPrintable();
}

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    PushPrintable(ast, WriteLabel("VarDecl", ast));
    {
        ADD_PRINTABLE_MEMBER(ident);
        VISIT_MEMBER(namespaceExpr);
        VISIT_MEMBER(arrayDims);
        VISIT_MEMBER(slotRegisters);
        VISIT_MEMBER(packOffset);
        VISIT_MEMBER(annotations);
        VISIT_MEMBER(initializer);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(BufferDecl)
{
    PushPrintable(ast, WriteLabel("BufferDecl", ast));
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
    PushPrintable(ast, WriteLabel("SamplerDecl", ast));
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
    PushPrintable(ast, WriteLabel("StructDecl", ast));
    {
        ADD_PRINTABLE_MEMBER(ident);
        ADD_PRINTABLE_MEMBER(baseStructName);
        VISIT_MEMBER(localStmts);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(AliasDecl)
{
    if (!ast->flags(AST::isBuiltin))
    {
        ADD_PRINTABLE_MEMBER(ident);
        Printable(ast, WriteLabel("AliasDecl", ast));
    }
}

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    PushPrintable(ast, WriteLabel("FunctionDecl", ast));
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
    PushPrintable(ast, WriteLabel("UniformBufferDecl", ast));
    {
        ADD_PRINTABLE_MEMBER(ident);
        Printable(ast, "bufferType", std::string(ast->bufferType == UniformBufferType::ConstantBuffer ? "cbuffer" : "tbuffer"));
        VISIT_MEMBER(slotRegisters);
        VISIT_MEMBER(localStmts);
    }
    PopPrintable();
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(BufferDeclStmt)
{
    PushPrintable(ast, WriteLabel("BufferDeclStmt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(bufferDecls);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(SamplerDeclStmt)
{
    PushPrintable(ast, WriteLabel("SamplerDeclStmt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(samplerDecls);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(VarDeclStmt)
{
    PushPrintable(ast, WriteLabel("VarDeclStmt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(typeSpecifier);
        VISIT_MEMBER(varDecls);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(AliasDeclStmt)
{
    if (!ast->flags(AST::isBuiltin))
    {
        PushPrintable(ast, WriteLabel("AliasDeclStmt"));
        {
            VISIT_MEMBER(attribs);
            VISIT_MEMBER(structDecl);
            VISIT_MEMBER(aliasDecls);
        }
        PopPrintable();
    }
}

IMPLEMENT_VISIT_PROC(BasicDeclStmt)
{
    PushPrintable(ast, WriteLabel("BasicDeclStmt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(declObject);
    }
    PopPrintable();
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(NullStmt)
{
    PushPrintable(ast, WriteLabel("NullStmt"));
    {
        VISIT_MEMBER(attribs);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(ScopeStmt)
{
    PushPrintable(ast, WriteLabel("ScopeStmt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(codeBlock);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(ForStmt)
{
    PushPrintable(ast, WriteLabel("ForStmt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(initStmt);
        VISIT_MEMBER(condition);
        VISIT_MEMBER(iteration);
        VISIT_MEMBER(bodyStmt);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(WhileStmt)
{
    PushPrintable(ast, WriteLabel("WhileStmt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(condition);
        VISIT_MEMBER(bodyStmt);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(DoWhileStmt)
{
    PushPrintable(ast, WriteLabel("DoWhileStmt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(bodyStmt);
        VISIT_MEMBER(condition);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(IfStmt)
{
    PushPrintable(ast, WriteLabel("IfStmt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(condition);
        VISIT_MEMBER(bodyStmt);
        VISIT_MEMBER(elseStmt);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(SwitchStmt)
{
    PushPrintable(ast, WriteLabel("SwitchStmt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(selector);
        VISIT_MEMBER(cases);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(ExprStmt)
{
    PushPrintable(ast, WriteLabel("ExprStmt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(expr);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(ReturnStmt)
{
    PushPrintable(ast, WriteLabel("ReturnStmt"));
    {
        VISIT_MEMBER(attribs);
        VISIT_MEMBER(expr);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(JumpStmt)
{
    PushPrintable(ast, WriteLabel("JumpStmt"));
    {
        VISIT_MEMBER(attribs);
        Printable(ast, "transfer", CtrlTransformToString(ast->transfer));
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(LayoutStmt)
{
    PushPrintable(ast, WriteLabel("LayoutStmt"));
    {
        VISIT_MEMBER(attribs);
        Printable(ast, "isInput", (ast->isInput ? "true" : "false"));
        Printable(ast, "isOutput", (ast->isOutput ? "true" : "false"));
    }
    PopPrintable();
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(NullExpr)
{
    Printable(ast, WriteLabel("NullExpr", ast));
}

IMPLEMENT_VISIT_PROC(SequenceExpr)
{
    PushPrintable(ast, WriteLabel("SequenceExpr", ast));
    {
        VISIT_MEMBER(exprs);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(LiteralExpr)
{
    PushPrintable(ast, WriteLabel("LiteralExpr", ast));
    {
        ADD_PRINTABLE_MEMBER(value);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(TypeSpecifierExpr)
{
    PushPrintable(ast, WriteLabel("TypeSpecifierExpr", ast));
    {
        VISIT_MEMBER(typeSpecifier);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(TernaryExpr)
{
    PushPrintable(ast, WriteLabel("TernaryExpr", ast));
    {
        VISIT_MEMBER(condExpr);
        VISIT_MEMBER(thenExpr);
        VISIT_MEMBER(elseExpr);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    PushPrintable(ast, WriteLabel("BinaryExpr", ast));
    {
        VISIT_MEMBER(lhsExpr);
        Printable(ast, "op", BinaryOpToString(ast->op));
        VISIT_MEMBER(rhsExpr);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    PushPrintable(ast, WriteLabel("UnaryExpr", ast));
    {
        if (ast->IsPostUnary())
        {
            VISIT_MEMBER(expr);
            Printable(ast, "op", UnaryOpToString(ast->op, true));
        }
        else
        {
            Printable(ast, "op", UnaryOpToString(ast->op, false));
            VISIT_MEMBER(expr);
        }
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(CallExpr)
{
    PushPrintable(ast, WriteLabel("CallExpr", ast));
    {
        VISIT_MEMBER(prefixExpr);
        ADD_PRINTABLE_MEMBER(isStatic);
        ADD_PRINTABLE_MEMBER(ident);
        VISIT_MEMBER(arguments);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(BracketExpr)
{
    PushPrintable(ast, WriteLabel("BracketExpr", ast));
    {
        VISIT_MEMBER(expr);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(AssignExpr)
{
    PushPrintable(ast, WriteLabel("AssignExpr", ast));
    {
        VISIT_MEMBER(lvalueExpr);
        Printable(ast, "op", AssignOpToString(ast->op));
        VISIT_MEMBER(rvalueExpr);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(IdentExpr)
{
    PushPrintable(ast, WriteLabel("IdentExpr", ast));
    {
        VISIT_MEMBER(prefixExpr);
        ADD_PRINTABLE_MEMBER(isStatic);
        ADD_PRINTABLE_MEMBER(ident);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(SubscriptExpr)
{
    PushPrintable(ast, WriteLabel("SubscriptExpr", ast));
    {
        VISIT_MEMBER(prefixExpr);
        VISIT_MEMBER(arrayIndices);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    PushPrintable(ast, WriteLabel("CastExpr", ast));
    {
        VISIT_MEMBER(typeSpecifier);
        VISIT_MEMBER(expr);
    }
    PopPrintable();
}

IMPLEMENT_VISIT_PROC(InitializerExpr)
{
    PushPrintable(ast, WriteLabel("InitializerExpr", ast));
    {
        VISIT_MEMBER(exprs);
    }
    PopPrintable();
}

#undef IMPLEMENT_VISIT_PROC
#undef VISIT_MEMBER
#undef ADD_PRINTABLE_MEMBER

/* --- Helper functions --- */

std::string ASTPrinter::WriteLabel(const std::string& astName, TypedAST* ast)
{
    std::string s;

    /* Append member name */
    const auto& memberName = TopMemberName();
    if (!memberName.empty())
    {
        s += memberName;
        s += " : ";
    }

    /* Append AST name */
    s += astName;

    /* Append type denoter of typed AST */
    if (ast)
    {
        s += " <";

        try
        {
            s += ast->GetTypeDenoter()->ToString();
        }
        catch (const std::exception&)
        {
            s += R_Unspecified;
        }

        s += '>';
    }

    return s;
}

void ASTPrinter::Print(const PrintableTree& tree, std::ostream& output)
{
    using Colors = ConsoleManip::ColorFlags;

    {
        ConsoleManip::ScopedColor highlight(Colors::Green | Colors::Blue, output);

        /* Write row of source position */
        output << std::string(maxRowStrLen_ - tree.row.size(), ' ');
        output << tree.row;
        output << ':';

        /* Write column of source position */
        output << std::string(maxColStrLen_ - tree.col.size(), ' ');
        output << tree.col;
        output << "  ";
    }

    /* Write node hierarchy level */
    if (!lastSubNodeStack_.empty())
    {
        for (std::size_t i = 0; i + 1 < lastSubNodeStack_.size(); ++i)
        {
            if (lastSubNodeStack_[i])
                output << "  ";
            else
                output << "| ";
        }

        if (lastSubNodeStack_.back())
            output << "`-";
        else
            output << "|-";
    }

    /* Write label */
    output << tree.label;

    /* Write value */
    if (!tree.value.empty())
    {
        output << " : ";
        ConsoleManip::ScopedColor highlight(Colors::Yellow, output);
        output << tree.value;
    }

    output << std::endl;

    /* Print children */
    if (!tree.children.empty())
    {
        lastSubNodeStack_.push_back(false);
        {
            /* Print all children except the last one */
            for (std::size_t i = 0, n = tree.children.size(); i + 1 < n; ++i)
                Print(tree.children[i], output);

            /* Print last children */
            lastSubNodeStack_.back() = true;
            Print(tree.children.back(), output);
        }
        lastSubNodeStack_.pop_back();
    }
}

bool ASTPrinter::PushPrintable(const AST* ast, const std::string& label, const std::string& value)
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
        children.push_back({ rowStr, colStr, label, value, {} });
        parentNodeStack_.push(&(children.back()));

        return true;
    }
    return false;
}

void ASTPrinter::PopPrintable()
{
    parentNodeStack_.pop();
}

void ASTPrinter::Printable(const AST* ast, const std::string& label, const std::string& value)
{
    PushPrintable(ast, label, value);
    PopPrintable();
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