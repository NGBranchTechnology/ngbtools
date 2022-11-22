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

#ifdef NGBTOOLS_USE_VIRTUAL_CONSOLE_COMMANDS
#define VESC_SEQUENCE(__VESC_CODE__) "\x1b[" #__VESC_CODE__ "m"
#define CONSOLE_STANDARD                    VESC_SEQUENCE(0)	// Default	Returns all attributes to the default state prior to modification
#define CONSOLE_BOLD                        VESC_SEQUENCE(1)	// Bold / Bright	Applies brightness / intensity flag to foreground color
#define CONSOLE_NO_BOLD                     VESC_SEQUENCE(22)   //	No bold / bright	Removes brightness / intensity flag from foreground color
#define CONSOLE_UNDERLINE                   VESC_SEQUENCE(4)    // Underline	Adds underline
#define CONSOLE_NO_UNDERLINE                VESC_SEQUENCE(24)	// No underline	Removes underline
#define CONSOLE_NEGATIVE                    VESC_SEQUENCE(7)    // Negative	Swaps foreground and background colors
#define CONSOLE_POSITIVE                    VESC_SEQUENCE(27)	// Positive(No negative)	Returns foreground / background to normal
#define CONSOLE_FOREGROUND_BLACK            VESC_SEQUENCE(30)	// Foreground Black	Applies non - bold / bright black to foreground
#define CONSOLE_FOREGROUND_RED              VESC_SEQUENCE(31)	// Foreground Red	Applies non - bold / bright red to foreground
#define CONSOLE_FOREGROUND_GREEN            VESC_SEQUENCE(32)	// Foreground Green	Applies non - bold / bright green to foreground
#define CONSOLE_FOREGROUND_YELLOW           VESC_SEQUENCE(33)	// Foreground Yellow	Applies non - bold / bright yellow to foreground
#define CONSOLE_FOREGROUND_BLUE             VESC_SEQUENCE(34)	// Foreground Blue	Applies non - bold / bright blue to foreground
#define CONSOLE_FOREGROUND_MAGENTA          VESC_SEQUENCE(35)	// Foreground Magenta	Applies non - bold / bright magenta to foreground
#define CONSOLE_FOREGROUND_CYAN             VESC_SEQUENCE(36)	// Foreground Cyan	Applies non - bold / bright cyan to foreground
#define CONSOLE_FOREGROUND_WHITE            VESC_SEQUENCE(37)	// Foreground White	Applies non - bold / bright white to foreground
#define CONSOLE_FOREGROUND_DEFAULT          VESC_SEQUENCE(39)	// Foreground Default	Applies only the foreground portion of the defaults(see 0)
#define CONSOLE_BACKGROUND_BLACK            VESC_SEQUENCE(40)  // Background Black	Applies non - bold / bright black to background
#define CONSOLE_BACKGROUND_RED              VESC_SEQUENCE(41)  // Background Red	Applies non - bold / bright red to background
#define CONSOLE_BACKGROUND_GREEN            VESC_SEQUENCE(42)  // Background Green	Applies non - bold / bright green to background
#define CONSOLE_BACKGROUND_YELLOW           VESC_SEQUENCE(43)  // Background Yellow	Applies non - bold / bright yellow to background
#define CONSOLE_BACKGROUND_BLUE             VESC_SEQUENCE(44)  // Background Blue	Applies non - bold / bright blue to background
#define CONSOLE_BACKGROUND_MAGENTA          VESC_SEQUENCE(45)  // Background Magenta	Applies non - bold / bright magenta to background
#define CONSOLE_BACKGROUND_CYAN             VESC_SEQUENCE(46)  // Background Cyan	Applies non - bold / bright cyan to background
#define CONSOLE_BACKGROUND_WHITE            VESC_SEQUENCE(47)  // Background White	Applies non - bold / bright white to background
#define CONSOLE_BACKGROUND_DEFAULT          VESC_SEQUENCE(49)  // Background Default	Applies only the background portion of the defaults(see 0)
#define CONSOLE_FOREGROUND_BRIGHT_BLACK     VESC_SEQUENCE(90)  // Bright Foreground Black	Applies bold / bright black to foreground
#define CONSOLE_FOREGROUND_BRIGHT_RED       VESC_SEQUENCE(91)  // Bright Foreground Red	Applies bold / bright red to foreground
#define CONSOLE_FOREGROUND_BRIGHT_GREEN     VESC_SEQUENCE(92)  // Bright Foreground Green	Applies bold / bright green to foreground
#define CONSOLE_FOREGROUND_BRIGHT_YELLOW    VESC_SEQUENCE(93)  // Bright Foreground Yellow	Applies bold / bright yellow to foreground
#define CONSOLE_FOREGROUND_BRIGHT_BLUE      VESC_SEQUENCE(94)  // Bright Foreground Blue	Applies bold / bright blue to foreground
#define CONSOLE_FOREGROUND_BRIGHT_MAGENTA   VESC_SEQUENCE(95)  // Bright Foreground Magenta	Applies bold / bright magenta to foreground
#define CONSOLE_FOREGROUND_BRIGHT_CYAN      VESC_SEQUENCE(96)  // Bright Foreground Cyan	Applies bold / bright cyan to foreground
#define CONSOLE_FOREGROUND_BRIGHT_WHITE     VESC_SEQUENCE(97)  // Bright Foreground White	Applies bold / bright white to foreground
#define CONSOLE_BACKGROUND_BRIGHT_BLACK     VESC_SEQUENCE(100)	Bright Background Black	Applies bold / bright black to background
#define CONSOLE_BACKGROUND_BRIGHT_RED       VESC_SEQUENCE(101)	Bright Background Red	Applies bold / bright red to background
#define CONSOLE_BACKGROUND_BRIGHT_GREEN     VESC_SEQUENCE(102)	Bright Background Green	Applies bold / bright green to background
#define CONSOLE_BACKGROUND_BRIGHT_YELLOW    VESC_SEQUENCE(103)	Bright Background Yellow	Applies bold / bright yellow to background
#define CONSOLE_BACKGROUND_BRIGHT_BLUE      VESC_SEQUENCE(104)	Bright Background Blue	Applies bold / bright blue to background
#define CONSOLE_BACKGROUND_BRIGHT_MAGENTA   VESC_SEQUENCE(105)	Bright Background Magenta	Applies bold / bright magenta to background
#define CONSOLE_BACKGROUND_BRIGHT_CYAN      VESC_SEQUENCE(106)	Bright Background Cyan	Applies bold / bright cyan to background
#define CONSOLE_BACKGROUND_BRIGHT_WHITE     VESC_SEQUENCE(107)	Bright Background White	Applies bold / bright white to background

