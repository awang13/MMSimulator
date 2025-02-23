#include "PlayerTrait.h"

// Define trait descriptions in a fast lookup table.
// ***Keep it alphabetical in this list
const std::unordered_map<EPlayerTrait, FTraitInfo> TraitDatabase = {
    {EPlayerTrait::Aggressive,    {Common,      0, 0, 0, 0, 0, 0, 0, "Aggressive", "Prefers risky, high-damage plays"}},
    {EPlayerTrait::Casual,        {Majority,    0, 0, 0, 0, 0, 0, 0, "Casual", "Plays for fun, not highly competitive"}},
    {EPlayerTrait::Competitive,   {Common,      0, 0, 0, 0, 0, 0, 0, "Competitive", "Prefers ranked play, always tries to win"}},
    {EPlayerTrait::Confident,     {Common,      0, 0, 0, 0, 0, 0, 0, "Confident", "More aggressive after wins"}},
    {EPlayerTrait::Defensive,     {Common,      0, 0, 0, 0, 0, 0, 0, "Defensive", "Avoids risk, plays conservatively"}},
    {EPlayerTrait::Leader,        {Rare,        0, 0, 0, 0, 0, 0, 0, "Leader", "Plays better when leading a team"}},
    {EPlayerTrait::LoneWolf,      {Uncommon,    0, 0, 0, 0, 0, 0, 0, "LoneWolf", "Prefers solo play, avoids teamwork"}},
    {EPlayerTrait::MetaAdaptive,  {Rare,        0, 0, 0, 0, 0, 0, 0, "MetaAdaptive", "Learns from opponents, adjusts strategy"}},
    {EPlayerTrait::Nervous,       {Uncommon,    0, 0, 0, 0, 0, 0, 0, "Nervous", "Worse performance under high-pressure"}},
    {EPlayerTrait::RiskAverse,    {Rare,        0, 0, 0, 0, 0, 0, 0, "RiskAverse", "Avoids unnecessary risks, values survival"}},
    {EPlayerTrait::Specialist,    {Rare,        0, 0, 0, 0, 0, 0, 0, "Specialist", "Sticks to one play-style or weapon"}},
    {EPlayerTrait::Streaky,       {Uncommon,    0, 0, 0, 0, 0, 0, 0, "Streaky", "Recent results affects performance"}},
    {EPlayerTrait::TeamOriented,  {Uncommon,    0, 0, 0, 0, 0, 0, 0, "TeamOriented", "Performs better in familiar teams"}},
    {EPlayerTrait::TiltProne,     {Rare,        0, 0, 0, 0, 0, 0, 0, "TiltProne", "Becomes reckless after consecutive losses"}},
    {EPlayerTrait::Unpredictable, {Rare,        0, 0, 0, 0, 0, 0, 0, "Unpredictable", "Inconsistent performance, high variance"}},
    {EPlayerTrait::Versatile,     {Rare,        0, 0, 0, 0, 0, 0, 0, "Versatile", "Adapts frequently, changes play-style"}},
};

const std::unordered_map<ETraitRarity, FRarityInfo> TraitRarityLookup = {
    {ETraitRarity::Majority,    {80, LightGrey}},
    {ETraitRarity::Common,      {35, White}},
    {ETraitRarity::Uncommon,    {15, Green}},
    {ETraitRarity::Rare,        { 3, SkyBlue}},
    {ETraitRarity::Unique,      { 1, Gold}},
};