#ifndef BRIDGE_H
#define BRIDGE_H

#include <condition_variable>
#include <mutex>
#include <string>

class Bridge {
public:
    Bridge() {};

    /* Thread-safe; indicate that the car with the given ID has arrived at
     * the bridge going eastbound.  Blocks until the car is able to start
     * crossing the bridge, and then returns.
     */
    void arrive_eastbound(size_t id);

    /* Thread-safe; indicate that the car with the given ID has finished
     * crossing the bridge going eastbound.
     */
    void leave_eastbound(size_t id);

    /* Thread-safe; indicate that the car with the given ID has arrived at
     * the bridge going westbound. Blocks until the car is able to start
     * crossing the bridge, and then returns.
     */
    void arrive_westbound(size_t id);

    /* Thread-safe; indicate that the car with the given ID has finished
     * crossing the bridge going westbound.
     */
    void leave_westbound(size_t id);
    
private:
    /* A private helper method to print out a status update for the given car,
     * and print a text representation of the bridge.
     */
    void print(size_t id, const std::string& message, bool isEastbound);

    // Synchronizes access to all information in this object.
    std::mutex bridgeLock;

    // Number of cars currently crossing in each direction (only one of
    // these can be nonzero at any given time).
    size_t crossing_eastbound = 0;
    size_t crossing_westbound = 0;

    // Signaled to indicate that there are no longer any cars crossing
    // in the eastbound direction.
    std::condition_variable_any done_eastbound;

    // Signaled to indicate that there are no longer any cars crossing
    // in the westbound direction.
    std::condition_variable_any done_westbound;
};

#endif /* BRIDGE_H */