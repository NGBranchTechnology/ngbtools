#pragma once


namespace ngbtools
{
    /**********************************************************************************************//**
     * \namespace   console
     *
     * \brief   one of the many motivations for writing this module is that console output on Windows is not UTF8 - it is based on a codepage.
     *          but all the text in ngbt is supposed to be uTF8, so we need to have a way to correctly output that.
     **************************************************************************************************/
    namespace console
    {
        enum class color : char
        {
            BLACK = 0,
            DARKBLUE = FOREGROUND_BLUE,
            DARKGREEN = FOREGROUND_GREEN,
            DARKCYAN = FOREGROUND_GREEN | FOREGROUND_BLUE,
            DARKRED = FOREGROUND_RED,
            DARKMAGENTA = FOREGROUND_RED | FOREGROUND_BLUE,
            DARKYELLOW = FOREGROUND_RED | FOREGROUND_GREEN,
            DARKGRAY = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
            GRAY = FOREGROUND_INTENSITY,
            BLUE = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
            GREEN = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
            CYAN = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
            RED = FOREGROUND_INTENSITY | FOREGROUND_RED,
            MAGENTA = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
            YELLOW = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
            WHITE = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
        };


#define CONSOLE_BLUE "\x1b\x01"
#define CONSOLE_GREEN "\x1b\x02"
#define CONSOLE_RED "\x1b\x0C"

#define CONSOLE_WHITE "\x1b\x0F"
#define CONSOLE_STANDARD "\x1b\xFF"

        /*
        #define FOREGROUND_BLUE      0x0001 // text color contains blue.
        #define FOREGROUND_GREEN     0x0002 // text color contains green.
        #define FOREGROUND_RED       0x0004 // text color contains red.
        #define FOREGROUND_INTENSITY 0x0008 // text color is intensified.
        #define BACKGROUND_BLUE      0x0010 // background color contains blue.
        #define BACKGROUND_GREEN     0x0020 // background color contains green.
        #define BACKGROUND_RED       0x0040 // background color contains red.
        #define BACKGROUND_INTENSITY 0x0080 // background color is intensified.
        */
        struct console_context
        {
            HANDLE hConsoleOutput;
            HANDLE hConsoleInput;
            bool has_ensured_process_has_console;
            bool has_tried_and_failed_to_get_console;
            WORD wOldColorAttrs;
            bool has_retrieved_old_color_attrs;
            bool write_output_has_failed_once;

        };

        console_context& get_context()
        {
            static console_context the_console_context{
                INVALID_HANDLE_VALUE, // hConsoleOutput
                INVALID_HANDLE_VALUE, // hConsoleInput
                false, // has_ensured_process_has_console
                false, // has_tried_and_failed_to_get_console
                0, // wOldColorAttrs
                false, // has_retrieved_old_color_attrs
                false // write_output_has_failed_once
            };
            return the_console_context;
        }


        inline std::string encode_as_output_bytes(const wchar_t* unicode_string, size_t wide_len)
        {
            const auto output_cp{ ::GetConsoleOutputCP() };

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

#ifdef _CONSOLE
        inline bool ensure_process_has_console()
        {
            return true;
        }
#else
        inline bool bool ensure_process_has_console()
        {
            auto& cc{ get_context() };

            if (cc.has_tried_and_failed_to_get_console)
                return false;

            if (cc.has_ensured_process_has_console || GetConsoleWindow())
                return true;

            if (!AttachConsole(ATTACH_PARENT_PROCESS))
            {
                cc.has_tried_and_failed_to_get_console = true;
                writeline("AttachConsole(ATTACH_PARENT_PROCESS) failed");
                return false;
            }
            cc.has_ensured_process_has_console = true;
            return true;
        }
#endif
        inline bool ensure_output_handle()
        {
            auto& cc{ get_context() };

            if (cc.hConsoleOutput != INVALID_HANDLE_VALUE)
                return true;

            ensure_process_has_console();

            cc.hConsoleOutput = ::GetStdHandle(STD_OUTPUT_HANDLE);
            if (INVALID_HANDLE_VALUE == cc.hConsoleOutput)
            {
                // todo: proper error handling
                //writeline("GetStdHandle(STD_OUTPUT_HANDLE) failed");
                return false;
            }
            // this is here so that fmt::format understands {:L} properly
            std::locale::global(std::locale("de_DE.UTF-8"));
            return true;
        }

        inline bool write_unicode_output(const std::wstring& utf16_encoded_string)
        {
            auto& cc{ get_context() };

            if (utf16_encoded_string.empty())
                return true;

            if (cc.write_output_has_failed_once || !ensure_output_handle())
                return false;

            DWORD chars_written = 0;

            const auto output_bytes{
                encode_as_output_bytes(utf16_encoded_string.c_str(), utf16_encoded_string.size())
            };

            if (!::WriteFile(cc.hConsoleOutput, &output_bytes[0], (DWORD)(output_bytes.size()), &chars_written, nullptr))
            {
                cc.write_output_has_failed_once = true;
                // it is unclear how this can work, given that WriteFile() failed
                //console::writeline("WriteFile() failed");
                return false;
            }
            return true;
        }




        inline bool do_write_output_as_unicode(const std::string& utf8_encoded_string)
        {
            auto& cc{ get_context() };
            const auto utf16_encoded_string{ string::encode_as_utf16(utf8_encoded_string) };
            const auto output_bytes{
                encode_as_output_bytes(utf16_encoded_string.c_str(), utf16_encoded_string.size())
            };

            DWORD chars_written = 0;
            if (!::WriteFile(cc.hConsoleOutput, &output_bytes[0], (DWORD)(output_bytes.size()), &chars_written, nullptr))
            {
                cc.write_output_has_failed_once = true;
                // unclear how we can log this erro here
                return false;
            }
            return true;
        }

        inline bool write_output_as_unicode(const std::string& utf8_encoded_string)
        {
            auto& cc{ get_context() };

            if (utf8_encoded_string.empty())
                return true;

            if (cc.write_output_has_failed_once || !ensure_output_handle())
                return false;

            const char* p = utf8_encoded_string.c_str();
            while (true)
            {
                const char* q = strchr(p, '\x1b');
                if (!q)
                {
                    return do_write_output_as_unicode(p);
                }
                if (!cc.has_retrieved_old_color_attrs)
                {
                    CONSOLE_SCREEN_BUFFER_INFO csbiInfo{};
                    GetConsoleScreenBufferInfo(cc.hConsoleOutput, &csbiInfo);
                    cc.wOldColorAttrs = csbiInfo.wAttributes;
                    cc.has_retrieved_old_color_attrs = true;
                }

                const std::string substring{ p, (size_t)(q - p) };
                do_write_output_as_unicode(substring);
                if (q[1] == '\xFF')
                {
                    SetConsoleTextAttribute(cc.hConsoleOutput, cc.wOldColorAttrs);
                }
                else
                {
                    SetConsoleTextAttribute(cc.hConsoleOutput, q[1]);
                }

                p = q + 2;
            }
        }
        inline bool write(const std::string& text)
        {
            return write_output_as_unicode(text);
        }

        inline bool writeline(const std::string& text)
        {
            return write(text + "\r\n");
        }

        template <typename... Args> bool formatline(const std::string_view text, Args&&... args)
        {
            return writeline(std::vformat(text, std::make_format_args(std::forward<Args>(args)...)));
        }
    };
}