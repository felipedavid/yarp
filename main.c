#include <windows.h>
#include <wingdi.h>

#define GAME_NAME "YARP"
#define GAME_RES_WIDTH 384
#define GAME_RES_HEIGHT 240
#define GAME_BPP 32
#define GAME_DRAWING_AREA_MEM_SIZE (GAME_RES_WIDTH * GAME_RES_HEIGHT * (GAME_BPP/8))

LRESULT CALLBACK main_window_proc(_In_ HWND window, _In_ UINT msg, _In_ WPARAM w_param, _In_ LPARAM l_param);
DWORD create_main_game_window(_Out_ HWND *window, _In_ HINSTANCE inst);
BOOL instance_already_running(void);
void ProcessPlayerInput(HWND window);
void RenderFrame(HWND window);
DWORD get_monitor_resolution(int *monitor_width, int *monitor_height, HWND window);

typedef struct Game_Bitmap {
    BITMAPINFO bitmap_info;
    BYTE buf[GAME_DRAWING_AREA_MEM_SIZE];
} Game_Bitmap;

typedef struct Pixel32 {
    BYTE blue;
    BYTE green;
    BYTE red;
    BYTE alpha;
} Pixel32;

BOOL game_running = FALSE;
Game_Bitmap back_buffer;

int WinMain(HINSTANCE inst, HINSTANCE prev_inst, PSTR cmd_line, INT cmd_show) {
    if (instance_already_running()) { 
        MessageBoxA(NULL, "Another instance of this program is already running!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    HWND window;
    if (create_main_game_window(&window, inst) != ERROR_SUCCESS) {
        MessageBoxA(NULL, "Window registration failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    int monitor_width, monitor_height;
    if (get_monitor_resolution(&monitor_width, &monitor_height, window)) {
        MessageBoxA(NULL, "Could not figure out monitor resolution!", "Error", MB_ICONEXCLAMATION | MB_OK);
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
    memset(back_buffer.buf, 255, sizeof(back_buffer.buf));
    
    game_running = TRUE;
    MSG msg;
    while (game_running) {
        while (PeekMessageA(&msg, window, 0, 0, PM_REMOVE)) DispatchMessage(&msg);
        ProcessPlayerInput(window);
        RenderFrame(window);
        Sleep(1);
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

DWORD create_main_game_window(_Out_ HWND *window, _In_ HINSTANCE inst) {
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

    *window = CreateWindowExA(WS_EX_CLIENTEDGE, window_class.lpszClassName, GAME_NAME, (WS_OVERLAPPEDWINDOW | WS_VISIBLE), 
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, inst, NULL);
    if (!*window) return GetLastError();

    
    return ERROR_SUCCESS;
}

// Let us know if only one instance of the program is running at a given time
BOOL instance_already_running(void) {
    CreateMutexA(NULL, FALSE, GAME_NAME "_MUTEX");
    return GetLastError() == ERROR_ALREADY_EXISTS;
}

void ProcessPlayerInput(HWND window) {
    if (GetAsyncKeyState(VK_ESCAPE)) {
        SendMessageA(window, WM_CLOSE, 0, 0);
    }
}

void RenderFrame(HWND window) {
    HDC device_context = GetDC(window);

    StretchDIBits(device_context, 0, 0, 100, 100, 0, 0, 100, 100, back_buffer.buf, &back_buffer.bitmap_info, DIB_RGB_COLORS, SRCCOPY);

    ReleaseDC(window, device_context);
}

DWORD get_monitor_resolution(int *monitor_width, int *monitor_height, HWND window) {
    MONITORINFO monitor_info = {sizeof(MONITORINFO)};
    if (!GetMonitorInfoA(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &monitor_info)) return ERROR_MONITOR_NO_DESCRIPTOR;

    *monitor_width = monitor_info.rcMonitor.right - monitor_info.rcMonitor.left;
    *monitor_height = monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top;

    return ERROR_SUCCESS;
}