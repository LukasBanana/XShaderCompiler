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
DECL_PTR( GlobalDecl        );
DECL_PTR( Stmnt             );
DECL_PTR( Expr              );

DECL_PTR( Program           );
DECL_PTR( CodeBlock         );
DECL_PTR( BufferDeclIdent   );
DECL_PTR( SamplerDeclIdent  );
DECL_PTR( FunctionCall      );
DECL_PTR( Structure         );
DECL_PTR( SwitchCase        );
DECL_PTR( SamplerValue      );

DECL_PTR( FunctionDecl      );
DECL_PTR( UniformBufferDecl );
//DECL_PTR( StorageBufferDecl );
DECL_PTR( TextureDecl       );
DECL_PTR( SamplerDecl       );
DECL_PTR( StructDecl        );

DECL_PTR( NullStmnt         );
DECL_PTR( CodeBlockStmnt    );
DECL_PTR( ForLoopStmnt      );
DECL_PTR( WhileLoopStmnt    );
DECL_PTR( DoWhileLoopStmnt  );
DECL_PTR( IfStmnt           );
DECL_PTR( ElseStmnt         );
DECL_PTR( SwitchStmnt       );
DECL_PTR( VarDeclStmnt      );
DECL_PTR( AssignStmnt       );
DECL_PTR( ExprStmnt         );
DECL_PTR( FunctionCallStmnt );
DECL_PTR( ReturnStmnt       );
DECL_PTR( StructDeclStmnt   );
DECL_PTR( CtrlTransferStmnt );

DECL_PTR( ListExpr          );
DECL_PTR( LiteralExpr       );
DECL_PTR( TypeNameExpr      );
DECL_PTR( TernaryExpr       );
DECL_PTR( BinaryExpr        );
DECL_PTR( UnaryExpr         );
DECL_PTR( PostUnaryExpr     );
DECL_PTR( FunctionCallExpr  );
DECL_PTR( BracketExpr       );
DECL_PTR( CastExpr          );
DECL_PTR( VarAccessExpr     );
DECL_PTR( InitializerExpr   );

DECL_PTR( PackOffset        );
DECL_PTR( VarSemantic       );
DECL_PTR( VarType           );
DECL_PTR( VarIdent          );
DECL_PTR( VarDecl           );

#undef DECL_PTR

// Visitor interface

#define VISITOR_VISIT_PROC(CLASS_NAME) \
    virtual void Visit##CLASS_NAME(CLASS_NAME* ast, void* args)

#define DECL_VISIT_PROC(CLASS_NAME) \
    void Visit##CLASS_NAME(CLASS_NAME* ast, void* args) override

class Visitor
{
    
    public:
        
        virtual ~Visitor();

        VISITOR_VISIT_PROC( Program           );
        VISITOR_VISIT_PROC( CodeBlock         );
        VISITOR_VISIT_PROC( BufferDeclIdent   );
        VISITOR_VISIT_PROC( SamplerDeclIdent  );
        VISITOR_VISIT_PROC( FunctionCall      );
        VISITOR_VISIT_PROC( Structure         );
        VISITOR_VISIT_PROC( SwitchCase        );
        VISITOR_VISIT_PROC( SamplerValue      );

        VISITOR_VISIT_PROC( FunctionDecl      );
        VISITOR_VISIT_PROC( UniformBufferDecl );
        VISITOR_VISIT_PROC( TextureDecl       );
        VISITOR_VISIT_PROC( SamplerDecl       );
        VISITOR_VISIT_PROC( StructDecl        );

        VISITOR_VISIT_PROC( NullStmnt         );
        VISITOR_VISIT_PROC( CodeBlockStmnt    );
        VISITOR_VISIT_PROC( ForLoopStmnt      );
        VISITOR_VISIT_PROC( WhileLoopStmnt    );
        VISITOR_VISIT_PROC( DoWhileLoopStmnt  );
        VISITOR_VISIT_PROC( IfStmnt           );
        VISITOR_VISIT_PROC( ElseStmnt         );
        VISITOR_VISIT_PROC( SwitchStmnt       );
        VISITOR_VISIT_PROC( VarDeclStmnt      );
        VISITOR_VISIT_PROC( AssignStmnt       );
        VISITOR_VISIT_PROC( ExprStmnt         );
        VISITOR_VISIT_PROC( FunctionCallStmnt );
        VISITOR_VISIT_PROC( ReturnStmnt       );
        VISITOR_VISIT_PROC( StructDeclStmnt   );
        VISITOR_VISIT_PROC( CtrlTransferStmnt );

        VISITOR_VISIT_PROC( ListExpr          );
        VISITOR_VISIT_PROC( LiteralExpr       );
        VISITOR_VISIT_PROC( TypeNameExpr      );
        VISITOR_VISIT_PROC( TernaryExpr       );
        VISITOR_VISIT_PROC( BinaryExpr        );
        VISITOR_VISIT_PROC( UnaryExpr         );
        VISITOR_VISIT_PROC( PostUnaryExpr     );
        VISITOR_VISIT_PROC( FunctionCallExpr  );
        VISITOR_VISIT_PROC( BracketExpr       );
        VISITOR_VISIT_PROC( CastExpr          );
        VISITOR_VISIT_PROC( VarAccessExpr     );
        VISITOR_VISIT_PROC( InitializerExpr   );

        VISITOR_VISIT_PROC( PackOffset        );
        VISITOR_VISIT_PROC( VarSemantic       );
        VISITOR_VISIT_PROC( VarType           );
        VISITOR_VISIT_PROC( VarIdent          );
        VISITOR_VISIT_PROC( VarDecl           );

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