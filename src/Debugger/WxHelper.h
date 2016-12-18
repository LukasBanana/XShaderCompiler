/*
 * WxHelper.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_WX_HELPER_H
#define XSC_WX_HELPER_H


namespace Xsc
{


template <typename T, typename... Args>
T* WxMake(Args&&... args)
{
    return new T(std::forward<Args>(args)...);
}


} // /namespace Xsc


#endif



// ================================================================================