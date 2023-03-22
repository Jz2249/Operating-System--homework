/*
 * This file tests the implementation of the Station class in station.cc. You
 * shouldn't need to modify this file (and we will test your code against
 * an unmodified version). Please report any bugs to the course staff.
 *
 * Note that passing these tests doesn't guarantee that your code is correct
 * or meets the specifications given, but hopefully it's at least pretty
 * close.
 */

#include <algorithm>
#include <atomic>
#include <cstdarg>
#include <functional>
#include <iostream>
#include <thread>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <map>

#include "caltrain.hh"

using namespace std;

// Interval for nanosleep corresponding to 1 ms.
struct timespec one_ms = {.tv_sec = 0, .tv_nsec = 1000000};

/// This function is used by tests to invoke wait_for_train in
/// a separate thread (so many passengers can be waiting at once).
/// This thread does *not* invoke boarded (even though this would
/// "normally" happen in the passenger thread); instead, boarded())
/// is invoked in the main thread, so it can precisely sequence the
/// calls to produce desired test scenarios.
void passenger(Station& station, atomic<int>& boarding_threads)
{
    station.wait_for_train();
    boarding_threads++;
}

/// Runs in a separate thread to simulate the arrival of a train.
/// \param station
///      Station where the train is arriving.
/// \param free_seats
///      Number of seats available on the train.
void train(Station& station, int free_seats, atomic<int>& loaded_trains)
{
    station.load_train(free_seats);
    loaded_trains++;
}

/// Wait for an atomic variable to be a given value.
/// \param var
///      The variable to watch.
/// \param count
///      Wait until @var is the value.
/// \param ms
///      Return after this many milliseconds even if @var hasn't
///      reached the desired value.
/// \return
///      True means the function succeeded, false means it failed.
bool wait_for(atomic<int>& var, int count, int ms)
{
    while (true) {
        if (var.load() == count) {
            return true;
        }
        if (ms <= 0) {
            return false;
        }
        nanosleep(&one_ms, nullptr);
        ms -= 1;
    }
}

// Trains arrive (both with and without space), but no passengers are waiting.
void no_waiting_passengers(void)
{
    Station station;

    /* an int that can be atomically accessed/ modified without a separate lock
     * that represents number of trains that have left the station
     * (i.e. load_train finished).
     */
    atomic<int> loaded_trains = 0;

    // Spawn a full train to enter the station
    cout << "Full train arrives with no waiting passengers" << endl;
    thread train1(train, ref(station), 0, ref(loaded_trains));
    train1.detach(); // so we don't have to call join

    // We should immediately see loaded_trains become 1
    if (!wait_for(loaded_trains, 1, 100)) {
        cout << "load_train didn't return immediately" << endl;
        return;
    } else {
        cout << "load_train returned" << endl;
    }

    // Spawn a train with 10 seats to enter the station
    cout << "Train with 10 seats arrives with no waiting passengers" << endl;
    thread train2(train, ref(station), 10, ref(loaded_trains));
    train2.detach(); // so we don't have to call join

    // We should immediately see loaded_trains become 2
    if (!wait_for(loaded_trains, 2, 100)) {
        cout << "load_train didn't return immediately" << endl;
        return;
    } else {
        cout << "load_train returned" << endl;
    }
}

/* A passenger arrives, followed by a train with space available.
 * This test does NOT check that load_train blocks until the passengers board -
 * even if load_train returns immediately, this test will still pass.
 */
void passenger_wait_for_train(void)
{   
    Station station;

    /* an int that can be atomically accessed/ modified without a separate lock
     * that represents number of trains that have left the station
     * (i.e. load_train finished) and number of passengers that have started 
     * boarding (i.e. wait_for_train finished).
     */
    atomic<int> loaded_trains = 0;
    atomic<int> boarding_threads = 0;

    cout << "Passenger arrives, begins waiting" << endl;
    thread pass1(passenger, ref(station), ref(boarding_threads));
    pass1.detach(); // so we don't have to call join

    // Wait to see if the passenger incorrectly boards
    if (wait_for(boarding_threads, 1, 1000)) {
        cout << "Error: passenger returned from wait_for_train when" 
        << " no empty seats were available" << endl;
        return;
    }

    // Spawn a train with 3 seats to enter the station
    cout << "Train arrives with 3 seats available" << endl;
    thread train1(train, ref(station), 3, ref(loaded_trains));
    train1.detach(); // so we don't have to call join
    
    // We should immediately see boarding_threads become 1
    if (wait_for(boarding_threads, 1, 100)) {
        cout << "Passenger started boarding" << endl;
    } else {
        cout << "Error: passenger didn't return from wait_for_train" << endl;
        return;
    }

    // Finish boarding one passenger at a time.
    cout << "Passenger finishes boarding" << endl;
    station.boarded();
}

