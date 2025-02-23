#include "MM_Elements.h"

#include <iomanip>
#include <queue>

#include "RandomGenerator.h"
#include <random>
#include <sstream>

VirtualPlayer::VirtualPlayer(int inId, EPlayerTrait inTrait)
{
    id = inId;
    traits = inTrait;
}

VirtualPlayer::VirtualPlayer(int inId)
{
    id = inId;
    traits = GenerateRandomTraits();
    ValidateTraits();
}

EPlayerTrait VirtualPlayer::GenerateRandomTraits()
{
    EPlayerTrait newTraits = EPlayerTrait::None; // init

    for (uint32_t bit = 1; bit <= static_cast<uint32_t>(EPlayerTrait::AllTraits); bit <<= 1)
    {
        auto TraitPair = TraitDatabase.find(static_cast<EPlayerTrait>(bit));
        FTraitInfo TraitInfo = TraitPair->second;
        
        if (GetRandomResult_IntPercentage(TraitRarityLookup.find(TraitInfo.rarity)->second.pScore))
        {
            newTraits |= TraitPair->first;
        }
    }

    return newTraits == EPlayerTrait::None ? EPlayerTrait::Casual : newTraits; // if randomized to have no traits then default to casual player
}

void VirtualPlayer::ValidateTraits()
{
    HandleConflictTrait_PickOne({EPlayerTrait::Aggressive, EPlayerTrait::Defensive});
    HandleConflictTrait_PickOne({EPlayerTrait::Casual, EPlayerTrait::Competitive});
}

void VirtualPlayer::HandleConflictTrait_PickOne(const std::vector<EPlayerTrait>& conflictingTraits)
{
    if (conflictingTraits.size() > 1)
    {
        int foundTraits = 0;
        for (const EPlayerTrait& trait : conflictingTraits)
        {
            if (HasTrait(trait))
            {
                ++foundTraits;
            }
        }
        
        if (foundTraits > 1) // player actually has more than 2 conflicting traits found. Pick one and remove all others
        {
            int rand = RandomInt(0, static_cast<int>(conflictingTraits.size()) - 1);
            for (int i = 0; i < static_cast<int>(conflictingTraits.size()); ++i)
            {
                if (i != rand)
                {
                    RemoveTrait(conflictingTraits[i]);
                }
            }
        }
    }   
}

void VirtualPlayer::RegisterMatchResult(int matchId, bool bIsWon)
{
    if (bIsWon)
    {
        wonMatches.push_back(matchId);
    }
    else
    {
        lostMatches.push_back(matchId);
    }
    
    UpdateWinRate();
}

std::chrono::steady_clock::time_point VirtualPlayer::SetNextRejoiningTime()
{
    currentIdleTime = RandomFloatWithAnchor(2.0f, 1.4f);
    return std::chrono::steady_clock::now() + std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<float>(currentIdleTime));
}

void VirtualPlayer::UpdateWinRate()
{
    size_t totalMatch = wonMatches.size() + lostMatches.size();
    winRate = totalMatch == 0 ? 0.0f : static_cast<float>(wonMatches.size()) / static_cast<float>(totalMatch);
}

void VirtualPlayer::SetState(EPlayerState inState, std::string& logMsg)
{
    std::ostringstream logEntry;
    if (inState == EPlayerState::Online)
    {
        SetNextRejoiningTime();
        
        logEntry << std::fixed << std::setprecision(2);
        logEntry << "Player " << id << " is now idling... (Joining queue in: " << GetCurrentIdleTime() << "s)";
        logMsg = logEntry.str();
    }
        
    if (inState == EPlayerState::InQueue)
    {
        logEntry << "Player " << id << " joins queue...";
        logMsg = logEntry.str();
    }

    // Records
    if (state != inState)
    {
        auto now = std::chrono::steady_clock::now();
        auto durationInState = now - stateChangeTimeStamp;

        // record by cases
        // if old state isn't offline, add duration to total online time
        if (state != EPlayerState::Disconnected && state != EPlayerState::Offline)
        {
            totalOnlineTime += durationInState;   
        }

        // if old state was in queue, update queue time
        if (state == EPlayerState::InQueue)
        {
            ++queueTimePair.first;
            queueTimePair.second += durationInState;
        }

        // if old state was in game, update game time
        if (state == EPlayerState::InGame)
        {
            ++gameTimePair.first;
            gameTimePair.second += durationInState;
        }

        // finished recording, update to new state
        state = inState;
        stateChangeTimeStamp = now;
    }
}

