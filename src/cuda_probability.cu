#include <cuda_runtime.h>
#include <curand_kernel.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include "cuda_probability.cuh"
#include "utils.hpp"  // For printProgress

#define CHECK_CUDA_ERROR(val) check_cuda((val), #val, __FILE__, __LINE__)
template <typename T> void check_cuda(T err, const char* const func, const char* const file, const int line) {
  if (err != cudaSuccess) {
    fprintf(stderr, "CUDA error at %s:%d code=%d(%s) \"%s\" \n", file, line, static_cast<unsigned int>(err),
            cudaGetErrorString(err), func);
    cudaDeviceReset();
    exit(EXIT_FAILURE);
  }
}

// Add these helper functions at the start of the file
void checkGPUMemory() {
  size_t free, total;
  cudaMemGetInfo(&free, &total);
  fprintf(stderr, "GPU Memory - Free: %zu MB, Total: %zu MB\n", free / (1024 * 1024), total / (1024 * 1024));
}

// Constants for CUDA execution
const int BLOCK_SIZE = 256;
const int NUM_BLOCKS = 256;

__device__ bool hasFlush(const unsigned char* hand, bool exclusive = true) {
  int suit = hand[0] & 3;
  for (int i = 1; i < 5; i++) {
    if ((hand[i] & 3) != suit) return false;
  }
  return true;  // Remove exclusivity check for now to break circular dependency
}

__device__ bool hasStraight(const unsigned char* hand, bool exclusive = true) {
  int ranks[13] = {0};
  int min_rank = 13, max_rank = 0;

  for (int i = 0; i < 5; i++) {
    int rank = (hand[i] >> 2);
    ranks[rank] = 1;
    min_rank = min(min_rank, rank);
    max_rank = max(max_rank, rank);
  }

  bool isStraight = false;

  // Check for Ace-low straight
  if (ranks[12] && ranks[0] && ranks[1] && ranks[2] && ranks[3]) {
    isStraight = true;
  }
  // Check normal straight
  else if (max_rank - min_rank == 4) {
    isStraight = true;
    for (int i = min_rank; i <= max_rank; i++) {
      if (!ranks[i]) {
        isStraight = false;
        break;
      }
    }
  }

  return isStraight && (!exclusive || !hasFlush(hand, false));
}

__device__ bool hasOnePair(const unsigned char* hand) {
  int ranks[13] = {0};
  int pairs = 0;
  for (int i = 0; i < 5; i++) {
    int rank = (hand[i] >> 2);  // Fix rank calculation
    if (++ranks[rank] == 2) pairs++;
  }
  return pairs == 1;
}

__device__ bool hasTwoPair(const unsigned char* hand) {
  int ranks[13] = {0};
  int pairs = 0;
  for (int i = 0; i < 5; i++) {
    int rank = (hand[i] >> 2);  // Fix rank calculation
    if (++ranks[rank] == 2) pairs++;
  }
  return pairs == 2;
}

__device__ bool hasThreeOfAKind(const unsigned char* hand) {
  int ranks[13] = {0};
  for (int i = 0; i < 5; i++) {
    int rank = (hand[i] >> 2);          // Fix rank calculation (same as other methods)
    ranks[rank]++;                      // Count ranks without range check
    if (ranks[rank] >= 3) return true;  // Return as soon as we find three
  }
  return false;
}

__device__ bool hasFullHouse(const unsigned char* hand) {
  int ranks[13] = {0};

  // Count all ranks first
  for (int i = 0; i < 5; i++) {
    int rank = (hand[i] >> 2);
    ranks[rank]++;
  }

  bool hasThree = false, hasTwo = false;
  for (int count : ranks) {
    if (count == 3) hasThree = true;
    if (count == 2) hasTwo = true;
  }

  return hasThree && hasTwo;
}

__device__ bool hasFourOfAKind(const unsigned char* hand) {
  int ranks[13] = {0};
  for (int i = 0; i < 5; i++) {
    int rank = (hand[i] >> 2);  // Fix rank calculation
    if (++ranks[rank] == 4) return true;
  }
  return false;
}

__device__ bool hasRoyalFlush(const unsigned char* hand) {
  if (!hasFlush(hand)) return false;
  int ranks[13] = {0};
  for (int i = 0; i < 5; i++) {
    int rank = (hand[i] >> 2);  // Fix rank calculation
    ranks[rank] = 1;
  }
  return ranks[8] && ranks[9] && ranks[10] && ranks[11] && ranks[12];
}

__device__ bool hasStraightFlush(const unsigned char* hand) {
  return hasFlush(hand, false) && hasStraight(hand, false);
}

__device__ HandType getHandType(const unsigned char* hand) {
  if (hasRoyalFlush(hand)) return HandType::RoyalFlush;
  if (hasStraightFlush(hand)) return HandType::StraightFlush;
  if (hasFourOfAKind(hand)) return HandType::FourOfAKind;
  if (hasFullHouse(hand)) return HandType::FullHouse;
  if (hasFlush(hand)) return HandType::Flush;
  if (hasStraight(hand)) return HandType::Straight;
  if (hasThreeOfAKind(hand)) return HandType::ThreeOfAKind;
  if (hasTwoPair(hand)) return HandType::TwoPair;
  if (hasOnePair(hand)) return HandType::OnePair;
  return HandType::HighCard;
}

__device__ void dealRandomHand(curandState* state, unsigned char* hand) {
  unsigned char deck[52];

  // Initialize deck with cards: rank << 2 | suit
  for (int i = 0; i < 52; i++) {
    unsigned char rank = (i / 4);  // Ranks 2-14
    unsigned char suit = i % 4;    // Suits 0-3
    deck[i] = (rank << 2) | suit;  // Combine into single byte
  }

  // Fisher-Yates shuffle for first 5 cards
  for (int i = 0; i < 5; i++) {
    int remaining = 52 - i;
    int j = curand(state) % remaining;
    hand[i] = deck[j];
    deck[j] = deck[remaining - 1];
  }
}