/* A passenger arrives, followed by a train with space available.
 * This test does check that load_train blocks until the passengers board.
 */
void train_wait_for_passenger(void)
{    
    Station station;

    /* an int that can be atomically accessed/ modified without a separate lock
     * that represents number of trains that have left the station
     * (i.e. load_train finished) and number of passengers that have started 
     * boarding (i.e. wait_for_train finished).
     */
    atomic<int> loaded_trains = 0;
    atomic<int> boarding_threads = 0;

    cout << "Passenger arrives, begins waiting" << endl;
    thread pass1(passenger, ref(station), ref(boarding_threads));
    pass1.detach(); // so we don't have to call join

    // Wait to see if the passenger incorrectly boards
    if (wait_for(boarding_threads, 1, 1000)) {
        cout << "Error: passenger returned from wait_for_train when" 
        << " no empty seats were available" << endl;
        return;
    }

    // Spawn a train with 3 seats to enter the station
    cout << "Train arrives with 3 seats available" << endl;
    thread train1(train, ref(station), 3, ref(loaded_trains));
    train1.detach(); // so we don't have to call join

    // We should immediately see boarding_threads become 1
    if (wait_for(boarding_threads, 1, 100)) {
        cout << "Passenger started boarding" << endl;
    } else {
        cout << "Error: passenger didn't return from wait_for_train" << endl;
        return;
    }

    // Train should not leave until passenger is boarded
    if (loaded_trains.load() == 1) {
        cout << "Error: train left before passenger finished boarding" << endl;
        return;
    }

    // Finish boarding passenger.
    cout << "Passenger finishes boarding" << endl;
    station.boarded();

    // We should immediately see loaded trains become 1
    if (wait_for(loaded_trains, 1, 100)) {
        cout << "load_train returned, train left" << endl;
    } else {
        cout << "Error: load_train didn't return after passengers "
        "finished boarding" << endl;
    }
}

/* A passenger arrives, followed by a full train, then an empty train.
 * This test checks that the first train leaves even though someone is waiting,
 * because there is no space.
 */
void full_train_departs(void)
{    
    Station station;

    /* an int that can be atomically accessed/ modified without a separate lock
     * that represents number of trains that have left the station
     * (i.e. load_train finished) and number of passengers that have started 
     * boarding (i.e. wait_for_train finished).
     */
    atomic<int> loaded_trains = 0;
    atomic<int> boarding_threads = 0;

    cout << "Passenger arrives, begins waiting" << endl;
    thread pass1(passenger, ref(station), ref(boarding_threads));
    pass1.detach(); // so we don't have to call join

    // First train has no room.
    cout << "Train arrives with no empty seats" << endl;
    thread train1(train, ref(station), 0, ref(loaded_trains));
    train1.detach(); // so we don't have to call join

    // We should immediately see loaded_trains become 1
    if (!wait_for(loaded_trains, 1, 100)) {
        cout << "load_train didn't return immediately" << endl;
        return;
    } else {
        cout << "load_train returned, train left" << endl;
    }

    // See if the passenger incorrectly boards
    if (boarding_threads.load() == 1) {
        cout << "Error: passenger returned from wait_for_train" 
        << " when no empty seats were available" << endl;
        return;
    }

    // Second train arrives with room.
    cout << "Train arrives with 3 seats available" << endl;
    thread train2(train, ref(station), 3, ref(loaded_trains));
    train2.detach(); // so we don't have to call join

    // We should immediately see boarding_threads become 1
    if (wait_for(boarding_threads, 1, 100)) {
        cout << "Passenger started boarding" << endl;
    } else {
        cout << "Error: passenger didn't return from wait_for_train" << endl;
        return;
    }

    // The train shouldn't leave yet because the passenger isn't boarded
    if (loaded_trains.load() == 2) {
        cout << "Error: train left before passenger finished boarding" << endl;
        return;
    }

    // Finish boarding one passenger at a time.
    cout << "Passenger finishes boarding" << endl;
    station.boarded();

    if (!wait_for(loaded_trains, 2, 100)) {
        cout << "Error: load_train didn't return after passengers "
        "finished boarding" << endl;
    }
}

