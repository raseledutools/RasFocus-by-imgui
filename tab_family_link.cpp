// tab_family_link.cpp — RasFocus Family Link Tab

#include "tab_family_link.h"
#include "tab_adult.h"
#include "imgui.h"

#include <vector>
#include <string>
#include <cstdio>

extern bool g_isPremiumUser;
extern bool g_showUpgradePopup;
extern bool g_parentForceAdultBlock;

static const ImVec4 ClrTeal  = {0.047f, 0.659f, 0.690f, 1.00f};
static const ImVec4 ClrGray  = {0.510f, 0.510f, 0.549f, 1.00f};
static const ImVec4 ClrRed   = {0.863f, 0.235f, 0.196f, 1.00f};
static const ImVec4 ClrGreen = {0.133f, 0.627f, 0.314f, 1.00f};
static const ImVec4 ClrGold  = {0.950f, 0.750f, 0.100f, 1.00f};

static char s_parentPassBuf[64] = {};
static bool s_parentLoggedIn    = false;
static bool s_showPassEntry     = false;
static bool s_passError         = false;

static bool s_forceAdultBlock   = false;
static bool s_forceStrictMode   = false;
static bool s_allowedAppsOnly   = false;

struct Child {
    char name[64];
    char device[64];
    bool online;
    int  screenTimeMins;
    int  blockedCount;
};

static std::vector<Child> s_children = {
    {"Child Device 1", "DESKTOP-RASFOCUS", false, 0, 0},
};

static bool ToggleSwitch(const char* id, bool* v)
{
    ImVec2 pos  = ImGui::GetCursorScreenPos();
    float  tw   = 36.0f, th = 18.0f, td = 14.0f;
    bool   hov  = ImGui::IsMouseHoveringRect(
        pos, {pos.x + tw, pos.y + th});
    bool   clicked = false;
    if (hov && ImGui::IsMouseClicked(0)) { *v = !*v; clicked = true; }
    auto* dl = ImGui::GetWindowDrawList();
    ImU32 tc = *v
        ? IM_COL32(12, 168, 176, 255)
        : IM_COL32(80,  85,  95, 255);
    dl->AddRectFilled(pos, {pos.x+tw, pos.y+th}, tc, 9.0f);
    float tx = *v ? pos.x + tw - td - 2.0f : pos.x + 2.0f;
    float ty = pos.y + (th - td) / 2.0f;
    dl->AddCircleFilled({tx+td/2, ty+td/2}, td/2,
        IM_COL32(255,255,255,255));
    ImGui::Dummy({tw, th});
    return clicked;
}

