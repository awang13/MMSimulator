#include "MatchMakingSystem.h"

#include <iomanip>
#include <sstream>
#include <algorithm>
#include <numeric>

#include "MM_Elements.h"
#include "../D3DHelper.h"

MatchMakingSystem::MatchMakingSystem()
{
    matchLog.reserve(1000);
    playerLog.reserve(1000);
}

void MatchMakingSystem::Update()
{
    Update_RejoiningPlayers();
    Update_Matches();

    // The main system function, runs periodically
    Update_Matchmake(matchMakingSystemDelay);
}

void MatchMakingSystem::CreatePlayer()
{
    int id = static_cast<int>(allPlayersLookupMap.size());
    allPlayersLookupMap.emplace(id, VirtualPlayer(id));

    VirtualPlayer& newP = allPlayersLookupMap.find(id)->second;

    std::string log;
    newP.SetState(EPlayerState::Online, log);
    RecordToLog(playerLog, log);

    AddPlayerToRejoiningQueue(&newP);
}

void MatchMakingSystem::Update_Matchmake(const int& Interval)
{
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastMatchmakingTime).count() < Interval)
    {
        return;
    }
    lastMatchmakingTime = now;
    
    int startedMatches = 0;
    std::vector<VirtualPlayer*> queuedPlayers;

    // only loop over known queued players
    for (int playerId : queuedPlayerIDs)
    {
        auto it = allPlayersLookupMap.find(playerId);
        if (it != allPlayersLookupMap.end())
        {
            queuedPlayers.emplace_back(&(it->second));
        }
    }

    /*
     * For every {matchMakingSystemDelay} ms, the system can handle initiating up to {matchesPerCycle} matches
     * Each match needs {numTeams} teams and {teamSize} players on each team
     * After each match is created, players are removed from queue and queuedPlayerIDs should be updated properly
     */
    for (int i = 0; i < MatchSetting.matchesPerCycle; ++i)
    {
        if (static_cast<int>(queuedPlayers.size()) < MatchSetting.numTeams * MatchSetting.teamSize)
        {
            break;
        }

        FMatch newMatch;
        
        for (int t = 0; t < MatchSetting.numTeams; ++t)
        {
            std::vector<VirtualPlayer> team;
            for (int j = 0; j < MatchSetting.teamSize; ++j)
            {
                if(VirtualPlayer* playerRef = queuedPlayers.back())
                {
                    std::string log;
                    team.emplace_back(*playerRef);
                    playerRef->SetState(EPlayerState::InGame, log);
                    
                    //SetPlayerState(playerRef->GetId(), EPlayerState::InGame);
                    queuedPlayerIDs.erase(playerRef->GetId());
                    queuedPlayers.pop_back();
                }
            }
            newMatch.teams.push_back(std::move(team));
        }

        int id = static_cast<int>(allMatchesLookupMap.size());
        newMatch.matchId = id;
        allMatchesLookupMap.emplace(id, newMatch);

        FMatch& matchRef = allMatchesLookupMap.find(id)->second;
        
        matchRef.StartMatch();
        ongoingMatchIds.insert(id);
        
        RecordToLog(matchLog, matchRef.CreateMatchStartMessage().str());
        
        ++startedMatches;
    }
}

void MatchMakingSystem::Update_Matches()
{
    if (ongoingMatchIds.empty())
    {
        return;
    }

    auto now = std::chrono::steady_clock::now();

    std::vector<FMatch*> queuedMatches;
    for (int matchId : ongoingMatchIds)
    {
        auto it = allMatchesLookupMap.find(matchId);
        if (it != allMatchesLookupMap.end())
        {
            queuedMatches.emplace_back(&(it->second));
        }
    }
    
    for (FMatch* match : queuedMatches)
    {
        auto elapsed = std::chrono::duration<float>(now - match->matchStartTime);
        if (elapsed.count() >= match->matchDuration)
        {
            match->EndMatch();
            ReportMatchResult(*match);

            for (const std::vector<VirtualPlayer>& team : match->teams)
            {
                for (const VirtualPlayer& player : team)
                {
                    auto it = allPlayersLookupMap.find(player.GetId());
                    if (it != allPlayersLookupMap.end())
                    {
                        std::string log;
                        it->second.SetState(EPlayerState::Online, log);
                        RecordToLog(playerLog, log);

                        queuedPlayerIDs.erase(it->second.GetId());
                        AddPlayerToRejoiningQueue(&it->second);
                    }
                }
            }
            
            UpdateLeaderboard(*match);
            ongoingMatchIds.erase(match->matchId);
        }
    }
}

