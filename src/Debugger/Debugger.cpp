/*
 * Debugger.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Debugger.h"
#include "DebuggerView.h"
#include "WxHelper.h"


namespace Xsc
{


IMPLEMENT_APP(Debugger)

bool Debugger::OnInit()
{
    if (!wxApp::OnInit())
        return false;

    auto debuggerView = WxMake<DebuggerView>(wxDefaultPosition, wxSize(1280, 768));
    debuggerView->Show();

    return true;
}


} // /namespace Xsc



// ================================================================================
