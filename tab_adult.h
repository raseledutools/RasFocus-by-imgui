#pragma once
#ifndef TAB_ADULT_H
#define TAB_ADULT_H

// tab_adult.h - ImGui Converted Adult Block + Strict Protocols Tab (v3.0)
// Cross-Platform Compatible (Windows, Linux, macOS)
// Converted from GDI+ to Dear ImGui

#include <string>
#include <vector>

// ─── External State (Defined in main.cpp or another module) ───
extern bool g_isPremiumUser;
extern bool g_showUpgradePopup;
extern bool g_parentForceAdultBlock;   // tab_family_link.cpp থেকে কল করার জন্য

// ─── Public API ───────────────────────────────────────────────

// Call this inside your main ImGui rendering loop/tab
void DrawAdultBlockTab();              

// Loads strict settings from the hidden data file
void LoadStrictSettings();

// Applies the adult block rules automatically based on schedule
void AdultBlock_ApplyForSchedule(bool enable);

#endif // TAB_ADULT_H