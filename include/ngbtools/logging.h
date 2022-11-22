#pragma once

#include "Windows.h"

#include <string>
#include <vector>

#include <ngbtools/string.h>
#include <ngbtools/windows_errors.h>

#ifdef _CONSOLE
#include <ngbtools/console.h>
#endif

namespace ngbtools
{
    namespace logging
    {
        /// <summary>
        /// If you are running a Console app, this will write a windows error to stdout with
        /// the error text highlighted in red.
        /// </summary>
        /// <param name="hResult">Windows error code</param>
        /// <param name="context">Caller context, typically from the FUNCTION_CONTEXT macro</param>
        /// <param name="comment">Comment for human-readable context, e.g. "I was unable to read file xyz"</param>
        inline void report_windows_error(HRESULT hResult, LPCSTR context, std::string_view comment)
        {
#ifdef _CONSOLE
            console::formatline(CONSOLE_FOREGROUND_RED, "ERROR {} at {}: {}\r\n{}" CONSOLE_STANDARD,
                hResult,
                context,
                comment,
                windows::hresult_as_string(hResult));
#endif
        }
    }
}
