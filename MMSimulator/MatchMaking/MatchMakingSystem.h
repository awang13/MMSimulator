#pragma once

#include <vector>
#include <string>
#include <queue>
#include <chrono>
#include <random>
#include <unordered_map>
#include <unordered_set>

#include "MM_Elements.h"

enum class EPlayerState;
class VirtualPlayer;

// a min-heap priority queue for downtime tracking
struct FPlayerRejoin
{
    VirtualPlayer* player = nullptr;
    
    // estimated time when player would rejoin the queue
    std::chrono::steady_clock::time_point rejoinTime;

    // Min-heap: earlier rejoin times come first
    bool operator > (const FPlayerRejoin& other) const
    {
        return rejoinTime > other.rejoinTime;
    }

    FPlayerRejoin(VirtualPlayer* inPlayer)
    {
        player = inPlayer;
        rejoinTime = std::chrono::steady_clock::now() +
            std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<float>(inPlayer->GetCurrentIdleTime()));
    }
};

struct FMatchSetting
{
    int numTeams = 2;
    int teamSize = 1;
    int matchDuration = 3;
    int matchesPerCycle = 2;
};

extern std::priority_queue<FPlayerRejoin, std::vector<FPlayerRejoin>, std::greater<>> rejoiningPlayers;

// This system simulates the match making process
class MatchMakingSystem
{
public:
    MatchMakingSystem();
    void Update();
    
    void CreatePlayer();
    std::vector<VirtualPlayer*> GetTopPlayersByWinRate() const;
    float GetAvgOnlineTime() const;
    float GetAvgQueueTime() const;
    float GetAvgGameTime() const;
    
    static void RecordToLog(std::vector<std::string>& targetLog, const std::string& message, bool bTimeStamp = true);

    // Getters and Setters
    FMatchSetting GetMatchSetting() const { return MatchSetting; }
    void SetMatchSetting(FMatchSetting Settings) { MatchSetting = Settings; }
    const std::unordered_set<int>& GetOngoingMatchIds() const { return ongoingMatchIds; }
    const std::vector<std::string>& GetMatchLog() const { return matchLog; }
    const std::vector<std::string>& GetPlayerLog() const { return playerLog; }
    VirtualPlayer GetCurrentLeadingPlayer() const { return currentLeadingPlayer; }
    const std::unordered_map<int, VirtualPlayer>& GetAllPlayers() const { return allPlayersLookupMap; }
    const std::unordered_map<int, FMatch>& GetAllMatches() const { return allMatchesLookupMap; }

private:
    void Update_Matchmake(const int& Interval); // interval in millisecond
    void Update_Matches();
    void Update_RejoiningPlayers();
    
    void UpdateLeaderboard(const FMatch& match);
    void ReportMatchResult(const FMatch& match);
    void AddPlayerToRejoiningQueue(VirtualPlayer* player);
    
    FMatchSetting MatchSetting;
    
    // stores all players, regardless of state, using a map lookup for faster iteration because it is assumed to have a big player pool
    std::unordered_map<int, VirtualPlayer> allPlayersLookupMap;
    
    // Stores all matches' history
    std::unordered_map<int, FMatch> allMatchesLookupMap;

    // Track player IDs of players currently in the queue
    std::unordered_set<int> queuedPlayerIDs;

    // Tracking players that are idle and waiting to go into queue
    std::priority_queue<FPlayerRejoin, std::vector<FPlayerRejoin>, std::greater<>> rejoiningPlayers;

    // Ongoing matches that gets updated
    //std::vector<FMatch> ongoingMatches;

    // Ongoing match ids
    std::unordered_set<int> ongoingMatchIds;

    // UI logging
    std::vector<std::string> matchLog;
    std::vector<std::string> playerLog;
    VirtualPlayer currentLeadingPlayer = VirtualPlayer(-1, EPlayerTrait::None);
    
    // delay between each match making processing in milliseconds
    std::chrono::steady_clock::time_point lastMatchmakingTime;

    // helper trackers
    int matchMakingSystemDelay = 500;
};
