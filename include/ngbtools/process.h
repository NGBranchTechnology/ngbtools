#pragma once

#include <string>
#include <ngbtools/string.h>
#include <ngbtools/console.h>

namespace ngbtools
{
    namespace process
    {
        /// <summary>
        /// Run a process asynchronously with admin privileges
        /// </summary>
        /// <param name="executable"></param>
        /// <param name="params"></param>
        /// <returns></returns>
        inline bool run_as_admin(std::string_view executable, std::string_view params)
        {
            const auto wide_executable{ string::encode_as_utf16(executable) };
            const auto wide_params{ string::encode_as_utf16(params) };
            const auto cwd{ fs::current_path().wstring() };
            int result = (int)(UINT_PTR) ::ShellExecuteW(HWND_DESKTOP,
                L"runas",
                wide_executable.c_str(),
                wide_params.c_str(),
                cwd.c_str(),
                SW_SHOW);
            if (result > 0 && result < 32)
            {
                logging::report_windows_error(GetLastError(), FUNCTION_CONTEXT,
                    fmt::format("Unable to start {}", executable));
                return false;
            }
            return true;
        }
    };
}
