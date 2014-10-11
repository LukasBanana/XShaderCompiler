/*
 * CodeWriter.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __HT_CODE_GENERATOR_H__
#define __HT_CODE_GENERATOR_H__


#include <ostream>


namespace HTLib
{


//! Output code writer.
class CodeWriter
{
    
    public:
        
        CodeWriter(const std::string& indentTab);

        //! \throws std::runtime_error If stream is invalid.
        void OutputStream(std::ostream& stream);

        void PushIndent();
        void PopIndent();

        void BeginLine();
        void EndLine();

        void Write(const std::string& text);
        void WriteLine(const std::string& text);

    private:
        
        std::ostream*   stream_ = nullptr;
        std::string     indentTab_;
        std::string     indent_;

};


} // /namespace HTLib


#endif



// ================================================================================