float VirtualPlayer::GetAvgQueueTime() const
{
    int total = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(queueTimePair.second).count());
    int denom = queueTimePair.first;
    if (state == EPlayerState::InQueue)
    {
        total += GetTimeInCurrentState_Sec();
        ++denom;
    }
    return denom == 0 ? 0.0f : static_cast<float>(total) / static_cast<float>(denom);
}

float VirtualPlayer::GetAvgGameTime() const
{
    int total = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(gameTimePair.second).count());
    int denom = gameTimePair.first;
    if (state == EPlayerState::InGame)
    {
        total += GetTimeInCurrentState_Sec();
        ++denom;
    }
    return denom == 0 ? 0.0f : static_cast<float>(total) / static_cast<float>(denom);
}

int VirtualPlayer::GetOnlineTime() const
{
    return static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(totalOnlineTime).count());
}

std::string VirtualPlayer::StateToString() const
{
    switch (state)
    {
    case EPlayerState::Offline:      return "Offline";
    case EPlayerState::Online:       return "Online";
    case EPlayerState::InQueue:      return "In Queue";
    case EPlayerState::InGame:       return "In Game";
    case EPlayerState::Disconnected: return "Disconnected";
    case EPlayerState::Rejoining:    return "Rejoining";
    }
    return "Unknown State";
}

std::string VirtualPlayer::TraitsToString() const
{
    std::string result;

    for (uint32_t bit = 1; bit <= static_cast<uint32_t>(EPlayerTrait::AllTraits); bit <<= 1)
    {
        auto TraitPair = TraitDatabase.find(static_cast<EPlayerTrait>(bit));
        FTraitInfo TraitInfo = TraitPair->second;
        if (HasTrait(TraitPair->first)) result += TraitPair->second.displayName + " ";
    }
    
    return result.empty() ? "None" : result;
}

int VirtualPlayer::GetTimeInCurrentState_Sec() const
{
    return static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - stateChangeTimeStamp).count());
}

// ===== FMath BEGIN =====
std::ostringstream FMatch::CreateMatchStartMessage() const
{
    std::ostringstream logEntry;
    logEntry << "Match Started [" << matchId << "] ";
    logEntry << CreateTeamVersusMessage().str();
    return logEntry;
}

std::ostringstream FMatch::CreateMatchFinishedMessage() const
{
    std::ostringstream logEntry;
    logEntry << std::fixed << std::setprecision(2);
    logEntry << "Match Finished [" << matchId << "] (" << matchDuration << "s): ";
    return logEntry;
}

std::ostringstream FMatch::CreateTeamVersusMessage() const
{
    std::ostringstream logEntry;
    for (size_t t = 0; t < teams.size(); ++t)
    {
        for (size_t p = 0; p < teams[t].size(); ++p)
        {
            logEntry << "[" << teams[t][p].GetId() << "]";
            if (p < teams[t].size() - 1)
            {
                logEntry << "&";
            }
        }
        if (t < teams.size() - 1)
        {
            logEntry << "vs";
        }
    }
    return logEntry;
}

void FMatch::StartMatch()
{
    // Set a unique randomized duration for each started match, this can be affected by game mode and player stats
    matchDuration = RandomFloatWithAnchor(matchDuration, 1.5f);
    matchStartTime = std::chrono::steady_clock::now();
    state = EMatchState::Ongoing;
}

void FMatch::EndMatch()
{
    if (teams.size() > 1)
    {
        winningTeam = teams[RandomInt(0,static_cast<int>(teams.size()) - 1)];
    }
    state = EMatchState::Completed;
}

bool FMatch::IsPlayerWinner(int playerId) const
{
    return !winningTeam.empty() && std::any_of(winningTeam.begin(), winningTeam.end(),
        [playerId](const VirtualPlayer& player) { return player.GetId() == playerId;} );
}

std::string FMatch::StateToString() const
{
    switch (state)
    {
    case EMatchState::Initiated:    return "Created";
    case EMatchState::Ongoing:      return "Ongoing";
    case EMatchState::Finished:     return "Finished";
    case EMatchState::Completed:    return "Completed";
    }
    return "Unknown State";
}