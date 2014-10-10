/*
 * main.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <HT/Translator.h>


class OutputLog : public HTLib::Logger
{
    
    public:
        
        void Info(const std::string& message) override
        {
            infos_.push_back(message);
        }

        void Warning(const std::string& message) override
        {
            warnings_.push_back(message);
        }

        void Error(const std::string& message) override
        {
            errors_.push_back(message);
        }

        void Report()
        {
            for (const auto& msg : infos_)
                std::cout << msg << std::endl;
            infos_.clear();
            
            if (!warnings_.empty())
            {
                PrintHead(std::to_string(warnings_.size()) + " WARNING(S)");

                for (const auto& msg : warnings_)
                    std::cout << "warning: " << msg << std::endl;
                warnings_.clear();
            }

            if (!errors_.empty())
            {
                PrintHead(std::to_string(errors_.size()) + " ERROR(S)");

                for (const auto& msg : errors_)
                    std::cerr << "errors: " << msg << std::endl;
                errors_.clear();
            }
        }

    private:

        void PrintHead(const std::string& head)
        {
            std::cout << std::endl;
            std::cout << head << std::endl;
            std::cout << std::string(head.size(), '-') << std::endl;
        }
        
        std::vector<std::string> infos_;
        std::vector<std::string> warnings_;
        std::vector<std::string> errors_;

};


int main()
{
    // Read file
    std::ifstream input("TestShader1.hlsl");
    std::ofstream output("TestShader1.glsl");

    // Translate HLSL code
    OutputLog log;
    HTLib::TranslateHLSLtoGLSL(
        input,                                  // input stream
        output,                                 // output stream
        "VS",                                   // entry point
        HTLib::ShaderTargets::GLSLVertexShader, // shader target
        HTLib::ShaderVersions::GLSL120,         // shader version
        nullptr,                                // include stream handler
        {},                                     // additional options
        &log                                    // output log
    );

    log.Report();

    std::cout << std::endl;
    system("pause");

    return 0;
}
