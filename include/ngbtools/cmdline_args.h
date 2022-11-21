#pragma once


#include <string>
#include <vector>
#include <cassert>

#include <ngbtools/string.h>
#include <ngbtools/string_writer.h>
#include <ngbtools/wstring.h>

namespace ngbtools
{
	class cmdline_args final
	{
	public:
		cmdline_args(std::string_view header, std::string_view appname)
			:
			m_header{ header },
			m_appname{ appname },
			m_requires_path_list{ nullptr },
			m_pathlist_must_not_be_empty{ false },
			m_fail_args_loop{ false },
			m_record_this_option{ nullptr }
		{
		}

		~cmdline_args() = default;

	private:
		cmdline_args(const cmdline_args&) = delete;
		cmdline_args& operator=(const cmdline_args&) = delete;
		cmdline_args(cmdline_args&&) = delete;
		cmdline_args& operator=(cmdline_args&&) = delete;

	public:
		bool parse(int argc, wchar_t* argv[])
		{
			for (int arg_index = 1; argv[arg_index]; ++arg_index)
			{
				if (!parse_arg(wstring::encode_as_utf8(argv[arg_index])))
					return false;
			}
			return validate_required_options();
		}

		bool parse(int argc, char* argv[])
		{
			for (int arg_index = 1; argv[arg_index]; ++arg_index)
			{
				if (!parse_arg(argv[arg_index]))
					return false;
			}
			return validate_required_options();
		}

		cmdline_args& add_non_empty_path_list(std::string_view name, std::vector<fs::path>& path_list)
		{
			m_pathlist_name = name;
			m_pathlist_must_not_be_empty = true;
			m_requires_path_list = &path_list;
			return *this;
		}

		cmdline_args& add_flag(std::string_view name,
			bool& param,
			const std::string& description)
		{
			m_options.push_back({
				&param,
				nullptr,
				name.data(),
				description
				});
			return *this;
		}

		cmdline_args& add_option(std::string_view name,
			std::string& param,
			const std::string& description)
		{
			m_options.push_back({
				nullptr,
				&param,
				name.data(),
				description
				});

			return *this;
		}

		int show_help() const
		{
			console::write(get_help_string());
			return 20;
		}

		std::string get_help_string() const
		{
			// should use stringwriter instead and write in one go
			string::writer output;
			output.append(m_header);
			output.newline();
			output.newline();
			output.append("USAGE: ");
			output.append(m_appname);
			if (m_requires_path_list)
			{
				output.append(' ');
				output.append(m_pathlist_name);
				output.append(" {");
				output.append(m_pathlist_name);
				output.append("}");
			}
			if (!m_options.empty())
			{
				output.append(" [OPTIONS]");
			}
			output.newline();
			output.newline();
			if (!m_options.empty())
			{
				output.append("OPTIONS:");
				output.newline();
				output.newline();
				// determine nr. of spaces needed
				size_t max_opt_len = 0;

				for (const auto& option : m_options)
				{
					auto opt_len = string::length(option.m_name);
					if (option.m_str_option_ptr)
						opt_len += 6; // ' PARAM'

					if (opt_len > max_opt_len)
						max_opt_len = opt_len;
				}

				max_opt_len += 3; // at least three dots - hardcoded for now

				for (const auto& option : m_options)
				{
					output.append("  /");
					output.append(string::uppercase(option.m_name));

					auto opt_len = string::length(option.m_name);
					if (option.m_str_option_ptr)
					{
						output.append(" param");
						opt_len += 6; // ' PARAM'
					}
					const auto dots_needed = max_opt_len - opt_len;
					output.append(' ');
					output.append_repeated(".", dots_needed);
					output.append(' ');
					output.append(option.m_description);
					if (option.m_bool_flag_ptr)
					{
						output.append(" (default: false)");
					}
					else if (option.m_str_option_ptr && !option.m_str_option_ptr->empty())
					{
						output.append_formatted(" (default: {})", *option.m_str_option_ptr);
					}
					output.newline();
				}
			}
			return output.as_string();
		}

	private:
		bool validate_required_options() const
		{
			if (m_requires_path_list)
			{
				if (m_pathlist_must_not_be_empty && m_requires_path_list->empty())
				{
					show_help();
					return false;
				}
			}
			return true;
		}

		inline bool check_options(const std::string& argname)
		{
			if (string::equals(argname, "?") || string::equals_nocase(argname, "HELP"))
			{
				show_help();
				m_fail_args_loop = true;
				return false;
			}
			for (auto& option : m_options)
			{
				if (string::equals_nocase(argname, option.m_name))
				{
					if (option.m_bool_flag_ptr)
					{
						*option.m_bool_flag_ptr = true;
					}
					else if (option.m_str_option_ptr)
					{
						m_record_this_option = &option;
					}
					return true;
				}
			}
			return false;
		}

		bool parse_arg(const std::string& arg)
		{
			if (m_fail_args_loop)
				return false;

			if (m_record_this_option)
			{
				assert(m_record_this_option->m_str_option_ptr != nullptr);
				*m_record_this_option->m_str_option_ptr = arg;
				m_record_this_option = nullptr;
				return true;
			}

			if (arg.starts_with("/"))
			{
				if (check_options(arg.substr(1)))
					return true;
			}
			else if (arg.starts_with("--"))
			{
				if (check_options(arg.substr(2)))
					return true;
			}
			else if (m_requires_path_list)
			{
				m_requires_path_list->push_back(fs::path{ arg });
				return true;
			}
			if (!m_fail_args_loop)
			{
				console::formatline("Unknown argument {}", arg);
			}
			return false;
		}

	private:
		struct cmdline_options
		{
			bool* m_bool_flag_ptr;
			std::string* m_str_option_ptr;
			std::string m_name;
			std::string m_description;
		};

		const std::string m_header;
		const std::string m_appname;
		std::string m_pathlist_name;
		std::vector<fs::path>* m_requires_path_list;
		std::vector<cmdline_options> m_options;
		cmdline_options* m_record_this_option;
		bool m_pathlist_must_not_be_empty;
		bool m_fail_args_loop;
	};
}

