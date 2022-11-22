#pragma once

#include "Windows.h"

#include <string>
#include <vector>

#include <ngbtools/wstring.h>

namespace ngbtools
{
    namespace windows
    {
        /// <summary>
        /// Return a string for a Windows error code, in en-US (system fallback) language.
        /// </summary>
        /// <param name="hResult"></param>
        /// <returns></returns>
        inline std::string hresult_as_string(HRESULT hResult)
        {
            std::vector<wchar_t> buffer;
            buffer.resize(1024);

            const DWORD dwLanguageID{ 0x409 }; // US English, see https://learn.microsoft.com/en-us/windows/win32/msi/localizing-the-error-and-actiontext-tables

            if (!::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, hResult, dwLanguageID, &buffer[0], (DWORD)buffer.size(), nullptr))
            {
                if (!::FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
                    GetModuleHandle(TEXT("NTDLL.DLL")),
                    hResult, dwLanguageID, &buffer[0], (DWORD)buffer.size(), nullptr))
                {
                    return fmt::format("{0:#x} ({0})", hResult);
                }

            }
            return wstring::encode_as_utf8(&buffer[0]);
        }
    }
}
