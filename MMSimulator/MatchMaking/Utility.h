#pragma once
#include <unordered_map>

// Common struct representing a color with transparency
struct FColor
{
    int r = 255;
    int g = 255;
    int b = 255;
    int a = 255;

    FColor(int inR, int inG, int inB, int inA) {r = inR; g = inG; b = inB; a = inA;} // common constructor
    FColor(int inR, int inG, int inB) {r = inR; g = inG; b = inB;} // solid color don't require alpha input
};

// Commonly used named color for faster lookup
enum EColor
{
    Black,
    Blue,
    Gold,
    Green,
    LightGrey,
    Red,
    SkyBlue,
    White,
};

extern const std::unordered_map<EColor, FColor> NamedColorMap;

static FColor GetColor(EColor color)
{
    return NamedColorMap.find(color)->second;
}

/*
ImVec4 ColorVec4(EColor color)
{
    FColor normColor = GetColor(color);
    return {static_cast<float>(normColor.r),static_cast<float>(normColor.g),static_cast<float>(normColor.b),static_cast<float>(normColor.a)};
}
 */
