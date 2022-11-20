#include "precomp.h"

namespace fs = std::filesystem;

#include <ngbtools/md5hash.h>
#include <ngbtools/string.h>
#include <ngbtools/wstring.h>
#include <ngbtools/console.h>
#include <ngbtools/string_writer.h>
#include <ngbtools/cmdline_args.h>

namespace ngbtools
{


	class ddupe final
	{
	public:
		ddupe()
			:
			m_recursive{ false },
			m_rename{ false },
			m_delete{ false },
			m_verify{ false },
			m_total_files{ 0 },
			m_total_folders{ 0 },
			m_total_bytes_used{ 0 },
			m_checksums_calculated{ 0 },
			m_bytes_used_for_checksums{ 0 },
			m_checksums_reused{ 0 },
			m_bytes_used_for_reused{ 0 },
			m_files_deleted{ 0 },
			m_bytes_used_for_deleted_files{ 0 }
		{
			m_buffer.resize(1024 * 1024 * 64);
		}
		~ddupe() = default;

		int run(int argc, wchar_t* argv[])
		{
			cmdline_args args{
				CONSOLE_GREEN "Detect(and possibly delete) duplicates - Version 5.0" CONSOLE_STANDARD "\r\n"
				"Freeware written by NG Branch Technology GmbH (http://ng-branch-technology.com)",
				"ddupe" };
			args.add_flag("RECURSIVE", m_recursive, "recurse subdirectories");
			args.add_flag("RENAME", m_rename, "rename files to include hash");
			args.add_flag("DELETE", m_delete, "delete duplicates");
			args.add_flag("VERIFY", m_verify, "verify hashes encoded in filenames");
			args.add_non_empty_path_list("PATH", m_pathlist);

			if (!args.parse(argc, argv))
				return 20;

			read_all_files();
			check_for_duplicates();
			return 0;
		}

	private:
		ddupe(const ddupe&) = delete;
		ddupe& operator=(const ddupe&) = delete;
		ddupe(ddupe&&) = delete;
		ddupe& operator=(ddupe&&) = delete;

	private:

		std::string read_checksum(const std::wstring& pathname, uintmax_t file_size)
		{
			MD5 checksum;
			++m_checksums_calculated;
			m_bytes_used_for_checksums += file_size;

			HANDLE hFile = ::CreateFileW(pathname.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			if(hFile != INVALID_HANDLE_VALUE)
			{
				while (file_size)
				{
					MD5::size_type bytes_to_read = (MD5::size_type)file_size;
					if (bytes_to_read > m_buffer.size())
						bytes_to_read = (MD5::size_type)m_buffer.size();
				
					DWORD bytesRead = 0;
					if (!::ReadFile(hFile, &m_buffer[0], bytes_to_read, &bytesRead, nullptr))
					{
						// warn with error
					}
					assert(bytesRead == bytes_to_read);
					checksum.update((const unsigned char*)&m_buffer[0], bytes_to_read);
					file_size -= bytes_to_read;
				}
				checksum.finalize();
			}
			return checksum.hexdigest();
		}

		bool check_for_duplicates_in(const fs::path& item, std::unordered_map<std::string, std::wstring>& checksum_lookup, uintmax_t file_size)
		{
			auto pathname{ item.wstring() };
			const auto filename{ item.filename().wstring() };

			++m_total_files;
			if ((m_total_files % 1000) == 0)
			{
				console::formatline("- checked total of {:L} files using {:L} bytes", m_total_files, m_total_bytes_used);
			}
			m_total_bytes_used += file_size;

			std::string checksum;
			if ((filename.size() > 34) && (filename[0] == L'{') && (filename[33] == L'}'))
			{
				m_checksums_reused += 1;
				m_bytes_used_for_reused += file_size;

				checksum = wstring::encode_as_utf8(filename.substr(1, 32));
				if (m_verify)
				{
					const auto actual_checksum = read_checksum(pathname, file_size);
					if (actual_checksum != checksum)
					{
						console::formatline(CONSOLE_RED "Checksum falsified for {}", wstring::encode_as_utf8(pathname));
						console::formatline("-----> is: {}", checksum);
						console::formatline("should be: {}" CONSOLE_STANDARD, actual_checksum);

						checksum = actual_checksum;
						if (m_rename)
						{
							const auto newname = string::encode_as_utf16("{" + actual_checksum + "}") + filename.substr(34);
							const auto newpath{ item.parent_path() / fs::path{newname} };
							console::formatline("renaming as : {}", newpath.string());
							if (!::MoveFileExW(
								pathname.c_str(),
								newpath.wstring().c_str(),
								0))
							{
								const auto hResult{ GetLastError() };
								if (hResult == ERROR_FILE_EXISTS)
								{
									console::formatline("Windows-Error {}: unable to rename file, because it already exists", hResult);
									if (m_delete)
									{
										if (!::DeleteFileW(pathname.c_str()))
										{
											console::formatline("Windows-Error {}: unable to delete {}", hResult, wstring::encode_as_utf8(item));
										}
										else
										{
											++m_files_deleted;
											m_bytes_used_for_deleted_files += file_size;
										}
									}
								}
								else
								{
									console::formatline("Windows-Error %ld: unable to rename file, aborting", hResult);
								}
								return false;
							}
						}
						// else we'll be using the actual checksum of course
					}
				}
			}
			else
			{
				checksum = read_checksum(pathname, file_size);
				if (checksum.empty())
				{
					console::formatline("Unable to read checksum for {}, assuming file is unique", wstring::encode_as_utf8(pathname));
					return false;
				}
				if (m_rename)
				{
					const auto newname = string::encode_as_utf16("{" + checksum + "}") + filename;
					const auto newpath{ item.parent_path() / fs::path{newname} };
					console::formatline("Renaming as: {}", newpath.string());

					if (!::MoveFileExW(
						pathname.c_str(),
						newpath.wstring().c_str(),
						0))
					{
						const auto hResult{ GetLastError() };
						if (hResult == ERROR_FILE_EXISTS)
						{
							console::formatline("Windows-Error {}: unable to rename file, because it already exists", hResult);
							if (m_delete)
							{
								if (!::DeleteFileW(pathname.c_str()))
								{
									console::formatline("Windows-Error {}: unable to delete {}", GetLastError(), wstring::encode_as_utf8(pathname));
									return false;
								}
								else
								{
									++m_files_deleted;
									m_bytes_used_for_deleted_files += file_size;
								}
							}
							return true;
						}
						else
						{
							console::formatline("Windows-Error %ld: unable to rename file, aborting", hResult);
							// ok, we'll continue using the old filename and assume the check is still valid
						}
					}
					else
					{
						// let's change the pathname so that we can use that going forward
						pathname = newpath;
					}
				}
			}

			const auto& existing_filename = checksum_lookup.find(checksum);
			if (existing_filename != checksum_lookup.end())
			{
				console::formatline(CONSOLE_RED "{} already exists as\r\n{}" CONSOLE_STANDARD,
					wstring::encode_as_utf8(pathname),
					wstring::encode_as_utf8(existing_filename->second));

				if (m_delete)
				{
					if (!::DeleteFileW(pathname.c_str()))
					{
						console::formatline("Windows-Error {}: unable to delete {}", GetLastError(), wstring::encode_as_utf8(pathname));
						return false;
					}
					else
					{
						++m_files_deleted;
						m_bytes_used_for_deleted_files += file_size;
					}
				}
			}
			else
			{
				checksum_lookup[checksum] = pathname;
			}
			return true;
		}

		void check_for_duplicates()
		{
			const auto start = std::chrono::high_resolution_clock::now();
			m_total_files = 0;
			for (const auto& lookup_item : m_lookup_by_size)
			{
				if (lookup_item.second->size() < 2)
					continue;

				std::unordered_map<std::string, std::wstring> checksum_lookup;
				for (const auto& item : *(lookup_item.second))
				{
					check_for_duplicates_in(item, checksum_lookup, lookup_item.first);
				}

			}

			const auto finish = std::chrono::high_resolution_clock::now();
			const auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
			console::formatline(CONSOLE_GREEN "Took {:L} microseconds to scan {:L} files for duplicates" CONSOLE_STANDARD, microseconds.count(), m_total_files);
			if(m_checksums_calculated)
				console::formatline("Calculated {:L} checksums using {:L} bytes", m_checksums_calculated, m_bytes_used_for_checksums);
			if (m_checksums_reused)
				console::formatline("Reused {:L} checksums using {:L} bytes", m_checksums_reused, m_bytes_used_for_reused);
			if (m_files_deleted)
				console::formatline("Deleted {:L} files using {:L} bytes", m_files_deleted, m_bytes_used_for_deleted_files);
		}

		void visit_dir_entry(const fs::directory_entry& dir_entry)
		{
			if (dir_entry.is_regular_file())
			{
				const auto file_size{ dir_entry.file_size() };
				const auto item{ m_lookup_by_size.find(file_size) };
				if (item == m_lookup_by_size.end())
				{
					m_lookup_by_size[file_size] = std::make_unique<files_with_same_size>(std::initializer_list<fs::path>{dir_entry.path()});
				}
				else
				{
					item->second->push_back(dir_entry.path());
				}
				++m_total_files;
				if ((m_total_files % 10000) == 0)
				{
					console::formatline("- {:L} files read...", m_total_files);
				}
			}
			else
			{
				++m_total_folders;
			}
		};

		void read_all_files()
		{
			const auto start = std::chrono::high_resolution_clock::now();
			for (const auto& path : m_pathlist)
			{
				if (m_recursive)
				{
					for (const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(path))
					{
						visit_dir_entry(dir_entry);
					}
				}
				else
				{
					for (const fs::directory_entry& dir_entry : fs::directory_iterator(path))
					{
						visit_dir_entry(dir_entry);
					}
				}
			}
			const auto finish = std::chrono::high_resolution_clock::now();
			const auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
			console::formatline(CONSOLE_GREEN "Took {:L} microseconds to scan {:L} files in {:L} folders" CONSOLE_STANDARD, microseconds.count(), m_total_files, m_total_folders);
		}

	private:
		bool m_recursive;
		bool m_rename;
		bool m_delete;
		bool m_verify;
		uintmax_t m_total_files;
		uintmax_t m_total_folders;
		uintmax_t m_total_bytes_used;
		uintmax_t m_checksums_calculated;
		uintmax_t m_checksums_reused;
		uintmax_t m_bytes_used_for_reused;
		uintmax_t m_bytes_used_for_checksums;
		uintmax_t m_files_deleted;
		uintmax_t m_bytes_used_for_deleted_files;
		std::vector<fs::path> m_pathlist;
		typedef std::vector<fs::path> files_with_same_size;
		std::unordered_map<uintmax_t, std::unique_ptr<files_with_same_size>> m_lookup_by_size;
		std::vector<char> m_buffer;
	};
}

int wmain(int argc, wchar_t* argv[])
{
	return ngbtools::ddupe().run(argc, argv);
}
