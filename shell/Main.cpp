/*
 * Main.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <HT/Translator.h>
#include "Shell.h"
#include <iostream>

using namespace HTLib;
using namespace HTLib::Util;


int main(int argc, char** argv)
{
    Shell shell(std::cout);
    CommandLine cmdLine(argc - 1, argv + 1);
    shell.ExecuteCommandLine(cmdLine);
    return 0;
}

