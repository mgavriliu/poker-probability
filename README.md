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
```
Starting poker probability simulation...
Analyzing all hand types
Implementation: CPU
Hands to simulate: 100,000,000
CPU Threads: 40

Running CPU implementation...
[=====================================================================>] 99 %
Simulating 100,000,000 poker hands...

Summary Table (CPU):
================================================================================
Hand Type                 Count  Calculated Theoretical       Error
--------------------------------------------------------------------------------
Royal Flush                 171     0.0002%     0.0002%     0.0000%
Straight Flush            1,316     0.0013%     0.0014%     0.0001%
Four of a Kind           23,946     0.0239%     0.0240%     0.0001%
Full House              143,991     0.1440%     0.1441%     0.0001%
Flush                   197,102     0.1971%     0.1965%     0.0006%
Straight                391,568     0.3916%     0.3925%     0.0009%
Three of a Kind       2,114,275     2.1143%     2.1128%     0.0015%
Two Pair              4,750,505     4.7505%     4.7539%     0.0034%
One Pair             42,255,323    42.2553%    42.2569%     0.0016%
High Card            50,121,803    50.1218%    50.1177%     0.0041%
--------------------------------------------------------------------------------
Total:              100,000,000
Time: 12.36s
Speed: 8,092,837 hands/s
================================================================================

Running CUDA implementation...

Simulating 100,000,000 poker hands...

Summary Table (CUDA GPU):
================================================================================
Hand Type                 Count  Calculated Theoretical       Error
--------------------------------------------------------------------------------
Royal Flush                 147     0.0001%     0.0002%     0.0000%
Straight Flush            1,429     0.0014%     0.0014%     0.0000%
Four of a Kind           23,898     0.0239%     0.0240%     0.0001%
Full House              144,187     0.1442%     0.1441%     0.0001%
Flush                   196,264     0.1963%     0.1965%     0.0002%
Straight                393,402     0.3934%     0.3925%     0.0009%
Three of a Kind       2,111,028     2.1110%     2.1128%     0.0018%
Two Pair              4,752,307     4.7523%     4.7539%     0.0016%
One Pair             42,260,141    42.2601%    42.2569%     0.0032%
High Card            50,117,191    50.1172%    50.1177%     0.0005%
--------------------------------------------------------------------------------
Total:               99,999,994
Time: 0.33s
Speed: 306,458,346 hands/s
================================================================================

Performance Comparison:
----------------
CPU Time: 12.36s
GPU Time: 0.33s
CUDA Speedup: 37.87x
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