#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "targetver.h"

#include <windows.h>      /* for the control of processes */
#include <iostream>
#include <fstream>
#include <set>
#include <map>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstdlib>
#include <cstring>
#include <time.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <ios>
#include <filesystem>
#include <cassert>

// see http://stackoverflow.com/questions/5641427/how-to-make-preprocessor-generate-a-string-for-line-keyword
#define S(x) #x
#define S_(x) S(x)
#define S__LINE__ S_(__LINE__)

// see http://msdn.microsoft.com/de-de/library/b0084kay.aspx
#define FUNCTION_CONTEXT __FUNCTION__ "[" S__LINE__ "]"

template <typename T> constexpr size_t array_size(T& array)
{
	return sizeof(array) / sizeof(array[0]);
}

#define FMT_HEADER_ONLY
#include <fmt/format.h>
