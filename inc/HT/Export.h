/*
 * Export.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef HTLIB_EXPORT_H
#define HTLIB_EXPORT_H


#if defined(_MSC_VER) && defined(_HT_DYNAMIC_LIB_)
#   define _HT_EXPORT_ __declspec(dllexport)
#else
#   define _HT_EXPORT_
#endif


#endif



// ================================================================================