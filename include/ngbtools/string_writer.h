#pragma once


#include <string>
#include <cassert>

namespace ngbtools
{
    namespace string
    {
        class writer final
        {
        public:
            writer()
                :
                m_writepos(0),
                m_dynamic_size(0),
                m_dynamic_buffer(nullptr),
                m_builtin_buffer{}
            {
            }

            writer(const writer& source)
                :
                m_writepos(0),
                m_dynamic_size(0),
                m_dynamic_buffer(nullptr)
            {
                m_builtin_buffer[0] = 0;
                assign(source);
            }

            writer& operator=(const writer& source)
            {
                assign(source);
                return *this;
            }

            writer(writer&& source) noexcept
            {
                m_writepos = source.m_writepos;
                source.m_writepos = 0;

                m_dynamic_size = source.m_dynamic_size;
                source.m_dynamic_size = 0;

                m_dynamic_buffer = source.m_dynamic_buffer;
                source.m_dynamic_buffer = nullptr;

                // if (and only if) the origin still uses the cache, we need to do so as well
                if (!m_dynamic_buffer)
                {
                    assert(m_writepos < array_size(m_builtin_buffer));
                    memcpy(m_builtin_buffer, source.m_builtin_buffer, m_writepos);
                }
            }

            writer& operator=(writer&& source) noexcept
            {
                assert(this != &source);
                if (m_dynamic_buffer)
                {
                    free(m_dynamic_buffer);
                }

                m_writepos = source.m_writepos;
                source.m_writepos = 0;

                m_dynamic_size = source.m_dynamic_size;
                source.m_dynamic_size = 0;

                m_dynamic_buffer = source.m_dynamic_buffer;
                source.m_dynamic_buffer = nullptr;

                // if (and only if) the origin still uses the cache, we need to do so as well
                if (!m_dynamic_buffer)
                {
                    assert(m_writepos < array_size(m_builtin_buffer));
                    memcpy(m_builtin_buffer, source.m_builtin_buffer, m_writepos);
                }
                return *this;
            }

            ~writer()
            {
                clear();
            }

            bool empty() const
            {
                return (m_writepos == 0);
            }

            std::string as_string() const
            {
                // this is needed to ensure that the string is zero-terminated
                const_cast<writer*>(this)->append('\0');
                --m_writepos;
                return m_dynamic_buffer ? m_dynamic_buffer : m_builtin_buffer;
            }
            
            void hexdump(const unsigned char* address, size_t size)
            {
                if (!address)
                {
                    append("nullptr\r\n");
                    return;
                }
                append_formatted("{0} bytes at {1}:\r\n", size, (const void*)address);

                if (!size)
                    return;

                size_t lines = size / BYTES_PER_LINE;
                if (size % BYTES_PER_LINE)
                    ++lines;

                char output[128];

                size_t bytesRemaining = size;
                for (size_t line = 0; line < lines; ++line)
                {
#ifdef _M_X64
                    sprintf_s(output, "%016p", address);
#else
                    sprintf_s(output, "%08p", address);
#endif
                    memset(output + START_OF_DWORDS - 1, ' ', sizeof(output) - 1 - START_OF_DWORDS);
                    output[START_OF_DWORDS - 1] = ':';
                    char* wp_bytes = &output[START_OF_DWORDS];
                    char* wp_text = &output[START_OF_CTEXT + 4];

                    size_t index, offset = 0;
                    for (index = 0; index < BYTES_PER_LINE; ++index)
                    {
                        if (!bytesRemaining)
                            break;

                        unsigned char c = address[index];
                        *(wp_bytes++ + offset) = string::hex_digit(string::upper_nibble(c));
                        *(wp_bytes++ + offset) = string::hex_digit(string::lower_nibble(c));

                        wp_text[index] = isprint(c) ? c : '.';
                        --bytesRemaining;
                        if (index % 4 == 3)
                            offset++;
                    }
                    wp_text[index++ + offset] = '\r';
                    wp_text[index++ + offset] = '\n';
                    assert(index + offset < 100);
                    wp_text[index + offset] = 0;
                    append(output);
                    address += BYTES_PER_LINE;
                }
            }