#else

#define CONSOLE_FOREGROUND_BRIGHT_BLACK "\x1b\x00"
#define CONSOLE_FOREGROUND_BRIGHT_BLUE "\x1b\x09"
#define CONSOLE_FOREGROUND_BRIGHT_CYAN "\x1b\x0b"
#define CONSOLE_FOREGROUND_BLUE "\x1b\x01"
#define CONSOLE_FOREGROUND_CYAN "\x1b\x03"
#define CONSOLE_FOREGROUND_GRAY "\x1b\x07"
#define CONSOLE_FOREGROUND_GREEN "\x1b\x02"
#define CONSOLE_FOREGROUND_MAGENTA "\x1b\x05"
#define CONSOLE_FOREGROUND_RED "\x1b\x04"
#define CONSOLE_FOREGROUND_YELLOW "\x1b\x06"
#define CONSOLE_FOREGROUND_BRIGHT_GRAY "\x1b\x08"
#define CONSOLE_FOREGROUND_BRIGHT_GREEN "\x1b\x0a"
#define CONSOLE_FOREGROUND_BRIGHT_INTENSITY "\x1b\x08"
#define CONSOLE_FOREGROUND_BRIGHT_MAGENTA "\x1b\x0d"
#define CONSOLE_FOREGROUND_BRIGHT_RED "\x1b\x0c"
#define CONSOLE_FOREGROUND_BRIGHT_WHITE "\x1b\x0f"
#define CONSOLE_FOREGROUND_BRIGHT_YELLOW "\x1b\x0e"

#define CONSOLE_STANDARD "\x1b\xFF"
#endif

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


        inline std::string encode_as_output_bytes(std::wstring_view text)
        {
            const wchar_t* unicode_string = text.data();
            const size_t wide_len = text.size();

            const auto output_cp{ ::GetConsoleOutputCP() };

            // handle nullptr gracefully
            if (!unicode_string || !wide_len)
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

        inline bool write_unicode_output(std::wstring_view utf16_encoded_string)
        {
            auto& cc{ get_context() };

            if (utf16_encoded_string.empty())
                return true;

            if (cc.write_output_has_failed_once || !ensure_output_handle())
                return false;

            DWORD chars_written = 0;

            const auto output_bytes{
                encode_as_output_bytes(utf16_encoded_string)
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




        inline bool do_write_output_as_unicode(std::string_view utf8_encoded_string)
        {
            auto& cc{ get_context() };
            const auto utf16_encoded_string{ string::encode_as_utf16(utf8_encoded_string) };
            const auto output_bytes{
                encode_as_output_bytes(utf16_encoded_string)
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

        inline bool write_output_as_unicode(std::string_view utf8_encoded_string)
        {
            auto& cc{ get_context() };

            if (utf8_encoded_string.empty())
                return true;

            if (cc.write_output_has_failed_once || !ensure_output_handle())
                return false;

#ifdef NGBTOOLS_USE_VIRTUAL_CONSOLE_COMMANDS
            return do_write_output_as_unicode(utf8_encoded_string);
#else
            const char* p = utf8_encoded_string.data();
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
#endif
        }

        inline bool write(std::string_view text)
        {
            return write_output_as_unicode(text);
        }

        inline bool writeline(const std::string& text)
        {
            return write(text + "\r\n");
        }

        inline bool writeline(std::u8string_view text)
        {
            return write(std::string((const char*)text.data()) + "\r\n");
        }

        template <typename... Args> bool formatline(const std::string_view text, Args&&... args)
        {
            return writeline(std::vformat(text, std::make_format_args(std::forward<Args>(args)...)));
        }
    };
}