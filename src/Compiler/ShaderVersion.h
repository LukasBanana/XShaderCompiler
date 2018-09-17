/*
 * ShaderVersion.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_SHADER_VERSION_H
#define XSC_SHADER_VERSION_H


#include <string>


namespace Xsc
{


// Shader version class.
class ShaderVersion
{

    public:

        ShaderVersion() = default;
        ShaderVersion(const ShaderVersion&) = default;
        ShaderVersion& operator = (const ShaderVersion&) = default;

        ShaderVersion(int major, int minor);

        std::string ToString() const;

        inline int Major() const
        {
            return major_;
        }

        inline int Minor() const
        {
            return minor_;
        }

    private:

        int major_ = 0;
        int minor_ = 0;

};


bool operator == (const ShaderVersion& lhs, const ShaderVersion& rhs);
bool operator != (const ShaderVersion& lhs, const ShaderVersion& rhs);

bool operator < (const ShaderVersion& lhs, const ShaderVersion& rhs);
bool operator <= (const ShaderVersion& lhs, const ShaderVersion& rhs);

bool operator > (const ShaderVersion& lhs, const ShaderVersion& rhs);
bool operator >= (const ShaderVersion& lhs, const ShaderVersion& rhs);


} // /namespace Xsc


#endif



// ================================================================================