void MatchMakingSystem::Update_RejoiningPlayers()
{
    auto now = std::chrono::steady_clock::now();
    while (!rejoiningPlayers.empty() && rejoiningPlayers.top().rejoinTime <= now)
    {
        VirtualPlayer* player = rejoiningPlayers.top().player;
        rejoiningPlayers.pop();

        if (player)
        {
            std::string log;
            player->SetState(EPlayerState::InQueue, log);
            RecordToLog(playerLog, log);
            
            queuedPlayerIDs.insert(player->GetId());
        }
    }
}

void MatchMakingSystem::AddPlayerToRejoiningQueue(VirtualPlayer* player)
{
    FPlayerRejoin playerRejoin = FPlayerRejoin(player);
    rejoiningPlayers.push(playerRejoin);
}

void MatchMakingSystem::RecordToLog(std::vector<std::string>& targetLog, const std::string& message, bool bTimeStamp)
{
    if (bTimeStamp)
    {
        auto now = std::chrono::system_clock::now();
        std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
        std::tm localTime;
        (void)localtime_s(&localTime, &currentTime);
    
        std::ostringstream ss;
        ss << std::put_time(&localTime, "[%H:%M:%S]") << " " << message;

        targetLog.push_back(ss.str());
        return;
    }
    
    targetLog.push_back(message);
    return;
}

void MatchMakingSystem::UpdateLeaderboard(const FMatch& match)
{
    VirtualPlayer* bestP = nullptr;
    float highestWinRate = -1;
    
    for (const auto& team : match.teams)
    {
        for (const auto& player : team)
        {
            auto it = allPlayersLookupMap.find(player.GetId());
            if (it != allPlayersLookupMap.end())
            {
                float pwr = it->second.GetWinRate();
                if (highestWinRate < 0 || pwr > highestWinRate)
                {
                    highestWinRate = pwr;
                    bestP = &(it->second); //pointer to actual VPlayer
                }
            }
        }
    }

    // if no leading player, or bestP is already leading, update to new number
    if (bestP && (currentLeadingPlayer.GetId() < 0 || currentLeadingPlayer.GetId() == bestP->GetId() || highestWinRate >= currentLeadingPlayer.GetWinRate()))
    {
        currentLeadingPlayer = *bestP;
    }
}

void MatchMakingSystem::ReportMatchResult(const FMatch& match)
{
    // Process match results efficiently
    for (auto& team : match.teams)
    {
        for (const auto& player : team)
        {
            auto it = allPlayersLookupMap.find(player.GetId());
            if (it != allPlayersLookupMap.end())
            {
                it->second.RegisterMatchResult(match.matchId, match.IsPlayerWinner(it->first));
            }
        }
    }
    
    RecordToLog(matchLog, match.CreateMatchFinishedMessage().str());
}

std::vector<VirtualPlayer*> MatchMakingSystem::GetTopPlayersByWinRate() const
{
    std::vector<VirtualPlayer*> topPlayers;
    topPlayers.reserve(allPlayersLookupMap.size());

    for (const auto& it : allPlayersLookupMap)
    {
        topPlayers.push_back(const_cast<VirtualPlayer*>(&it.second));
    }

    size_t totalPlayers = topPlayers.size();
    if (totalPlayers < 10 || allMatchesLookupMap.size() < 20)
    {
        return {};
    }

    size_t topCount = std::max<size_t>(5, std::min<size_t>(100, totalPlayers * 2 / 100));

    std::partial_sort(topPlayers.begin(), topPlayers.begin() + topCount, topPlayers.end(),
        [](const VirtualPlayer* a, const VirtualPlayer* b)
        {
           return a->GetWinRate() > b->GetWinRate(); // sort in descending order 
        });

    topPlayers.resize(topCount);
    return topPlayers;
}

float MatchMakingSystem::GetAvgQueueTime() const
{
    if (!allPlayersLookupMap.empty())
    {
        float totalQTime = std::accumulate(allPlayersLookupMap.begin(), allPlayersLookupMap.end(), 0.0f,
            [](float total, const std::pair<const int, VirtualPlayer>& entry)
            {
                const VirtualPlayer& player = entry.second;
                return total + player.GetAvgQueueTime();
            }
        );
        return totalQTime / static_cast<float>(allPlayersLookupMap.size());
    }
    return 0.0f;
}
