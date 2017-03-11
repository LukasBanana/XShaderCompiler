/*
 * XscCSharp.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/Xsc.h>
#include <vcclr.h>
#using <System.dll>

using namespace System;

public ref class XscCompiler
{

    public:

        XscCompiler()
        {
        }

        ~XscCompiler()
        {
        }

        property String ^ get_Version
        {
            String ^ get()
            {
                return gcnew String(XSC_VERSION_STRING);
            }
        };

    private:



};



// ================================================================================
