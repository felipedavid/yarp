#include <windows.h>
#include <stdio.h>

#include "defines.h"

#define GAME_NAME "YARP"
#define GAME_RES_WIDTH 384
#define GAME_RES_HEIGHT 240
#define GAME_BPP 32
#define GAME_DRAWING_AREA_MEM_SIZE (GAME_RES_WIDTH * GAME_RES_HEIGHT * (GAME_BPP/8))
#define TARGET_MS_PER_FRAME 16667

typedef struct Game_Bitmap {
    BITMAPINFO bitmap_info;
    u8 buf[GAME_DRAWING_AREA_MEM_SIZE];
} Game_Bitmap;

typedef u32 Pixel32; 

typedef struct Monitor {
    i32 top;
    i32 left;
    i32 right;
    i32 bottom;

    i32 width;
    i32 height;

    MONITORINFO monitor_info;
} Monitor;

typedef struct Perf_Data {
	u64 frames_rendered;
	f32 fps_average; // Aiming to 60 fps
	f32 raw_fps_average;

	i64 frequency;
} Perf_Data;

LRESULT CALLBACK main_window_proc(HWND window, UINT msg, WPARAM w_param, LPARAM l_param);
u32 create_game_window(HINSTANCE inst);
BOOL instance_already_running(void);
void process_player_input(void);
void render_frame(void);
u32 get_monitor_info(void);
u32 resize_game_window(void);

Monitor monitor;
HWND window;
Game_Bitmap back_buffer;
Perf_Data perf;
BOOL game_running;