/* A passenger arrives, followed by an empty train, followed by a second
 * passenger. This test checks that the train waits for both passengers before
 * departing.
 */
void multiple_waiting(void)
{    
    Station station;

    /* an int that can be atomically accessed/ modified without a separate lock
     * that represents number of trains that have left the station
     * (i.e. load_train finished) and number of passengers that have started 
     * boarding (i.e. wait_for_train finished).
     */
    atomic<int> loaded_trains = 0;
    atomic<int> boarding_threads = 0;

    cout << "Passenger arrives, begins waiting" << endl;
    thread pass1(passenger, ref(station), ref(boarding_threads));
    pass1.detach(); // so we don't have to call join

    // Wait to see if the passenger incorrectly boards
    if (wait_for(boarding_threads, 1, 1000)) {
        cout << "Error: passenger returned from wait_for_train when" 
        << " no empty seats were available" << endl;
        return;
    }

    // train arrives with room.
    cout << "Train arrives with 3 seats available" << endl;
    thread train1(train, ref(station), 3, ref(loaded_trains));
    train1.detach(); // so we don't have to call join

    // We should immediately see boarding_threads become 1
    if (wait_for(boarding_threads, 1, 100)) {
        cout << "Passenger started boarding" << endl;
    } else {
        cout << "Error: passenger didn't return from wait_for_train" << endl;
        return;
    }

    // The train shouldn't leave yet because the passenger isn't boarded
    if (loaded_trains.load() == 1) {
        cout << "Error: train left before first passenger finished boarding" 
        << endl;
        return;
    }

    // Introduce a second passenger while the first one is boarding.
    cout << "Second passenger arrives" << endl;
    thread pass2(passenger, ref(station), ref(boarding_threads));
    pass2.detach(); // so we don't have to call join

    // We should immediately see boarding_threads become 2
    if (wait_for(boarding_threads, 2, 100)) {
        cout << "Second passenger started boarding" << endl;
    } else {
        cout << "Error: second passenger didn't return from wait_for_train"
        << endl;
        return;
    }

    // The train shouldn't leave yet because the passengers aren't boarded
    if (loaded_trains.load() == 1) {
        cout << "Error: train left before passengers finished boarding"
        << endl;
        return;
    }

    // Finish boarding one passenger at a time.
    cout << "First passenger finishes boarding" << endl;
    station.boarded();

    // The train shouldn't leave yet because the second passenger isn't boarded
    if (wait_for(loaded_trains, 1, 100)) {
        cout << "Error: train left before second passenger finished boarding"
        << endl;
        return;
    }

    cout << "Second passenger finishes boarding" << endl;
    station.boarded();

    // Now the train should leave since no-one else is waiting
    if (wait_for(loaded_trains, 1, 100)) {
        cout << "load_train returned, train left" << endl;
    } else {
        cout << "Error: load_train didn't return after passengers "
        "finished boarding" << endl;
    }
}

/* 4 passengers arrive, followed by a train with enough space.  Make sure they
 * all board in parallel (i.e. wait_for_train has returned, but the passenger
 * has not invoked boarded).
 */
