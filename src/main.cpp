#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>  // Add this include for stringstream
#include <string>
#include <thread>
#include <vector>
#include "cuda_probability.cuh"
#include "deck.hpp"
#include "hand.hpp"
#include "probability.hpp"
#include "utils.hpp"

static bool noProgressBar = false;

void printUsage(const char* programName) {
  std::cout << "Usage: " << programName << " [options]\n"
            << "Options:\n"
            << "  -h, --help     Show this help message\n"
            << "  -c, --cpu      Use CPU implementation (default)\n"
            << "  -g, --gpu      Use CUDA GPU implementation\n"
            << "  -b, --bench    Run both implementations and compare\n"
            << "  -a, --all      Calculate probabilities for all hand types\n"
            << "  -t TYPE        Hand type to calculate probability for:\n"
            << "                 rf (Royal Flush), sf (Straight Flush),\n"
            << "                 4k (Four of a Kind), fh (Full House),\n"
            << "                 fl (Flush), st (Straight), 3k (Three of a Kind),\n"
            << "                 2p (Two Pair), 1p (One Pair), hc (High Card)\n"
            << "  -n NUMBER      Number of hands to simulate (default: 100000000)\n"
            << "  --no_progress_bar  Disable progress bar\n"
            << std::endl;
}

HandType parseHandType(const std::string& type) {
  if (type == "rf") return HandType::RoyalFlush;
  if (type == "sf") return HandType::StraightFlush;
  if (type == "4k") return HandType::FourOfAKind;
  if (type == "fh") return HandType::FullHouse;
  if (type == "fl") return HandType::Flush;
  if (type == "st") return HandType::Straight;
  if (type == "3k") return HandType::ThreeOfAKind;
  if (type == "2p") return HandType::TwoPair;
  if (type == "1p") return HandType::OnePair;
  if (type == "hc") return HandType::HighCard;
  throw std::runtime_error("Invalid hand type: " + type);
}

void printProgress(float progress) {
  if (noProgressBar) return;
  int barWidth = 70;
  std::cout << "[";
  int pos = static_cast<int>(barWidth * progress);
  for (int i = 0; i < barWidth; ++i) {
    if (i < pos)
      std::cout << "=";
    else if (i == pos)
      std::cout << ">";
    else
      std::cout << " ";
  }
  std::cout << "] " << int(progress * 100.0) << " %\r";
  std::cout.flush();
}

std::string formatNumber(unsigned long long num) {
    std::stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::fixed << std::setprecision(0) << num;
    return ss.str();
}

// Fix function signature to avoid parameter redefinition
void runAndPrintResults(bool isCuda, HandType type, const HandTypeCounts& results, double elapsed, int simCount) {
  double probability = results.getProbability(type);
  double theoretical = getTheoreticalProbability(type);
  double error = std::abs((probability * 100) - theoretical);

  std::cout << "\nResults (" << (isCuda ? "CUDA GPU" : "CPU") << "):\n"
            << "----------------\n"
            << "Hand type: " << Hand::getHandTypeName(type) << "\n"
            << "Hands found: " << formatNumber(results.counts[static_cast<size_t>(type)]) << "\n"
            << "Probability: " << std::fixed << std::setprecision(4) << (probability * 100) << "%\n"
            << "Theoretical: " << theoretical << "%\n"
            << "Error margin: " << error << "%\n"
            << "Time: " << std::fixed << std::setprecision(2) << elapsed << " seconds\n"
            << "Speed: " << formatNumber(static_cast<unsigned long long>(simCount / elapsed)) << " hands/sec\n";
}

void runAndPrintAllResults(bool isCuda, int handsToSimulate, double elapsed, const HandTypeCounts& results) {
    std::cout << "\nSimulating " << formatNumber(handsToSimulate) << " poker hands..." << std::endl;

    // Print summary table
    std::cout << "\nSummary Table (" << (isCuda ? "CUDA GPU" : "CPU") << "):\n";
    std::cout << std::string(80, '=') << "\n";
    std::cout << std::left << std::setw(16) << "Hand Type" << std::right << std::setw(15) << "Count" << std::setw(12)
              << "Calculated" << std::setw(12) << "Theoretical" << std::setw(12) << "Error" << "\n";
    std::cout << std::string(80, '-') << "\n";

    unsigned long long totalHands = 0;
    for (const auto& count : results.counts) totalHands += count;

    for (int t = 0; t < static_cast<int>(HandType::Count); ++t) {
        HandType type = static_cast<HandType>(t);
        double prob = results.getProbability(type);
        double theoretical = getTheoreticalProbability(type);

        std::cout << std::left << std::setw(16) << Hand::getHandTypeName(type) << std::right << std::setw(15)
                  << formatNumber(results.counts[t]) << std::fixed << std::setprecision(4) << std::setw(11)
                  << (prob * 100) << "%" << std::setw(11) << theoretical << "%" << std::setw(11)
                  << std::abs((prob * 100) - theoretical) << "%\n";
    }

    std::cout << std::string(80, '-') << "\n";
    std::cout << std::left << std::setw(16) << "Total:" << std::right << std::setw(15) << formatNumber(totalHands)
              << "\nTime: " << std::fixed << std::setprecision(2) << elapsed << "s"
              << "\nSpeed: " << formatNumber(static_cast<unsigned long long>(totalHands / elapsed)) << " hands/s\n";
    std::cout << std::string(80, '=') << "\n";
}

