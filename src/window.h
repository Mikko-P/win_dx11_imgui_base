#pragma once
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include <iostream>
#include <Windows.h>
#include <d3d11.h>
#include <array>


LRESULT CALLBACK Wndproc(
    HWND handle,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);

struct WindowSize
{
    int height;
    int width;
};

class Window
{
public:
    Window(HINSTANCE hInstance, WindowSize windowSize, int nCmdShow);
    ~Window();

    void mainLoop() const;
    void resize(int newWidth, int newHeight);
private:
    void initWindowClass();
    void createWindow();

    void initDirectX();
    
    void initImgui() const;
    void imguiStart() const noexcept;
    void imguiRender() const noexcept;
    void imguiEnd() const noexcept;
    
    void render() const;

    HWND handle{ nullptr };
    WNDCLASSEXW wc{ 0 };
    HINSTANCE hInstance{ nullptr };

    ID3D11Device* device{ nullptr };
    ID3D11DeviceContext* deviceContext{ nullptr };
    IDXGISwapChain* swapChain{ nullptr };
    ID3D11RenderTargetView* renderTargetView{ nullptr };
    WindowSize windowSize{ 0 };
    int nCmdShow;
};

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK Wndproc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);