#include "bridge.hh"
#include <iostream>
#include "ostreamlock.h"
#include <algorithm>
#include <iomanip>

using namespace std;

void Bridge::arrive_eastbound(size_t id) {
    unique_lock<mutex> lock(bridgeLock);
    print(id, "arrived", true);

    /* We can't capture instance variables directly;
     * instead, we must capture 'this', which is the
     * current instance of Bridge being operated on.
     */
    done_westbound.wait(lock, [this]() -> bool {
        return crossing_westbound == 0;
    });
    crossing_eastbound++;
    print(id, "crossing", true);
}

void Bridge::leave_eastbound(size_t id) {
    unique_lock<mutex> lock(bridgeLock);
    crossing_eastbound--;
    if (crossing_eastbound == 0) {
        done_eastbound.notify_all();
    }
    print(id, "crossed", true);
}

void Bridge::arrive_westbound(size_t id) {
    unique_lock<mutex> lock(bridgeLock);
    print(id, "arrived", false);

    /* We can't capture instance variables directly;
     * instead, we must capture 'this', which is the
     * current instance of Bridge being operated on.
     */
    done_eastbound.wait(lock, [this]() -> bool {
        return crossing_eastbound == 0;
    });
    crossing_westbound++;
    print(id, "crossing", false);
}

void Bridge::leave_westbound(size_t id) {
    unique_lock<mutex> lock(bridgeLock);
    crossing_westbound--;
    if (crossing_westbound == 0) {
        done_westbound.notify_all();
    }
    print(id, "crossed", false);
}

void Bridge::print(size_t id, const string& message, bool isEastbound) {
    // Make a single string to contain e.g. "car #XX crossing going eastbound".
    // This must be a single string to right-align the BRIDGE graphic correctly.
    ostringstream ss;
    ss << "car #" << id << " " << message << " going " 
       << (isEastbound ? "eastbound" : "westbound");
    string statusMessage = ss.str();

    // Print the update, spaced equally so that the BRIDGE prints at the same
    // position on the line, no matter the length of the message.
    cout << oslock << setw(60) << left << statusMessage;

    // Now print the text representation of the bridge
    if (crossing_eastbound + crossing_westbound == 0) {
        cout << "BRIDGE: [none crossing]" << endl << osunlock;
    } else if (crossing_eastbound > 0 && crossing_westbound == 0) {
        cout << "BRIDGE: [";
        for (size_t i = 0; i < crossing_eastbound; i++) cout << "🚗";
        cout << " -->>]" << endl << osunlock;
    } else if (crossing_westbound > 0 && crossing_eastbound == 0) {
        cout << "BRIDGE: [<<-- ";
        for (size_t i = 0; i < crossing_westbound; i++) cout << "🚙";
        cout << "]" << endl << osunlock;
    } else {
        cout << "ERROR!!  cars stuck, trying to go both directions" << endl << osunlock;
    }
}
