#include "precomp.h"

namespace fs = std::filesystem;

#include <ngbtools/string.h>
#include <ngbtools/wstring.h>
#include <ngbtools/console.h>
#include <ngbtools/directory.h>
#include <ngbtools/cmdline_args.h>
#include <ngbtools/environment_variables.h>

namespace ngbtools
{

	class pathed final
	{
	public:
		pathed()
			:
			m_variable_name{ "PATH" }
		{
			
		}
		~pathed() = default;

		int run(int argc, char* argv[])
		{
			std::string variable_to_add;
			std::string variable_to_append;
			std::string variable_to_remove;
			bool use_machine_reg = false;
			bool use_user_reg = false;
			bool remove_duplicates = false;
			bool remove_broken_folders = false;
			bool use_getenv_cmd = false;

			cmdline_args args{
				CONSOLE_GREEN "PATH var editor - Version 5.0" CONSOLE_STANDARD "\r\n"
				"Freeware written by NG Branch Technology GmbH (http://ng-branch-technology.com)",
				"pathed" };
			args.add_flag("MACHINE", use_machine_reg, "Read path from MACHINE");
			args.add_flag("USER", use_user_reg, "Read path from USER");
			args.add_flag("SLIM", remove_duplicates, "Remove duplicate entries");
			args.add_flag("FIX", remove_broken_folders, "Remove broken entries");
			args.add_flag("CMD", use_getenv_cmd, "Use GetEnvironmentVariable rather than the registry");
			args.add_option("ADD", variable_to_add, "Add entry to top of list");
			args.add_option("APPEND", variable_to_append, "Add entry to bottom of list");
			args.add_option("REMOVE", variable_to_remove, "Remove entry at position");
			args.add_option("ENV", m_variable_name, "variable name");

			if (!args.parse(argc, argv))
				return 20;

			m_wide_variable_name = string::encode_as_utf16(m_variable_name);
			std::wstring wstr_env_data;

			if (use_machine_reg)
			{
				if (!read_machine_path(wstr_env_data, false))
					return 10;
			}
			else if (use_user_reg)
			{
				if (!read_user_path(wstr_env_data, false))
					return 10;
			}
			else if (use_getenv_cmd)
			{
				std::vector<wchar_t> buffer;
				buffer.resize(0x8000);
				if (!::GetEnvironmentVariableW(m_wide_variable_name.c_str(), &buffer[0], (DWORD)buffer.size()))
				{
					console::formatline("GetEnvironmentVariableW({}) failed with {}", m_variable_name, GetLastError());
					return 0;
				}
				wstr_env_data = &buffer[0];
			}
			else
			{
				read_machine_path(wstr_env_data, true);
				std::wstring upper_part;
				read_user_path(upper_part, true);
				if (!upper_part.empty())
				{
					if (!wstr_env_data.empty() && !wstr_env_data.ends_with(L";"))
					{
						wstr_env_data += L";";
					}
					wstr_env_data += upper_part;
				}
			}
			bool apply_changes = false;
			std::vector<std::string> new_path;
			if (!variable_to_add.empty())
			{
				new_path.push_back(variable_to_add);
				apply_changes = true;
			}
			std::unordered_map<std::string, int> duplicates;
			console::writeline("");
			const auto tokens{ string::split(wstring::encode_as_utf8(wstr_env_data).c_str(), ";", false) };
			int index = 0;
			for (const auto token : tokens)
			{
				bool add_this_file = true;
				bool file_has_had_errors = false;
				const auto expanded_token{ environment_variables::expand(token) };

				if (!directory::exists(expanded_token))
				{
					console::formatline(CONSOLE_RED "{:02} {} <- does not exist" CONSOLE_STANDARD, index, token);
					file_has_had_errors = true;
					if (remove_broken_folders)
					{
						add_this_file = false;
					}
				}
				const auto key{ string::lowercase(token) };
				const auto duplicate_item = duplicates.find(key);
				if (duplicate_item == duplicates.end())
				{
					if (!file_has_had_errors)
					{
						console::formatline("{:02} {}", index, token);
					}
					duplicates[key] = index;
				}
				else
				{
					console::formatline(CONSOLE_BLUE "{:02} {} <- duplicate at {:02}" CONSOLE_STANDARD, index, token, duplicate_item->second);
					if (remove_duplicates)
					{
						add_this_file = false;
						file_has_had_errors = true;
					}
				}
				if (add_this_file)
				{
					new_path.push_back(token);

					if (!string::equals(expanded_token, token))
					{
						console::formatline(CONSOLE_GREEN "   {}" CONSOLE_STANDARD, expanded_token);
					}
				}
				++index;
			}
			if (!variable_to_append.empty())
			{
				new_path.push_back(variable_to_append);
				apply_changes = true;
			}

			if (apply_changes)
			{
				const std::string combined{ string::join(new_path, ";") };

				if (!::SetEnvironmentVariableW(m_wide_variable_name.c_str(), string::encode_as_utf16(combined).c_str()))
				{
					console::formatline("SetEnvironmentVariableW() failed with {}", GetLastError());
				}
				else
				{
				}
			}

			return 0;
		}

	private:
		pathed(const pathed&) = delete;
		pathed& operator=(const pathed&) = delete;
		pathed(pathed&&) = delete;
		pathed& operator=(pathed&&) = delete;

	private:

		bool read_machine_path(std::wstring& result, bool quiet)
		{
			return read_registry_path(result, HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment", quiet);
		}

		bool read_user_path(std::wstring& result, bool quiet)
		{
			return read_registry_path(result, HKEY_CURRENT_USER, L"Environment", quiet);
		}

		bool read_registry_path(std::wstring& result, HKEY hkParent, const std::wstring& reg_path, bool quiet)
		{
			std::vector<wchar_t> buffer;
			buffer.resize(0x8000);

			HKEY hKey = 0;
			auto hResult = ::RegOpenKeyExW(
				hkParent,
				reg_path.c_str(),
				0,
				KEY_READ,
				&hKey);
			if (hResult != S_OK)
			{
				if (!quiet)
				{
					console::formatline("RegOpenKeyExW() failed with {}", GetLastError());
				}
				return false;
			}
			DWORD dwType = 0;
			DWORD dwSize = (DWORD)(buffer.size() * sizeof(wchar_t));
			hResult = ::RegQueryValueExW(
				hKey,
				m_wide_variable_name.c_str(),
				nullptr,
				&dwType,
				(LPBYTE)&buffer[0],
				&dwSize);
			if (hResult != S_OK)
			{
				if (!quiet)
				{
					console::formatline("RegQueryValueExW() failed with {}", GetLastError());
				}
				return false;
			}
			::RegCloseKey(hKey);
			result = &buffer[0];
			return true;
		}

	private:
		std::string m_variable_name;
		std::wstring m_wide_variable_name;
	};
}

int main(int argc, char* argv[])
{
	return ngbtools::pathed().run(argc, argv);
}
