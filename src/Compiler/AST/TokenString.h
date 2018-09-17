/*
 * TokenString.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_TOKEN_STRING_H
#define XSC_TOKEN_STRING_H


#include "Token.h"
#include <vector>
#include <ostream>


namespace Xsc
{


void AssertReachedEnd(bool reachedEnd);
void AssertCurrentTokenType(const Token::Types type, const Token::Types expectedType);

/*
Token string template class.
This is a helper class to iterate only over a list of tokens that are of interest (e.g. to ignore white spaces).
This class is mainly used by the preprocessor, because the preprocessor must keep all white spaces and new-lines for its output.
'TokenType' should be either from type 'Token*' or 'TokenPtr'.
'TokenOfInterestFunctor' must be a type with a static function of the following interface:
"bool IsOfInterest(const TokenType& token)"
*/
template <typename TokenType, typename TokenOfInterestFunctor>
class BasicTokenString
{

    public:

        using ValueType = TokenType;
        using Container = std::vector<TokenType>;

        class ConstIterator
        {

            public:

                ConstIterator() = default;

                ConstIterator(const ConstIterator&) = default;
                ConstIterator& operator = (const ConstIterator&) = default;

                ConstIterator(const typename Container::const_iterator& it, const typename Container::const_iterator& itEnd) :
                    it_    { it    },
                    itEnd_ { itEnd }
                {
                    NextTokenOfInterest();
                }

                ConstIterator& operator ++ ()
                {
                    if (it_ != itEnd_)
                    {
                        ++it_;
                        NextTokenOfInterest();
                    }
                    return *this;
                }

                ConstIterator operator ++ (int)
                {
                    ConstIterator tmp = *this;
                    operator ++ ();
                    return tmp;
                }

                bool ReachedEnd() const
                {
                    return (it_ == itEnd_);
                }

                ValueType AcceptIt()
                {
                    AssertReachedEnd(ReachedEnd());
                    return *((*this)++);
                }

                ValueType Accept(const Token::Types type)
                {
                    AssertReachedEnd(ReachedEnd());
                    AssertCurrentTokenType((*it_)->Type(), type);
                    return AcceptIt();
                }

                const ValueType& operator * ()
                {
                    return *it_;
                }

                typename Container::const_iterator operator -> ()
                {
                    return it_;
                }

            private:

                void NextTokenOfInterest()
                {
                    while (it_ != itEnd_ && !TokenOfInterestFunctor::IsOfInterest(*it_))
                        ++it_;
                }

                typename Container::const_iterator it_, itEnd_;

        };

        BasicTokenString() = default;
        BasicTokenString(const BasicTokenString&) = default;
        BasicTokenString& operator = (const BasicTokenString&) = default;

        BasicTokenString(const TokenType& token)
        {
            PushBack(token);
        }

        ConstIterator Begin() const
        {
            return ConstIterator(tokens_.begin(), tokens_.end());
        }

        ConstIterator End() const
        {
            return ConstIterator(tokens_.end(), tokens_.end());
        }

        void PushBack(const TokenType& token)
        {
            tokens_.push_back(token);
        }

        void PushBack(const BasicTokenString& tokenString)
        {
            tokens_.insert(tokens_.end(), tokenString.tokens_.begin(), tokenString.tokens_.end());
        }

        void PopFront()
        {
            tokens_.erase(tokens_.begin());
        }

        void PopBack()
        {
            tokens_.pop_back();
        }

        const ValueType& Front() const
        {
            return tokens_.front();
        }

        const ValueType& Back() const
        {
            return tokens_.back();
        }

        bool Empty() const
        {
            return tokens_.empty();
        }

        const Container& GetTokens() const
        {
            return tokens_;
        }

        Container& GetTokens()
        {
            return tokens_;
        }

        // Removes all tokens that are not of interest for the specified token string from the front.
        void TrimFront()
        {
            while (!Empty() && !TokenOfInterestFunctor::IsOfInterest(Front()))
                PopFront();
        }

        // Removes all tokens that are not of interest for the specified token string from the back.
        void TrimBack()
        {
            while (!Empty() && !TokenOfInterestFunctor::IsOfInterest(Back()))
                PopBack();
        }

    private:

        Container tokens_;

};


/* ----- Global operators ----- */

template <typename TokenType, typename TokenOfInterestFunctor>
bool operator == (const BasicTokenString<TokenType, TokenOfInterestFunctor>& lhs, const BasicTokenString<TokenType, TokenOfInterestFunctor>& rhs)
{
    /* Get first tokens */
    auto lhsIt = lhs.Begin();
    auto rhsIt = rhs.Begin();

    /* Check if all tokens of interest are equal in both strings */
    for (; (!lhsIt.ReachedEnd() && !rhsIt.ReachedEnd()); ++lhsIt, ++rhsIt)
    {
        auto lhsTkn = lhsIt->get();
        auto rhsTkn = rhsIt->get();

        /* Compare types */
        if (lhsTkn->Type() != rhsTkn->Type())
            return false;

        /* Compare values */
        if (lhsTkn->Spell() != rhsTkn->Spell())
            return false;
    }

    /* Check if both strings reached the end */
    return (lhsIt.ReachedEnd() && rhsIt.ReachedEnd());
}

template <typename TokenType, typename TokenOfInterestFunctor>
bool operator != (const BasicTokenString<TokenType, TokenOfInterestFunctor>& lhs, const BasicTokenString<TokenType, TokenOfInterestFunctor>& rhs)
{
    return !(lhs == rhs);
}

template <typename TokenType, typename TokenOfInterestFunctor>
std::ostream& operator << (std::ostream& lhs, const BasicTokenString<TokenType, TokenOfInterestFunctor>& rhs)
{
    for (const auto& tkn : rhs.GetTokens())
        lhs << tkn->Spell();
    return lhs;
}


/* --- Default BasicTokenString types --- */

struct DefaultTokenOfInterestFunctor
{
    static bool IsOfInterest(const TokenPtr& token);
};

using TokenPtrString = BasicTokenString<TokenPtr, DefaultTokenOfInterestFunctor>;


} // /namespace Xsc


#endif



// ================================================================================