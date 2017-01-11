/*
 * ReportView.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_REPORT_VIEW_H
#define XSC_REPORT_VIEW_H


#include <Xsc/Log.h>
#include <wx/listbox.h>


namespace Xsc
{


class ReportView : public wxListBox
{

    public:

        ReportView(wxWindow* parent, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);

        void AddReport(const Report& r);

    private:

        void Add(const std::string& s);

};


} // /namespace Xsc


#endif



// ================================================================================