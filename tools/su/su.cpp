#include "precomp.h"

namespace fs = std::filesystem;
#include "shlobj_core.h"

#include <functional>

#include <ngbtools/string.h>
#include <ngbtools/wstring.h>
#include <ngbtools/console.h>
#include <ngbtools/directory.h>
#include <ngbtools/cmdline_args.h>
#include <ngbtools/environment_variables.h>
#include <ngbtools/path.h>
#include <ngbtools/process.h>



int wmain(int argc, wchar_t* argv[])
{
	using namespace ngbtools;

	if (IsUserAnAdmin())
	{
		console::writeline(CONSOLE_FOREGROUND_RED "You are already already running as admin" CONSOLE_STANDARD);
		return 0;
	}

    std::string cmd_to_execute;
    string::writer cmd_params;

    // we do NOT use the cmdline_args, because this is a special case:
    // - normally we just pass all arguments directly to the calling app
    // - if you pass /? or /HELP, show help information
    // - otherwise just execute that string, by determining the first string is the app name.
    for (int i = 0; i < argc; ++i)
    {
        const auto arg{ wstring::encode_as_utf8(argv[i]) };
        if (i == 1)
        {
            if (string::equals(arg, "/?") || string::equals(arg, "/HELP"))
            {
                console::writeline(CONSOLE_FOREGROUND_GREEN "SU - Run programs as superuser on Windows - Version 5.0" CONSOLE_STANDARD "\r\n"
                    "Freeware written by NG Branch Technology GmbH (http://ng-branch-technology.com)\r\n"
                    "\r\n"
                    "USAGE: su [cmd [options]]\r\n");
                return 1;
            }
            cmd_to_execute = arg;
        }
        else if (i > 1)
        {
            if (i > 2)
            {
                cmd_params.append(" ");
            }
            if (arg.contains(' '))
            {
                cmd_params.append('"');
                cmd_params.append(arg);
                cmd_params.append('"');
            }
            else
            {
                cmd_params.append(arg);
            }
        }
    }

	if (cmd_to_execute.empty())
	{
        const auto executable{ path::combine(directory::system(), "cmd.exe") };
        const auto cwd{ fs::current_path().wstring() };
        const auto params{ fmt::format("/K \"cd /d {}\"", wstring::encode_as_utf8(cwd)) };

        if (process::run_as_admin(executable, params))
        {
            return 0;
        }
        return 10;
	}
	else
	{
        std::string executable;
        if (path::find_executable(cmd_to_execute, executable))
        {
            if (process::run_as_admin(executable, cmd_params.as_string()))
            {
                return 0;
            }
            return 10;
        }
        else
        {
            console::formatline(CONSOLE_FOREGROUND_RED "Unable to locate executable for {}." CONSOLE_STANDARD,
                cmd_to_execute);
            return 10;
        }
        
	}
}
