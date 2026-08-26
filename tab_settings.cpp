// tab_settings.cpp — RasFocus Settings Tab

#include "tab_settings.h"
#include "tab_adult.h"
#include "imgui.h"

extern bool g_isPremiumUser;
extern bool g_showUpgradePopup;

static bool s_startWithWindows  = false;
static bool s_minimizeToTray    = true;
static bool s_showNotifications = true;
static bool s_darkMode          = true;
static int  s_language          = 0;  // 0=Bangla 1=English
static char s_pinBuf[32]        = {};
static bool s_showPin           = false;

static const ImVec4 ClrTeal    = {0.047f, 0.659f, 0.690f, 1.00f};
static const ImVec4 ClrGray    = {0.510f, 0.510f, 0.549f, 1.00f};
static const ImVec4 ClrRed     = {0.863f, 0.235f, 0.196f, 1.00f};
static const ImVec4 ClrGold    = {0.950f, 0.750f, 0.100f, 1.00f};

void DrawSettingsTab()
{
    ImGui::Columns(2, "SettingsCols", false);
    ImGui::SetColumnWidth(0, 380);

    // ── Left: General ─────────────────────────────────────────
    ImGui::TextColored(ClrTeal, "General");
    ImGui::Spacing();

    ImGui::Checkbox("Start with Windows",   &s_startWithWindows);
    ImGui::Checkbox("Minimize to tray",     &s_minimizeToTray);
    ImGui::Checkbox("Show notifications",   &s_showNotifications);
    ImGui::Checkbox("Dark mode",            &s_darkMode);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(ClrTeal, "Language");
    ImGui::Spacing();
    const char* langs[] = { "Bangla", "English" };
    ImGui::SetNextItemWidth(200);
    ImGui::Combo("##lang", &s_language, langs, 2);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(ClrTeal, "PIN Protection");
    ImGui::TextDisabled("Set a PIN to prevent unauthorized changes.");
    ImGui::Spacing();

    ImGui::SetNextItemWidth(180);
    ImGui::InputText(
        s_showPin ? "##pin" : "##pin",
        s_pinBuf, sizeof(s_pinBuf),
        s_showPin ? 0 : ImGuiInputTextFlags_Password);
    ImGui::SameLine();
    ImGui::Checkbox("Show", &s_showPin);
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ClrTeal);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
        ImVec4(0.07f, 0.75f, 0.80f, 1.0f));
    if (ImGui::Button("Save PIN")) { /* TODO: hash & save */ }
    ImGui::PopStyleColor(2);

    // ── Right: About / Premium ────────────────────────────────
    ImGui::NextColumn();

    ImGui::TextColored(ClrTeal, "About RasFocus");
    ImGui::Spacing();

    ImGui::BeginChild("##about", {320, 120}, true);
    ImGui::TextColored(ClrTeal, "RasFocus");
    ImGui::SameLine();
    ImGui::TextDisabled("v1.0.0");
    ImGui::Spacing();
    ImGui::TextWrapped(
        "Focus and parental control tool for Windows.\n"
        "Built with Dear ImGui + DirectX 11.");
    ImGui::Spacing();
    ImGui::TextDisabled("(c) 2025 RasEduTools");
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(ClrTeal, "Subscription");
    ImGui::Spacing();

    if (g_isPremiumUser) {
        ImGui::BeginChild("##sub", {320, 80}, true);
        ImGui::TextColored(ClrGold, "Premium Active");
        ImGui::Spacing();
        ImGui::TextDisabled("All features unlocked.");
        ImGui::EndChild();
    } else {
        ImGui::BeginChild("##sub", {320, 100}, true);
        ImGui::TextColored(ClrGray, "Free Plan");
        ImGui::Spacing();
        ImGui::TextWrapped("Upgrade to unlock Strict Focus,\nPanic Mode, Family DNS, and more.");
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Button, ClrGold);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
            ImVec4(1.0f, 0.85f, 0.20f, 1.0f));
        if (ImGui::Button("Upgrade to Premium", {200, 30}))
            g_showUpgradePopup = true;
        ImGui::PopStyleColor(2);
        ImGui::EndChild();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(ClrTeal, "Data");
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button, ClrRed);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
        ImVec4(0.95f, 0.35f, 0.30f, 1.0f));
    if (ImGui::Button("Reset all settings", {180, 30})) {
        // TODO: confirm dialog then wipe saved files
    }
    ImGui::PopStyleColor(2);

    ImGui::Columns(1);
}
