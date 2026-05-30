#include "engine.h"

#if OS_Linux
# include "linux/os_linux.cpp"
#elif OS_Windows
# include "win32/os_windows.cpp"
#else
# error "platform implementation missing"
#endif

#define RGFW_IMPLEMENTATION
#define RGFW_OPENGL
#define RGFW_NO_X11_CURSOR
#include "vendor/rgfw.h"

global struct {
	bool initialized;

	Arena *persist;

} os_ctx;

funcdef string
os_string(OS os)
{
	return OS_STRINGS[(u64) os];
}

funcdef void
os_init()
{
	if (os_ctx.initialized) return;
	os_ctx.persist = arena_make(KB(4));
	os_ctx.initialized = true;
}

funcdef void
os_deinit()
{
	arena_delete(os_ctx.persist);
}

funcdef OS_Handle
os_open_window(string title)
{
	os_init();

	RGFW_glHints *hints = RGFW_getGlobalHints_OpenGL();
	hints->major = 4;
	hints->minor = 0;
	RGFW_setGlobalHints_OpenGL(hints);

	RGFW_window *win = alloc_struct(os_ctx.persist, RGFW_window);
	RGFW_createWindowPtr(
		(char *) title.raw, 0, 0, 1280, 800,
		RGFW_windowCenter | RGFW_windowAllowDND | RGFW_windowOpenGL,
		win
	);

	RGFW_window_makeCurrentContext_OpenGL(win);
	RGFW_window_swapInterval_OpenGL(win, 0);
	RGFW_window_setMinSize(win, 200, 200);

	OS_Handle window_handle = {
		(uintptr_t) win
	};

	return window_handle;
}

funcdef void
os_close_window(OS_Handle window)
{
	RGFW_window *win = (RGFW_window *) window.v;
	RGFW_window_closePtr(win);
}

funcdef bool
os_window_should_close(OS_Handle window)
{
	return RGFW_window_shouldClose((RGFW_window *) window.v);
}


funcdef OS_Input
os_prepare_frame(OS_Handle window)
{
	RGFW_window *win = (RGFW_window *) window.v;

	RGFW_window_swapBuffers_OpenGL(win);

	OS_Input input = {};
	RGFW_event event = {};
	while (RGFW_window_checkEvent(win, &event)) {
		switch (event.type) {
		default:
			break;
		}
	}

	return input;
}

funcdef void *
os_get_gl_proc_address()
{
	return (void *) RGFW_getProcAddress_OpenGL;
}


funcdef OS_TimeDuration
os_time_diff(OS_TimeStamp t0, OS_TimeStamp t1)
{
    u64 delta_ns = t1 - t0;

    OS_TimeDuration d;
    d.seconds      = (f64)delta_ns * 1e-9;
    d.milliseconds = (f64)delta_ns * 1e-6;
    d.microseconds = (f64)delta_ns * 1e-3;
    return d;
}
