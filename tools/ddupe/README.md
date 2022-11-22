# ddupe - Data Horders Best Friend

This tool allows you to detect and delete duplicates. Sounds simple, and is most commonly used if you have lots of data that is exceeding your available space, and you simply want to get rid of at least the duplicates. 

Now, tons of tools like this exist "on the market", so what makes ddupe stand out? It uses a "performance trick": if you allow it, it will rename your files to include a MD5 hash in the filename. 

For example, let's assume you have two identical files `FOO.TXT` and `BAR.TXT`. If you run

	ddupe /RENAME /DELETE 

on the folde where these two files reside, it will 

1. rename `FOO.TXT` as `{E8CEE7CD9296A469F137FC6EF3C6264D}FOO.TXT`
1. delete `BAR.TXT`

Why is that useful? Because the next time you scan the folder, `ddupe` doesn't need to read the content of the file, it can simply read the checksum from the filename. This can be a great speedup if you have large sets of files.

Plus, I have rewritten this tool many times just to play with programming languages. The "specification" for this tool is easy enough, it is hard to improve for the spec it has so you can compare outcomes for things like performance and handling of edge-cases.

## Usage:

As a starting point use `/?` or `/HELP` to get a list of options:

	C:\Users\GersonKurz>ddupe /?
	$${\color{green}Detect(and possibly delete) duplicates - Version 5.0}$$
	Freeware written by NG Branch Technology GmbH (http://ng-branch-technology.com)

	USAGE: ddupe PATH {PATH} [OPTIONS]

	OPTIONS:

	  /RECURSIVE ... recurse subdirectories (default: false)
	  /RENAME ...... rename files to include hash (default: false)
	  /DELETE ...... delete duplicates (default: false)
	  /VERIFY ...... verify hashes encoded in filenames (default: false)

The first thing to note is that you can specify multiple paths. For example, this is a sure-fire method to list all files as duplicates:

	C:\Users\GersonKurz>ddupe . .

Which means: check "current directory" and compare it with "current directory", and of course everything in these two folders is identical. This option makes more sense if you have split up your data, like this:

	C:\Users\GersonKurz>ddupe C:\TEMP D:\TEMP

The options listed above should be pretty self-explanatory. Maybe only `/VERIFY` needs a comment: it is here so that you can verify checksums created with older versions of this tool...

## License

This tool is MIT licensed like pretty everything else in **ngbtools**.
