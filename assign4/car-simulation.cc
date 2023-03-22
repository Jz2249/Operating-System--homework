/**
 * Implements a simulation where cars cross a one-lane bridge.  Each car is
 * represented by a thread, going either westbound or eastbound.
 */

#include <thread>
#include "bridge.hh"
#include "random-generator.h"
#include "thread-utils.h"
using namespace std;

// The number of cars to spawn to go across the bridge
static const unsigned int kNumCars = 24;

// The bounds for how long it could take for a car to get to the bridge
static const unsigned int kLowApproachTimeMS = 0;
static const unsigned int kHighApproachTimeMS = 3000;

// The bounds for how long it could take for a car to cross the bridge
static const unsigned int kLowCrossTimeMS = 100;
static const unsigned int kHighCrossTimeMS = 500;

static mutex rgenLock;
static RandomGenerator rgen;

/* Locks around the shared random generator and returns a random value
 * between low and high to sleep for (in milliseconds).
 */
static unsigned int getSleepDuration(unsigned int low, unsigned int high) {
  unique_lock<mutex> ul(rgenLock);
  return rgen.getNextInt(low, high);
}

// Simulates "crossing the bridge" by sleeping for a random amount of time.
static void driveAcross() {
  sleep_for(getSleepDuration(kLowCrossTimeMS, kHighCrossTimeMS));
}

// Simulates "driving up to the bridge" by sleeping for a random amount of time.
static void approachBridge() {
  sleep_for(getSleepDuration(kLowApproachTimeMS, kHighApproachTimeMS));
}

// Simulates flipping a fair coin, returning true or false with equal probability
static bool flipCoin() {
  unique_lock<mutex> ul(rgenLock);
  return rgen.getNextBool(0.5);
}

// Arrive at, cross, and depart from the bridge going east.
static void crossBridgeEast(size_t id, Bridge& bridge) {
  approachBridge();   // sleep
  bridge.arrive_eastbound(id);
  driveAcross();      // sleep
  bridge.leave_eastbound(id);
}

// Arrive at, cross, and depart from the bridge going west.
static void crossBridgeWest(size_t id, Bridge& bridge) {
  approachBridge();   // sleep
  bridge.arrive_westbound(id);
  driveAcross();      // sleep
  bridge.leave_westbound(id);
}


int main(int argc, const char *argv[]) {
  Bridge bridge;
  thread cars[kNumCars];

  for (size_t i = 0; i < kNumCars; i++) {
    if (flipCoin()) {
      cars[i] = thread(crossBridgeEast, i, ref(bridge));
    } else {
      cars[i] = thread(crossBridgeWest, i, ref(bridge));
    }
  }

  for (thread& car : cars) {
    car.join();
  }
  return 0;
}
