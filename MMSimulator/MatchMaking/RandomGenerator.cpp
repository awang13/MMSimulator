#include "RandomGenerator.h"

Xoshiro256SS rng; // ✅ Define a global instance

void SeedRandomGenerator(uint64_t seed)
{
    rng.Seed(seed);
}

int RandomInt(int min, int max)
{
    return min + rng.Next() % (max - min + 1);
}

float RandomFloat()
{
    return rng.Next() / static_cast<float>(UINT64_MAX);
}

float RandomFloat(float min, float max)
{
    return min + (rng.Next() / static_cast<float>(UINT64_MAX)) * (max - min);
}

float RandomFloatWithAnchor(float anchor, float deviation)
{
    return RandomFloat(anchor - deviation, anchor + deviation);
}

bool GetRandomResult(float probability)
{
    return RandomFloat() < probability;
}

bool GetRandomResult_IntPercentage(int percentage)
{
    return RandomInt(0, 100) < percentage;
}
