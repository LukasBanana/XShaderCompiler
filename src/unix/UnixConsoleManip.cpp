/*
 * UnixConsoleManip.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <HT/ConsoleManip.h>
#include <stack>


namespace HTLib
{

namespace ConsoleManip
{


/* 
 * Internal members
 */

class IOModifier
{

    public:
    
        struct Codes
        {
            enum
            {
                Red         = 1,
                Green       = 2,
                Blue        = 4,
                
                Foreground  = 30,
                Background  = 40,
                Bright      = 60,
            };
        };
        
        IOModifier() = default;

        inline IOModifier(int code) :
            codeFg_{ code }
        {
        }

        inline IOModifier(int codeFg, int codeBg) :
            codeFg_{ codeFg },
            codeBg_{ codeBg }
        {
        }
        
        inline int CodeFg() const
        {
            return codeFg_;
        }

        inline int CodeBg() const
        {
            return codeBg_;
        }

    private:
        
        int codeFg_ = 0;
        int codeBg_ = 0;
        
};

static std::ostream& operator << (std::ostream& os, const IOModifier& mod)
{
    os << "\x1b[";
    if (mod.CodeFg() && mod.CodeBg())
        os << mod.CodeFg() << ";" << mod.CodeBg();
    else if (mod.CodeFg())
        os << mod.CodeFg();
    else if (mod.CodeBg())
        os << mod.CodeBg();
    return os << "m";
}

class IOModifierState
{

    public:

        ~IOModifierState();

        void Push(std::ostream& stream, const IOModifier& mode);
        void Pop();

    private:

        struct StackEntry
        {
            std::ostream&   stream;
            IOModifier      modifier;
        };

        std::stack<StackEntry> modifierStack_;

};

static IOModifierState g_modifierState;

static int GetModCode(long color, bool fg)
{
    using Cd = IOModifier::Codes;
    
    int code = 0;
    
    if ((color & Color::Red) != 0)
        code += Cd::Red;
    if ((color & Color::Green) != 0)
        code += Cd::Green;
    if ((color & Color::Blue) != 0)
        code += Cd::Blue;
    if ((color & Color::Intens) != 0)
        code += Cd::Bright;
    
    code += (fg ? Cd::Foreground : Cd::Background);
    
    return code;
}

IOModifierState::~IOModifierState()
{
    while (!modifierStack_.empty())
        Pop();
}

void IOModifierState::Push(std::ostream& stream, const IOModifier& mode)
{
    modifierStack_.push({ stream, mod });
    stream << mod;
}

void IOModifierState::Pop()
{
    if (!modifierStack_.empty())
    {
        auto& stream = modifierStack_.top().stream;

        modifierStack_.pop();

        if (modifierStack_.empty())
            stream << IOModifier();
        else
            stream << modifierStack_.top().modifier;
    }
}


/*
 * Interface implementation
 */

void PushColor(std::ostream& stream, long front)
{
    if (IsEnabled())
        g_modifierState.Push(stream, IOModifier(GetModCode(front, true)));
}

void PushColor(std::ostream& stream, long front, long back)
{
    if (IsEnabled())
        g_modifierState.Push(stream, IOModifier(GetModCode(front, true), GetModCode(front, false)));
}

void PopColor(std::ostream& stream)
{
    if (IsEnabled())
        g_modifierState.Pop();
}


} // /namespace ConsoleManip

} // /namespace HTLib



// ================================================================================