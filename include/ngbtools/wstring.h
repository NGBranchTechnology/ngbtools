#pragma once

#include <string>
#include <vector>
#include "Windows.h"

namespace ngbtools
{
	namespace wstring
	{
		inline bool equals_nocase(const std::wstring_view& astr, const std::wstring_view& bstr)
		{
			const auto a = astr.data();
			const auto b = bstr.data();
			if (!a)
				return !b;

			if (!b)
				return false;

			return _wcsicmp(a, b) == 0;
		}

		inline bool equals(const std::wstring_view& astr, const std::wstring_view& bstr)
		{
			const auto a = astr.data();
			const auto b = bstr.data();
			if (!a)
				return !b;

			if (!b)
				return false;

			return wcscmp(a, b) == 0;
		}

		inline std::string encode_as_utf8(std::wstring_view wstr)
		{
			const wchar_t* unicode_string = wstr.data();
			size_t wide_len = wstr.size();

			// handle nullptr gracefully
			if (!unicode_string || !wide_len)
				return {};

			// we try the stack first (because that is of an order of magnitudes faster)
			// only if this fails we revert back to allocating the data on the stack
			const size_t size_of_stack_buffer = 1024;
			if (wide_len < size_of_stack_buffer)
			{
				char buffer_on_the_stack[size_of_stack_buffer];
				int rc = WideCharToMultiByte(CP_UTF8, 0, unicode_string, (unsigned long)wide_len,
					buffer_on_the_stack, size_of_stack_buffer - 1, nullptr, nullptr);
				if (rc > 0)
				{
					buffer_on_the_stack[rc] = 0;
					return std::string(buffer_on_the_stack);
				}
				rc = GetLastError();
			}
			size_t size_to_allocate = 2 * wide_len;
			size_t max_size_to_allocate = 64 * size_to_allocate;
			while (max_size_to_allocate)
			{
				std::vector<char> buffer_on_the_heap;
				buffer_on_the_heap.resize(size_to_allocate + 1);

				int rc = WideCharToMultiByte(CP_UTF8, 0, unicode_string, (unsigned long)wide_len,
					&buffer_on_the_heap[0], (unsigned long)size_to_allocate, nullptr, nullptr);

				if (rc > 0)
				{
					buffer_on_the_heap[rc] = 0;
					return std::string(&buffer_on_the_heap[0]);
				}
				size_to_allocate *= 2;
			}
			return {};
		}
	}
}