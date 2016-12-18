/*
 * SourceView.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SourceView.h"
#include "SourceViewLanguageHLSL.h"
#include "SourceViewLanguageGLSL.h"


namespace Xsc
{


SourceView::SourceView(wxWindow* parent, const wxPoint& pos, const wxSize& size) :
    wxStyledTextCtrl{ parent, wxID_ANY, pos, size }
{
    /* Initialize source view style */
    wxFont font(wxFontInfo(10).Family(wxFONTFAMILY_MODERN));
    StyleSetFont(wxSTC_STYLE_DEFAULT, font);
    StyleSetForeground(wxSTC_STYLE_DEFAULT, *wxBLACK);
    StyleSetBackground(wxSTC_STYLE_DEFAULT, *wxWHITE);
    StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour("DARK GREY"));
    StyleSetBackground(wxSTC_STYLE_LINENUMBER, *wxWHITE);
    StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColour("DARK GREY"));

    SetTabWidth(4);
    SetUseTabs(false);
    SetTabIndents(true);
    SetBackSpaceUnIndents(true);
    SetIndentationGuides(4);
    SetWrapMode(wxSTC_WRAP_NONE);
    SetIndent(4);

    /* Initialize line number */
    int lineNoID = 0;
    SetMarginType(lineNoID, wxSTC_MARGIN_NUMBER);
    SetMarginWidth(lineNoID, 50);
    StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour("DARK GREY"));
    StyleSetBackground(wxSTC_STYLE_LINENUMBER, *wxWHITE);

    /* Initialize folding */
    int foldingID = 1;
    SetMarginType(foldingID, wxSTC_MARGIN_SYMBOL);
    SetMarginMask(foldingID, wxSTC_MASK_FOLDERS);
    StyleSetBackground(foldingID, *wxWHITE);
    SetMarginWidth(foldingID, 0);
    SetMarginSensitive(foldingID, true);

    SetFoldFlags(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);

    /* Initialize properties */
    SetProperty("fold", "1");
    SetProperty("fold.comment", "1");
    SetProperty("fold.compact", "1");
    SetProperty("fold.preprocessor", "1");

    /* Initializer marker */
    wxColor Grey(100, 100, 100);
    
    MarkerDefine        (wxSTC_MARKNUM_FOLDER,          wxSTC_MARK_ARROW    );
    MarkerSetForeground (wxSTC_MARKNUM_FOLDER,          Grey                );
    MarkerSetBackground (wxSTC_MARKNUM_FOLDER,          Grey                );
    
    MarkerDefine        (wxSTC_MARKNUM_FOLDEROPEN,      wxSTC_MARK_ARROWDOWN);
    MarkerSetForeground (wxSTC_MARKNUM_FOLDEROPEN,      Grey                );
    MarkerSetBackground (wxSTC_MARKNUM_FOLDEROPEN,      Grey                );
    
    MarkerDefine        (wxSTC_MARKNUM_FOLDERSUB,       wxSTC_MARK_EMPTY    );
    MarkerSetForeground (wxSTC_MARKNUM_FOLDERSUB,       Grey                );
    MarkerSetBackground (wxSTC_MARKNUM_FOLDERSUB,       Grey                );
    
    MarkerDefine        (wxSTC_MARKNUM_FOLDEREND,       wxSTC_MARK_ARROW    );
    MarkerSetForeground (wxSTC_MARKNUM_FOLDEREND,       Grey                );
    MarkerSetBackground (wxSTC_MARKNUM_FOLDEREND,       "WHITE"             );
    
    MarkerDefine        (wxSTC_MARKNUM_FOLDEROPENMID,   wxSTC_MARK_ARROWDOWN);
    MarkerSetForeground (wxSTC_MARKNUM_FOLDEROPENMID,   Grey                );
    MarkerSetBackground (wxSTC_MARKNUM_FOLDEROPENMID,   "WHITE"             );
    
    MarkerDefine        (wxSTC_MARKNUM_FOLDERMIDTAIL,   wxSTC_MARK_EMPTY    );
    MarkerSetForeground (wxSTC_MARKNUM_FOLDERMIDTAIL,   Grey                );
    MarkerSetBackground (wxSTC_MARKNUM_FOLDERMIDTAIL,   Grey                );
    
    MarkerDefine        (wxSTC_MARKNUM_FOLDERTAIL,      wxSTC_MARK_EMPTY    );
    MarkerSetForeground (wxSTC_MARKNUM_FOLDERTAIL,      Grey                );
    MarkerSetBackground (wxSTC_MARKNUM_FOLDERTAIL,      Grey                );

    /* Initialize style */
    StyleClearAll();
    SetLexer(wxSTC_LEX_CPP);

    StyleSetForeground(wxSTC_C_STRING,                  wxColour(180,   0,   0));
    StyleSetForeground(wxSTC_C_PREPROCESSOR,            wxColour( 30, 160,  30));
    StyleSetForeground(wxSTC_C_IDENTIFIER,              wxColour( 40,   0,  60));
    StyleSetForeground(wxSTC_C_NUMBER,                  wxColour(  0, 150,   0));
    StyleSetForeground(wxSTC_C_CHARACTER,               wxColour(150,   0,   0));
    StyleSetForeground(wxSTC_C_WORD,                    wxColour(  0,   0, 150));
    StyleSetForeground(wxSTC_C_WORD2,                   wxColour(  0, 150,   0));
    StyleSetForeground(wxSTC_C_COMMENT,                 wxColour(150, 150, 150));
    StyleSetForeground(wxSTC_C_COMMENTLINE,             wxColour(150, 150, 150));
    StyleSetForeground(wxSTC_C_COMMENTDOC,              wxColour(150, 150, 150));
    StyleSetForeground(wxSTC_C_COMMENTDOCKEYWORD,       wxColour(  0,   0, 200));
    StyleSetForeground(wxSTC_C_COMMENTDOCKEYWORDERROR,  wxColour(  0,   0, 200));
    
    StyleSetBold(wxSTC_C_WORD,              true);
    StyleSetBold(wxSTC_C_WORD2,             true);
    StyleSetBold(wxSTC_C_COMMENTDOCKEYWORD, true);
    
    StyleSetItalic(wxSTC_C_COMMENT,                 true);
    StyleSetItalic(wxSTC_C_COMMENTLINE,             true);
    StyleSetItalic(wxSTC_C_COMMENTDOC,              true);
    StyleSetItalic(wxSTC_C_COMMENTDOCKEYWORD,       true);
    StyleSetItalic(wxSTC_C_COMMENTDOCKEYWORDERROR,  true);
}

void SourceView::SetLanguage(const SourceViewLanguage language)
{
    switch (language)
    {
        case SourceViewLanguage::HLSL:
            SetKeyWords(0, keywordsHLSL0);
            SetKeyWords(1, keywordsHLSL1);
            break;
        case SourceViewLanguage::GLSL:
            SetKeyWords(0, keywordsGLSL0);
            SetKeyWords(1, keywordsGLSL1);
            break;
        default:
            SetKeyWords(0, "");
            SetKeyWords(1, "");
            break;
    }

    SetText(GetText());
    Refresh();
}


/*
 * ======= Private: =======
 */




} // /namespace Xsc



// ================================================================================
