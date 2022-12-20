#include <windows.h>

LRESULT CALLBACK main_window_proc(HWND window, UINT msg, WPARAM w_param, LPARAM l_param);

int WinMain(HINSTANCE inst, HINSTANCE prev_inst, PSTR cmd_line, INT cmd_show) {
    WNDCLASSEXA window_class = {
        .cbSize = sizeof(WNDCLASSEXA),
        .style = 0,
        .lpfnWndProc = main_window_proc,
        .hInstance = inst,
        .hIcon = LoadIconA(NULL, IDI_APPLICATION),
        .hCursor = LoadCursorA(NULL, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszClassName = "YARP",
        .hIconSm = LoadIconA(NULL, IDI_APPLICATION),
    };
    if (!RegisterClassExA(&window_class)) {
        MessageBoxA(NULL, "Window registration failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    HWND window = CreateWindowExA(WS_EX_CLIENTEDGE, window_class.lpszClassName, "YARP", WS_OVERLAPPEDWINDOW, 
        CW_USEDEFAULT, CW_USEDEFAULT, 240, 120, NULL, NULL, inst, NULL);
    if (!window) {
        MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    ShowWindow(window, TRUE);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 0;
}

LRESULT CALLBACK main_window_proc(HWND window, UINT msg, WPARAM w_param, LPARAM l_param) { 
    switch (msg) { 
        case WM_CREATE: 
            return 0; 
 
        case WM_PAINT: 
            return 0; 
 
        case WM_SIZE: 
            return 0; 
 
        case WM_DESTROY: 
            return 0; 
 
        default: 
            return DefWindowProcA(window, msg, w_param, l_param); 
    } 
    return 0; 
} 