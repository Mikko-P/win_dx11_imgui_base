#include "window.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include <iostream>
#include <Windows.h>
#include <d3d11.h>
#include <array>

Window::Window(HINSTANCE hInstance, WindowSize windowSize, int nCmdShow) : hInstance(hInstance), windowSize(windowSize), nCmdShow(nCmdShow)
{
    initWindowClass();
    createWindow();
    initDirectX();
    initImgui();
}

Window::~Window()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (renderTargetView) renderTargetView->Release();
    if (swapChain) swapChain->Release();
    if (deviceContext) deviceContext->Release();
    if (device) device->Release();

    renderTargetView = nullptr;
    swapChain = nullptr;
    deviceContext = nullptr;
    device = nullptr;

    if (wc.lpszClassName) UnregisterClassW(wc.lpszClassName, hInstance);
}

void Window::mainLoop() const
{
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);

        DispatchMessageW(&msg);

        render();
    }
}

void Window::resize(int newWidth, int newHeight)
{
    if (deviceContext) deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
    if (renderTargetView) renderTargetView->Release();

    windowSize.width = newWidth;
    windowSize.height = newHeight;

    swapChain->ResizeBuffers(0, newWidth, newHeight, DXGI_FORMAT_UNKNOWN, 0);

    ID3D11Texture2D* backBuffer = nullptr;
    swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));

    device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
    backBuffer->Release();

    deviceContext->OMSetRenderTargets(1, &renderTargetView, nullptr);

    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(newWidth);
    viewport.Height = static_cast<float>(newHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;

    deviceContext->RSSetViewports(1, &viewport);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(static_cast<float>(newWidth), static_cast<float>(newHeight));
    render();
}

void Window::initWindowClass()
{
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &Wndproc;
    wc.cbClsExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = nullptr;
    wc.hCursor = nullptr;
    wc.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
    wc.lpszClassName = L"Class";

    RegisterClassExW(&wc);
}

void Window::createWindow()
{
    RECT rect = { 0, 0, windowSize.width, windowSize.height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    int adjustedWidth = rect.right - rect.left;
    int adjustedHeight = rect.bottom - rect.top;
    constexpr int windowStartingPosX = 300;
    constexpr int windowStartingPosY = 300;

    handle = CreateWindowExW(
        WS_EX_OVERLAPPEDWINDOW,
        L"Class",
        L"Window",
        WS_OVERLAPPEDWINDOW,
        windowStartingPosX,
        windowStartingPosY,
        adjustedWidth,
        adjustedHeight,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    ShowWindow(handle, nCmdShow);
    SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
}

void Window::initDirectX()
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc{};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.Width = windowSize.width;
    swapChainDesc.BufferDesc.Height = windowSize.height;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = handle;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &swapChain,
        &device,
        &featureLevel,
        &deviceContext
    );

    ID3D11Texture2D* backBuffer = nullptr;
    swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
    backBuffer->Release();
    deviceContext->OMSetRenderTargets(1, &renderTargetView, nullptr);

    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(windowSize.width);
    viewport.Height = static_cast<float>(windowSize.height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;

    deviceContext->RSSetViewports(1, &viewport);
}

void Window::initImgui() const
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui_ImplWin32_Init(handle);
    ImGui_ImplDX11_Init(device, deviceContext);
}

void Window::imguiStart() const noexcept
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void Window::imguiRender() const noexcept
{
    static bool showDemoWindow{ false };
    ImGui::Begin("Main");
    ImGui::Checkbox("Show demo window", &showDemoWindow);
    ImGui::End();

    if (showDemoWindow)
        ImGui::ShowDemoWindow();
}

void Window::imguiEnd() const noexcept
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void Window::render() const
{
    imguiStart();
    imguiRender();
    constexpr std::array<float, 4> clearColor{ 0.1f, 0.1f, 0.1f, 1.0f };
    deviceContext->ClearRenderTargetView(renderTargetView, clearColor.data());
    imguiEnd();
    swapChain->Present(1, 0);
}

LRESULT CALLBACK Wndproc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(handle, message, wParam, lParam))
        return true;

    switch (message)
    {
    case WM_SIZE:
    {
        if (wParam != SIZE_MINIMIZED)
        {
            auto width = LOWORD(lParam);
            auto height = HIWORD(lParam);

            auto* window = reinterpret_cast<Window*>(GetWindowLongPtr(handle, GWLP_USERDATA));
            if (window)
            {
                window->resize(width, height);
            }
        }
        return 0;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    } break;
    }

    return DefWindowProc(handle, message, wParam, lParam);
}