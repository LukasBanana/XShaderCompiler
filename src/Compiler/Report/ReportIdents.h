/*
 * ReportIdents.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_REPORT_IDENTS_H
#define XSC_REPORT_IDENTS_H


#include "JoinString.h"


namespace Xsc
{


/* ----- Localized global report strings ------ */

#define DECL_REPORT(NAME, VALUE) \
    static const Xsc::JoinableString R_##NAME { VALUE }

#include "ReportIdentsEN.h"

#undef DECL_REPORT


} // /namespace Xsc


#endif



// ================================================================================