#include "UIConstructor.h"

#include <sstream>

#include "MatchMaking/MatchMakingSystem.h"
#include "MatchMaking/MM_Elements.h"
#include "MatchMaking/Utility.h"

float COLOR_CLEAR[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
std::unordered_map<int, bool> playerListHeaderState;
std::unordered_map<int, bool> matchListHeaderState;
int numOfPlayersToAdd = 5;

void InitImGui(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, deviceContext);
}

// Runs the MM simulator on update and renders UI
void RenderUI(MatchMakingSystem* mmSystem)
{
    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if(0) ImGui::ShowDemoWindow();
    
    // Draw MMSystem UIs
    DrawControlPanel(mmSystem);
    DrawStatusPanel(mmSystem);
    DrawLogPanel(mmSystem);
    DrawLeaderBoard(mmSystem);
    DrawMatchHistory(mmSystem);
    
    // Render
    ImGui::Render();
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, COLOR_CLEAR);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    g_pSwapChain->Present(1, 0);
}

void DrawControlPanel(MatchMakingSystem* mmSystem)
{
    ImGui::Begin("Controls");

    // Create Players button
    if (ImGui::Button("Create Player"))
    {
        for(int i = 0; i < numOfPlayersToAdd; ++i)
        {
            mmSystem->CreatePlayer();
        }
    }
    ImGui::SameLine();
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputInt("##numPlayerToAdd", &numOfPlayersToAdd);
    ImGui::PopItemWidth();
    
    FMatchSetting Setting = mmSystem->GetMatchSetting();
    ImGui::Text("# Teams/Match: ");
    ImGui::InputInt("##numTeams", &Setting.numTeams);
    ImGui::Text("# Players/Team: ");
    ImGui::InputInt("##teamSize", &Setting.teamSize);
    ImGui::Text("# Match/Cycle: ");
    ImGui::InputInt("##matchPerCycle", &Setting.matchesPerCycle);
    mmSystem->SetMatchSetting(Setting);
    
    ImGui::End();
}

void DrawStatusPanel(const MatchMakingSystem* mmSystem)
{
    ImGui::Begin("Current Status");
    ImGui::Text("# of ongoing matches: %d", static_cast<int>(mmSystem->GetOngoingMatchIds().size()));

    ImGui::NewLine();
    
    std::unordered_map<int, VirtualPlayer> allPlayers = mmSystem->GetAllPlayers();
    if (allPlayers.empty())
    {
        ImGui::Text("No players available.");
    }
    else
    {
        ImGui::Text("Total players: %d", static_cast<int>(allPlayers.size()));
        ImGui::Text("Average Queue time: %.2f", mmSystem->GetAvgQueueTime());
        for (int i = 0; i < static_cast<int>(allPlayers.size()); ++i)
        {
            if (playerListHeaderState.find(i) == playerListHeaderState.end())
            {
                playerListHeaderState[i] = false;
            }

            DrawPlayerEntry(allPlayers.find(i)->second, i, playerListHeaderState);
        }
    }
    ImGui::End();

}

