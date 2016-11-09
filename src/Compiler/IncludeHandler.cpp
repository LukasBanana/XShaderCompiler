/*
 * IncludeHandler.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/IncludeHandler.h>
#include <fstream>


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

std::unique_ptr<std::istream> IncludeHandler::Include(const std::string& filename, bool useSearchPaths)
{
    /* Read file from relative path */
    if (!useSearchPaths)
    {
        auto file = ReadFile(filename);
        if (file)
            return file;
    }
    
    //if (useSearchPaths)
    {
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
    }

    if (useSearchPaths)
    {
        /* Read file from relative path */
        auto file = ReadFile(filename);
        if (file)
            return file;
    }

    throw std::runtime_error("failed to include file: \"" + filename + "\"");
}


} // /namespace Xsc



// ================================================================================