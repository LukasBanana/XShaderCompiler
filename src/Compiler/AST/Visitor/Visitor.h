/*
 * Visitor.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_VISITOR_H
#define XSC_VISITOR_H


#include <memory>
#include <vector>


namespace Xsc
{


// Declare all AST node classes

#define DECL_PTR(CLASS_NAME)                            \
    struct CLASS_NAME;                                  \
    using CLASS_NAME##Ptr = std::shared_ptr<CLASS_NAME>

DECL_PTR( AST               );
DECL_PTR( Stmnt             );
DECL_PTR( Expr              );

DECL_PTR( Program           );
DECL_PTR( CodeBlock         );
DECL_PTR( FunctionCall      );
DECL_PTR( Attribute         );
DECL_PTR( SwitchCase        );
DECL_PTR( SamplerValue      );
DECL_PTR( Register          );
DECL_PTR( PackOffset        );
DECL_PTR( VarType           );
DECL_PTR( VarIdent          );

DECL_PTR( VarDecl           );
DECL_PTR( BufferDecl       );
DECL_PTR( SamplerDecl       );
DECL_PTR( StructDecl        );
DECL_PTR( AliasDecl         );

DECL_PTR( FunctionDecl      );
DECL_PTR( UniformBufferDecl );
DECL_PTR( BufferDeclStmnt  );
DECL_PTR( SamplerDeclStmnt  );
DECL_PTR( StructDeclStmnt   );
DECL_PTR( VarDeclStmnt      );
DECL_PTR( AliasDeclStmnt    );

DECL_PTR( NullStmnt         );
DECL_PTR( CodeBlockStmnt    );
DECL_PTR( ForLoopStmnt      );
DECL_PTR( WhileLoopStmnt    );
DECL_PTR( DoWhileLoopStmnt  );
DECL_PTR( IfStmnt           );
DECL_PTR( ElseStmnt         );
DECL_PTR( SwitchStmnt       );
DECL_PTR( ExprStmnt         );
DECL_PTR( ReturnStmnt       );
DECL_PTR( CtrlTransferStmnt );

DECL_PTR( NullExpr          );
DECL_PTR( ListExpr          );
DECL_PTR( LiteralExpr       );
DECL_PTR( TypeNameExpr      );
DECL_PTR( TernaryExpr       );
DECL_PTR( BinaryExpr        );
DECL_PTR( UnaryExpr         );
DECL_PTR( PostUnaryExpr     );
DECL_PTR( FunctionCallExpr  );
DECL_PTR( BracketExpr       );
DECL_PTR( SuffixExpr        );
DECL_PTR( ArrayAccessExpr   );
DECL_PTR( CastExpr          );
DECL_PTR( VarAccessExpr     );
DECL_PTR( InitializerExpr   );

#undef DECL_PTR

// Visitor interface

#define VISITOR_VISIT_PROC(CLASS_NAME) \
    virtual void Visit##CLASS_NAME(CLASS_NAME* ast, void* args)

#define DECL_VISIT_PROC(CLASS_NAME) \
    void Visit##CLASS_NAME(CLASS_NAME* ast, void* args) override

#define VISIT_DEFAULT(CLASS_NAME) \
    Visitor::Visit##CLASS_NAME(ast, args)

class Visitor
{
    
    public:
        
        virtual ~Visitor();

        VISITOR_VISIT_PROC( Program           );
        VISITOR_VISIT_PROC( CodeBlock         );
        VISITOR_VISIT_PROC( FunctionCall      );
        VISITOR_VISIT_PROC( Attribute         );
        VISITOR_VISIT_PROC( SwitchCase        );
        VISITOR_VISIT_PROC( SamplerValue      );
        VISITOR_VISIT_PROC( Register          );
        VISITOR_VISIT_PROC( PackOffset        );
        VISITOR_VISIT_PROC( VarType           );
        VISITOR_VISIT_PROC( VarIdent          );

        VISITOR_VISIT_PROC( VarDecl           );
        VISITOR_VISIT_PROC( BufferDecl       );
        VISITOR_VISIT_PROC( SamplerDecl       );
        VISITOR_VISIT_PROC( StructDecl        );
        VISITOR_VISIT_PROC( AliasDecl         );

        VISITOR_VISIT_PROC( FunctionDecl      );
        VISITOR_VISIT_PROC( UniformBufferDecl );
        VISITOR_VISIT_PROC( BufferDeclStmnt  );
        VISITOR_VISIT_PROC( SamplerDeclStmnt  );
        VISITOR_VISIT_PROC( StructDeclStmnt   );
        VISITOR_VISIT_PROC( VarDeclStmnt      );
        VISITOR_VISIT_PROC( AliasDeclStmnt    );

        VISITOR_VISIT_PROC( NullStmnt         );
        VISITOR_VISIT_PROC( CodeBlockStmnt    );
        VISITOR_VISIT_PROC( ForLoopStmnt      );
        VISITOR_VISIT_PROC( WhileLoopStmnt    );
        VISITOR_VISIT_PROC( DoWhileLoopStmnt  );
        VISITOR_VISIT_PROC( IfStmnt           );
        VISITOR_VISIT_PROC( ElseStmnt         );
        VISITOR_VISIT_PROC( SwitchStmnt       );
        VISITOR_VISIT_PROC( ExprStmnt         );
        VISITOR_VISIT_PROC( ReturnStmnt       );
        VISITOR_VISIT_PROC( CtrlTransferStmnt );

        VISITOR_VISIT_PROC( NullExpr          );
        VISITOR_VISIT_PROC( ListExpr          );
        VISITOR_VISIT_PROC( LiteralExpr       );
        VISITOR_VISIT_PROC( TypeNameExpr      );
        VISITOR_VISIT_PROC( TernaryExpr       );
        VISITOR_VISIT_PROC( BinaryExpr        );
        VISITOR_VISIT_PROC( UnaryExpr         );
        VISITOR_VISIT_PROC( PostUnaryExpr     );
        VISITOR_VISIT_PROC( FunctionCallExpr  );
        VISITOR_VISIT_PROC( BracketExpr       );
        VISITOR_VISIT_PROC( SuffixExpr        );
        VISITOR_VISIT_PROC( ArrayAccessExpr   );
        VISITOR_VISIT_PROC( CastExpr          );
        VISITOR_VISIT_PROC( VarAccessExpr     );
        VISITOR_VISIT_PROC( InitializerExpr   );

    protected:
        
        template <typename T>
        void Visit(T ast, void* args = nullptr)
        {
            if (ast)
                ast->Visit(this, args);
        }

        template <typename T>
        void Visit(const std::vector<T>& astList, void* args = nullptr)
        {
            for (const auto& ast : astList)
                Visit(ast, args);
        }

};

#undef VISITOR_VISIT_PROC


} // /namespace Xsc


#endif



// ================================================================================