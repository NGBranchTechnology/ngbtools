#pragma once

#include <string>
#include "Windows.h"

#include <ngbtools/string.h>
#include <ngbtools/logging.h>
#include <ngbtools/windows_errors.h>

namespace ngbtools
{
    /// <summary>
    /// Groups common file-related operations
    /// </summary>
    namespace file
    {
        /// <summary>
        /// Given a filename, return the extension (e.g. '.exe' for 'notepad.exe')
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        inline std::string_view get_extension(const std::string_view name)
        {
            const auto pos = name.rfind('.');
            if (pos != std::string_view::npos)
            {
                return name.substr(pos);
            }
            return "";
        }

        /// <summary>
        /// Checks if file exists
        /// </summary>
        /// <param name="path">filename to check</param>
        /// <returns>true if file exists, or false otherwise</returns>
        inline bool exists(std::string_view path)
        {
            const auto wpath{ string::encode_as_utf16(path) };
            const auto dwAttributes{
                ::GetFileAttributes(wpath.data())
            };
            if (dwAttributes != INVALID_FILE_ATTRIBUTES)
                return true; // file exists, attributes can be read

            const auto hResult{ (HRESULT)GetLastError() };
            if (hResult == ERROR_FILE_NOT_FOUND)
                return false;

            if (hResult == ERROR_PATH_NOT_FOUND)
                return false;

            logging::report_windows_error(hResult, FUNCTION_CONTEXT,
                fmt::format("GetFileAttributes({}) failed", path));
            return false;
        }

        /// <summary>
        /// Delete this file
        /// </summary>
        /// <param name="pathname"></param>
        /// <returns></returns>
        inline bool remove(std::string_view pathname)
        {
            const auto wpathname{ string::encode_as_utf16(pathname) };
            if (!::DeleteFileW(wpathname.data()))
            {
                logging::report_windows_error(GetLastError(), FUNCTION_CONTEXT,
                    fmt::format("DeleteFileW({}) failed", pathname));
                return false;
            }
            return true;
        }

    }
}