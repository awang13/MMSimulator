#pragma once

#include <vector>
#include <chrono>
#include <string>

#include "PlayerTrait.h"

// ===== VIRTUAL PLAYER BEGIN =====

// States
enum class EPlayerState
{
    Offline, // Not in the system
    Online, // Logged in but idle
    InQueue, // Waiting for MM
    InGame, // in a match
    Disconnected, // Left the game but can return
    Rejoining, // Temporarily offline but expected to return
};

// base class for a player that goes online and plays matches in an imaginary game hosted by the MatchMakingSystem
class VirtualPlayer
{
public:
    VirtualPlayer() = default;
    VirtualPlayer(int inId); // create a player with everything randomized
    VirtualPlayer(int inId, EPlayerTrait inTrait);

    void RegisterMatchResult(int matchId, bool bIsWon);
    void UpdateWinRate();
    void SetState(EPlayerState inState, std::string& logMsg);

    // information & getters
    float GetAvgQueueTime() const;
    float GetAvgGameTime() const;
    int GetOnlineTime() const;
    
    int GetId() const { return id; }
    EPlayerState GetState() const { return state; }
    EPlayerTrait GetTraits() const { return traits; }
    std::vector<int> GetWonMatches() const { return wonMatches; }
    std::vector<int> GetLostMatches() const { return lostMatches; }
    float GetWinRate() const { return winRate; }
    float GetCurrentIdleTime() const { return currentIdleTime; }

    // Trait management
    static EPlayerTrait GenerateRandomTraits();
    void ValidateTraits();
    bool HasTrait(EPlayerTrait trait) const {return ::HasTrait(traits, trait); }
    void AddTrait(EPlayerTrait newTrait) {traits |= newTrait; }
    void RemoveTrait(EPlayerTrait traitToRemove) { traits = traits & ~traitToRemove; }
    void HandleConflictTrait_PickOne(const std::vector<EPlayerTrait>& conflictingTraits); // if player has multiple of the conflicting traits, randomly (evenly) pick one and remove otehrs 

    // misc
    std::string StateToString() const;
    std::string TraitsToString() const;

private:
    int id;
    EPlayerState state = EPlayerState::Offline;
    EPlayerTrait traits = EPlayerTrait::None; // Supports multiple traits through bitmask
    std::vector<int> wonMatches;
    std::vector<int> lostMatches;
    
    // idle time: time when player stays online but not in queue
    float currentIdleTime = 0.0f;
    float winRate = 0.0f;

    // Quantified play style
    int agr = 0; // Aggressiveness - Willingness to take risks and engage in high-pressure plays
    int fle = 0; // Flexibility - Ability to adapt to new strategies and opponents
    int gri = 0; // Grit - Mental resilience and ability to recover from set-bakcs
    int end = 0; // Endurance - Long-term consistency across multiple matches
    int ins = 0; // Instinct - Quick and accurate decision-making under pressure
    int cre = 0; // Creativity - Likelihood of turning the tide unexpectedly; wildcard behavior
    int pre = 0; // Precision - Ability to execute mechanical actions with accuracy and efficiency
    
    // time when last state changed. Use this to record player activity history
    std::chrono::steady_clock::time_point stateChangeTimeStamp;
    std::chrono::steady_clock::duration totalOnlineTime;
    std::pair<int, std::chrono::steady_clock::duration> queueTimePair;
    std::pair<int, std::chrono::steady_clock::duration> gameTimePair;

    int GetTimeInCurrentState_Sec() const;
    std::chrono::steady_clock::time_point SetNextRejoiningTime();
};

// ===== VIRTUAL PLAYER END =====

// ===== VIRTUAL MATCH BEGIN =====

enum class EMatchState
{
    Initiated,  // when match is first created
    Ongoing,    // when all conditions are met and match has started
    Finished,   // when the game provided by this match is concluded
    Completed,  // when match is completed and players are removed
};

// Struct that stores a match's data
struct FMatch
{
    // general information
    int matchId = -1;
    std::vector<std::vector<VirtualPlayer>> teams; // supports multiple team and uneven player counts on each team
    std::chrono::steady_clock::time_point matchStartTime;
    float matchDuration = 3.0f;
    EMatchState state = EMatchState::Initiated;

    // end of match info
    std::vector<VirtualPlayer> winningTeam;

    // display
    std::ostringstream CreateMatchStartMessage() const;
    std::ostringstream CreateMatchFinishedMessage() const;
    std::ostringstream CreateTeamVersusMessage() const;

    // Process
    void StartMatch();
    void EndMatch();
    
    bool IsPlayerWinner(int playerId) const;
    std::string StateToString() const;
};

// ===== VIRTUAL MATCH END =====