i32 WinMain(HINSTANCE inst, HINSTANCE prev_inst, PSTR cmd_line, INT cmd_show) {
    if (instance_already_running()) { 
        MessageBoxA(NULL, "Another instance of this program is already running!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    if (create_game_window(inst) != ERROR_SUCCESS) {
        MessageBoxA(NULL, "Window registration failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    if (get_monitor_info() != ERROR_SUCCESS) {
        MessageBoxA(NULL, "Could not figure out some monitor information!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    if (resize_game_window() != ERROR_SUCCESS) {
        MessageBoxA(NULL, "Could not resize the window!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    back_buffer.bitmap_info.bmiHeader = (BITMAPINFOHEADER) {
        .biSize = sizeof(back_buffer.bitmap_info.bmiHeader),
        .biWidth = GAME_RES_WIDTH,
        .biHeight = GAME_RES_HEIGHT,
        .biBitCount = GAME_BPP,
        .biCompression = BI_RGB,
        .biPlanes = 1,
    };

	QueryPerformanceFrequency((LARGE_INTEGER *)&perf.frequency);
    
    game_running = TRUE;
    MSG msg;
	i64 frame_start, frame_end;
	i64 ms_per_frame, ms_per_frame_acc_raw = 0, ms_per_frame_acc_cooked = 0;
    while (game_running) {
		QueryPerformanceCounter((LARGE_INTEGER *)&frame_start);

        while (PeekMessageA(&msg, window, 0, 0, PM_REMOVE)) DispatchMessage(&msg);
        process_player_input();
        render_frame();

		QueryPerformanceCounter((LARGE_INTEGER *)&frame_end);
		ms_per_frame = frame_end - frame_start;
		ms_per_frame *= 1000000;
		ms_per_frame /= perf.frequency;
		perf.frames_rendered++;
		ms_per_frame_acc_raw += ms_per_frame;	

		while (ms_per_frame <= TARGET_MS_PER_FRAME) {
			Sleep(0);
			ms_per_frame = frame_end - frame_start;
			ms_per_frame *= 1000000;
			ms_per_frame /= perf.frequency;
			QueryPerformanceCounter((LARGE_INTEGER *)&frame_end);
		}
		ms_per_frame_acc_cooked += ms_per_frame;	

		if ((perf.frames_rendered % 100) == 0) {
			i64 avg_ms_per_frame_raw = ms_per_frame_acc_raw / 100;
			i64 avg_ms_per_frame_cooked = ms_per_frame_acc_cooked / 100;
			perf.fps_average = 1.0f / ((ms_per_frame_acc_cooked / 60) * 0.000001f);
			perf.raw_fps_average = 1.0f / ((ms_per_frame_acc_raw / 60) * 0.000001f);

			char buf[512];
			sprintf(buf, "Avg milliseconds/frame raw: %lld\tAvg FPS Cooked: %.01f\tAvg FPS Raw: %0.01f\n", avg_ms_per_frame_raw, perf.fps_average, perf.raw_fps_average);
			OutputDebugStringA(buf);

			ms_per_frame_acc_raw = 0;
			ms_per_frame_acc_cooked = 0;
		}
    }

    return 0;
}

LRESULT CALLBACK main_window_proc(HWND window, UINT msg, WPARAM w_param, LPARAM l_param) { 
    switch (msg) { 
    case WM_CLOSE:
        game_running = FALSE;
        PostQuitMessage(0);
        break;
    default: 
        return DefWindowProcA(window, msg, w_param, l_param); 
    } 
    return 0; 
} 

u32 create_game_window(HINSTANCE inst) {
    // Make sures we get the correct resolution of the screen, even when windows scaling is enabled 
    // Microsoft recommeds not to use this function for copatibility reasons. But we are going to anyways
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    WNDCLASSEXA window_class = {
        .cbSize = sizeof(WNDCLASSEXA),
        .style = 0,
        .lpfnWndProc = main_window_proc,
        .hInstance = inst,
        .hIcon = LoadIconA(NULL, IDI_APPLICATION),
        .hIconSm = LoadIconA(NULL, IDI_APPLICATION),
        .hCursor = LoadCursorA(NULL, IDC_ARROW),
        .hbrBackground = CreateSolidBrush(RGB(255, 0, 255)),
        .lpszClassName = GAME_NAME "_WINDOWCLASS",
    };
    if (!RegisterClassExA(&window_class)) return GetLastError();

    window = CreateWindowExA(0, window_class.lpszClassName, GAME_NAME, (WS_OVERLAPPEDWINDOW | WS_VISIBLE), 
			CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, inst, NULL);
    if (!window) return GetLastError();

    return ERROR_SUCCESS;
}

// Let us know if only one instance of the program is running at a given time
BOOL instance_already_running(void) {
    CreateMutexA(NULL, FALSE, GAME_NAME "_MUTEX");
    return GetLastError() == ERROR_ALREADY_EXISTS;
}

inline void process_player_input(void) {
#define Q_KEY_CODE 0x51
    if (GetAsyncKeyState(Q_KEY_CODE)) {
        SendMessageA(window, WM_CLOSE, 0, 0);
    }
}

inline void render_frame(void) {
	// Runs through every pixel in our back buffer and sets it to blue color 
	Pixel32 color = 0x7f;
	Pixel32 *pixel_ptr = (Pixel32 *) back_buffer.buf;
	void *buf_end = back_buffer.buf + GAME_DRAWING_AREA_MEM_SIZE;
	while (pixel_ptr < buf_end) {
		*pixel_ptr++ = color;
	}

    HDC device_context = GetDC(window);

    StretchDIBits(device_context, 0, 0, monitor.width, monitor.height, 0, 0, GAME_RES_WIDTH, 
		GAME_RES_HEIGHT, back_buffer.buf, &back_buffer.bitmap_info, DIB_RGB_COLORS, SRCCOPY);

    ReleaseDC(window, device_context);
}

u32 get_monitor_info(void) {
    MONITORINFO monitor_info = {sizeof(MONITORINFO)};
    if (!GetMonitorInfoA(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &monitor_info)) 
		return ERROR_MONITOR_NO_DESCRIPTOR;

    monitor.monitor_info = monitor_info;
    monitor.right = monitor_info.rcMonitor.right;
    monitor.left = monitor_info.rcMonitor.left;
    monitor.top = monitor_info.rcMonitor.top;
    monitor.bottom = monitor_info.rcMonitor.bottom;
    monitor.width = monitor_info.rcMonitor.right - monitor_info.rcMonitor.left;
    monitor.height = monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top;

    return ERROR_SUCCESS;
}

u32 resize_game_window(void) {
    if (!SetWindowLongPtrA(window, GWL_STYLE, ((WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_OVERLAPPEDWINDOW)))
        return GetLastError();        

    if (!SetWindowPos(window, HWND_TOP, monitor.left, monitor.top, monitor.width, monitor.height, SWP_FRAMECHANGED))
        return GetLastError();

    return ERROR_SUCCESS;
}
