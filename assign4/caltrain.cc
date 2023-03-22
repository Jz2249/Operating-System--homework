#include "caltrain.hh"

using namespace std;

Station::Station()
{
    // Initialize any instance variables here as needed
}

void Station::load_train(int available)
{
    // You need to implement this
    unique_lock<mutex> lock(mutex_);
    seat_available = available;
    if (available != 0) done_waiting.notify_all();
    done_boarded.wait(lock, [this]() -> bool {
        return wait_boarded == 0 && (seat_available == 0 || (num_waiting == 0 && seat_available > 0));
        });
    seat_available = 0;
}

void Station::wait_for_train()
{
    // You need to implement this
    unique_lock<mutex> lock(mutex_);
    num_waiting++;
    done_waiting.wait(lock, [this]() -> bool {
        return seat_available > 0;
    });
    num_waiting--;
    wait_boarded++;
    seat_available--;
}

void Station::boarded()
{
    // You need to implement this
    unique_lock<mutex> lock(mutex_);
    wait_boarded--;
    done_boarded.notify_all();
}