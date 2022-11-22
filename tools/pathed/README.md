# pathed - PATH editor

Arguably the best tool in this collection, `pathed` can be used to edit the `PATH` variable and much else. In fact, it is one of the view tools that allow you to modify system/user `PATH` variable content without resorting to the Windows settings dialogs, from the comfort of your command line.

## Usage:
As a starting point use `/?` or `/HELP` to get a list of options:

	C:\temp>pathed /?
	PATH var editor - Version 5.0}
	Freeware written by NG Branch Technology GmbH (http://ng-branch-technology.com)

	USAGE: pathed [OPTIONS]

	OPTIONS:

	  /MACHINE ........ Read path from MACHINE (default: false)
	  /USER ........... Read path from USER (default: false)
	  /SLIM ........... Remove duplicate entries (default: false)
	  /FIX ............ Remove broken entries (default: false)
	  /CMD ............ Use GetEnvironmentVariable rather than the registry (default: false)
	  /ADD param ...... Add entry to top of list
	  /APPEND param ... Add entry to bottom of list
	  /REMOVE param ... Remove entry at position
	  /ENV param ...... variable name (default: PATH)

So, what to make of this? Well, let's run PATHED without any options first.

	C:\temp>pathed
	00 C:\WINDOWS\system32
	01 C:\WINDOWS
	02 C:\Python310-32
	03 C:\Python310-32\Scripts
	04 C:\Tools
	05 C:\Program Files\SciTE
	06 C:\Projects\flutter\bin
	07 C:\Program Files\Git\bin
	08 C:\WINDOWS\system32\WindowsPowerShell\v1.0
	09 C:\Program Files\Go\bin
	10 C:\Program Files\Docker\Docker\resources\bin
	11 C:\Program Files (x86)\p-nand-q.com\GTools
	12 C:\WINDOWS\System32\Wbem
	13 C:\WINDOWS\System32\WindowsPowerShell\v1.0\
	14 C:\WINDOWS\System32\OpenSSH\
	15 C:\Program Files (x86)\ProAKT\
	16 C:\Program Files\dotnet\
	17 C:\Program Files\CMake\bin
	18 C:\Program Files\MSI-Simplified
	19 C:\NGBT\NGBT_PUBLIC\ngbtools\bin\x64\Release
	20 %USERPROFILE%\AppData\Local\Microsoft\WindowsApps
	   C:\Users\GersonKurz\AppData\Local\Microsoft\WindowsApps}
	21 %USERPROFILE%\.dotnet\tools
	   C:\Users\GersonKurz\.dotnet\tools}
	22 C:\Users\GersonKurz\AppData\Local\Programs\Microsoft VS Code\bin
	23 C:\msys64\usr\bin
	24 C:\Program Files (x86)\WiX Toolset v3.11\bin
	25 C:\Users\GersonKurz\AppData\Roaming\npm
	26 C:\Program Files\Azure Data Studio\bin
	27 %USERPROFILE%\go\bin
	   C:\Users\GersonKurz\go\bin
	28 C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools

Interesting. This output is surely much nicer that simply typing `PATH` and visually scanning the data for the directory you're looking for. Also note that % variables are expanded and listed in green: the green bit is the underlying folder in your filesystem.

Next up, you can manipulate this list, with the obvious parameters 

	  /ADD param ...... Add entry to top of list
	  /APPEND param ... Add entry to bottom of list
	  /REMOVE param ... Remove entry at position

Example:

	C:\temp>pathed /ADD %CD%

will add the current folder to the top of the list. At the end of the tool, you'll see these lines:

	set PATH=C:\temp;C:\WINDOWS\system32;C:\WINDOWS;C:\Python310-32;C:\Python310-32\Scripts;C:\Tools;C:\Program Files\SciTE;C:\Projects\flutter\bin;C:\Program Files\Git\bin;C:\WINDOWS\system32\WindowsPowerShell\v1.0;C:\Program Files\Go\bin;C:\Program Files\Docker\Docker\resources\bin;C:\Program Files (x86)\p-nand-q.com\GTools;C:\WINDOWS\System32\Wbem;C:\WINDOWS\System32\WindowsPowerShell\v1.0\;C:\WINDOWS\System32\OpenSSH\;C:\Program Files (x86)\ProAKT\;C:\Program Files\dotnet\;C:\Program Files\CMake\bin;C:\Program Files\MSI-Simplified;C:\NGBT\NGBT_PUBLIC\ngbtools\bin\x64\Release;%USERPROFILE%\AppData\Local\Microsoft\WindowsApps;%USERPROFILE%\.dotnet\tools;C:\Users\GersonKurz\AppData\Local\Programs\Microsoft VS Code\bin;C:\msys64\usr\bin;C:\Program Files (x86)\WiX Toolset v3.11\bin;C:\Users\GersonKurz\AppData\Roaming\npm;C:\Program Files\Azure Data Studio\bin;%USERPROFILE%\go\bin;C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools
	Warning: in order to be able to write back changes, use this command with either /MACHINE or /USER

What does this last line mean? It tells you that while you can copy this commandline and execute it in terminal, a tool cannot really change the PATH of the running terminal. (Only SET can do that, and SET is not an executable, it is built into the terminal). 

So how is `pathed` useful then? Ah, here we come to the magic part:

	C:\temp>pathed /APPEND %CD% /USER
	00 %USERPROFILE%\AppData\Local\Microsoft\WindowsApps
	   C:\Users\GersonKurz\AppData\Local\Microsoft\WindowsApps}
	01 %USERPROFILE%\.dotnet\tools
	   C:\Users\GersonKurz\.dotnet\tools}
	02 C:\Users\GersonKurz\AppData\Local\Programs\Microsoft VS Code\bin
	03 C:\msys64\usr\bin
	04 C:\Program Files (x86)\WiX Toolset v3.11\bin
	05 C:\Users\GersonKurz\AppData\Roaming\npm
	06 C:\Program Files\Azure Data Studio\bin
	07 %USERPROFILE%\go\bin
	   C:\Users\GersonKurz\go\bin}
	08 C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools
	09 C:\temp
	Setting changed, now broadcasting WM_SETTINGCHANGE

If you now open another CMD - *for example, simply click on "New Tab" in the wonderful Windows Terminal* the PATH changes will have taken effect in that shell. The difference between /MACHINE and /USER should be obvious:

- `/MACHINE` is for system-wide PATH variable
- `/USER` is for the user-specific PATH variable

You can also use this tool to manipulate other environment variables such as `INCLUDE`:

	C:\temp>pathed /APPEND %CD% /USER /ENV INCLUDE

This means: *append the current directory to the `INCLUDE` variable of the current user*. 

If that wasn't enough, there are two more options:

	  /SLIM ........... Remove duplicate entries (default: false)
	  /FIX ............ Remove broken entries (default: false)

Duplicate entries should be clear; broken entries are non-existing folders

## License

This tool is MIT licensed like pretty everything else in **ngbtools**.
