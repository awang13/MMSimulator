#pragma once
#include "Xoshiro256SS.h"

extern Xoshiro256SS rng;

// Init RNG with a seed
void SeedRandomGenerator(uint64_t seed);

// Generate a random integer
int RandomInt(int min, int max);

// Generate a random float between 0 and 1
float RandomFloat();

// Generate a random float
float RandomFloat(float min, float max);

// Generate a random float using anchor and deviation
float RandomFloatWithAnchor(float anchor, float deviation);

// Returns a random result based on probability between 0 and 1
bool GetRandomResult(float probability);

// Returns a random result based on probability between 0 and 100
bool GetRandomResult_IntPercentage(int percentage);