/*
 * DebuggerView.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DebuggerView.h"


namespace Xsc
{


static long DebuggerViewStyle()
{
    return (wxSYSTEM_MENU | wxCAPTION | wxCLIP_CHILDREN | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxRESIZE_BORDER | wxCLOSE_BOX);
}

DebuggerView::DebuggerView(const wxPoint& pos, const wxSize& size) :
    wxFrame{ nullptr, wxID_ANY, "Xsc Debugger", pos, size, DebuggerViewStyle() }
{
    #ifdef _WIN32
    SetIcon(wxICON(APP_ICON));
    #endif

    //CreateLayout();
    Centre();
}


} // /namespace Xsc



// ================================================================================
