/*
 * SourceView.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SourceView.h"
#include "SourceViewLanguageHLSL.h"
#include "SourceViewLanguageGLSL.h"


wxBEGIN_EVENT_TABLE(Xsc::SourceView, wxStyledTextCtrl)
    EVT_STC_CHARADDED(wxID_ANY, Xsc::SourceView::OnCharAdded)
    EVT_KEY_DOWN(Xsc::SourceView::OnKeyDown)
wxEND_EVENT_TABLE()


namespace Xsc
{


static const int g_AnnotationStyle = (wxSTC_STYLE_LASTPREDEFINED + 1);

SourceView::SourceView(wxWindow* parent, const wxPoint& pos, const wxSize& size) :
    wxStyledTextCtrl { parent, wxID_ANY, pos, size }
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
    AnnotationSetVisible(wxSTC_ANNOTATION_BOXED);

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

    /* Annotations style */
    StyleSetBackground(g_AnnotationStyle, wxColour(244, 220, 220));
    StyleSetForeground(g_AnnotationStyle, *wxBLACK);
    StyleSetSizeFractional(g_AnnotationStyle, (StyleGetSizeFractional(wxSTC_STYLE_DEFAULT)*4)/5);
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

void SourceView::SetTextAndRefresh(const wxString& text)
{
    int hScroll = GetScrollPos(wxHORIZONTAL);
    int vScroll = GetScrollPos(wxVERTICAL);

    SetText(text);

    SetScrollPos(wxHORIZONTAL, hScroll);
    SetScrollPos(wxVERTICAL, vScroll);

    Refresh();
}

void SourceView::SetCharEnterCallback(const CharEnterCallback& callback)
{
    charEnterCallback_ = callback;
}

void SourceView::SetMoveCursorCallback(const MoveCursorCallback& callback)
{
    moveCursorCallback_ = callback;
}

void SourceView::AddAnnotation(int line, const wxString& text)
{
    AnnotationSetText(line, text);
    AnnotationSetStyle(line, g_AnnotationStyle);
}

void SourceView::ClearAnnotations()
{
    AnnotationClearAll();
}


/*
 * ======= Private: =======
 */

void SourceView::OnCharAdded(wxStyledTextEvent& event)
{
    auto chr = static_cast<char>(event.GetKey());
    auto currentLine = GetCurrentLine();

    if (chr == '\n')
    {
        int lineIndent = 0;

        if (currentLine > 0)
            lineIndent = GetLineIndentation(currentLine - 1);
        if (lineIndent == 0)
            return;

        SetLineIndentation(currentLine, lineIndent);
        GotoPos(PositionFromLine(currentLine) + lineIndent);
    }

    if (charEnterCallback_)
        charEnterCallback_(chr);
}

void SourceView::OnKeyDown(wxKeyEvent& event)
{
    wxStyledTextCtrl::OnKeyDown(event);

    auto key = event.GetKeyCode();

    if (key == WXK_BACK || key == WXK_DELETE)
    {
        if (charEnterCallback_)
            charEnterCallback_('\b');
    }

    if (key == WXK_UP || key == WXK_DOWN || key == WXK_LEFT || key == WXK_RIGHT || key == WXK_HOME || key == WXK_END)
    {
        if (moveCursorCallback_)
            moveCursorCallback_(GetCurrentLine() + 1, GetColumn(GetCurrentPos()) + 1);
    }

    if (key == WXK_F5)
    {
        if (charEnterCallback_)
            charEnterCallback_(0);
    }
}


} // /namespace Xsc



// ================================================================================
