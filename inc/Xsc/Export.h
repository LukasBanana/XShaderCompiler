/*
 * Export.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_EXPORT_H
#define XSC_EXPORT_H


#if defined(_MSC_VER) && defined(XSC_SHARED_LIB)
#   define XSC_EXPORT __declspec(dllexport)
#else
#   define XSC_EXPORT
#endif


#endif



// ================================================================================