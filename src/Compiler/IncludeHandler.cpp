/*
 * IncludeHandler.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/IncludeHandler.h>
#include <fstream>
#include "Exception.h"


namespace Xsc
{


IncludeHandler::~IncludeHandler()
{
}

static std::unique_ptr<std::istream> ReadFile(const std::string& filename)
{
    auto stream = std::unique_ptr<std::istream>(new std::ifstream(filename));
    return (stream->good() ? std::move(stream) : nullptr);
}

std::unique_ptr<std::istream> IncludeHandler::Include(const std::string& filename, bool useSearchPathsFirst)
{
    if (!useSearchPathsFirst)
    {
        /* Read file from relative path */
        auto file = ReadFile(filename);
        if (file)
            return file;
    }
    
    /* Search file in search paths */
    for (const auto& path : searchPaths)
    {
        if (!path.empty())
        {
            /* Get complete filename */
            std::string s = path;
            if (path.back() != '/' && path.back() != '\\')
                s += '/';
            s += filename;

            /* Read file from current path */
            auto file = ReadFile(s);
            if (file)
                return file;
        }
    }

    if (useSearchPathsFirst)
    {
        /* Read file from relative path */
        auto file = ReadFile(filename);
        if (file)
            return file;
    }

    RuntimeErr("failed to include file: \"" + filename + "\"");
}


} // /namespace Xsc



// ================================================================================