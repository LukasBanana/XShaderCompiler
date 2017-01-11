/*
 * MemoryPool.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifdef XSC_ENABLE_MEMORY_POOL

#include "MemoryPool.h"


namespace Xsc
{


/*
 * MemoryPool
 */

MemoryPool& MemoryPool::Instance()
{
    static MemoryPool instance;
    return instance;
}

void* MemoryPool::Alloc(std::size_t count)
{
    if (count > pageSize_)
        throw std::bad_alloc();

    if (pages_.empty())
        NewPage();

    /* Allocate memory from active page */
    auto ptr = ActivePage().Alloc(count);

    if (!ptr)
    {
        /* Make new page if previous one is full */
        NewPage();
        ptr = ActivePage().Alloc(count);
    }

    if (!ptr)
        throw std::bad_alloc();

    return ptr;
}

void MemoryPool::Free(void* ptr)
{
    // currently do nothing
}

//private
void MemoryPool::NewPage()
{
    pages_.emplace_back(MemoryPage(pageSize_));
}

//Private
MemoryPool::MemoryPage& MemoryPool::ActivePage()
{
    return pages_.back();
}


/*
 * MemoryPage
 */

MemoryPool::MemoryPage::MemoryPage(std::size_t size) :
    size_{ size }
{
    buffer_ = std::unique_ptr<char[]>(new char[size]);
}

MemoryPool::MemoryPage::MemoryPage(MemoryPage&& rhs) :
    size_   { rhs.size_              },
    ptr_    { rhs.ptr_               },
    buffer_ { std::move(rhs.buffer_) }
{
}

void* MemoryPool::MemoryPage::Alloc(std::size_t count)
{
    if (ptr_ + count < size_)
    {
        auto p = buffer_.get() + ptr_;
        ptr_ += count;
        return p;
    }
    else
        return nullptr;
}


} // /namespace Xsc

#endif



// ================================================================================
