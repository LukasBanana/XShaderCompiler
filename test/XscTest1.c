/*
 * XscTest1.c
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <XscC/XscC.h>
#include <stdio.h>


void PrintGLSLExtensions()
{
    char extension[256];
    int version;

    // Get first extension
    void* iterator = XscGetGLSLExtensionEnumeration(NULL, extension, 256, &version);

    while (iterator != NULL)
    {
        // Print extension name and version
        printf("%s ( %d )\n", extension, version);
    
        // Get next extension
        iterator = XscGetGLSLExtensionEnumeration(iterator, extension, 256, &version);
    }
}


int main()
{
    printf("XscTest1\n");

    PrintGLSLExtensions();


    return 0;
}



// ================================================================================