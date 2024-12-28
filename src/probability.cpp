#include "probability.hpp"
#include <algorithm>
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include "deck.hpp"
#include "hand.hpp"
#include "utils.hpp"

std::atomic<unsigned long long> handTypeCount{0};
std::atomic<HandTypeCounts*> globalCounts;

void simulateHands(HandType targetType, int numHands, int threadId, int totalThreads) {
  unsigned long long localCount = 0;  // Local counter for this thread
  Deck deck;                          // Create deck once per thread

  for (int i = 0; i < numHands; ++i) {
    if (i % 10 == 0) {
      deck.reset();
      deck.shuffle();
    }
    
    Hand hand(deck.dealHand(5));

    if (hand.getHandType() == targetType) {
      localCount++;
    }

    if (threadId == 0 && i % 10000 == 0) {
      float progress = static_cast<float>(i) / numHands;
      printProgress(progress);
    }
  }

  handTypeCount += localCount;  // Add local count to global atomic counter
}

void simulateHandsAllTypes(int numHands, int threadId, int totalThreads) {
  auto localCounts = std::make_unique<HandTypeCounts>();
  Deck deck;

  for (int i = 0; i < numHands; ++i) {
    if (i % 10 == 0) {
      deck.reset();
      deck.shuffle();
    }

    Hand hand(deck.dealHand(5));

    HandType type = hand.getHandType();
    localCounts->counts[static_cast<size_t>(type)]++;

    if (threadId == 0 && i % 10000 == 0) {
      float progress = static_cast<float>(i) / numHands;
      printProgress(progress);
    }
  }

  // Update global counts atomically
  HandTypeCounts* current;
  HandTypeCounts* updated;
  do {
    current = globalCounts.load(std::memory_order_acquire);
    updated = new HandTypeCounts(*current);
    for (size_t i = 0; i < static_cast<size_t>(HandType::Count); ++i) {
      updated->counts[i] += localCounts->counts[i];
    }
  } while (
      !globalCounts.compare_exchange_weak(current, updated, std::memory_order_release, std::memory_order_relaxed));
  delete current;
}

HandTypeCounts calculateAllProbabilities(int totalHands) {
  auto initial = new HandTypeCounts();
  globalCounts.store(initial);

  unsigned int numThreads = std::thread::hardware_concurrency();
  if (numThreads == 0) numThreads = 4;

  std::vector<std::thread> threads;
  int handsPerThread = totalHands / numThreads;

  for (unsigned int i = 0; i < numThreads - 1; ++i) {
    threads.emplace_back(simulateHandsAllTypes, handsPerThread, i, numThreads);
  }
  simulateHandsAllTypes(totalHands - (handsPerThread * (numThreads - 1)), numThreads - 1, numThreads);

  for (auto& thread : threads) {
    thread.join();
  }

  HandTypeCounts result = *globalCounts.load();
  delete globalCounts.load();
  return result;
}

double calculateHandTypeProbability(HandType type, int totalHands) {
  HandTypeCounts results = calculateAllProbabilities(totalHands);
  return results.getProbability(type);
}

double getTheoreticalProbability(HandType type) {
  // Reference: https://en.wikipedia.org/wiki/Poker_probability
  switch (type) {
    case HandType::RoyalFlush: return 0.000154;    // 0.0154%
    case HandType::StraightFlush: return 0.00139;  // 0.139%
    case HandType::FourOfAKind: return 0.02401;    // 0.0240%
    case HandType::FullHouse: return 0.1441;       // 0.1441%
    case HandType::Flush: return 0.1965;           // 0.1965%
    case HandType::Straight: return 0.3925;        // 0.3925%
    case HandType::ThreeOfAKind: return 2.1128;    // 2.1128%
    case HandType::TwoPair: return 4.7539;         // 4.7539%
    case HandType::OnePair: return 42.2569;        // 42.2569%
    case HandType::HighCard: return 50.1177;       // 50.1177%
    default: return 0.0;
  }
}

Hand generateRandomHand() {
  Deck deck;
  deck.shuffle();
  return Hand(deck.dealHand(5));
}