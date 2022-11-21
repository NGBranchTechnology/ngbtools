#pragma once


#include <string>
#include <cassert>
#include <unordered_map>
#include "Windows.h"
#include <ngbtools/string.h>
#include <ngbtools/wstring.h>
#include <ngbtools/string_writer.h>

namespace ngbtools
{
	namespace environment_variables
	{
        bool get(std::string_view name, std::string& value)
        {
            std::vector<wchar_t> buffer;
            const auto wide_var_name{ string::encode_as_utf16(name) };
            const auto dwBytesNeeded{ GetEnvironmentVariableW(wide_var_name.c_str(), nullptr, 0) };
            if (dwBytesNeeded)
            {
                buffer.resize(dwBytesNeeded + 1);
                if (GetEnvironmentVariable(wide_var_name.c_str(), &buffer[0], (DWORD)buffer.size()))
                {
                    value = wstring::encode_as_utf8(&buffer[0]);
                    return true;
                }
            }
            return false;
        }

        class string_expander final
        {
            enum class RECORDING_PATTERN
            {
                PLAINTEXT,
                ENVIRONMENT_VARIABLE,
            };

        public:
            string_expander()
                :
                m_variables{nullptr},
                m_use_environment_variables{ true }
            {
            }

            string_expander(const std::unordered_map<std::string, std::string>& variables, bool use_environment_variables = true)
                :
                m_variables{ &variables },
                m_use_environment_variables{ use_environment_variables }
            {
            }

        private:
            string_expander(const string_expander&) = delete;
            string_expander& operator=(const string_expander&) = delete;
            string_expander(string_expander&&) = delete;
            string_expander& operator=(string_expander&&) = delete;

        public:

            std::string expand(std::string_view svinput) const
            {
                const char* text = svinput.data();

                if (string::is_empty(text))
                    return text;

                auto recording_pattern{ RECORDING_PATTERN::PLAINTEXT };
                bool isFirstCharAfterStartOfPattern = false;
                string::writer output;
                string::writer pattern;
                const char* copyOfOriginalTextParameter = text;

                for (; text;)
                {
                    const char c{ *(text++) };

                    if (!c)
                    {
                        if (recording_pattern == RECORDING_PATTERN::ENVIRONMENT_VARIABLE)
                        {
                            output.append('%');
                            output.append(pattern.as_string());
                        }
                        break;
                    }
                    else if (c == '%')
                    {
                        if (recording_pattern == RECORDING_PATTERN::PLAINTEXT) // start recording
                        {
                            recording_pattern = RECORDING_PATTERN::ENVIRONMENT_VARIABLE;
                            isFirstCharAfterStartOfPattern = true;
                        }
                        else
                        {
                            assert(recording_pattern == RECORDING_PATTERN::ENVIRONMENT_VARIABLE);
                            if (isFirstCharAfterStartOfPattern)
                            {
                                output.append('%');
                            }
                            else
                            {
                                const auto variable{ pattern.as_string() };
                                std::string replacement;
                                if (!locate_variable(variable, replacement))
                                {
                                    output.append("%");
                                    output.append(variable);
                                    output.append("%");
                                }
                                else
                                {
                                    output.append(replacement);
                                }
                                pattern.clear();
                            }
                            recording_pattern = RECORDING_PATTERN::PLAINTEXT;
                        }
                    }
                    else if (recording_pattern != RECORDING_PATTERN::PLAINTEXT)
                    {
                        pattern.append(c);
                        isFirstCharAfterStartOfPattern = false;
                    }
                    else
                    {
                        output.append(c);
                    }
                }
                return output.as_string();
            }

        private:
            bool locate_variable(std::string_view variable, std::string& result) const
            {
                const auto p = locate_variable(variable);
                if (p)
                {
                    result = p;
                    return true;
                }
                if (!m_use_environment_variables)
                    return false;

                if (environment_variables::get(variable, result))
                {
                    return true;
                }

                return false;
            }

            const char* locate_variable(std::string_view variable) const
            {
                if (m_variables)
                {
                    const std::string key{ variable };
                    const auto item{ m_variables->find(key) };
                    if (item != m_variables->end())
                    {
                        return item->second.c_str();
                    }
                }
                return nullptr;
            }

        private:
            const std::unordered_map<std::string, std::string>* m_variables;            
            const bool m_use_environment_variables;
        };

        inline auto expand(std::string_view text, const std::unordered_map<std::string, std::string>& variables, bool use_environment_variables = true)
        {
            return string_expander{ variables, true }.expand(text);
        }

        inline auto expand(std::string_view text)
        {
            return string_expander().expand(text);
        }
    }
}

