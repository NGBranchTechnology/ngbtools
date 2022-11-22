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

        inline bool is_empty(std::string_view txt)
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

        inline size_t length(std::string_view text)
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

        inline bool equals(wchar_t a, wchar_t b)
        {
            return a == b;
        }

        inline bool equals(std::wstring_view a, std::wstring_view b)
        {
            if (a.empty())
                return b.empty();

            if (b.empty())
                return false;

            return wcscmp(a.data(), b.data()) == 0;
        }

        inline bool equals_nocase(std::wstring_view a, std::wstring_view b)
        {
            if (a.empty())
                return b.empty();

            if (b.empty())
                return false;

            return _wcsicmp(a.data(), b.data()) == 0;
        }

        inline bool equals(std::string_view a, std::string_view b)
        {
            if (a.empty())
                return b.empty();

            if (b.empty())
                return false;

            return strcmp(a.data(), b.data()) == 0;
        }

        inline bool equals_nocase(std::string_view a, std::string_view b)
        {
            if (a.empty())
                return b.empty();

            if (b.empty())
                return false;

            return _stricmp(a.data(), b.data()) == 0;
        }


        inline std::string join(const std::vector<std::string>& items, std::string_view joiner)
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

        const std::string uppercase(std::string_view text)
        {
            std::string result{ text };
            auto p = const_cast<char*>(result.c_str());
            _strupr(p);
            return result;
        }

        inline std::string lowercase(std::string_view text)
        {
            std::string result{ text };
            const auto p = const_cast<char*>(result.c_str());
            _strlwr(p);
            return result;
        }

        inline std::vector<std::string> split(std::string_view svtext, std::string_view svseparators, bool handle_quotation_marks = false)
        {
            std::vector<std::string> result;

            bool is_recording_quoted_string = false;
            auto text = svtext.data();
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

                    if (std::strchr(svseparators.data(), c))
                    {
                        assert(text - start >= 1);
                        result.push_back(std::string{ start, (size_t)(text - start - 1) });
                        start = text;
                    }
                }
            }
            return result;
        }

		inline std::wstring encode_as_utf16(std::string_view utf8_encoded_text)
		{
            const auto utf8_len = utf8_encoded_text.size();
            const auto utf8_encoded_string = utf8_encoded_text.data();
			
            // handle nullptr gracefully
			if (!utf8_encoded_string || !utf8_len)
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

	}


}