            bool newline()
            {
                return append(string::newline());
            }

            bool append(char c)
            {
                if (m_dynamic_buffer == nullptr)
                {
                    if (m_writepos < array_size(m_builtin_buffer))
                    {
                        m_builtin_buffer[m_writepos++] = c;
                        return true;
                    }
                }
                else if (m_writepos < m_dynamic_size)
                {
                    m_dynamic_buffer[m_writepos++] = c;
                    return true;
                }

                if (m_dynamic_buffer)
                {
                    char* np = (char*)realloc(m_dynamic_buffer, m_dynamic_size * 2);
                    if (!np)
                        return false;

                    m_dynamic_buffer = np;
                    m_dynamic_size *= 2;
                    m_dynamic_buffer[m_writepos++] = c;
                    return true;
                }
                char* np = (char*)malloc(array_size(m_builtin_buffer) * 2);
                if (!np)
                    return false;

                memcpy(np, m_builtin_buffer, m_writepos);
                m_dynamic_buffer = np;
                m_dynamic_size = array_size(m_builtin_buffer) * 2;
                m_dynamic_buffer[m_writepos++] = c;
                return true;
            }

            bool append(const char* ps)
            {
                if (!ps || !*ps)
                    return true;

                size_t len = strlen(ps);
                char* wp = ensure_free_space(len);
                if (!wp)
                    return false;

                memcpy(wp, ps, len);
                m_writepos += len;
                return true;
            }

            bool append(const std::string& s)
            {
                return append(s.c_str());
            }

            bool append_repeated(char c, size_t number_of_times)
            {
                if (!c || !number_of_times)
                    return true;

                char* wp = ensure_free_space(number_of_times);
                if (!wp)
                    return false;

                memset(wp, c, number_of_times);
                m_writepos += number_of_times;
                return true;
            }

            bool append_repeated(std::string_view text, size_t number_of_times)
            {
                if (text.empty() || !number_of_times)
                    return true;

                size_t length_of_single_string = string::length(text);
                size_t bytes_required = length_of_single_string * number_of_times;

                char* wp = ensure_free_space(bytes_required);
                if (!wp)
                    return false;

                const char* sp = text.data();
                for (size_t index = 0; index < number_of_times; ++index)
                {
                    memcpy(wp, sp, length_of_single_string);
                    wp += length_of_single_string;
                }

                m_writepos += bytes_required;
                return true;
            }

            bool append_sized_string(const char* ps, size_t len)
            {
                if (!ps || !*ps or !len)
                    return true;

                size_t actual_len = strlen(ps);
                if (len > actual_len)
                    len = actual_len;

                char* wp = ensure_free_space(len);
                if (!wp)
                    return false;

                memcpy(wp, ps, len);
                m_writepos += len;
                return true;
            }

            template <typename... Args> bool append_formatted(const std::string_view text, Args&&... args)
            {
                return append(std::vformat(text, std::make_format_args(std::forward<Args>(args)...)));
            }

            /** \brief   Clears this object to its blank/initial state. */
            void clear()
            {
                if (m_dynamic_buffer)
                {
                    free(m_dynamic_buffer);
                    m_dynamic_buffer = nullptr;
                    m_dynamic_size = 0;
                }
                m_writepos = 0;
            }

        private:

            void correct(int bytes_written)
            {
                if (m_dynamic_buffer == nullptr)
                {
                    if ((m_writepos + bytes_written) >= array_size(m_builtin_buffer))
                    {
                        assert(false);
                    }
                }
                else
                {
                    if ((m_writepos + bytes_written) >= m_dynamic_size)
                    {
                        assert(false);
                    }
                }

                if (bytes_written >= 0)
                {
                    m_writepos += bytes_written;
                }
                else
                {
                    assert(false);
                }
            }

