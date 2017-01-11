/*
 * MemoryPool.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_MEMORY_POOL_H
#define XSC_MEMORY_POOL_H

#ifdef XSC_ENABLE_MEMORY_POOL


#include <list>
#include <memory>


namespace Xsc
{


class MemoryPool
{

    public:

        static MemoryPool& Instance();

        void* Alloc(std::size_t count);
        void Free(void* ptr);

    private:

        MemoryPool() = default;

        class MemoryPage
        {

            public:

                MemoryPage(std::size_t size);
                MemoryPage(MemoryPage&& rhs);

                void* Alloc(std::size_t count);

            private:

                std::size_t             size_   = 0;
                std::size_t             ptr_    = 0;
                std::unique_ptr<char[]> buffer_;

        };

        void NewPage();
        MemoryPage& ActivePage();

        std::size_t             pageSize_   = 16384;
        std::list<MemoryPage>   pages_;
    
};


} // /namespace Xsc


#endif

#endif



// ================================================================================