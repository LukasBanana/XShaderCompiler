/*
 * Float16Compressor.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_FLOAT16_COMPRESSOR_H
#define XSC_FLOAT16_COMPRESSOR_H


#include <cstdint>


namespace Xsc
{


// Compresses the specified 32-bit float into a 16-bit float (represented as 16-bit unsigend integer).
std::uint16_t CompressFloat16(float value);

// Decompresses the specified 16-bit float (represented as 16-bit unsigned integer) into a 32-bit float.
float DecompressFloat16(std::uint16_t value);


} // /namespace Xsc


#endif



// ================================================================================