            char* ensure_free_space(size_t space_needed)
            {
                const size_t space_total = m_writepos + space_needed;

                // if we're still using the stack buffer
                if (m_dynamic_buffer == nullptr)
                {
                    // and have enough space left
                    if (space_total < array_size(m_builtin_buffer))
                    {
                        return m_builtin_buffer + m_writepos;
                    }
                }

                // if we are using the dynamic buffer and have enough space left
                if (space_total < m_dynamic_size)
                {
                    return m_dynamic_buffer + m_writepos;
                }

                // we DO have a dynamic buffer, but not enough size: allocate large enough
                if (m_dynamic_buffer)
                {
                    size_t size_needed = m_dynamic_size * 2;
                    while (size_needed < space_total)
                    {
                        size_needed *= 2;
                    }

                    char* np = (char*)realloc(m_dynamic_buffer, size_needed);
                    if (!np)
                        return nullptr;

                    m_dynamic_buffer = np;
                    m_dynamic_size = size_needed;
                    return m_dynamic_buffer + m_writepos;;
                }
                size_t size_needed = array_size(m_builtin_buffer) * 2;
                while (size_needed < space_total)
                {
                    size_needed *= 2;
                }
                char* np = (char*)malloc(size_needed);
                if (!np)
                    return nullptr;

                memcpy(np, m_builtin_buffer, m_writepos);
                m_dynamic_buffer = np;
                m_dynamic_size = size_needed;
                return m_dynamic_buffer + m_writepos;;
            }

            /**
             * \brief   Assigns the whole contents of a given writer object (used by both copy constructor and assignment operator)
             *
             * \param   objectSrc   The object source.
             *
             * \return  true if it succeeds, false if it fails.
             */

            bool assign(const writer& objectSrc)
            {
                if (this == &objectSrc)
                    return true;

                clear();

                // we need to copy EITHER the cache OR the dynamic buffer
                if (objectSrc.m_dynamic_buffer)
                {
                    m_dynamic_buffer = (char*)malloc(objectSrc.m_dynamic_size);

                    if (!m_dynamic_buffer)
                        return false;

                    // but we need to copy only the used bytes
                    memcpy(m_dynamic_buffer, objectSrc.m_dynamic_buffer, objectSrc.m_writepos);

                    m_writepos = objectSrc.m_writepos;
                    m_dynamic_size = objectSrc.m_dynamic_size;
                    return true;
                }
                else
                {
                    clear();
                    m_writepos = objectSrc.m_writepos;
                    if (m_writepos)
                    {
                        memcpy(m_builtin_buffer, objectSrc.m_builtin_buffer, m_writepos);
                    }
                    return true;
                }
            }

        private:
            /** \brief   The current write position */
            mutable size_t m_writepos;

            /** \brief   Size of the dynamic memory (if any) */
            size_t m_dynamic_size;

            /** \brief   Buffer for dynamic data. Allocated only if necessary */
            char* m_dynamic_buffer;

            /** \brief   The builtin cache (1024 bytes). */
            char m_builtin_buffer[1024];


            const unsigned NIBBLES_PER_DWORD = 8;
            const unsigned DWORDS_PER_LINE = 5;
            const unsigned BYTES_PER_LINE = DWORDS_PER_LINE * 4;
#ifdef _M_X64
            const unsigned START_OF_DWORDS = 17; // first 8 chars are the address, followed by a single blank
            const unsigned START_OF_CTEXT = 17 + (DWORDS_PER_LINE * (NIBBLES_PER_DWORD + 1)) - 1;
#else
            const unsigned START_OF_DWORDS = 9; // first 8 chars are the address, followed by a single blank
            const unsigned START_OF_CTEXT = 9 + (DWORDS_PER_LINE * (NIBBLES_PER_DWORD + 1)) - 1;
#endif
        };

        inline std::string multiply(std::string_view text, uint32_t n)
        {
            writer result;
            result.append_repeated(text, n);
            return result.as_string();
        }

        inline std::string multiply(const char c, uint32_t n)
        {
            writer result;
            result.append_repeated(c, n);
            return result.as_string();
        }


    }
}

