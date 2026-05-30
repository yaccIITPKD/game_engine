// ~gaureesh @NOTE: This file acts as our 'build system'
// it selectively includes and compiles all the required
// source files. We do this to avoid the use of build systems
// like CMake, premake etc. which complicates our project
// beyond what is required.


// thirdparty

#include "../base.cpp"
#include "../os.cpp"
#include "../arena.cpp"
#include "../string.cpp"
#include "../gfx.cpp"
#include "../main.cpp"

#include "../vendor/glad.c"


#if OS_Windows && !DEBUG_BUILD

// ~gaureesh @NOTE: windows has it's own main function that
// is meant for desktop applications. otherwise the
// application will have the standard output console visible
// always.

int WINAPI
WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int show_cmd)
{
	(void) instance;
	(void) prev_instance;
	(void) cmd_line;
	(void) show_cmd;

	int argc = 0;
	LPWSTR *wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
	slice<string> args = strings_from_cstrings(scratch(), argc, wargv);

	LocalFree(wargv);

	entry_point(args);
	return 0;
}

#else

int main(int argc, char **argv) {
	slice<string> args = strings_from_cstrings(scratch(), argc, argv);
	entry_point(args);

	return 0;
}

#endif

