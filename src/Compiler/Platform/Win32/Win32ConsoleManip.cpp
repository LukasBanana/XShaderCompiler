/*
 * Win32ConsoleManip.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/ConsoleManip.h>
#include <stack>

#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NOUSER
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX

#include <Windows.h>


namespace Xsc
{

namespace ConsoleManip
{


/*
 * Internal members
 */

class ScreenBufferInfo
{

    public:

        ~ScreenBufferInfo();

        void Push();
        void Pop();

    private:

        std::stack<CONSOLE_SCREEN_BUFFER_INFO> infoStack_;

};

thread_local static ScreenBufferInfo g_screenBufferInfo;

static HANDLE StdOut()
{
    return GetStdHandle(STD_OUTPUT_HANDLE);
}

ScreenBufferInfo::~ScreenBufferInfo()
{
    while (!infoStack_.empty())
        Pop();
}

void ScreenBufferInfo::Push()
{
    /* Get current console screen buffer info */
    CONSOLE_SCREEN_BUFFER_INFO bufInfo;
	GetConsoleScreenBufferInfo(StdOut(), &bufInfo);

    /* Push buffer info onto stack */
    g_screenBufferInfo.infoStack_.push(bufInfo);
}

void ScreenBufferInfo::Pop()
{
    if (!infoStack_.empty())
    {
        /* Pop buffer info from stack and reset previous console screen buffer info */
        SetConsoleTextAttribute(StdOut(), infoStack_.top().wAttributes);
        infoStack_.pop();
    }
}


/*
 * Public functions
 */

void PushColor(long front, std::ostream& /*stream*/)
{
    if (!IsEnabled())
        return;

    /* Push color attribute onto stack */
    g_screenBufferInfo.Push();

    /* Get current console screen buffer infor */
    CONSOLE_SCREEN_BUFFER_INFO bufInfo;
	GetConsoleScreenBufferInfo(StdOut(), &bufInfo);

    /* Setup attributes for new console color */
    WORD attrib = (bufInfo.wAttributes & 0xFFF0);

    if (( front & ColorFlags::Red    ) != 0) { attrib |= FOREGROUND_RED;       }
    if (( front & ColorFlags::Green  ) != 0) { attrib |= FOREGROUND_GREEN;     }
    if (( front & ColorFlags::Blue   ) != 0) { attrib |= FOREGROUND_BLUE;      }
    if (( front & ColorFlags::Intens ) != 0) { attrib |= FOREGROUND_INTENSITY; }

    /* Set new console attribute */
    SetConsoleTextAttribute(StdOut(), attrib);
}

void PushColor(long front, long back, std::ostream& /*stream*/)
{
    if (!IsEnabled())
        return;

    /* Push color attribute onto stack */
    g_screenBufferInfo.Push();

    /* Setup attributes for new console color */
    WORD attrib = 0;

    if (( front & ColorFlags::Red    ) != 0) { attrib |= FOREGROUND_RED;       }
    if (( front & ColorFlags::Green  ) != 0) { attrib |= FOREGROUND_GREEN;     }
    if (( front & ColorFlags::Blue   ) != 0) { attrib |= FOREGROUND_BLUE;      }
    if (( front & ColorFlags::Intens ) != 0) { attrib |= FOREGROUND_INTENSITY; }

    if (( back  & ColorFlags::Red    ) != 0) { attrib |= BACKGROUND_RED;       }
    if (( back  & ColorFlags::Green  ) != 0) { attrib |= BACKGROUND_GREEN;     }
    if (( back  & ColorFlags::Blue   ) != 0) { attrib |= BACKGROUND_BLUE;      }
    if (( back  & ColorFlags::Intens ) != 0) { attrib |= BACKGROUND_INTENSITY; }

    /* Set new console attribute */
    SetConsoleTextAttribute(StdOut(), attrib);
}

void PopColor(std::ostream& /*stream*/)
{
    if (IsEnabled())
        g_screenBufferInfo.Pop();
}


} // /namespace ConsoleManip

} // /namespace Xsc



// ================================================================================