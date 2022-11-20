#include "precomp.h"
#include "console.h"
#include "string.h"
#include "wstring.h"
#include "console.h"

namespace gtools23
{
    namespace console
    {
        /** \brief   The console output handle for the current process. */
        static HANDLE hConsoleOutput = INVALID_HANDLE_VALUE;

        /** \brief   The console input handle for the current process. */
        static HANDLE hConsoleInput = INVALID_HANDLE_VALUE;

		static bool has_ensured_process_has_console = false;
        static bool has_tried_and_failed_to_get_console = false;

        static WORD wOldColorAttrs = 0;
        static bool has_retrieved_old_color_attrs = false;

#ifndef _CONSOLE
        bool ensure_process_has_console()
		{
            if (has_tried_and_failed_to_get_console)
                return false;

			if (has_ensured_process_has_console || GetConsoleWindow())
				return true;

			if (!AttachConsole(ATTACH_PARENT_PROCESS))
			{
                has_tried_and_failed_to_get_console = true;
                lc().windows_error("AttachConsole(ATTACH_PARENT_PROCESS) failed");
				return false;
			}
			has_ensured_process_has_console = true;
			return true;
		}
#endif

        /**********************************************************************************************//**
         * \fn  static inline bool EnsureOutputHandle()
         *
         * \brief   Ensures that the output handle is valid.
         *
         * \return  true if it succeeds, false if it fails.
         **************************************************************************************************/

        static inline bool EnsureOutputHandle()
        {
            if (hConsoleOutput != INVALID_HANDLE_VALUE)
                return true;

			ensure_process_has_console();

            hConsoleOutput = ::GetStdHandle(STD_OUTPUT_HANDLE);
            if (INVALID_HANDLE_VALUE == hConsoleOutput)
            {
                console::writeline("GetStdHandle(STD_OUTPUT_HANDLE) failed");
                return false;
            }
            return true;
        }

        /**********************************************************************************************//**
         * \fn  static inline bool EnsureInputHandle()
         *
         * \brief   Ensures that the input handle is valid
         *
         * \return  true if it succeeds, false if it fails.
         **************************************************************************************************/

        static inline bool EnsureInputHandle()
        {
            if (hConsoleInput != INVALID_HANDLE_VALUE)
                return true;

			ensure_process_has_console();

            hConsoleInput = ::GetStdHandle(STD_INPUT_HANDLE);
            if (INVALID_HANDLE_VALUE == hConsoleInput)
            {
                console::writeline("GetStdHandle(STD_INPUT_HANDLE) failed");
                return false;
            }

            DWORD dwMode = 0;
            if (!GetConsoleMode(hConsoleInput, &dwMode))
            {
                console::writeline("GetConsoleMode(hConsoleInput) failed");
                return false;
            }
            if (!(dwMode & ENABLE_LINE_INPUT))
            {
                if (!::SetConsoleMode(hConsoleInput, dwMode | ENABLE_LINE_INPUT))
                {
                    console::writeline("SetConsoleMode(hConsoleInput, ENABLE_LINE_INPUT) failed");
                    return false;
                }
            }
            return true;
        }


        static std::string encode_as_output_bytes(const wchar_t* unicode_string, size_t wide_len)
        {
            const UINT output_cp = ::GetConsoleOutputCP();

            // handle nullptr gracefully
            if (!unicode_string or !wide_len)
                return std::string();

            // we try the stack first (because that is of an order of magnitudes faster)
            // only if this fails we revert back to allocating the data on the stack
            const size_t size_of_stack_buffer = 1024;
            if (wide_len < size_of_stack_buffer)
            {
                char buffer_on_the_stack[size_of_stack_buffer];
                int rc = WideCharToMultiByte(output_cp, 0, unicode_string, (int)wide_len,
                    buffer_on_the_stack, size_of_stack_buffer - 1, nullptr, nullptr);
                if (rc > 0)
                {
                    assert(rc < sizeof(buffer_on_the_stack));
                    buffer_on_the_stack[rc] = 0;
                    return std::string(buffer_on_the_stack);
                }
                rc = GetLastError();

                // we don't expect these
                assert(rc != ERROR_INVALID_FLAGS);
                assert(rc != ERROR_INVALID_PARAMETER);
                assert(rc != ERROR_NO_UNICODE_TRANSLATION);

                // we do expect this
                assert(rc == ERROR_INSUFFICIENT_BUFFER);
            }
            size_t size_to_allocate = 2 * wide_len;
            size_t max_size_to_allocate = 64 * size_to_allocate;
            while (max_size_to_allocate)
            {
                std::vector<char> buffer_on_the_heap;
                buffer_on_the_heap.resize(size_to_allocate + 1);

                int rc = WideCharToMultiByte(output_cp, 0, unicode_string, (int)wide_len,
                    &buffer_on_the_heap[0], (int)size_to_allocate, nullptr, nullptr);

                if (rc > 0)
                {
                    assert(size_t(rc) < size_to_allocate);
                    buffer_on_the_heap[rc] = 0;
                    return std::string(&buffer_on_the_heap[0]);
                }
                size_to_allocate *= 2;
            }
            assert(false);
            return std::string();
        }

