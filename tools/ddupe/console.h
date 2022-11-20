#pragma once


namespace gtools23
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
        /**********************************************************************************************//**
         * \brief	Ensures that process has console.
         *
         * \return	true if it succeeds, false if it fails.
         **************************************************************************************************/

#ifdef _CONSOLE
        #define ensure_process_has_console() true
#else	
        bool ensure_process_has_console();
#endif	

        /**********************************************************************************************//**
         * \brief   Writes a UTF8 encoded string to the console in the correct target encoding. 
         *
         * \param   utf8_encoded_string The UTF 8 encoded string.
         **************************************************************************************************/

        bool write_output_as_unicode(const std::string& utf8_encoded_string);

        /**********************************************************************************************//**
         * \brief   Writes a text string to the console that is already UTF16 encoded. Because it is,
         *          we can use the wide-console functions immediately
         *
         * \param   utf16_encoded_string    The UTF 16 encoded string.
         **************************************************************************************************/

        bool write_unicode_output(const std::wstring& utf16_encoded_string);

        /**********************************************************************************************//**
         * \brief   Given a standard std::string text, write it to the console as (UTF8-encoded) string 
         *
         * \param   text    The text to write.
         **************************************************************************************************/

        inline bool write(const std::string& text)
        {
            return write_output_as_unicode(text);
        }

        inline bool writeline(const std::string& text)
        {
            return write(text + "\r\n");
        }

        /**********************************************************************************************//**
         * \brief   Reads the next input line from the console, decoding the console codepage
         *
         * \return  A std::string.
         **************************************************************************************************/

        std::string readline();
    }
}