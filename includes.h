#pragma once

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define _HAS_EXCEPTIONS 0
#define _ITERATOR_DEBUG_LEVEL 0
#define _CRT_SECURE_INVALID_PARAMETER
#define NOMINMAX

#define NOINLINE __declspec( noinline )
#define EXPORT   __declspec( dllexport )

using ulong_t = unsigned long;

// windows
#include <Windows.h>
#include <windows.h>
#include <ctime>
#include <iostream>
#include <cstring>
#include <string>
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include <vector>
#include <array>

// other
#include "SerialPort.h"
#include "defs.h"
#include "offsets.h"