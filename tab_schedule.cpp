// tab_schedule.cpp — RasFocus Schedule Tab

#include "tab_schedule.h"
#include "tab_adult.h"
#include "imgui.h"

#include <vector>
#include <string>
#include <cstdio>

extern bool g_isPremiumUser;
extern bool g_showUpgradePopup;

static const ImVec4 ClrTeal  = {0.047f, 0.659f, 0.690f, 1.00f};
static const ImVec4 ClrRed   = {0.863f, 0.235f, 0.196f, 1.00f};
static const ImVec4 ClrGreen = {0.133f, 0.627f, 0.314f, 1.00f};
static const ImVec4 ClrGray  = {0.510f, 0.510f, 0.549f, 1.00f};

struct ScheduleEntry {
    char  label[64];
    int   startHour, startMin;
    int   endHour,   endMin;
    bool  days[7];   // Sun Mon Tue Wed Thu Fri Sat
    bool  blockAdult;
    bool  enabled;
};

static std::vector<ScheduleEntry> s_entries;
static bool s_showAddForm = false;
static ScheduleEntry s_newEntry = {
    "Study Time", 8, 0, 10, 0,
    {false,true,true,true,true,true,false},
    true, true
};

static const char* s_dayLabels[] = {
    "Sun","Mon","Tue","Wed","Thu","Fri","Sat"
};

static void DrawEntryForm(ScheduleEntry& e, bool isNew)
{
    ImGui::SetNextItemWidth(220);
    ImGui::InputText("Label##sch", e.label, sizeof(e.label));

    ImGui::Spacing();
    ImGui::Text("Start time");
    ImGui::SameLine(100);
    ImGui::SetNextItemWidth(60);
    ImGui::InputInt("##sh", &e.startHour, 1, 1);
    e.startHour = (e.startHour + 24) % 24;
    ImGui::SameLine(); ImGui::Text("h");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(60);
    ImGui::InputInt("##sm", &e.startMin, 5, 5);
    e.startMin = (e.startMin + 60) % 60;
    ImGui::SameLine(); ImGui::Text("m");

    ImGui::Text("End time  ");
    ImGui::SameLine(100);
    ImGui::SetNextItemWidth(60);
    ImGui::InputInt("##eh", &e.endHour, 1, 1);
    e.endHour = (e.endHour + 24) % 24;
    ImGui::SameLine(); ImGui::Text("h");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(60);
    ImGui::InputInt("##em", &e.endMin, 5, 5);
    e.endMin = (e.endMin + 60) % 60;
    ImGui::SameLine(); ImGui::Text("m");

    ImGui::Spacing();
    ImGui::Text("Days");
    ImGui::SameLine(100);
    for (int d = 0; d < 7; d++) {
        char id[8]; snprintf(id, sizeof(id), "%s##d%d", s_dayLabels[d], d);
        ImGui::Checkbox(id, &e.days[d]);
        if (d < 6) ImGui::SameLine();
    }

    ImGui::Spacing();
    ImGui::Checkbox("Block adult content during this time", &e.blockAdult);
    ImGui::Checkbox("Enabled", &e.enabled);
}

void DrawScheduleTab()
{
    // ── Header ────────────────────────────────────────────────
    ImGui::TextColored(ClrTeal, "Scheduled Blocks");
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 120);

    ImGui::PushStyleColor(ImGuiCol_Button, ClrTeal);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
        ImVec4(0.07f, 0.75f, 0.80f, 1.0f));
    if (ImGui::Button("+ Add Schedule")) {
        if (!g_isPremiumUser) {
            g_showUpgradePopup = true;
        } else {
            s_newEntry = {
                "New Block", 8, 0, 10, 0,
                {false,true,true,true,true,true,false},
                true, true
            };
            s_showAddForm = true;
        }
    }
    ImGui::PopStyleColor(2);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ── Entry list ────────────────────────────────────────────
    if (s_entries.empty()) {
        ImGui::Spacing();
        ImGui::SetCursorPosX(
            (ImGui::GetContentRegionAvail().x - 240) * 0.5f);
        ImGui::TextColored(ClrGray,
            "No schedules yet. Add one above.");
    }

    for (int i = 0; i < (int)s_entries.size(); i++) {
        ScheduleEntry& e = s_entries[i];

        ImVec4 cardBg = e.enabled
            ? ImVec4(0.10f, 0.14f, 0.16f, 1.0f)
            : ImVec4(0.10f, 0.11f, 0.13f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, cardBg);

        char cid[32]; snprintf(cid, sizeof(cid), "##sch%d", i);
        ImGui::BeginChild(cid, {0, 64}, true,
            ImGuiWindowFlags_NoScrollbar);

        // Status dot
        ImGui::TextColored(
            e.enabled ? ClrGreen : ClrGray,
            e.enabled ? "[ON]" : "[OFF]");
        ImGui::SameLine();

        ImGui::Text("%s", e.label);
        ImGui::SameLine();
        ImGui::TextDisabled(
            "%02d:%02d - %02d:%02d",
            e.startHour, e.startMin,
            e.endHour,   e.endMin);

        // Days
        ImGui::SameLine(320);
        for (int d = 0; d < 7; d++) {
            if (e.days[d])
                ImGui::TextColored(ClrTeal, "%s ", s_dayLabels[d]);
            else
                ImGui::TextDisabled("%s ", s_dayLabels[d]);
            if (d < 6) ImGui::SameLine();
        }

        // Adult block badge
        if (e.blockAdult) {
            ImGui::SameLine(580);
            ImGui::TextColored(ClrRed, "[Adult Block]");
        }

        // Delete
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 14);
        ImGui::PushStyleColor(ImGuiCol_Button,
            ImVec4(0,0,0,0));
        ImGui::PushStyleColor(ImGuiCol_Text, ClrRed);
        char xid[16]; snprintf(xid, sizeof(xid), "X##del%d", i);
        if (ImGui::SmallButton(xid)) {
            s_entries.erase(s_entries.begin() + i);
            ImGui::PopStyleColor(2);
            ImGui::EndChild();
            ImGui::PopStyleColor();
            break;
        }
        ImGui::PopStyleColor(2);

        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

    // ── Add form overlay ──────────────────────────────────────
    if (s_showAddForm) {
        ImGui::SetNextWindowSize(ImVec2(480, 0), ImGuiCond_Always);
        ImGui::SetNextWindowPos(
            ImGui::GetMainViewport()->GetCenter(),
            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::OpenPopup("Add Schedule");
    }
    if (ImGui::BeginPopupModal("Add Schedule", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoMove))
    {
        ImGui::TextColored(ClrTeal, "New scheduled block");
        ImGui::Separator();
        ImGui::Spacing();

        DrawEntryForm(s_newEntry, true);

        ImGui::Spacing();
        if (ImGui::Button("Cancel##asc", {110, 34})) {
            s_showAddForm = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ClrTeal);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
            ImVec4(0.07f, 0.75f, 0.80f, 1.0f));
        if (ImGui::Button("Save##asc", {110, 34})) {
            s_entries.push_back(s_newEntry);
            s_showAddForm = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(2);
        ImGui::EndPopup();
    }
}