        static bool write_output_has_failed_once = false;

        bool write_unicode_output(const std::wstring& utf16_encoded_string)
        {
            if (utf16_encoded_string.empty())
                return true;
            
            if (write_output_has_failed_once || !EnsureOutputHandle())
                return false;

            DWORD chars_written = 0;

            const std::string output_bytes{
                encode_as_output_bytes(utf16_encoded_string.c_str(), utf16_encoded_string.size())
            };

            if (!::WriteFile(hConsoleOutput, &output_bytes[0], (DWORD)(output_bytes.size()), &chars_written, nullptr))
            {
                write_output_has_failed_once = true;
                console::writeline("WriteConsoleW() failed");
                return false;
            }
            return true;
        }
        static bool do_write_output_as_unicode(const std::string& utf8_encoded_string)
        {
            UINT cp = ::GetConsoleOutputCP();

            const std::wstring utf16_encoded_string{ string::encode_as_utf16(utf8_encoded_string) };
            const std::string output_bytes{
                encode_as_output_bytes(utf16_encoded_string.c_str(), utf16_encoded_string.size())
            };

            DWORD chars_written = 0;
            if (!::WriteFile(hConsoleOutput, &output_bytes[0], (DWORD)(output_bytes.size()), &chars_written, nullptr))
            {
                write_output_has_failed_once = true;
                console::writeline("WriteConsoleW() failed");
                return false;
            }
            return true;
        }

        bool write_output_as_unicode(const std::string& utf8_encoded_string)
        {
            if (utf8_encoded_string.empty())
                return true;
            
            if (write_output_has_failed_once || !EnsureOutputHandle())
                return false;

            // if we find and escape sign, we set the color, so:
            const char* p = utf8_encoded_string.c_str();

            while (true)
            {
                const char* q = strchr(p, '\x1b');
                if (!q)
                {
                    return do_write_output_as_unicode(p);
                }
                if (!has_retrieved_old_color_attrs)
                {
                    CONSOLE_SCREEN_BUFFER_INFO csbiInfo{};
                    GetConsoleScreenBufferInfo(hConsoleOutput, &csbiInfo);
                    wOldColorAttrs = csbiInfo.wAttributes;
                    has_retrieved_old_color_attrs = true;
                }

                const std::string substring{ p, (size_t)(q - p) };
                do_write_output_as_unicode(substring);
                if (q[1] == '\xFF')
                {
                    SetConsoleTextAttribute(hConsoleOutput, wOldColorAttrs);
                }
                else
                {
                    SetConsoleTextAttribute(hConsoleOutput, q[1]);
                }
                
                p = q + 2;
            }
        }

        std::string readline()
        {
            std::string empty_string;

            if (!EnsureInputHandle())
                return empty_string;

            std::vector<wchar_t> input;
            input.resize(1024);

            DWORD dwCharsRead = 0;
            if (!ReadConsoleW(hConsoleInput, &input[0], (DWORD)input.size(), &dwCharsRead, nullptr))
            {
                console::writeline("ReadConsoleW() failed");
                return empty_string;
            }

            return wstring::encode_as_utf8(&input[0], dwCharsRead);
        }
        
   }
}