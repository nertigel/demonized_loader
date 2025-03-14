#include "nlohmann/json.hpp"

#include "loader.h"
#include "icons.h"
#include "user_interface.h"
#include "network.h"
#include "xorstr.h"
#include "dataset.h"

std::string loader_version = xorstr_("d1.0.2");

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    //up-to-date version check
    std::string utd_version = network::get_app_version();
    if (utd_version != loader_version) {
        MessageBoxA(nullptr,
            xorstr_("This version is outdated. Please update to the latest version."),
            loader_version.c_str(),
            MB_ICONERROR | MB_OK);
        return 0;
    }

    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandleA(0), 0,0,0, 0, (L"explorer"), 0 };
    RegisterClassEx(&wc);
    Window = CreateWindow(wc.lpszClassName, xorstr_(L"Internet Explorer"), WS_POPUP, 0, 0, 5, 5, 0, 0, wc.hInstance, 0);

    if (!CreateDeviceD3D(Window)) 
    {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(Window, SW_HIDE);
    UpdateWindow(Window);

    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; 
   
    static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 };
    ImFontConfig icons_config;

    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.OversampleH = 3;
    icons_config.OversampleV = 3;

    ImFontConfig CustomFont;
    CustomFont.FontDataOwnedByAtlas = false;
    font_gram_ttf = io.Fonts->AddFontFromMemoryTTF(const_cast<std::uint8_t*>(font_gram), sizeof(font_gram), 17.5, &CustomFont);
    font_gram2_ttf =  io.Fonts->AddFontFromMemoryTTF(const_cast<std::uint8_t*>(font_gram), sizeof(font_gram), 18.5, &CustomFont);
    font_gram3_ttf = io.Fonts->AddFontFromMemoryTTF(const_cast<std::uint8_t*>(font_gram_bold), sizeof(font_gram_bold), 20.5, &CustomFont);
    font_gram_bold2_ttf = io.Fonts->AddFontFromMemoryTTF(const_cast<std::uint8_t*>(font_gram_bold), sizeof(font_gram_bold), 80.5, &CustomFont);
    font_gram_bold_ttf = io.Fonts->AddFontFromMemoryTTF(const_cast<std::uint8_t*>(font_gram_bold), sizeof(font_gram_bold), 32.5, &CustomFont);

    Font = io.Fonts->AddFontFromMemoryCompressedTTF(font_awesome_data, font_awesome_size, 22.5f, &icons_config, icons_ranges);
    io.Fonts->AddFontDefault();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplWin32_Init(Window);
    ImGui_ImplDX9_Init(g_pd3dDevice);
    MSG msg;

    memset(&msg, 0, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        user_interface::render_ui();

        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
            ResetDevice();
        }
        if (!Enabled) {
            msg.message = WM_QUIT;
        }
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(Window);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {

            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}