void board_in_parallel(void)
{
    Station station;

    /* an int that can be atomically accessed/ modified without a separate lock
     * that represents number of trains that have left the station
     * (i.e. load_train finished) and number of passengers that have started 
     * boarding (i.e. wait_for_train finished).
     */
    atomic<int> loaded_trains = 0;
    atomic<int> boarding_threads = 0;

    cout << "4 passengers arrive, begin waiting" << endl;
    thread pass1(passenger, ref(station), ref(boarding_threads));
    pass1.detach(); // so we don't have to call join
    thread pass2(passenger, ref(station), ref(boarding_threads));
    pass2.detach(); // so we don't have to call join
    thread pass3(passenger, ref(station), ref(boarding_threads));
    pass3.detach(); // so we don't have to call join
    thread pass4(passenger, ref(station), ref(boarding_threads));
    pass4.detach(); // so we don't have to call join
    
    // Wait for a bit to give all the passengers time to wait on
    // condition variables. This is technically a race, since it's
    // possible that one of the threads could be blocked from running
    // for longer than this amount of time, but it's the best we can
    // without additional information from the Station class. Thus
    // this test may occasionally fail.
    usleep(1000000);

    // see if any passengers incorrectly board
    if (boarding_threads.load() > 0) {
        cout << "Error: " << boarding_threads.load() 
        << " passenger(s) returned from wait_for_train when" 
        << " no empty seats were available" << endl;
        return;
    }

    cout << "Train arrives with 4 empty seats" << endl;
    thread train1(train, ref(station), 4, ref(loaded_trains));
    train1.detach(); // so we don't have to call join

    // The passengers should immediately begin boarding
    if (wait_for(boarding_threads, 4, 100)) {
        cout << "4 passengers began boarding" << endl;
    } else {
        cout << "Error: expected 4 passengers to begin boarding, but "
        "actual number is " << boarding_threads.load() << endl;
        return;
    }

    cout << "3 passengers finished boarding" << endl;
    station.boarded();
    station.boarded();
    station.boarded();

    // Train should not depart yet, 1 more passenger is still boarding
    if (wait_for(loaded_trains, 1, 100)) {
        cout << "Error: load_train returned too soon" << endl;
        return;
    }

    cout << "Fourth passenger finished boarding" << endl;
    station.boarded();

    // Now train should leave, since no-one else is waiting
    if (wait_for(loaded_trains, 1, 100)) {
        cout << "load_train returned, train left" << endl;
    } else {
        cout << "Error: load_train didn't return when train was full" << endl;
        return;
    }
}

/* 4 passengers arrive, followed by a train with enough space for all but 1.  
 * Make sure any possible threads can board in parallel 
 * (i.e. wait_for_train has returned, but the passenger hasn't invoked boarded).
 */
void board_in_parallel_wait(void)
{
    Station station;

    /* an int that can be atomically accessed/ modified without a separate lock
     * that represents number of trains that have left the station
     * (i.e. load_train finished) and number of passengers that have started 
     * boarding (i.e. wait_for_train finished).
     */
    atomic<int> loaded_trains = 0;
    atomic<int> boarding_threads = 0;

    cout << "4 passengers arrive, begin waiting" << endl;
    thread pass1(passenger, ref(station), ref(boarding_threads));
    pass1.detach(); // so we don't have to call join
    thread pass2(passenger, ref(station), ref(boarding_threads));
    pass2.detach(); // so we don't have to call join
    thread pass3(passenger, ref(station), ref(boarding_threads));
    pass3.detach(); // so we don't have to call join
    thread pass4(passenger, ref(station), ref(boarding_threads));
    pass4.detach(); // so we don't have to call join
    
    // Wait for a bit to give all the passengers time to wait on
    // condition variables. This is technically a race, since it's
    // possible that one of the threads could be blocked from running
    // for longer than this amount of time, but it's the best we can
    // without additional information from the Station class. Thus
    // this test may occasionally fail.
    usleep(1000000);

    // see if any passengers incorrectly board
    if (boarding_threads.load() > 0) {
        cout << "Error: " << boarding_threads.load() 
        << " passenger(s) returned from wait_for_train when" 
        << " no empty seats were available" << endl;
        return;
    }

    cout << "Train arrives with 3 empty seats" << endl;
    thread train1(train, ref(station), 3, ref(loaded_trains));
    train1.detach(); // so we don't have to call join
    
    // Three passengers should immediately begin boarding
    if (wait_for(boarding_threads, 3, 100)) {
        cout << "3 passengers began boarding" << endl;
    } else {
        cout << "Error: expected 3 passengers to begin boarding, but "
        "actual number is " << boarding_threads.load() << endl;
        return;
    }

    cout << "2 passengers finished boarding" << endl;
    station.boarded();
    station.boarded();

    // Train should not depart yet, 1 more passenger is still boarding
    if (wait_for(loaded_trains, 1, 100)) {
        cout << "Error: load_train returned too soon" << endl;
        return;
    }

    cout << "Third passenger finished boarding" << endl;
    station.boarded();

    // Now train should leave, since it is full
    if (wait_for(loaded_trains, 1, 100)) {
        cout << "load_train returned, train left" << endl;
    } else {
        cout << "Error: load_train didn't return when train was full" << endl;
        return;
    }
    
    // Send another train for the last passenger.
    cout << "Another train arrives with 10 empty seats" << endl;
    thread train2(train, ref(station), 10, ref(loaded_trains));
    train2.detach(); // so we don't have to call join

    // Now last thread should immediately board
    if (wait_for(boarding_threads, 4, 100)) {
        cout << "Last passenger began boarding" << endl;
    } else {
        cout << "Error: last passenger didn't begin boarding" << endl;
        return;
    }

    cout << "Last passenger finished boarding" << endl;
    station.boarded();

    // Now train should leave, since no-one else is waiting
    if (wait_for(loaded_trains, 2, 100)) {
        cout << "load_train returned, train left" << endl;
    } else {
        cout << "Error: load_train didn't return after passenger "
        "finished boarding" << endl;
        return;
    }
}

