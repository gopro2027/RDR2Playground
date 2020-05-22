// pch.h: is a precompiled header file
// Files below is compiling only once, so the compilation time is significantly reduces.
// Don't include here files, which you change oftenly.

#ifndef PCH_H
#define PCH_H

// Add here header files for precompilation
#include "scripthook\inc\main.h"
#include "keyboard.h"
#include "script.h"
#include <string>
#include <iostream>
#include <cstdint>
#include <vector>
#include <functional>
#include <deque>
#include "third-party/CMemory.h"

#define DLL_EXPORT __declspec(dllexport)


//various functions
void consolePrint(char *text);
void print(char *text, int time = 50);

#endif //PCH_H
