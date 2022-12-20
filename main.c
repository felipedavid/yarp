#include <windows.h>

#define GAME_NAME "YARP"

LRESULT CALLBACK main_window_proc(_In_ HWND window, _In_ UINT msg, _In_ WPARAM w_param, _In_ LPARAM l_param);
DWORD create_main_game_window(_Out_ HWND *window, _In_ HINSTANCE inst);
BOOL instance_already_running(void);
void ProcessPlayerInput(HWND window);

BOOL game_running = FALSE;

int WinMain(HINSTANCE inst, HINSTANCE prev_inst, PSTR cmd_line, INT cmd_show) {
    if (instance_already_running()) { 
        MessageBoxA(NULL, "Another instance of this program is already running!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    HWND window;
    DWORD error_code = create_main_game_window(&window, inst);
    if (error_code != ERROR_SUCCESS) {
        MessageBoxA(NULL, "Window registration failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    game_running = TRUE;
    MSG msg;
    while (game_running) {
        while (PeekMessageA(&msg, window, 0, 0, PM_REMOVE)) DispatchMessage(&msg);
        ProcessPlayerInput(window);
        //RenderFrame();
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
        .hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszClassName = GAME_NAME "_WINDOWCLASS",
    };
    if (!RegisterClassExA(&window_class)) return GetLastError();

    *window = CreateWindowExA(WS_EX_CLIENTEDGE, window_class.lpszClassName, GAME_NAME, (WS_OVERLAPPEDWINDOW | WS_VISIBLE), 
        CW_USEDEFAULT, CW_USEDEFAULT, 240, 120, NULL, NULL, inst, NULL);
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