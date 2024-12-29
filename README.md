# README.md

# Poker Probability Simulation

A high-performance simulator that calculates poker hand probabilities using both CPU and CUDA GPU implementations.

## Features

- Multi-threaded CPU implementation with optimized card representation
- High-performance CUDA GPU implementation
- Automatic batch processing for large simulations
- Comprehensive benchmarking of CPU vs GPU performance
- Complete analysis of all poker hand types
- Formatted large number output (e.g., 1,234,567)
- Accurate probability calculations (within 0.001% of theoretical)
- Memory-efficient card representation (4 bits rank, 2 bits suit)

## Project Structure

```
poker-probability/
├── src/
│   ├── main.cpp              # Application entry point
│   ├── card.cpp              # Card class implementation
│   ├── deck.cpp              # Deck class implementation
│   ├── hand.cpp              # Hand class implementation
│   ├── probability.cpp       # CPU probability implementation
│   └── cuda_probability.cu   # CUDA probability implementation
├── include/
│   ├── card.hpp             # Card class header
│   ├── deck.hpp             # Deck class header
│   ├── hand.hpp             # Hand class header
│   ├── probability.hpp       # CPU probability header
│   └── cuda_probability.cuh  # CUDA probability header
├── CMakeLists.txt           # CMake build configuration
└── README.md                # Project documentation
```

## Requirements

- C++17 compatible compiler
- CMake 3.18 or higher
- CUDA Toolkit 11.0 or higher
- NVIDIA GPU with Compute Capability 6.0 or higher

## Building the Project

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## Usage

```bash
poker-probability [options]

Options:
  -h, --help     Show help message
  -c, --cpu      Use CPU implementation (default)
  -g, --gpu      Use CUDA GPU implementation
  -b, --bench    Run both implementations and compare
  -a, --all      Calculate probabilities for all hand types (default)
  -t TYPE        Calculate specific hand type probability
  -n NUMBER      Number of hands to simulate (default: 100,000,000)

Hand Types:
  rf  Royal Flush       (0.0001539%)
  sf  Straight Flush   (0.00139%)
  4k  Four of a Kind   (0.0240%)
  fh  Full House       (0.1441%)
  fl  Flush            (0.1965%)
  st  Straight         (0.3925%)
  3k  Three of a Kind  (2.1128%)
  2p  Two Pair         (4.7539%)
  1p  One Pair        (42.2569%)
  hc  High Card       (50.1177%)
```

### Examples

Run benchmark comparison of all hand types:
```bash
./poker-probability -b -a
```

Analyze specific hand type with GPU:
```bash
./poker-probability -g -t fh -n 1,000,000,000
```

## Performance

- CPU: ~10-20 million hands/sec (multi-threaded)
- GPU: ~300-400 million hands/sec
- Typical speedup: 30-50x with GPU
- Memory usage: 1 byte per card
- Scales efficiently up to billions of hands

## Output Format

### All Hand Types Analysis

- CPU: 2x Intel(R) Xeon(R) Gold 6148 CPU @ 2.40GHz
- GPU: Nvidia Titan V 12GB

```
Starting poker probability simulation...
Analyzing all hand types
Implementation: CPU
Hands to simulate: 2,000,000,000
CPU Threads: 40

Running CPU implementation...
[=====================================================================>] 99 %
Simulating 2,000,000,000 poker hands...

Summary Table (CPU):
================================================================================
Hand Type                 Count  Calculated Theoretical       Error
--------------------------------------------------------------------------------
Royal Flush               3,021     0.0002%     0.0002%     0.0000%
Straight Flush           27,447     0.0014%     0.0014%     0.0000%
Four of a Kind          479,143     0.0240%     0.0240%     0.0001%
Full House            2,880,489     0.1440%     0.1441%     0.0001%
Flush                 3,934,757     0.1967%     0.1965%     0.0002%
Straight              7,846,467     0.3923%     0.3925%     0.0002%
Three of a Kind      42,267,240     2.1134%     2.1128%     0.0006%
Two Pair             95,092,577     4.7546%     4.7539%     0.0007%
One Pair            845,161,786    42.2581%    42.2569%     0.0012%
High Card         1,002,307,073    50.1154%    50.1177%     0.0023%
--------------------------------------------------------------------------------
Total:            2,000,000,000
Time: 227.52s
Speed: 8,790,598 hands/s
================================================================================

Running CUDA implementation...

Simulating 2,000,000,000 poker hands...

Summary Table (CUDA GPU):
================================================================================
Hand Type                 Count  Calculated Theoretical       Error
--------------------------------------------------------------------------------
Royal Flush               3,160     0.0002%     0.0002%     0.0000%
Straight Flush           27,919     0.0014%     0.0014%     0.0000%
Four of a Kind          479,600     0.0240%     0.0240%     0.0000%
Full House            2,881,449     0.1441%     0.1441%     0.0000%
Flush                 3,929,038     0.1965%     0.1965%     0.0000%
Straight              7,844,940     0.3922%     0.3925%     0.0003%
Three of a Kind      42,244,609     2.1122%     2.1128%     0.0006%
Two Pair             95,071,649     4.7536%     4.7539%     0.0003%
One Pair            845,137,324    42.2569%    42.2569%     0.0000%
High Card         1,002,380,307    50.1190%    50.1177%     0.0013%
--------------------------------------------------------------------------------
Total:            1,999,999,995
Time: 2.92s
Speed: 684,003,509 hands/s
================================================================================

Performance Comparison:
----------------
CPU Time: 227.52s
GPU Time: 2.92s
CUDA Speedup: 77.81x
```

## Implementation Details

- Compact card representation: `rank << 2 | suit`
- Optimized bit operations for hand evaluation
- GPU-optimized memory access patterns
- Automatic batch processing for large datasets
- Lock-free thread synchronization
- Efficient deck shuffling algorithm

## License

MIT License - See LICENSE file for details
