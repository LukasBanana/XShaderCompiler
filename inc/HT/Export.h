/*
 * Export.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_EXPORT_H
#define XSC_EXPORT_H


#if defined(_MSC_VER) && defined(HTLIB_SHARED_LIB)
#   define HTLIB_EXPORT __declspec(dllexport)
#else
#   define HTLIB_EXPORT
#endif


#endif



// ================================================================================