//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public
//
//  PlatformPC.cpp
//  Snake3D_public
//
//  Created by Or Avnat on 18/07/2025.
//

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <string>
#include <stdarg.h>
#include <memory>

using namespace std;

const char* g_path;

void SnakeLogError(const char* text)
{
    cerr << text << endl;
}

void SnakeLogErrorFmt(const char* format, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof buf, format, args);
    va_end(args);
    cerr << buf << endl;
}

std::shared_ptr<std::istream> SnakeOpenFile(const char* fname)
{
    string sPath = g_path;
    sPath += fname;
    return std::make_shared<std::ifstream>(sPath.c_str(), std::ios_base::in | std::ios_base::binary);
}