/* This test checks that, if a train leaves with space still available, it
 * doesn't allow a passenger to come later (when there is no train) and think
 * it can board.
 */
void leftover(void)
{    
    Station station;

    /* an int that can be atomically accessed/ modified without a separate lock
     * that represents number of trains that have left the station
     * (i.e. load_train finished) and number of passengers that have started 
     * boarding (i.e. wait_for_train finished).
     */
    atomic<int> loaded_trains = 0;
    atomic<int> boarding_threads = 0;

    // train arrives with space, but nobody waiting.
    cout << "Train arrives with 3 empty seats" << endl;
    thread train1(train, ref(station), 3, ref(loaded_trains));
    train1.detach(); // so we don't have to call join

    // The train should leave immediately because nobody is waiting
    if (!wait_for(loaded_trains, 1, 100)) {
        cout << "load_train didn't return immediately" << endl;
        return;
    } else {
        cout << "load_train returned, train left" << endl;
    }

    cout << "Passenger arrives, begins waiting" << endl;
    thread pass1(passenger, ref(station), ref(boarding_threads));
    pass1.detach(); // so we don't have to call join

    // The passenger shouldn't board, since there is no train
    if (wait_for(boarding_threads, 1, 1000)) {
        cout << "Error: passenger returned from wait_for_train when no empty seats were available"
        << endl;
        return;
    }

    // Second train arrives with room.
    cout << "Train arrives with 3 seats available" << endl;
    thread train2(train, ref(station), 3, ref(loaded_trains));
    train2.detach(); // so we don't have to call join

    // Passenger should start boarding immediately    
    if (wait_for(boarding_threads, 1, 100)) {
        cout << "Passenger started boarding" << endl;
    } else {
        cout << "Error: passenger didn't return from wait_for_train"
        << endl;
        return;
    }

    // The train shouldn't leave yet, since the passenger is still boarding
    if (loaded_trains.load() == 2) {
        cout << "Error: train left before passenger finished boarding"
        << endl;
        return;
    }

    // Finish boarding the passenger.
    cout << "Passenger finishes boarding" << endl;
    station.boarded();

    if (wait_for(loaded_trains, 2, 100)) {
        cout << "load_train returned, train left" << endl;
    } else {
        cout << "Error: load_train didn't return after passenger "
        "finished boarding" << endl;
        return;
    }
}

