/*
 * Debugger.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_DEBUGGER_H
#define XSC_DEBUGGER_H


#include <wx/app.h>


namespace Xsc
{


// The shell is the main class of the command line tool for the compiler.
class Debugger : public wxApp
{
    
    public:
        
        bool OnInit() override;

};


} // /namespace Xsc


#endif



// ================================================================================