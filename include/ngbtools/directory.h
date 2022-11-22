#pragma once

#include "Windows.h"

#include <string>

#include <ngbtools/string.h>
#include <ngbtools/wstring.h>
#include <ngbtools/directory.h>
#include <ngbtools/logging.h>

namespace ngbtools
{
    class directory final
    {
    public:
        static bool exists(std::string_view directory)
        {
            const auto dwAttrib = GetFileAttributesW(
                string::encode_as_utf16(directory).c_str()
            );

            return  (dwAttrib != INVALID_FILE_ATTRIBUTES) and
                (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
        }

        static std::string system()
        {
            wchar_t buffer[MAX_PATH];

            if (::GetSystemDirectoryW(buffer, (UINT)std::size(buffer)))
            {
                return wstring::encode_as_utf8(buffer);
            }
            logging::report_windows_error(GetLastError(), FUNCTION_CONTEXT, "GetSystemDirectoryW() failed");
            return ".";
        }

        static std::string current()
        {
            wchar_t buffer[MAX_PATH];

            if (::GetCurrentDirectoryW((UINT)std::size(buffer), buffer))
            {
                return wstring::encode_as_utf8(buffer);
            }
            logging::report_windows_error(GetLastError(), FUNCTION_CONTEXT, "GetCurrentDirectoryW() failed");
            return ".";
        }

        static std::string windows()
        {
            wchar_t buffer[MAX_PATH];

            if (::GetWindowsDirectoryW(buffer, (UINT)std::size(buffer)))
            {
                return wstring::encode_as_utf8(buffer);
            }
            logging::report_windows_error(GetLastError(), FUNCTION_CONTEXT, "GetWindowsDirectoryW() failed");
            return ".";
        }

        static std::string application()
        {
            wchar_t buffer[MAX_PATH];
            if (!GetModuleFileNameW(0, buffer, (DWORD)std::size(buffer)))
            {
                logging::report_windows_error(GetLastError(), FUNCTION_CONTEXT, "GetModuleFileNameW() failed");
                return ".";
            }
            const auto last_sep = wcsrchr(buffer, L'\\');
            if (last_sep)
            {
                *last_sep = 0;
            }
            else
            {
                buffer[0] = 0;
            }
            return wstring::encode_as_utf8(buffer);
        }

    };
}