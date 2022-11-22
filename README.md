# `ngbtools` - Yet another modern C++ header-only tools library

As co-owner of NG Branch Technology I decided to open-source some of our modern C++ tools, to make them available to the wider public. So you are rightly wondering: *WHY*?

- `ngbtools` is but a small subset of our toolset. But it does *right* what it does, and that is no small feat. For example, console output that is fully unicode-aware is surprisingly hard. No, just compiling with `UNICODE` wont fix your problems, but `ngbtools` can do it. For details, see the section on *Console Output* below.

- `ngbtools` is header-only because I am tired of playing with `cmake`, [`DDS`](https://dds.pizza/) and the opaque [`vcpkg`](https://vcpkg.io/en/index.html). Let's face it: the C++ module system is a mess. I want to write a small tool, and I want to get on with it and not strugglying with hand-made scripts.

- `ngbtools` is meant to be included as a GIT submodule in projects that need common infrastructure, without having to reinvent every wheel (some wheel reinvention is I believe necessary for programmers)

- `ngbtools` does not have an example folder that is constantly outdated, but instead a set of tools that are actually useful.
    - `pathed` allows you to edit environment variables much more comfortable than anything built into Windows Terminal.
    - `su` allows you to run commands with superuser privileges.
    - `ddupe` ok, is more of an example for the data horders amongst us, but it has some (well one) innovative idea (at least that's what I think and that isn't much).

## General principles

- Readable code. Yes, there are some - very few - templates in here, but in general I put a prime on code readability, and I think the code in here is pretty obvious.
- Mostly correct code ;)
- We follow the [UTF8 Everywhere](http://utf8everywhere.org/) philosophy. Strings are encoded in utf8 byte encoding and converted to UTF16LE only when interfacing with the Win32 API.
- Header-Only.
- Windows-Only. Sorry, I am an unashamed Windows guy.
- Tested with C++20. If you are stuck with Visual Studio 6.0 please go play somewhere else.

## OK, let's see what you've got

# Mostly Correct Colorful C++ Console Output

This is where I invite you take a look at `ngbtools\console.h`. At its most basic level, the functions in `ngbtools::console` allow you to output utf-8 encoded strings to a Windows terminal *in the correct encoding*. This is [suprisingly](https://devblogs.microsoft.com/oldnewthing/20051027-37/?p=33593) [hard](https://stackoverflow.com/questions/388490/how-to-use-unicode-characters-in-windows-command-line).

Now, your naive approach would be something like this:

    std::cout << "std::cout << \"Hello Wörld!\"" << std::endl;
    puts("puts(Hello Wörld!)");

But it fails hard:

    C:\Users\GersonKurz>test_console_encoding
    std::cout << "Hello W÷rld!"
    puts(Hello W÷rld!)

Also if you say "I'm gonna fix it by prepending my strings with u8" you're out of luck: neither function accepts u8 strings. But `ngbtools` does:

    C:\Users\GersonKurz>test_console_encoding
    console::writeline(u8"Hello Wörld!")

So, two points here: 

- `console::writeline` expects utf-8 encoded strings. Since that is what we use internally in all our tooling, this normally means your regular `std::string` type strings. But if you want to *embedd* a utf-8 string in your sourcecode, you should use the `u8` prefix for *Visual Studio 2022* to do the right thing.

- `console::writeline` is pretty *obvious*.

Now, that's not all, you can add color:

    console::writeline(CONSOLE_FOREGROUND_YELLOW "Yellow" CONSOLE_FOREGROUND_RED "Red" CONSOLE_STANDARD "Normal");

Actually, this last one is a bit tricky. The default implementation uses [`SetConsoleTextAttribute`](https://learn.microsoft.com/en-us/windows/console/setconsoletextattribute). Newer consoles such as the really great [Windows Terminal](https://apps.microsoft.com/store/detail/windows-terminal/9N0DX20HK701) support virtual codes, which you can use as an alternative implementation by adding

    #define NGBTOOLS_USE_VIRTUAL_CONSOLE_COMMANDS

before you include `<ngbtools/console.h>`.

There are some additional utility functions I want to point out:

    console::formatline("TSV {}", 1860);

uses the magic [fmt::format](https://fmt.dev/latest/index.html) library, and

    bool console::write_unicode_output(std::wstring_view utf16_encoded_string)

allows you to write UTF16-LE encoded strings with the correct output encoding (in case you don't follow our advise to be [UTF8 Everywhere](http://utf8everywhere.org/)).


