/*
 * PreProcessor.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_PRE_PROCESSOR_H
#define XSC_PRE_PROCESSOR_H


#include <Xsc/Xsc.h>
#include <Xsc/Log.h>
#include "PreProcessorScanner.h"
#include "TokenString.h"
#include "ASTEnums.h"
#include "Parser.h"
#include "SourceCode.h"
#include <iostream>
#include <functional>
#include <initializer_list>
#include <stack>
#include <map>
#include <set>


namespace Xsc
{


/*
Pre-processor to substitute macros and include directives.
The preprocessor works on something similar to a Concrete Syntax Tree (CST) rather than an Abstract Syntax Tree (AST).
This is because the output is not an intermediate representation but rather concrete source code.
Therefore, all white spaces and new-line characters must NOT be ignored.
All other parsers and analyzers only work on an AST.
*/
class PreProcessor : public Parser
{
    
    public:
        
        PreProcessor(IncludeHandler& includeHandler, Log* log = nullptr);

        std::unique_ptr<std::iostream> Process(
            const SourceCodePtr& input,
            const std::string& filename = "",
            bool writeLineMarks = true,
            bool enableWarnings = false
        );

        // Returns a list of all defined macro identifiers after pre-processing.
        std::vector<std::string> ListDefinedMacroIdents() const;

    protected:

        // Macro object structure.
        struct Macro
        {
            Macro() = default;
            Macro(const Macro&) = default;
            Macro& operator = (const Macro&) = default;

            Macro(const TokenPtr& identTkn);
            Macro(const TokenPtr& identTkn, const TokenPtrString& value);
            
            Macro(
                const TokenPtr& identTkn,
                const TokenPtrString& value,
                const std::vector<std::string>& parameters,
                bool varArgs = false,
                bool stdMacro = false,
                bool emptyParamList = false
            );

            bool HasParameterList() const;

            TokenPtr                    identTkn;                   // Macro identifier token
            TokenPtrString              tokenString;                // Macro definition value as token string
            std::vector<std::string>    parameters;                 // Parameter identifiers
            bool                        varArgs         = false;    // Specifies whether the macro supports variadic arguments
            bool                        stdMacro        = false;    // Specifies whether the macro is a standard macro (i.e. part of the language) or not
            bool                        emptyParamList  = false;    // Macro has an empty parameter list
        };

        // Parses the specified directive, that is not part of the standard pre-processor directive (e.g. "version" or "extension" for GLSL).
        virtual void ParseDirective(const std::string& directive, bool ignoreUnknown);

        // Ignores the remaining tokens of the current directive.
        void IgnoreDirective();

        // Defines a macro with the specified identifier, value token string, and parameters.
        void DefineMacro(const Macro& macro);

        // Defines a standard macro (i.e. not part of the source code) with value set to integer literal '1'.
        void DefineStandardMacro(const std::string& ident, int intValue = 1);

        // Removes the macro definition with the specified identifier.
        void UndefineMacro(const std::string& ident, const Token* tkn = nullptr);

        // Returns true if the specified macro identifier is defined.
        bool IsDefined(const std::string& ident) const;

        // Callback function when a macro is about to be defined
        virtual bool OnDefineMacro(const Macro& macro);

        // Callback function when a macro is about to be redefined, returns true if the redefinition is allowed.
        virtual bool OnRedefineMacro(const Macro& macro, const Macro& previousMacro);

        // Callback function when a macro is about to be undefined, return true if the undefinition is allowed.
        virtual bool OnUndefineMacro(const Macro& macro);

        // Returns the output stream as reference.
        inline std::stringstream& Out()
        {
            return *output_;
        }

    private:
        
        /* === Structures === */

        struct IfBlock
        {
            void SetActive(bool activate);

            TokenPtr        directiveToken;
            SourceCodePtr   directiveSource;
            bool            parentActive    = true;     // Is the parent if-block active?
            bool            active          = true;     // Is this if-block active?
            bool            wasActive       = false;    // Was this if-block active?
            bool            elseAllowed     = true;     // Is an else-block allowed?
        };

        using MacroPtr = std::shared_ptr<Macro>;

        /* === Functions === */

        ScannerPtr MakeScanner() override;

        void PushScannerSource(const SourceCodePtr& source, const std::string& filename = "") override;
        bool PopScannerSource() override;

        void PushIfBlock(const TokenPtr& directiveToken, bool active = false, bool elseAllowed = true);
        void SetIfBlock(const TokenPtr& directiveToken, bool active = false, bool elseAllowed = true);
        void PopIfBlock();

        // Returns the if-block state from the top of the stack. If the stack is empty, the default state is returned.
        IfBlock TopIfBlock() const;

        /*
        Replaces all identifiers (specified by 'macro.parameters') in the token string (specified by 'macro.tokenString')
        by the respective replacement (specified by 'arguments'). The number of identifiers and the number of replacements must be equal.
        */
        TokenPtrString ExpandMacro(const Macro& macro, const std::vector<TokenPtrString>& arguments);

        // Writes a '#line'-directive to the output with the current source position and filename.
        void WritePosToLineDirective();

        /* ----- Parsing ----- */

        void            ParseProgram();

        void            ParesComment();
        void            ParseIdent();
        TokenPtrString  ParseIdentAsTokenString();
        TokenPtrString  ParseIdentArgumentsForMacro(const TokenPtr& identToken, const Macro& macro);
        void            ParseMisc();
        
        void            ParseDirective();
        void            ParseAnyIfDirectiveAndSkipValidation();
        void            ParseDirectiveDefine();
        void            ParseDirectiveUndef();
        void            ParseDirectiveInclude();
        void            ParseDirectiveIf(bool skipEvaluation = false);
        void            ParseDirectiveIfdef(bool skipEvaluation = false);
        void            ParseDirectiveIfndef(bool skipEvaluation = false);
        void            ParseDirectiveElif(bool skipEvaluation = false);
        void            ParseDirectiveIfOrElifCondition(bool isElseBranch, bool skipEvaluation = false);
        void            ParseDirectiveElse();
        void            ParseDirectiveEndif();
        void            ParseDirectivePragma();
        void            ParseDirectiveLine();
        void            ParseDirectiveError();

        ExprPtr         ParseExpr();
        ExprPtr         ParsePrimaryExpr() override;

        TokenPtrString  ParseDirectiveTokenString(bool expandDefinedDirective = false, bool ignoreComments = false);
        TokenPtrString  ParseArgumentTokenString();

        std::string     ParseDefinedMacro();

        /* === Members === */

        IncludeHandler&                     includeHandler_;

        std::unique_ptr<std::stringstream>  output_;

        std::map<std::string, MacroPtr>     macros_;
        std::set<std::string>               onceIncluded_;
        std::map<std::string, std::size_t>  includeCounter_; // Counter for each included file

        /*
        Stack to store the info which if-block in the hierarchy is active.
        Once an if-block is inactive, all subsequent if-blocks are inactive, too.
        */
        std::stack<IfBlock>                 ifBlockStack_;

        bool                                writeLineMarks_         = true;

};


} // /namespace Xsc


#endif



// ================================================================================