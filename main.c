#include <windows.h>

#define GAME_NAME "YARP"

LRESULT CALLBACK main_window_proc(_In_ HWND window, _In_ UINT msg, _In_ WPARAM w_param, _In_ LPARAM l_param);
DWORD create_main_game_window(HINSTANCE instr);
BOOL game_running(void);

int WinMain(HINSTANCE inst, HINSTANCE prev_inst, PSTR cmd_line, INT cmd_show) {
    if (game_running()) { 
        MessageBoxA(NULL, "Another instance of this program is already running!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    if (create_main_game_window(inst) != ERROR_SUCCESS) {
        MessageBoxA(NULL, "Window registration failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 0;
}

LRESULT CALLBACK main_window_proc(HWND window, UINT msg, WPARAM w_param, LPARAM l_param) { 

    switch (msg) { 
    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    default: 
        return DefWindowProcA(window, msg, w_param, l_param); 
    } 
    return 0; 
} 

DWORD create_main_game_window(_In_ HINSTANCE inst) {
    WNDCLASSEXA window_class = {
        .cbSize = sizeof(WNDCLASSEXA),
        .style = 0,
        .lpfnWndProc = main_window_proc,
        .hInstance = inst,
        .hIcon = LoadIconA(NULL, IDI_APPLICATION),
        .hIconSm = LoadIconA(NULL, IDI_APPLICATION),
        .hCursor = LoadCursorA(NULL, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszClassName = GAME_NAME "_WINDOWCLASS",
    };
    if (!RegisterClassExA(&window_class)) return GetLastError();

    HWND window = CreateWindowExA(WS_EX_CLIENTEDGE, window_class.lpszClassName, GAME_NAME, (WS_OVERLAPPEDWINDOW | WS_VISIBLE), 
        CW_USEDEFAULT, CW_USEDEFAULT, 240, 120, NULL, NULL, inst, NULL);
    if (!window) return GetLastError();
    
    return ERROR_SUCCESS;
}

BOOL game_running(void) {
    CreateMutexA(NULL, FALSE, GAME_NAME "_MUTEX");
    return GetLastError() == ERROR_ALREADY_EXISTS;
}