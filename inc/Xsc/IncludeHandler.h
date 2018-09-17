/*
 * IncludeHandler.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_INCLUDE_HANDLER_H
#define XSC_INCLUDE_HANDLER_H


#include "Export.h"
#include <string>
#include <istream>
#include <memory>
#include <vector>


namespace Xsc
{


/* ===== Public classes ===== */

/**
\brief Interface for handling new include streams.
\remarks The default implementation will read the files from an std::ifstream.
*/
class XSC_EXPORT IncludeHandler
{

    public:

        IncludeHandler();
        virtual ~IncludeHandler();

        /**
        \brief Returns an input stream for the specified filename.
        \param[in] includeName Specifies the include filename.
        \param[in] useSearchPathsFirst Specifies whether to first use the search paths to find the file.
        \return Unique pointer to the new input stream.
        */
        virtual std::unique_ptr<std::istream> Include(const std::string& filename, bool useSearchPathsFirst);

        //! Returns the list of search paths.
        std::vector<std::string>& GetSearchPaths();

        //! Returns the constant list of search paths.
        const std::vector<std::string>& GetSearchPaths() const;

    private:

        // PImple idiom
        struct OpaqueData;
        OpaqueData* data_ = nullptr;

};


} // /namespace Xsc


#endif



// ================================================================================