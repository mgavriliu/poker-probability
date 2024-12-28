#ifndef CUDA_PROBABILITY_CUH
#define CUDA_PROBABILITY_CUH

#include "hand.hpp"
#include "probability.hpp"

HandTypeCounts calculateAllProbabilitiesCUDA(int totalHands);
double calculateHandTypeProbabilityCUDA(HandType type, int totalHands);

#endif // CUDA_PROBABILITY_CUH
