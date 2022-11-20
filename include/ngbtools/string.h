#pragma once

#include <string>

namespace ngbtools
{
	namespace string
	{
        constexpr const char* newline()
        {
            return "\r\n";
        }

        inline bool is_empty(const char* p)
        {
            return !p || !*p;
        }

        inline bool is_empty(const std::string_view& txt)
        {
            return txt.empty();
        }
        inline bool is_empty(const wchar_t* p)
        {
            return !p || !*p;
        }

        inline bool is_empty(const std::wstring_view& txt)
        {
            return txt.empty();
        }

        inline char hex_digit(uint8_t nibble)
        {
            assert(nibble < 16);
            return "0123456789ABCDEF"[nibble];
        }

        inline uint8_t upper_nibble(unsigned char c)
        {
            return (c & 0xF0) >> 4;
        }

        inline uint8_t lower_nibble(unsigned char c)
        {
            return (c & 0x0F);
        }

        inline size_t length(const char* text)
        {
            return text ? strlen(text) : 0;
        }

        inline size_t length(const wchar_t* text)
        {
            return text ? wcslen(text) : 0;
        }

        inline size_t length(const std::string& text)
        {
            return text.length();
        }

        inline size_t length(const std::wstring& text)
        {
            return text.length();
        }

        inline size_t length(const std::string_view& text)
        {
            return text.length();
        }

        inline size_t length(const std::wstring_view& text)
        {
            return text.length();
        }

        inline bool equals(char a, char b)
        {
            return a == b;
        }

        inline bool equals(const wchar_t* a, const wchar_t* b)
        {
            if (!a)
                return !b;

            if (!b)
                return false;

            return wcscmp(a, b) == 0;
        }

        inline bool equals(const std::wstring& a, const std::wstring& b)
        {
            return equals(a.c_str(), b.c_str());
        }

        inline bool equals(const char* a, const char* b)
        {
            if (!a)
                return !b;

            if (!b)
                return false;

            return strcmp(a, b) == 0;
        }

        inline bool equals(const std::string& a, const std::string& b)
        {
            return equals(a.c_str(), b.c_str());
        }

        inline bool equals_nocase(char a, char b)
        {
            return tolower(a) == tolower(b);
        }

        inline bool equals_nocase(const char* a, const char* b)
        {
            if (!a)
                return !b;

            if (!b)
                return false;

            return _strcmpi(a, b) == 0;
        }

        inline bool equals_nocase(const std::string& a, const std::string& b)
        {
            return equals_nocase(a.c_str(), b.c_str());
        }


        inline std::string join(const std::vector<std::string>& items, const std::string& joiner)
        {
            std::string combined;
            bool first = true;
            for (const auto& item : items)
            {
                if (first)
                    first = false;
                else
                    combined += joiner;
                combined += item;
            }
            return combined;
        }

        const std::string uppercase(const std::string_view& text)
        {
            std::string result{ text };
            auto p = const_cast<char*>(result.c_str());
            _strupr(p);
            return result;
        }

        inline std::string lowercase(const std::string_view& text)
        {
            std::string result{ text };
            const auto p = const_cast<char*>(result.c_str());
            _strlwr(p);
            return result;
        }

        inline std::vector<std::string> split(const char* text, const char* separators, bool handle_quotation_marks)
        {
            std::vector<std::string> result;

            bool is_recording_quoted_string = false;
            if (text)
            {
                auto start = text;
                for (;;)
                {
                    char c = *(text++);
                    if (!c)
                    {
                        if (*start)
                        {
                            result.push_back(start);
                        }
                        break;
                    }
                    if (is_recording_quoted_string)
                    {
                        if (c == '"')
                        {
                            assert(text - start >= 1);
                            result.push_back(std::string{ start, (size_t)(text - start - 1) });
                            start = text;
                            is_recording_quoted_string = false;
                        }
                        continue;
                    }
                    else if (handle_quotation_marks and (c == '"'))
                    {
                        assert(text - start >= 1);
                        if (text - start > 1)
                        {
                            result.push_back(std::string{ start, (size_t)(text - start - 1) });
                        }
                        start = text;
                        is_recording_quoted_string = true;
                        continue;
                    }

                    if (strchr(separators, c))
                    {
                        assert(text - start >= 1);
                        result.push_back(std::string{ start, (size_t)(text - start - 1) });
                        start = text;
                    }
                }
            }
            return result;
        }
		inline size_t Length(const char* lpszString)
		{
			if (!lpszString)
				return 0;

			return strlen(lpszString);
		}

		inline std::wstring encode_as_utf16(const char* utf8_encoded_string, size_t utf8_len)
		{
			// handle nullptr gracefully
			if (!utf8_encoded_string or !utf8_len)
				return {};

            // we try the stack first (because that is of an order of magnitudes faster)
            // only if this fails we revert back to allocating the data on the stack
            const size_t size_of_stack_buffer = 1024;
            if (utf8_len < size_of_stack_buffer)
            {
                wchar_t buffer_on_the_stack[size_of_stack_buffer];
                int rc = MultiByteToWideChar(CP_UTF8, 0, utf8_encoded_string, (int)utf8_len,
                    buffer_on_the_stack, size_of_stack_buffer - 1);
                if (rc > 0)
                {
                    assert(rc < sizeof(buffer_on_the_stack));
                    buffer_on_the_stack[rc] = 0;
                    return std::wstring(buffer_on_the_stack);
                }
                rc = GetLastError();

                // we don't expect these
                assert(rc != ERROR_INVALID_FLAGS);
                assert(rc != ERROR_INVALID_PARAMETER);
                assert(rc != ERROR_NO_UNICODE_TRANSLATION);

                // we do expect this
                assert(rc == ERROR_INSUFFICIENT_BUFFER);
            }
            size_t size_to_allocate = utf8_len + 16;
            size_t max_size_to_allocate = 64 * size_to_allocate;
            while (max_size_to_allocate)
            {
                std::vector<wchar_t> buffer_on_the_heap;
                buffer_on_the_heap.resize(size_to_allocate + 1);

                int rc = MultiByteToWideChar(CP_UTF8, 0, utf8_encoded_string, (int)utf8_len,
                    &buffer_on_the_heap[0], (int)size_to_allocate);

                if (rc > 0)
                {
                    assert(size_t(rc) < size_to_allocate);
                    buffer_on_the_heap[rc] = 0;
                    return std::wstring{ &buffer_on_the_heap[0], size_t(rc) };
                }
                size_to_allocate *= 2;
            }
            assert(false);
			return {};
		}

		inline auto encode_as_utf16(const std::string& utf8_encoded_string)
		{
			return encode_as_utf16(utf8_encoded_string.c_str(), utf8_encoded_string.length());
		}

		inline auto encode_as_utf16(const char* utf8_encoded_string)
		{
			// handle nullptr gracefully
			if (!utf8_encoded_string)
				return std::wstring{};

			return encode_as_utf16(utf8_encoded_string, Length(utf8_encoded_string));
		}
	}


}