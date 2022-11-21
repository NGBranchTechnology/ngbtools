#pragma once

#include "string.h"
#include "wstring.h"
#include <ngbtools/directory.h>
#include "Windows.h"

namespace ngbtools
{
	namespace directory
	{
        inline bool exists(std::string_view directory)
        {
            const auto dwAttrib = GetFileAttributesW(
                string::encode_as_utf16(directory).c_str()
            );

            return  (dwAttrib != INVALID_FILE_ATTRIBUTES) and
                (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
        }

	}
}