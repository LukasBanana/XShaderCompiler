/*
 * Main.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/Xsc.h>
#include "Shell.h"
#include "CommandLine.h"
#include <iostream>
#include <fstream>

using namespace Xsc;
using namespace Xsc::Util;


int main(int argc, char** argv)
{
    Shell shell(std::cout);

    /* Execute command line from optional init file */
    {
        std::ifstream file("xsc.ini");
        if (file.good())
        {
            while (!file.eof())
            {
                /* Parse and execute command line from init file */
                std::string line;
                std::getline(file, line);

                CommandLine cmdLine(line);
                shell.ExecuteCommandLine(cmdLine);
            }
        }
    }

    /* Execute command line from program arguments */
    CommandLine cmdLine(argc - 1, argv + 1);
    shell.ExecuteCommandLine(cmdLine);

    return 0;
}