void DrawLogPanel(const MatchMakingSystem* mmSystem)
{
    ImGui::Begin("Match Log - System Online...");

    ImGui::Text("Player Log");
    if (ImGui::BeginChild("PlayerLog", ImVec2(0, 80), true))
    {
        for (const auto& log : mmSystem->GetPlayerLog())
        {
            ImGui::TextWrapped("%s", log.c_str());
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 5.0f)
        {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndChild();

    ImGui::NewLine();
    
    ImGui::Text("Match Log");
    if (ImGui::BeginChild("MatchLog", ImVec2(0,80), true))
    {
        for (const auto& log : mmSystem->GetMatchLog())
        {
            ImGui::TextWrapped("%s", log.c_str());
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 5.0f)
        {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndChild();
    ImGui::End();
}

void DrawLeaderBoard(const MatchMakingSystem* mmSystem)
{
    ImGui::Begin("===== Leader Board =====");

    std::vector<VirtualPlayer*> topPlayers = mmSystem->GetTopPlayersByWinRate();
    if (topPlayers.empty())
    {
        ImGui::TextWrapped("Waiting for players and matches...");
    }
    else
    {
        for (int i = 0; i < static_cast<int>(topPlayers.size()); ++i)
        {
            ImGui::Text("Rank %d: [%d], (%.2f)", i + 1, topPlayers[i]->GetId(), topPlayers[i]->GetWinRate() * 100.0f);
        }
    }

    ImGui::End();
}

void DrawMatchHistory(const MatchMakingSystem* mmSystem)
{
    ImGui::Begin("Match History");

    std::unordered_map<int, FMatch> allMatches = mmSystem->GetAllMatches();
    if (allMatches.empty())
    {
        ImGui::Text("No matches available.");
    }
    else
    {
        for (int i = 0; i < static_cast<int>(allMatches.size()); ++i)
        {
            if (matchListHeaderState.find(i) == matchListHeaderState.end())
            {
                matchListHeaderState[i] = false;
            }

            ImGui::SetNextItemOpen(matchListHeaderState[i]);

            FMatch match = allMatches.find(i)->second;
            if (ImGui::CollapsingHeader(("["+ match.StateToString() +"] ID: " + std::to_string(match.matchId)).c_str()))
            {
                matchListHeaderState[i] = true;
                ImGui::Text("Duration: %.2fs", match.matchDuration);
                ImGui::Text("Teams: %s", match.CreateTeamVersusMessage().str().c_str());
                std::string teamDisplay;
                if (!match.winningTeam.empty())
                {
                    for (const VirtualPlayer& player : match.winningTeam)
                    {
                        teamDisplay.append("[" + std::to_string(player.GetId()) + "] ");
                    }
                    ImGui::Text("Winning team: { %s }", teamDisplay.c_str());
                }
            }
            else
            {
                matchListHeaderState[i] = false;
            }
        }
    }
    
    ImGui::End();
}

void DrawPlayerEntry(const VirtualPlayer& player, int drawIndex, std::unordered_map<int, bool>& headerStateMapping)
{
    ImGui::SetNextItemOpen(headerStateMapping[drawIndex]);
    
    if (ImGui::CollapsingHeader(("[" + player.StateToString() + "] id: " + std::to_string(player.GetId())).c_str()))
    {
        headerStateMapping[drawIndex] = true;

        ImGui::NewLine();
        for (uint32_t bit = 1; bit <= static_cast<uint32_t>(EPlayerTrait::AllTraits); bit <<= 1)
        {
            auto TraitPair = TraitDatabase.find(static_cast<EPlayerTrait>(bit));
            FTraitInfo TraitInfo = TraitPair->second;
            if (player.HasTrait(TraitPair->first))
            {
                FColor c = GetColor(TraitRarityLookup.find(TraitInfo.rarity)->second.color);
                ImGui::TextColored({
                    static_cast<float>(c.r) / 255.0f,
                    static_cast<float>(c.g) / 255.0f,
                    static_cast<float>(c.b) / 255.0f,
                    static_cast<float>(c.a) / 255.0f},
                    (TraitPair->second.displayName + " ").c_str());
            }
        }
        ImGui::NewLine();
        
        ImGui::Text("Win Rate: %.2f%%", player.GetWinRate() * 100.0f);
        ImGui::Text("W: %d, L: %d", static_cast<int>(player.GetWonMatches().size()), static_cast<int>(player.GetLostMatches().size()));
        ImGui::Text("Total Online Time: %d", player.GetOnlineTime());
        ImGui::Text("Average Queue Time: %.2f", player.GetAvgQueueTime());
        ImGui::Text("Average Game Time: %.2f", player.GetAvgGameTime());
    }
    else
    {
        headerStateMapping[drawIndex] = false;
    }
}

void CleanupImGui()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}