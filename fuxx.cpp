/*
 * Compile with:
 * 
 *   g++ fuxx.cpp -o fuxx -Wall -std=c++11 -O3
 *
 * Run with:
 *
 *   fuxx <memUsageMiB> <diskUsageMiB> [<verbose>]" 
 *
 * where memory usage and disk usage are in Mebibytes (1024*1024 bytes).
 * <verbose> can be any nonempty string.
 *
 */

#include <iostream>
#include <string>
#include <chrono>
#include <vector>

typedef std::chrono::high_resolution_clock::time_point timePointType;
typedef std::chrono::duration<uint64_t, std::nano>     nanoSecondsType;

static inline timePointType timeNow () {
    return std::chrono::high_resolution_clock::now();
}

static inline uint64_t timeDiff (timePointType& a, timePointType& b) {
    return std::chrono::duration_cast<nanoSecondsType>(b-a).count();
}

size_t memUsage = 0;
size_t diskUsage = 0;
size_t timeFactor = 100;
bool verbose = false;
uint64_t dummy = 0;

void report(std::string operation, uint64_t totalTime, uint64_t nrOps) {
  std::cout << "Total Time: " << totalTime << " nanoseconds for " 
            << nrOps << " operations." << std::endl;
  std::cout << "Time per '" << operation << "': " 
            << (double) totalTime / nrOps
            << " nanoseconds" << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void memoryLatencyRead() {
  std::cout << R"(
memoryLatencyRead:

This allocates as much memory as allowed and does single byte reads in a
random access fashion. The result is  the average time it takes to fetch
a cache line.

)";

  uint8_t* p = new uint8_t[memUsage];
  size_t pos = 0;
  size_t const nrTries = 300000000 * timeFactor / 100;

  for (size_t i = 0; i < memUsage; ++i) {
    p[i] = i & 0xff;
  }

  timePointType start = timeNow();

  for (size_t i = 0; i < nrTries; ++i) {
    dummy += p[pos];
    pos += 17*64;
    while (pos >= memUsage) {
      pos -= memUsage;
    }
  }

  timePointType end = timeNow();

  delete [] p;

  report("single byte read", timeDiff(start, end), nrTries);
}

////////////////////////////////////////////////////////////////////////////////

void memoryLatencyWrite() {
  std::cout << R"(
memoryLatencyWrite:

This allocates as much memory as allowed and does single byte writes
in a random access fashion. The result is the average time it takes to
write a byte.

)";

  uint8_t* p = new uint8_t[memUsage];
  size_t pos = 0;
  size_t const nrTries = 300000000 * timeFactor / 100;

  for (size_t i = 0; i < memUsage; ++i) {
    p[i] = i & 0xff;
  }

  timePointType start = timeNow();

  for (size_t i = 0; i < nrTries; ++i) {
    p[pos] = i % 0xff;
    pos += 17*64;
    while (pos >= memUsage) {
      pos -= memUsage;
    }
  }

  timePointType end = timeNow();

  delete [] p;

  report("single byte write", timeDiff(start, end), nrTries);
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cout << R"(
Usage: fuxx <memUsageMiB> <diskUsageMiB> [<timeFactor>] 
       where <memUsage> and <diskUsage> are in Mebibytes
       and timeFactor is in percent of the normal runtime.

)";
    return 0;
  }
  memUsage = std::stoul(argv[1]);
  memUsage = memUsage > 0 ? memUsage : 128;                // at least 1 MiB
  memUsage = memUsage < 1024 * 1024 ? memUsage : 1024;     // at most  1 TiB
  std::cout << "Memory usage in MiB: " << memUsage  << std::endl;

  diskUsage = std::stoul(argv[2]);
  diskUsage = diskUsage >= 16 ? diskUsage : 16;            // at least 16 MiB
  diskUsage = diskUsage < 1024 * 1024 ? diskUsage : 1024;  // at most   1 TiB
  std::cout << "Disk   usage in MiB: " << diskUsage << std::endl;

  memUsage *= 1024 * 1024;    // go to bytes
  diskUsage *= 1024 * 1024;   // go to bytes

  if (argc >= 4) {
    timeFactor = std::stoul(argv[3]);
    timeFactor = timeFactor > 0 ? timeFactor : 100;
    timeFactor = timeFactor < 1000 ? timeFactor : 100;
    std::cout << "Time factor        : " << timeFactor << "%" << std::endl;
  }

  memoryLatencyRead();
  memoryLatencyWrite();

  std::cout << "\nPlease ignore: " << dummy << std::endl;
  return 0;
}

