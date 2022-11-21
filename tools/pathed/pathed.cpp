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
			m_variable_name{ "PATH" },
			m_remove_broken_folders{ false },
			m_remove_duplicates{ false },
			m_index_to_remove{ -1 }
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
			bool use_getenv_cmd = false;

			cmdline_args args{
				CONSOLE_GREEN "PATH var editor - Version 5.0" CONSOLE_STANDARD "\r\n"
				"Freeware written by NG Branch Technology GmbH (http://ng-branch-technology.com)",
				"pathed" };
			args.add_flag("MACHINE", use_machine_reg, "Read path from MACHINE");
			args.add_flag("USER", use_user_reg, "Read path from USER");
			args.add_flag("SLIM", m_remove_duplicates, "Remove duplicate entries");
			args.add_flag("FIX", m_remove_broken_folders, "Remove broken entries");
			args.add_flag("CMD", use_getenv_cmd, "Use GetEnvironmentVariable rather than the registry");
			args.add_option("ADD", variable_to_add, "Add entry to top of list");
			args.add_option("APPEND", variable_to_append, "Add entry to bottom of list");
			args.add_option("REMOVE", variable_to_remove, "Remove entry at position");
			args.add_option("ENV", m_variable_name, "variable name");

			if (!args.parse(argc, argv))
				return 20;

			m_wide_variable_name = string::encode_as_utf16(m_variable_name);
			std::wstring wstr_env_data;
			std::wstring wstr_machine_env_data;
			std::wstring wstr_user_env_data;

			if (!variable_to_remove.empty())
			{
				try
				{
					m_index_to_remove = std::stoi(variable_to_remove);
				}
				catch (...)
				{
					console::writeline(CONSOLE_RED "You must pass an integer index to the /REMOVE option" CONSOLE_STANDARD);
					return 1;
				}
			}

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
				wstr_machine_env_data = wstr_env_data;
				std::wstring upper_part;
				read_user_path(upper_part, true);
				if (!upper_part.empty())
				{
					wstr_user_env_data = upper_part;
					if (!wstr_env_data.empty() && !wstr_env_data.ends_with(L";"))
					{
						wstr_env_data += L";";
					}
					wstr_env_data += upper_part;
				}
			}
			bool apply_changes = false;
			int index = 0;

			if (!variable_to_add.empty())
			{
				add_individual_path_element(variable_to_add, index, apply_changes);
				apply_changes = true;
			}
			
			const auto tokens{ string::split(wstring::encode_as_utf8(wstr_env_data).c_str(), ";", false) };
			for (const auto token : tokens)
			{
				add_individual_path_element(token, index, apply_changes);
			}
			if (!variable_to_append.empty())
			{
				if (add_individual_path_element(variable_to_append, index, apply_changes))
				{
					// it can be that the caller specifies /SLIM and we're adding a dupe - so it would be removed -
					// so that means only if the path can be added we need to apply the change...
					apply_changes = true;
				}
			}

			if (apply_changes)
			{
				const auto combined{ string::join(m_new_variable_content, ";") };

				// now for the tricky part: we need to split those into MACHINE /  USER part
				if (use_machine_reg)
				{
					if (!write_machine_path(combined))
						return 10;
				}
				else if (use_user_reg)
				{
					if (!write_user_path(combined))
						return 10;
				}
				else 
				{
					console::formatline("\r\nset {}={}", m_variable_name, combined);
					console::writeline(CONSOLE_RED "Warning: in order to be able to write back changes, use this command with either /MACHINE or /USER" CONSOLE_STANDARD);
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

		bool add_individual_path_element(std::string_view path_element, int& index, bool& apply_changes)
		{
			if (index == m_index_to_remove)
			{
				console::formatline(CONSOLE_RED "{:02} {} <- requested to remove" CONSOLE_STANDARD, index, path_element);
				++index;
				apply_changes = true;
				return false;
			}

			bool success_of_operation = false;
			bool add_this_file = true;
			bool file_has_had_errors = false;
			const auto expanded_token{ environment_variables::expand(path_element) };

			if (!directory::exists(expanded_token))
			{
				console::formatline(CONSOLE_RED "{:02} {} <- does not exist" CONSOLE_STANDARD, index, path_element);
				file_has_had_errors = true;
				if (m_remove_broken_folders)
				{
					add_this_file = false;					
				}
			}
			const auto key{ string::lowercase(path_element) };
			const auto duplicate_item = m_duplicates.find(key);
			if (duplicate_item == m_duplicates.end())
			{
				if (!file_has_had_errors)
				{
					console::formatline("{:02} {}", index, path_element);
				}
				m_duplicates[key] = index;
			}
			else
			{
				if (m_remove_duplicates)
				{
					add_this_file = false;
					file_has_had_errors = true;
					console::formatline(CONSOLE_RED "{:02} {} <- duplicate of {:02} removed" CONSOLE_STANDARD, index, path_element, duplicate_item->second);
				}
				else
				{
					console::formatline(CONSOLE_BLUE "{:02} {} <- duplicate of {:02}" CONSOLE_STANDARD, index, path_element, duplicate_item->second);

				}
			}
			if (add_this_file)
			{
				m_new_variable_content.push_back(std::string{ path_element });
				success_of_operation = true;
				++index;

				if (!string::equals(expanded_token, path_element))
				{
					console::formatline(CONSOLE_GREEN "   {}" CONSOLE_STANDARD, expanded_token);
					
				}
			}
			else
			{
				apply_changes = true;
			}
			return success_of_operation;
		}

		bool read_machine_path(std::wstring& result, bool quiet)
		{
			return read_registry_path(result, HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment", quiet);
		}

		bool read_user_path(std::wstring& result, bool quiet)
		{
			return read_registry_path(result, HKEY_CURRENT_USER, L"Environment", quiet);
		}

		bool write_machine_path(std::string_view content)
		{
			return write_registry_path(content, HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment");
		}

		bool write_user_path(std::string_view content)
		{
			return write_registry_path(content, HKEY_CURRENT_USER, L"Environment");
		}

		bool read_registry_path(std::wstring& result, HKEY hkParent, std::wstring_view reg_path, bool quiet)
		{
			HKEY hKey = 0;
			auto hResult = ::RegOpenKeyExW(
				hkParent,
				reg_path.data(),
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

			std::vector<wchar_t> buffer;
			buffer.resize(0x8000);
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

		bool write_registry_path(std::string_view content, HKEY hkParent, std::wstring_view reg_path)
		{
			const auto wchar_content{ string::encode_as_utf16(content) };

			HKEY hKey = 0;
			auto hResult = ::RegOpenKeyExW(
				hkParent,
				reg_path.data(),
				0,
				KEY_WRITE|KEY_READ,
				&hKey);
			if (hResult != S_OK)
			{
				console::formatline("RegOpenKeyExW() failed with {}", GetLastError());
				return false;
			}
			std::vector<wchar_t> buffer;
			buffer.resize(0x8000);			
			DWORD dwType = 0;
			DWORD dwSize = (DWORD)(buffer.size() * sizeof(wchar_t));

			// we read the type first so that we can use it when updating the content
			hResult = ::RegQueryValueExW(
				hKey,
				m_wide_variable_name.c_str(),
				nullptr,
				&dwType,
				(LPBYTE)&buffer[0],
				&dwSize);
			if (hResult != S_OK)
			{
				console::formatline("RegQueryValueExW() failed with {}", GetLastError());
				::RegCloseKey(hKey);
				return false;
			}

			hResult = ::RegSetValueExW(
				hKey,
				m_wide_variable_name.c_str(),
				0,
				dwType,
				(const BYTE*)wchar_content.c_str(),
				(DWORD)(wchar_content.size() + 1) * sizeof(wchar_t)
			);
			if (hResult != S_OK)
			{
				console::formatline("RegSetValueExW() failed with {}", GetLastError());
				::RegCloseKey(hKey);
				return false;
			}
			::RegCloseKey(hKey);
			
			console::writeline("Setting changed, now broadcasting WM_SETTINGCHANGE");
			DWORD_PTR res{};
			SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)L"Environment", 0, 1000, &res);
			return true;
		}

	private:
		std::string m_variable_name;
		std::wstring m_wide_variable_name;
		std::unordered_map<std::string, int> m_duplicates;
		std::vector<std::string> m_new_variable_content;
		int m_index_to_remove;
		bool m_remove_broken_folders;
		bool m_remove_duplicates;
	};
}

int main(int argc, char* argv[])
{
	return ngbtools::pathed().run(argc, argv);
}
