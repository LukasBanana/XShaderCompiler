/*
 * Debugger.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Debugger.h"
#include "DebuggerView.h"


namespace Xsc
{


IMPLEMENT_APP(Debugger)

bool Debugger::OnInit()
{
    if (!wxApp::OnInit())
        return false;

    auto debuggerView = new DebuggerView(wxDefaultPosition, wxSize(1280, 768));
    debuggerView->Show();
    debuggerView->LoadSettings();

    return true;
}


} // /namespace Xsc



// ================================================================================