int main(int argc, char* argv[]) {
  std::locale::global(std::locale(""));
  std::cout.imbue(std::locale(""));

  bool useCuda = false;
  bool benchmark = false;
  bool allTypes = true;  // Changed default to true
  int totalHands = 100'000'000;
  HandType targetType = HandType::ThreeOfAKind;
  bool typeSpecified = false;  // New flag to track if -t was used

  // Parse command line arguments
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "-t" && i + 1 < argc) {
      try {
        targetType = parseHandType(argv[++i]);
        allTypes = false;  // Override default when type is specified
        typeSpecified = true;
      } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        printUsage(argv[0]);
        return 1;
      }
    } else if (arg == "-h" || arg == "--help") {
      printUsage(argv[0]);
      return 0;
    } else if (arg == "-c" || arg == "--cpu") {
      useCuda = false;
      benchmark = false;
    } else if (arg == "-g" || arg == "--gpu") {
      useCuda = true;
      benchmark = false;
    } else if (arg == "-b" || arg == "--bench") {
      benchmark = true;
    } else if (arg == "-a" || arg == "--all") {
      allTypes = true;
      typeSpecified = false;
    } else if (arg == "-n" && i + 1 < argc) {
      totalHands = std::stoi(argv[++i]);
      if (totalHands <= 0) {
        std::cerr << "Error: Number of hands must be positive\n";
        return 1;
      }
    } else if (arg == "--no_progress_bar") {
      noProgressBar = true;
    } else {
      std::cerr << "Unknown argument: " << arg << "\n";
      printUsage(argv[0]);
      return 1;
    }
  }

  std::cout << "Starting poker probability simulation...\n";
  if (!allTypes) {
    std::cout << "Hand type: " << Hand::getHandTypeName(targetType) << "\n";
  } else {
    std::cout << "Analyzing all hand types\n";
  }
  std::cout << "Implementation: " << (useCuda ? "CUDA GPU" : "CPU") << "\n"
            << "Hands to simulate: " << totalHands << "\n"
            << "CPU Threads: " << std::thread::hardware_concurrency() << std::endl;

  double cpuProb = 0, cudaProb = 0;
  double cpuTime = 0, cudaTime = 0;

  if (allTypes) {
    if (benchmark) {
      std::cout << "\nRunning CPU implementation...\n";
      auto cpuStart = std::chrono::high_resolution_clock::now();
      HandTypeCounts cpuResults = calculateAllProbabilities(totalHands);
      auto cpuEnd = std::chrono::high_resolution_clock::now();
      double cpuElapsed = std::chrono::duration<double>(cpuEnd - cpuStart).count();
      runAndPrintAllResults(false, totalHands, cpuElapsed, cpuResults);

      std::cout << "\nRunning CUDA implementation...\n";
      auto cudaStart = std::chrono::high_resolution_clock::now();
      HandTypeCounts cudaResults = calculateAllProbabilitiesCUDA(totalHands);
      auto cudaEnd = std::chrono::high_resolution_clock::now();
      double cudaElapsed = std::chrono::duration<double>(cudaEnd - cudaStart).count();
      runAndPrintAllResults(true, totalHands, cudaElapsed, cudaResults);

      // Print speedup comparison
      double speedup = cpuElapsed / cudaElapsed;
      std::cout << "\nPerformance Comparison:\n"
                << "----------------\n"
                << "CPU Time: " << std::fixed << std::setprecision(2) << cpuElapsed << "s\n"
                << "GPU Time: " << std::fixed << std::setprecision(2) << cudaElapsed << "s\n"
                << "CUDA Speedup: " << std::fixed << std::setprecision(2) << speedup << "x\n";
    } else {
      auto start = std::chrono::high_resolution_clock::now();
      HandTypeCounts results = useCuda ? calculateAllProbabilitiesCUDA(totalHands) : calculateAllProbabilities(totalHands);
      auto end = std::chrono::high_resolution_clock::now();
      double elapsed = std::chrono::duration<double>(end - start).count();
      runAndPrintAllResults(useCuda, totalHands, elapsed, results);
    }
  } else {
    if (benchmark) {
      // CPU implementation
      std::cout << "\nRunning CPU implementation...\n";
      auto start = std::chrono::high_resolution_clock::now();
      HandTypeCounts cpuResults = calculateAllProbabilities(totalHands);
      auto end = std::chrono::high_resolution_clock::now();
      auto cpuElapsed = std::chrono::duration<double>(end - start).count();
      runAndPrintResults(false, targetType, cpuResults, cpuElapsed, totalHands);

      // CUDA implementation
      std::cout << "\nRunning CUDA implementation...\n";
      start = std::chrono::high_resolution_clock::now();
      HandTypeCounts cudaResults = calculateAllProbabilitiesCUDA(totalHands);
      end = std::chrono::high_resolution_clock::now();
      auto cudaElapsed = std::chrono::duration<double>(end - start).count();
      runAndPrintResults(true, targetType, cudaResults, cudaElapsed, totalHands);

      // Print speedup comparison
      double speedup = cpuElapsed / cudaElapsed;
      std::cout << "\nPerformance Comparison:\n"
                << "----------------\n"
                << "CUDA Speedup: " << std::fixed << std::setprecision(2) << speedup << "x\n";
    } else {
      auto start = std::chrono::high_resolution_clock::now();
      HandTypeCounts results =
          useCuda ? calculateAllProbabilitiesCUDA(totalHands) : calculateAllProbabilities(totalHands);
      auto end = std::chrono::high_resolution_clock::now();
      auto elapsed = std::chrono::duration<double>(end - start).count();
      runAndPrintResults(useCuda, targetType, results, elapsed, totalHands);
    }
  }

  return 0;
}