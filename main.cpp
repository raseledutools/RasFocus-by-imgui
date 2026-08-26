// main.cpp — RasFocus ImGui App Entry Point
// Win32 + DirectX 11 backend

#include <windows.h>
#include <d3d11.h>
#include <tchar.h>

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"

#include "tab_adult.h"
#include "tab_settings.h"
#include "tab_schedule.h"
#include "tab_family_link.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

// ── Premium feature gates (extern in tab_adult.h) ─────────────
bool g_isPremiumUser       = false;
bool g_showUpgradePopup    = false;
bool g_parentForceAdultBlock = false;

// ── DirectX globals ───────────────────────────────────────────
static ID3D11Device*            g_pd3dDevice           = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext    = nullptr;
static IDXGISwapChain*          g_pSwapChain           = nullptr;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

static bool CreateDeviceD3D(HWND hWnd);
static void CleanupDeviceD3D();
static void CreateRenderTarget();
static void CleanupRenderTarget();

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ── Window Procedure ──────────────────────────────────────────
static LRESULT WINAPI WndProc(HWND hWnd, UINT msg,
                               WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (g_pd3dDevice && wParam != SIZE_MINIMIZED) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(
                0, LOWORD(lParam), HIWORD(lParam),
                DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

// ── WinMain ───────────────────────────────────────────────────
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    WNDCLASSEXW wc = {
        sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L,
        GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
        L"RasFocusClass", nullptr
    };
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowW(
        wc.lpszClassName, L"RasFocus",
        WS_OVERLAPPEDWINDOW,
        100, 100, 1100, 700,
        nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    // ── ImGui setup ───────────────────────────────────────────
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // ── Style ─────────────────────────────────────────────────
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsDark();

    // Custom color overrides — teal accent theme
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg]          = ImVec4(0.08f, 0.09f, 0.11f, 1.00f);
    colors[ImGuiCol_ChildBg]           = ImVec4(0.10f, 0.11f, 0.14f, 1.00f);
    colors[ImGuiCol_PopupBg]           = ImVec4(0.10f, 0.11f, 0.14f, 1.00f);
    colors[ImGuiCol_Border]            = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    colors[ImGuiCol_FrameBg]           = ImVec4(0.13f, 0.15f, 0.19f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]    = ImVec4(0.17f, 0.20f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBgActive]     = ImVec4(0.12f, 0.14f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBg]           = ImVec4(0.06f, 0.07f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgActive]     = ImVec4(0.06f, 0.07f, 0.09f, 1.00f);
    colors[ImGuiCol_Tab]               = ImVec4(0.10f, 0.11f, 0.14f, 1.00f);
    colors[ImGuiCol_TabHovered]        = ImVec4(0.05f, 0.55f, 0.58f, 0.80f);
    colors[ImGuiCol_TabActive]         = ImVec4(0.05f, 0.60f, 0.63f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]= ImVec4(0.05f, 0.45f, 0.48f, 1.00f);
    colors[ImGuiCol_Header]            = ImVec4(0.05f, 0.55f, 0.58f, 0.40f);
    colors[ImGuiCol_HeaderHovered]     = ImVec4(0.05f, 0.55f, 0.58f, 0.70f);
    colors[ImGuiCol_HeaderActive]      = ImVec4(0.05f, 0.60f, 0.63f, 1.00f);
    colors[ImGuiCol_Button]            = ImVec4(0.05f, 0.50f, 0.53f, 1.00f);
    colors[ImGuiCol_ButtonHovered]     = ImVec4(0.07f, 0.62f, 0.66f, 1.00f);
    colors[ImGuiCol_ButtonActive]      = ImVec4(0.04f, 0.42f, 0.45f, 1.00f);
    colors[ImGuiCol_CheckMark]         = ImVec4(0.05f, 0.75f, 0.80f, 1.00f);
    colors[ImGuiCol_SliderGrab]        = ImVec4(0.05f, 0.65f, 0.70f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]  = ImVec4(0.05f, 0.80f, 0.85f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]  = ImVec4(0.05f, 0.55f, 0.58f, 0.80f);
    colors[ImGuiCol_SeparatorActive]   = ImVec4(0.05f, 0.65f, 0.68f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]    = ImVec4(0.05f, 0.55f, 0.58f, 0.35f);

    style.WindowRounding    = 8.0f;
    style.ChildRounding     = 6.0f;
    style.FrameRounding     = 5.0f;
    style.PopupRounding     = 6.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding      = 4.0f;
    style.TabRounding       = 5.0f;
    style.WindowPadding     = ImVec2(14.0f, 14.0f);
    style.FramePadding      = ImVec2(8.0f,  5.0f);
    style.ItemSpacing       = ImVec2(10.0f, 8.0f);
    style.ItemInnerSpacing  = ImVec2(6.0f,  6.0f);
    style.ScrollbarSize     = 12.0f;
    style.GrabMinSize       = 10.0f;
    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 0.0f;
    style.TabBorderSize     = 0.0f;

    // Font — Segoe UI if available
    io.Fonts->AddFontFromFileTTF(
        "C:\\Windows\\Fonts\\segoeui.ttf", 15.0f,
        nullptr, io.Fonts->GetGlyphRangesDefault());
    // Fallback: default ImGui font is used automatically if file missing

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // ── Main loop ─────────────────────────────────────────────
    const ImVec4 clearColor = ImVec4(0.07f, 0.08f, 0.10f, 1.00f);
    MSG msg{};

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // ── Full-screen docking window ────────────────────────
        {
            ImGuiViewport* vp = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(vp->Pos);
            ImGui::SetNextWindowSize(vp->Size);
            ImGui::SetNextWindowBgAlpha(1.0f);

            ImGuiWindowFlags wf =
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoMove       |
                ImGuiWindowFlags_NoResize     |
                ImGuiWindowFlags_NoBringToFrontOnFocus;

            ImGui::Begin("##Root", nullptr, wf);

            // ── App header ────────────────────────────────────
            ImGui::PushStyleColor(ImGuiCol_Text,
                ImVec4(0.05f, 0.75f, 0.80f, 1.00f));
            ImGui::SetWindowFontScale(1.25f);
            ImGui::Text("RasFocus");
            ImGui::SetWindowFontScale(1.00f);
            ImGui::PopStyleColor();

            ImGui::SameLine(0, 12);
            ImGui::TextDisabled("Focus & Parental Control");
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 160);

            // Premium badge
            if (g_isPremiumUser) {
                ImGui::PushStyleColor(ImGuiCol_Button,
                    ImVec4(0.85f, 0.65f, 0.05f, 1.00f));
                ImGui::SmallButton("  PREMIUM  ");
                ImGui::PopStyleColor();
            } else {
                if (ImGui::SmallButton("  Upgrade to Premium  "))
                    g_showUpgradePopup = true;
            }

            ImGui::Separator();
            ImGui::Spacing();

            // ── Tabs ──────────────────────────────────────────
            if (ImGui::BeginTabBar("MainTabs")) {

                if (ImGui::BeginTabItem(" Safe Browsing ")) {
                    ImGui::Spacing();
                    DrawAdultBlockTab();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem(" Schedule ")) {
                    ImGui::Spacing();
                    DrawScheduleTab();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem(" Family Link ")) {
                    ImGui::Spacing();
                    DrawFamilyLinkTab();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem(" Settings ")) {
                    ImGui::Spacing();
                    DrawSettingsTab();
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

            ImGui::End();
        }

        // ── Upgrade popup ─────────────────────────────────────
        if (g_showUpgradePopup) {
            ImGui::SetNextWindowSize(ImVec2(380, 0), ImGuiCond_Always);
            ImGui::SetNextWindowPos(
                ImGui::GetMainViewport()->GetCenter(),
                ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::OpenPopup("Upgrade to Premium");
        }
        if (ImGui::BeginPopupModal("Upgrade to Premium",
                nullptr,
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoMove))
        {
            ImGui::TextColored(
                ImVec4(0.95f, 0.75f, 0.10f, 1.0f),
                "Unlock Premium Features");
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::TextWrapped(
                "This feature requires a premium subscription.\n"
                "Premium includes: Strict Focus, Panic Mode,\n"
                "Family DNS, 24-Hour Lock, and more.");
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Button,
                ImVec4(0.95f, 0.75f, 0.10f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                ImVec4(1.0f, 0.85f, 0.20f, 1.0f));
            if (ImGui::Button("Upgrade Now", ImVec2(160, 36))) {
                // TODO: open payment/subscription flow
                g_showUpgradePopup = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopStyleColor(2);

            ImGui::SameLine();
            if (ImGui::Button("Maybe Later", ImVec2(120, 36))) {
                g_showUpgradePopup = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // ── Render ────────────────────────────────────────────
        ImGui::Render();
        const float cc[4] = {
            clearColor.x, clearColor.y,
            clearColor.z, clearColor.w };
        g_pd3dDeviceContext->OMSetRenderTargets(
            1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(
            g_mainRenderTargetView, cc);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(1, 0);
    }

    // ── Cleanup ───────────────────────────────────────────────
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}

// ── DirectX helpers ───────────────────────────────────────────
static bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount                        = 2;
    sd.BufferDesc.Width                   = 0;
    sd.BufferDesc.Height                  = 0;
    sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator   = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow                       = hWnd;
    sd.SampleDesc.Count                   = 1;
    sd.SampleDesc.Quality                 = 0;
    sd.Windowed                           = TRUE;
    sd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;

    UINT createFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };

    if (D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
            createFlags, featureLevels, 2,
            D3D11_SDK_VERSION, &sd,
            &g_pSwapChain, &g_pd3dDevice,
            &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

static void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain)       { g_pSwapChain->Release();       g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext){ g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice)       { g_pd3dDevice->Release();       g_pd3dDevice = nullptr; }
}

static void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (pBackBuffer) {
        g_pd3dDevice->CreateRenderTargetView(
            pBackBuffer, nullptr, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }
}

static void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) {
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = nullptr;
    }
}