void DrawFamilyLinkTab()
{
    if (!g_isPremiumUser) {
        ImGui::Spacing();
        ImGui::SetCursorPosX(
            (ImGui::GetContentRegionAvail().x - 300) * 0.5f);
        ImGui::BeginChild("##flprem", {300, 130}, true);
        ImGui::TextColored(ClrGold, "Premium Feature");
        ImGui::Spacing();
        ImGui::TextWrapped(
            "Family Link lets parents remotely\n"
            "control and monitor child devices.");
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Button, ClrGold);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
            ImVec4(1.0f, 0.85f, 0.20f, 1.0f));
        if (ImGui::Button("Upgrade to Premium", {220, 32}))
            g_showUpgradePopup = true;
        ImGui::PopStyleColor(2);
        ImGui::EndChild();
        return;
    }

    // ── Parent login gate ─────────────────────────────────────
    if (!s_parentLoggedIn) {
        ImGui::Spacing();
        ImGui::SetCursorPosX(
            (ImGui::GetContentRegionAvail().x - 300) * 0.5f);
        ImGui::BeginChild("##fllogin", {300, 140}, true);
        ImGui::TextColored(ClrTeal, "Parent Login");
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextDisabled("Enter parent PIN to access controls.");
        ImGui::Spacing();
        ImGui::SetNextItemWidth(220);
        ImGui::InputText("##flpin", s_parentPassBuf,
            sizeof(s_parentPassBuf),
            ImGuiInputTextFlags_Password);
        if (s_passError)
            ImGui::TextColored(ClrRed, "Incorrect PIN");
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Button, ClrTeal);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
            ImVec4(0.07f, 0.75f, 0.80f, 1.0f));
        if (ImGui::Button("Login##fl", {100, 30})) {
            // TODO: verify against saved hash
            if (strlen(s_parentPassBuf) > 0) {
                s_parentLoggedIn = true;
                s_passError      = false;
            } else {
                s_passError = true;
            }
        }
        ImGui::PopStyleColor(2);
        ImGui::EndChild();
        return;
    }

    // ── Parent dashboard ──────────────────────────────────────
    ImGui::TextColored(ClrTeal, "Family Link");
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 80);
    ImGui::PushStyleColor(ImGuiCol_Button,
        ImVec4(0.20f, 0.22f, 0.27f, 1.0f));
    if (ImGui::SmallButton("  Logout  ")) {
        s_parentLoggedIn = false;
        memset(s_parentPassBuf, 0, sizeof(s_parentPassBuf));
    }
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Columns(2, "FLCols", false);
    ImGui::SetColumnWidth(0, 420);

    // ── Left: child list ──────────────────────────────────────
    ImGui::TextColored(ClrTeal, "Child Devices");
    ImGui::Spacing();

    for (int i = 0; i < (int)s_children.size(); i++) {
        Child& c = s_children[i];
        ImGui::BeginChild(
            ("##child" + std::to_string(i)).c_str(),
            {400, 80}, true, ImGuiWindowFlags_NoScrollbar);

        ImGui::TextColored(
            c.online ? ClrGreen : ClrGray,
            c.online ? "[Online] " : "[Offline]");
        ImGui::SameLine();
        ImGui::Text("%s", c.name);
        ImGui::SameLine();
        ImGui::TextDisabled("(%s)", c.device);

        ImGui::Spacing();
        ImGui::TextDisabled(
            "Screen time today: %dh %dm   Blocked: %d",
            c.screenTimeMins / 60,
            c.screenTimeMins % 60,
            c.blockedCount);

        ImGui::EndChild();
        ImGui::Spacing();
    }

    // ── Right: remote controls ────────────────────────────────
    ImGui::NextColumn();
    ImGui::TextColored(ClrTeal, "Remote Controls");
    ImGui::Spacing();

    ImGui::BeginChild("##flctrl", {280, 200}, true);

    // Force adult block toggle
    ImGui::Text("Force adult block");
    ImGui::SameLine(200);
    if (ToggleSwitch("##fab", &s_forceAdultBlock)) {
        g_parentForceAdultBlock = s_forceAdultBlock;
        AdultBlock_ApplyForSchedule(s_forceAdultBlock);
    }
    ImGui::TextDisabled("Overrides child's own settings.");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Force strict mode");
    ImGui::SameLine(200);
    ToggleSwitch("##fsm", &s_forceStrictMode);
    ImGui::TextDisabled("Blocks TaskMgr and Regedit.");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Allowed apps only");
    ImGui::SameLine(200);
    ToggleSwitch("##fao", &s_allowedAppsOnly);
    ImGui::TextDisabled("Block all unlisted apps.");

    ImGui::EndChild();

    ImGui::Spacing();

    // Emergency stop
    ImGui::PushStyleColor(ImGuiCol_Button, ClrRed);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
        ImVec4(0.95f, 0.35f, 0.30f, 1.0f));
    if (ImGui::Button("Emergency Stop All", {200, 36})) {
        s_forceAdultBlock  = false;
        s_forceStrictMode  = false;
        s_allowedAppsOnly  = false;
        g_parentForceAdultBlock = false;
        AdultBlock_ApplyForSchedule(false);
    }
    ImGui::PopStyleColor(2);
    ImGui::TextDisabled("Immediately lifts all parent-forced locks.");

    ImGui::Columns(1);
}
