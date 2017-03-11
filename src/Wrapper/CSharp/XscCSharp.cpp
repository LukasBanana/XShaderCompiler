/*
 * XscCSharp.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/Xsc.h>
#include <vcclr.h>

#using <System.dll>
#using <System.Core.dll>


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

        property String ^ Version
        {
            String ^ get()
            {
                return gcnew String(XSC_VERSION_STRING);
            }
        }

        property Collections::Generic::Dictionary<String^, int>^ GLSLExtensionEnumeration
        {
            Collections::Generic::Dictionary<String^, int>^ get()
            {
                auto dict = gcnew Collections::Generic::Dictionary<String^, int>();

                for (const auto it : Xsc::GetGLSLExtensionEnumeration())
                    dict->Add(gcnew String(it.first.c_str()), it.second);

                return dict;
            }
        }


    private:



};





// ================================================================================