void randomized(int total_passengers, int max_train_capacity)
{    
    Station station;

    /* an int that can be atomically accessed/ modified without a separate lock
     * that represents number of trains that have left the station
     * (i.e. load_train finished) and number of passengers that have started 
     * boarding (i.e. wait_for_train finished).
     */
    atomic<int> loaded_trains = 0;
    atomic<int> boarding_threads = 0;

    try {
        // A large number of passengers arrive, then a series of trains
        // arrive with varying numbers of available seats. Make sure that
        // each train leaves with the right number of passengers.
        int errors = 0;

        // Create a large number of passengers waiting in the station.
        cout << "Starting randomized test with " << total_passengers
        << " passengers" << endl;
        thread passengers[total_passengers];
        for (int i = 0; i < total_passengers; i++) {
            passengers[i] = thread(passenger, ref(station), 
                ref(boarding_threads));
            passengers[i].detach(); // so we don't have to call join
        }

        usleep(100000);

        if (boarding_threads.load() > 0) {
            cout << "Error: passenger returned from wait_for_train"
            << " when no train in station"
            << endl;
            errors++;
        }

        // Each iteration through this loop processes a train with random
        // capacity.
        int passengers_left = total_passengers;
        while (passengers_left > 0) {

            // Pick a random train size and reset state
            int free_seats = rand() % max_train_capacity;
            boarding_threads = 0;
            loaded_trains = 0;

            cout << "Train entering station with " << free_seats
            << " free seats, " << passengers_left 
            << " waiting passengers" << endl;

            thread train_thread(train, ref(station), free_seats, 
                ref(loaded_trains));
            train_thread.detach(); // so we don't have to call join

            int expected_boarders = min(passengers_left, free_seats);
            int boarded = 0;

            // Board passengers on this train
            while (true) {
                // boarding_threads will reach at most expected_boarders,
                // boarded should eventually reach boarding_threads
                while (boarding_threads > boarded) {
                    // board one more passenger
                    station.boarded();
                    boarded++;
                    passengers_left--;
                }

                if (boarded >= expected_boarders) {
                    break;
                }

                // boarding_threads should continue increasing
                if (boarding_threads.load() == boarded) {
                    usleep(100000);
                }
                if (boarding_threads.load() == boarded) {
                    cout << "Error: stuck waiting for passenger " << boarded
                    << " to start boarding" << endl;
                    errors++;
                }

                // The train shouldn't leave yet
                if (loaded_trains.load() == 1) {
                    cout << "Error: load_train returned after only " 
                    << boarded <<  " passengers finished boarding"
                    << endl;
                    errors++;
                }
            }

            // Now train should leave
            if (!wait_for(loaded_trains, 1, 100)) {
                cout << "Error: load_train didn't return after " << boarded
                << " passengers boarded" << endl;
                errors++;
            }

            nanosleep(&one_ms, nullptr);

            if (expected_boarders != boarding_threads) {
                cout << "Error: " << boarding_threads.load()
                << " passengers started boarding (expected "
                << expected_boarders << ")" << endl;
                errors++;
            }
        }
        cout << "Test completed with " << errors << " errors" << endl;
    }
    catch (system_error &e) {
        if (e.code().value() != EAGAIN)
            throw;
        cout << "Error: resource temporarily unavailable" << endl
        << "This usually means that the system ran out of threads."
        << "  Try running again, or, if it fails repeatedly,"
        << " log out and log in to a different myth machine." << endl
        << "  You can also try changing total_passengers in the"
        << " code to a smaller number (see code at end of main()),"
        << " but remember to change it back!"
        << endl;
        exit(1);
    }
}


/*
 * This creates a bunch of threads to simulate arriving trains and passengers.
 */
int main(int argc, char *argv[])
{
    srand(getpid() ^ time(NULL));

    // Add any new functions to map here
    unordered_map<string, function<void(void)>> testFns;
    testFns["no_waiting_passengers"] = no_waiting_passengers;
    testFns["passenger_wait_for_train"] = passenger_wait_for_train;
    testFns["train_wait_for_passenger"] = train_wait_for_passenger;
    testFns["full_train_departs"] = full_train_departs;
    testFns["multiple_waiting"] = multiple_waiting;
    testFns["board_in_parallel"] = board_in_parallel;
    testFns["board_in_parallel_wait"] = board_in_parallel_wait;
    testFns["leftover"] = leftover;
    // randomized is omitted, as it takes arguments

    if (argc == 1) {
        cout << "Available tests are:" << endl;
        for (const pair<const string&, const function<void(void)>&>& p : testFns) {
            cout << "\t" << p.first << endl;
        }
        cout << "\t" << "randomized [NUM_PASSENGERS] [MAX_TRAIN_SIZE]" << endl;
        return 0;
    }

    auto search = testFns.find(argv[1]);
    if (search != testFns.end()) {
        search->second();
    } else if (string(argv[1]) == "randomized") {
        // CHANGE HERE TO CHANGE DEFAULTS (change numbers after : )
        int passengers = argc > 2 ? atoi(argv[2]) : 1000;
        int max_train_size = argc > 3 ? atoi(argv[3]) : 50;
        if (max_train_size < 2) cout << "max train capacity is exclusive - please specify a number > 1" << endl;
        else randomized(passengers, max_train_size);
    } else {
        cout << "No test named '" << argv[1] << "'" << endl;
    }

    return 0;
}
