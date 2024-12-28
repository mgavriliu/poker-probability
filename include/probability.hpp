#ifndef PROBABILITY_HPP
#define PROBABILITY_HPP

#include <array>
#include "hand.hpp"

struct HandTypeCounts {
    std::array<unsigned long long, static_cast<size_t>(HandType::Count)> counts{};
    void addHand(HandType type) { counts[static_cast<size_t>(type)]++; }
    double getProbability(HandType type) const {
        unsigned long long total = 0;
        for (const auto& count : counts) total += count;
        return total > 0 ? static_cast<double>(counts[static_cast<size_t>(type)]) / total : 0.0;
    }
};

HandTypeCounts calculateAllProbabilities(int totalHands = 1000000);
double calculateHandTypeProbability(HandType type, int totalHands = 1000000);
double getTheoreticalProbability(HandType type);

#endif  // PROBABILITY_HPP
