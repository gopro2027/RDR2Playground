/*
	THIS FILE IS A PART OF RDR 2 SCRIPT HOOK SDK
				http://dev-c.com
			(C) Alexander Blade 2019
*/

#pragma once

#include "scripthook\inc\natives.h"
#include "scripthook\inc\types.h"
#include "scripthook\inc\enums.h"

#include "scripthook\inc\main.h" 

void ScriptMain();
extern uintptr_t(*GetNativeAddress)(uint64_t hash);
float *getSnowLevel();
void setSnowLevelAdvanced(float level, bool a);
void getAxisValues(float* rotation, float* magnitude);