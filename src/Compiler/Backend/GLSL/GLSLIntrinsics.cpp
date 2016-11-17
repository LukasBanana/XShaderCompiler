/*
 * GLSLIntrinsics.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLIntrinsics.h"
#include <map>


namespace Xsc
{


static std::map<Intrinsic, std::string> GenerateIntrinsicMap()
{
    using I = Intrinsic;

    return
    {
        { I::Frac,                              "fract"                 },
        { I::RSqrt,                             "inversesqrt"           },
        { I::Lerp,                              "mix"                   },
        { I::Saturate,                          "clamp"                 },
        { I::DDX,                               "dFdx"                  },
        { I::DDXCoarse,                         "dFdxCoarse"            },
        { I::DDXFine,                           "dFdxFine"              },
        { I::DDY,                               "dFdy"                  },
        { I::DDYCoarse,                         "dFdyCoarse"            },
        { I::DDYFine,                           "dFdyFine"              },
        { I::ATan2,                             "atan"                  },
        { I::GroupMemoryBarrier,                "groupMemoryBarrier"    },
        { I::GroupMemoryBarrierWithGroupSync,   "barrier"               },
        { I::AllMemoryBarrier,                  "memoryBarrier"         },
        { I::AllMemoryBarrierWithGroupSync,     "barrier"               },
        { I::InterlockedAdd,                    "atomicAdd"             },
        { I::InterlockedAnd,                    "atomicAnd"             },
        { I::InterlockedOr,                     "atomicOr"              },
        { I::InterlockedXor,                    "atomicXor"             },
        { I::InterlockedMin,                    "atomicMin"             },
        { I::InterlockedMax,                    "atomicMax"             },
        { I::InterlockedCompareExchange,        "atomicCompSwap"        },
        { I::InterlockedExchange,               "atomicExchange"        },

        //TODO: continue this list ...
    };
}

const std::string* IntrinsicToGLSLKeyword(const Intrinsic intr)
{
    static const auto intrinsicMap = GenerateIntrinsicMap();
    auto it = intrinsicMap.find(intr);
    return (it != intrinsicMap.end() ? &(it->second) : nullptr);
}


} // /namespace Xsc



// ================================================================================