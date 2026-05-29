#pragma once
// tab_adult.h - ImGui converted Adult Block + Strict Protocols Tab (v3.0)
// Converted from GDI+ to Dear ImGui

#include <string>
#include <vector>

// ─── External state (defined in main.cpp or another module) ───
extern bool g_isPremiumUser;
extern bool g_showUpgradePopup;
extern bool g_parentForceAdultBlock;   // tab_family_link.cpp থেকে

// ─── Public API ───────────────────────────────────────────────
void DrawAdultBlockTab();              // Call inside ImGui window/tab
void LoadStrictSettings();
void AdultBlock_ApplyForSchedule(bool enable);
