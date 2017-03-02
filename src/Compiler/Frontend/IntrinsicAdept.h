/*
 * Scanner.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_INTRINSIC_ADEPT_H
#define XSC_INTRINSIC_ADEPT_H


#include "ASTEnums.h"
#include "Visitor.h"
#include "TypeDenoter.h"
#include <string>
#include <array>


namespace Xsc
{


// Converts the Intrinsic enum value into a zero-based integral.
#define INTRINSIC_IDX(I) (static_cast<std::size_t>(I) - static_cast<std::size_t>(Intrinsic::Abort))

// Base class for intrinsic type analysis.
class IntrinsicAdept
{
    
    public:
        
        IntrinsicAdept(const IntrinsicAdept&) = delete;
        IntrinsicAdept& operator = (const IntrinsicAdept&) = delete;

        virtual ~IntrinsicAdept();

        // Returns the active intrinsic adept instance.
        static const IntrinsicAdept& Get();

        // Returns the identifier of the specified intrinsic or "<undefined>" if the input ID is out of range.
        const std::string& GetIntrinsicIdent(const Intrinsic intrinsic) const;

        // Returns the return type denoter of the specified intrinsic with its arguments or throws an error if the call is ambiguous.
        virtual TypeDenoterPtr GetIntrinsicReturnType(const Intrinsic intrinsic, const std::vector<ExprPtr>& args) const = 0;

        // Returns a list of all parameter types of the specified intrinsic with its arguments or throws an error if the call is ambiguous.
        virtual std::vector<TypeDenoterPtr> GetIntrinsicParameterTypes(const Intrinsic intrinsic, const std::vector<ExprPtr>& args) const = 0;

        // Returns a list of indices that refer to all output parameters of the specified intrinsic.
        virtual std::vector<std::size_t> GetIntrinsicOutputParameterIndices(const Intrinsic intrinsic) const = 0;

    protected:

        IntrinsicAdept();

        // Sets the identifier of the specified intrinsic.
        void SetIntrinsicIdent(const Intrinsic intrinsic, const std::string& ident);

        // Fill all remaining intrinsic identifiers for overloaded intrinsics.
        void FillOverloadedIntrinsicIdents();

        [[noreturn]]
        void ThrowAmbiguousIntrinsicCall(const Intrinsic intrinsic, const std::vector<ExprPtr>& args);

    private:

        static const std::size_t numIntrinsics = (INTRINSIC_IDX(Intrinsic::StreamOutput_RestartStrip) + 1u);

        std::array<std::string, numIntrinsics> intrinsicIdents_;

};


} // /namespace Xsc


#endif



// ================================================================================