__device__ void dealRandomHand(unsigned char* deck, int& deckPosition, curandState* state, unsigned char* hand) {
  // If we need to reshuffle
  if (deckPosition >= 50) {  // 52 - 2 to ensure we always have 5 cards available
    // Reset deck
    for (int i = 0; i < 52; i++) {
      unsigned char rank = (i / 4);  // Ranks 0-12
      unsigned char suit = i % 4;    // Suits 0-3
      deck[i] = (rank << 2) | suit;  // Combine into single byte
    }
    // Fisher-Yates shuffle
    for (int i = 51; i > 0; i--) {
      int j = curand(state) % (i + 1);
      unsigned char temp = deck[i];
      deck[i] = deck[j];
      deck[j] = temp;
    }
    deckPosition = 0;
  }

  // Deal 5 cards from current deck position
  for (int i = 0; i < 5; i++) {
    hand[i] = deck[deckPosition++];
  }
}

__global__ void simulateHandsKernel(HandType targetType, curandState* states, unsigned long long* counts,
                                    int handsPerThread) {
  int tid = blockIdx.x * blockDim.x + threadIdx.x;
  curandState localState = states[tid];
  unsigned char hand[5];
  unsigned long long localCount = 0;

  for (int i = 0; i < handsPerThread; i++) {
    dealRandomHand(&localState, hand);
    if (getHandType(hand) == targetType) {
      localCount++;
    }
  }

  counts[tid] = localCount;
  states[tid] = localState;
}

__global__ void initRNG(curandState* states, unsigned long seed) {
  int tid = blockIdx.x * blockDim.x + threadIdx.x;
  curand_init(seed + tid, 0, 0, &states[tid]);
}

double calculateHandTypeProbabilityCUDA(HandType type, int totalHands) {
  HandTypeCounts results = calculateAllProbabilitiesCUDA(totalHands);
  return results.getProbability(type);
}

__global__ void simulateHandsKernelAllTypes(curandState* states, unsigned long long* counts, int handsPerThread) {
  int tid = blockIdx.x * blockDim.x + threadIdx.x;
  curandState localState = states[tid];
  unsigned char hand[5];
  unsigned char deck[52];
  int deckPosition = 52;  // Force initial shuffle
  unsigned long long localCounts[10] = {0};

  for (int i = 0; i < handsPerThread; i++) {
    dealRandomHand(deck, deckPosition, &localState, hand);
    HandType type = getHandType(hand);
    localCounts[static_cast<int>(type)]++;
  }

  // Write results to global memory
  for (int i = 0; i < 10; i++) {
    atomicAdd(&counts[i], localCounts[i]);
  }

  states[tid] = localState;
}

// ...existing code through device functions...

HandTypeCounts calculateAllProbabilitiesCUDA(int totalHands) {
  // Single batch, using maximum thread capacity
  const int MAX_THREADS = 65536;  // 256 blocks * 256 threads
  const int numThreads = std::min(MAX_THREADS, BLOCK_SIZE * NUM_BLOCKS);
  const int actualBlocks = (numThreads + BLOCK_SIZE - 1) / BLOCK_SIZE;
  const int handsPerThread = (totalHands + numThreads - 1) / numThreads;

  // Allocate resources
  curandState* d_states = nullptr;
  unsigned long long* d_counts = nullptr;
  unsigned long long* h_counts = new unsigned long long[10]();
  HandTypeCounts result;

  try {
    // Allocate GPU memory
    if (cudaMalloc(&d_states, numThreads * sizeof(curandState)) != cudaSuccess ||
        cudaMalloc(&d_counts, 10 * sizeof(unsigned long long)) != cudaSuccess) {
      throw std::runtime_error("Failed to allocate GPU memory");
    }

    // Initialize memory and RNG
    if (cudaMemset(d_counts, 0, 10 * sizeof(unsigned long long)) != cudaSuccess) {
      throw std::runtime_error("Failed to initialize GPU memory");
    }

    initRNG<<<actualBlocks, BLOCK_SIZE>>>(d_states, time(nullptr));
    if (cudaGetLastError() != cudaSuccess) {
      throw std::runtime_error("Failed to initialize RNG");
    }

    // Run simulation
    simulateHandsKernelAllTypes<<<actualBlocks, BLOCK_SIZE>>>(d_states, d_counts, handsPerThread);
    if (cudaGetLastError() != cudaSuccess) {
      throw std::runtime_error("Kernel execution failed");
    }

    // Get results
    if (cudaMemcpy(h_counts, d_counts, 10 * sizeof(unsigned long long), cudaMemcpyDeviceToHost) != cudaSuccess) {
      throw std::runtime_error("Failed to copy results from GPU");
    }

    // Scale results if needed
    unsigned long long total = 0;
    for (int i = 0; i < 10; i++) {
      total += h_counts[i];
    }

    if (total > static_cast<unsigned long long>(totalHands)) {
      double scale = static_cast<double>(totalHands) / total;
      for (int i = 0; i < 10; i++) {
        result.counts[i] = static_cast<unsigned long long>(h_counts[i] * scale);
      }
    } else {
      for (int i = 0; i < 10; i++) {
        result.counts[i] = h_counts[i];
      }
    }
  } catch (const std::exception& e) {
    // Cleanup on error
    if (d_states) cudaFree(d_states);
    if (d_counts) cudaFree(d_counts);
    delete[] h_counts;
    throw;
  }

  // Normal cleanup
  cudaFree(d_states);
  cudaFree(d_counts);
  delete[] h_counts;

  return result;
}
