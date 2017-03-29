/*
 * Main.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
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

    bool actionPerformed = false;

    /* Get filename of init file */
    std::string filename(argv[0]);
    std::size_t pos = 0;

    if ( ( pos = filename.rfind("xsc") ) != std::string::npos )
    {
        filename.resize(pos + 3);
        filename += ".ini";

        /* Execute command line from optional init file */
        std::ifstream file;
        
        file.open(filename);
        if (!file.good())
            file.open("xsc.ini");

        if (file.good())
        {
            while (!file.eof())
            {
                /* Parse and execute command line from init file */
                std::string line;
                std::getline(file, line);

                CommandLine cmdLine(line);
                if (shell.ExecuteCommandLine(cmdLine, false))
                    actionPerformed = true;
            }
        }
    }

    /* Execute command line from program arguments */
    CommandLine cmdLine(argc - 1, argv + 1);
    if (shell.ExecuteCommandLine(cmdLine, !actionPerformed))
        actionPerformed = true;

    /* Print hint if no action has been performed */
    if (!actionPerformed)
        std::cout << "no action performed" << std::endl;

    /* Wait for user (if enabled) */
    shell.WaitForUser();

    return 0;
}

