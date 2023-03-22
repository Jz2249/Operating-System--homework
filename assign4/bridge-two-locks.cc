#include "bridge-two-locks.hh"
#include <iostream>
#include "ostreamlock.h"
#include "thread-utils.h"
#include <algorithm>
#include <iomanip>

using namespace std;

void Bridge::arrive_eastbound(size_t id) {
    print(id, "arrived", true);

    // Wait for no more westbound crossing cars
    westLock.lock();
    done_westbound.wait(westLock, [this]() -> bool {
        return crossing_westbound == 0;
    });
    westLock.unlock();

    // Now start crossing eastbound
    eastLock.lock();
    crossing_eastbound++;
    eastLock.unlock();

    print(id, "crossing", true);
}

void Bridge::leave_eastbound(size_t id) {
    eastLock.lock();
    crossing_eastbound--;
    if (crossing_eastbound == 0) {
        done_eastbound.notify_all();
    }
    eastLock.unlock();
    print(id, "crossed", true);
}

void Bridge::arrive_westbound(size_t id) {
    print(id, "arrived", false);

    // Wait for no more eastbound crossing cars
    eastLock.lock();
    done_eastbound.wait(eastLock, [this]() -> bool {
        return crossing_eastbound == 0;
    });
    eastLock.unlock();
    sleep_for(500);
    // Now start crossing westbound
    westLock.lock();
    crossing_westbound++;
    westLock.unlock();

    print(id, "crossing", false);
}

void Bridge::leave_westbound(size_t id) {
    westLock.lock();
    crossing_westbound--;
    if (crossing_westbound == 0) {
        done_westbound.notify_all();
    }
    westLock.unlock();
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
    eastLock.lock();
    westLock.lock();
    size_t num_east = crossing_eastbound;
    size_t num_west = crossing_westbound;
    eastLock.unlock();
    westLock.unlock();

    if (num_east + num_west == 0) {
        cout << "BRIDGE: [none crossing]" << endl << osunlock;
    } else if (num_east > 0 && num_west == 0) {
        cout << "BRIDGE: [";
        for (size_t i = 0; i < num_east; i++) cout << "🚗";
        cout << " -->>]" << endl << osunlock;
    } else if (num_west > 0 && num_east == 0) {
        cout << "BRIDGE: [<<-- ";
        for (size_t i = 0; i < num_west; i++) cout << "🚙";
        cout << "]" << endl << osunlock;
    } else {
        cout << "ERROR!!  cars stuck, trying to go both directions" << endl << osunlock;
    }
}
