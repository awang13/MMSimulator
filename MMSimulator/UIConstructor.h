#pragma once

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include <unordered_map>

#include "D3DHelper.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"

class MatchMakingSystem;
class VirtualPlayer;
extern float COLOR_CLEAR[4];

// ImGui Rendering
void InitImGui(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
void CleanupImGui();
void RenderUI(MatchMakingSystem* mmSystem);

// MMSystem UI
void DrawControlPanel(MatchMakingSystem* mmSystem);
void DrawStatusPanel(const MatchMakingSystem* mmSystem);
void DrawLogPanel(const MatchMakingSystem* mmSystem);
void DrawLeaderBoard(const MatchMakingSystem* mmSystem);
void DrawMatchHistory(const MatchMakingSystem* mmSystem);

// Virtual Player Display
void DrawPlayerEntry(const VirtualPlayer& player, int drawIndex, std::unordered_map<int, bool>& headerStateMapping);
