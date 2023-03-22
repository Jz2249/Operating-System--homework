/*
 * This file tests the implementation of the Party class in party.cc. You
 * shouldn't need to modify this file (and we will test your code against
 * an unmodified version). Please report any bugs to the course staff.
 *
 * Note that passing these tests doesn't guarantee that your code is correct
 * or meets the specifications given, but hopefully it's at least pretty
 * close.
 */

#include <atomic>
#include <cstdarg>
#include <functional>
#include <iostream>
#include <thread>

#include <string.h>
#include <unistd.h>
#include <map>

#include "party.hh"

// Interval for nanosleep corresponding to 1 ms.
struct timespec one_ms = {.tv_sec = 0, .tv_nsec = 1000000};

// Total number of guests that have been matched in this experiment.
std::atomic<int> matched;

// Total number of guests that have started execution in this experiment
// (used to ensure a particular ordering of guests, but it's not perfect).
std::atomic<int> started;

// Total number of errors detected. //
int num_errors;

/**
 * Verify whether a guest matched as expected and print info about
 * a match or error.
 * \param guest
 *      Name of a particular guest.
 * \param expected
 *      Expected match for that guest (empty string means no match expected
 *      yet).
 * \param actual
 *      Actual match for the guest so far: empty means no one has been
 *      matched to this guest yet.
 * \return
 *      1 means that expected and actual didn't match, zero means they did.
 */
int check_match(std::string guest, std::string expected, std::string actual)
{
    if (actual == expected) {
        if (expected.length() > 0) {
            std::cout << guest.c_str() << " received " << actual.c_str()
                    << " as its match" << std::endl;
        }
        return 0;
    } else if (expected.length() == 0) {
        std::cout << "Error: " << guest.c_str() << " matched prematurely with "
                << actual.c_str() << std::endl;
        return 1;
    } else if (actual.length() == 0) {
        std::cout << "Error: " << guest.c_str() << " was supposed to receive "
                << expected.c_str() << " as match, but it hasn't matched yet"
                << std::endl;
        return 1;
    } else {
        std::cout << "Error: " << guest.c_str() << " was supposed to receive "
                << expected.c_str() << " as match, but it "
                "received " << actual.c_str() << " instead" << std::endl;
        return 1;
    }
}

/*
 * Wait for a given number of matches to occur.
 * \param count
 *      Wait until matched reaches this value
 * \param ms
 *      Return after this many milliseconds even if match hasn't
 *      reached the desired value.
 * \return
 *      True means the function succeeded, false means it failed.
 */
bool wait_for_matches(int count, int ms)
{
    while (1) {
        if (matched >= count) {
            return true;
        }
        if (ms <= 0) {
            return false;
        }
        nanosleep(&one_ms, nullptr);
        ms -= 1;
    }
}

/**
 * This structure is shared among all of the threads participating in
 * the cond_fifo test.
 */
struct FifoState {
    std::mutex mutex;
    std::condition_variable cond;
    
    /* Number of threads that have waited on cond. */
    std::atomic<int> arrivals;
    
    /* Number of threads that have reawakened after waiting on cond. */
    std::atomic<int> departures;
    
    FifoState() : mutex(), cond(), arrivals(0), departures(0) {}
};

/**
 * This method is run as the top-level method in a thread as part of the
 * cond_fifo test. It waits on a condition variable and verifies that it
 * woke up in fifo order.
 * @param state
 *      Shared info for the test.
 */
void waiter(FifoState *state)
{
    std::unique_lock lock(state->mutex);
    int my_order = state->arrivals;
    state->arrivals++;
    state->cond.wait(lock);
    if (state->departures != my_order) {
        int d = state->departures;
        std::cout << "Error: arrival " << my_order << " departed as " << d
                << std::endl;
    }
    state->departures++;
}

/**
 * This test isn't part of the official test suite; it's used just for
 * checking to see if condition variable wakeups are done in FIFO order.
 * The test will fail occasionally, indicating that condition variable
 * wakeups are not perfectly FIFO. Student projects are not expected to
 * pass this test.
 */
void cond_fifo()
{
    FifoState state;
    const int NUM_THREADS = 100;
    std::thread *threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i] = new std::thread([&state] {waiter(&state);});
    }
    while (state.arrivals != NUM_THREADS) /* Do nothing */;
    for (int i = 0; i < NUM_THREADS; i++) {
        state.cond.notify_one();
        // If the line below is commented out, there will be many more
        // out-of-order events. This seems to suggest that a waking thread
        // doesn't get added back to the lock queue until after it wakes
        // up, so if many threads wake up about the same time, they will
        // race to determine who gets the lock first.
        while (state.departures <= i) /* Do nothing */;
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i]->join();
        delete threads[i];
    }
}

/**
 * Invokes the meet method on a Party, records result information.
 * \param party
 *      Party object to invoke.
 * \param name
 *      Name of entering guest.
 * \param my_sign
 *       Zodiac sign of entering guest.
 * \param other_sign
 *       Desired Zodiac sign for match.
 * \param other_name
 *       The name of the matching guest is stored here.
 */
void guest(Party *party, std::string name, int my_sign, int other_sign,
        std::string *other_name)
{
    started++;
    *other_name = party->meet(name, my_sign, other_sign);
    matched++;
}

// Each method below tests a particular scenario.

void two_guests_perfect_match(void)
{
    // Two guests arrive and they match each other.
    
    Party party1;
    std::string match_a, match_b;

    matched = 0;
    std::cout << "guest_a arrives: my_sign 0, other_sign 5" << std::endl;
    std::thread guest_a([&party1, &match_a] {
        guest(&party1, "guest_a", 0, 5, &match_a);
    });
    guest_a.detach();
    std::cout << "guest_b arrives: my_sign 5, other_sign 0" << std::endl;
    std::thread guest_b([&party1, &match_b] {
        guest(&party1, "guest_b", 5, 0, &match_b);
    });
    guest_b.detach();
    wait_for_matches(2, 100);
    check_match("guest_a", "guest_b", match_a);
    check_match("guest_b", "guest_a", match_b);
}

void return_in_order(void)
{
    // Several guests arrive, all wanting the same match; then the desired
    // matches all arrive. Make sure that the guests match in order of
    // arrival.
    
    Party party1;
    std::string match_a, match_b, match_c, match_d, match_e, match_f;

    started = 0;
    matched = 0;
    std::cout << "guest_a arrives: my_sign 1, other_sign 3" << std::endl;
    std::thread guest_a([&party1, &match_a] {
        guest(&party1, "guest_a", 1, 3, &match_a);
    });
    guest_a.detach();
    while (started < 1) /* Do nothing */;
    std::cout << "guest_b arrives: my_sign 1, other_sign 3" << std::endl;
    std::thread guest_b([&party1, &match_b] {
        guest(&party1, "guest_b", 1, 3, &match_b);
    });
    guest_b.detach();
    while (started < 2) /* Do nothing */;
    std::cout << "guest_c arrives: my_sign 1, other_sign 3" << std::endl;
    std::thread guest_c([&party1, &match_c] {
        guest(&party1, "guest_c", 1, 3, &match_c);
    });
    guest_c.detach();
    while (started < 3) /* Do nothing */;
    wait_for_matches(1, 10);
    if (check_match("guest_a", "", match_a) + check_match("guest_b", "",
            match_b) + check_match("guest_c", "", match_c)) {
        return;
    }

    std::cout << "guest_d arrives: my_sign 3, other_sign 1" << std::endl;
    std::thread guest_d([&party1, &match_d] {
        guest(&party1, "guest_d", 3, 1, &match_d);
    });
    guest_d.detach();
    wait_for_matches(2, 100);
    if (check_match("guest_a", "guest_d", match_a)
            + check_match("guest_b", "", match_b)
            + check_match("guest_c", "", match_c)
            + check_match("guest_d", "guest_a", match_d)) {
        return;
    }

    std::cout << "guest_e arrives: my_sign 3, other_sign 1" << std::endl;
    std::thread guest_e([&party1, &match_e] {
        guest(&party1, "guest_e", 3, 1, &match_e);
    });
    guest_e.detach();
    wait_for_matches(4, 100);
    if (check_match("guest_b", "guest_e", match_b)
            + check_match("guest_c", "", match_c)
            + check_match("guest_e", "guest_b", match_e)) {
        return;
    }

    std::cout << "guest_f arrives: my_sign 3, other_sign 1" << std::endl;
    std::thread guest_f([&party1, &match_f] {
        guest(&party1, "guest_f", 3, 1, &match_f);
    });
    guest_f.detach();
    wait_for_matches(6, 100);
    if (check_match("guest_c", "guest_f", match_c)
            + check_match("guest_f", "guest_c", match_f)) {
        return;
    }
}

void sign_matching(void)
{
    // Try various combinations of my_sign and other_sign to make sure
    // that signs are matched correctly.
    
    Party party1;
    std::string match_a, match_b, match_c, match_d, match_e, match_f;

    matched = 0;
    std::cout << "guest_a arrives: my_sign 1, other_sign 3" << std::endl;
    std::thread guest_a([&party1, &match_a] {
        guest(&party1, "guest_a", 1, 3, &match_a);
    });
    guest_a.detach();
    std::cout << "guest_b arrives: my_sign 2 other_sign 1" << std::endl;
    std::thread guest_b([&party1, &match_b] {
        guest(&party1, "guest_b", 2, 1, &match_b);
    });
    guest_b.detach();
    std::cout << "guest_c arrives: my_sign 3, other_sign 2" << std::endl;
    std::thread guest_c([&party1, &match_c] {
        guest(&party1, "guest_c", 3, 2, &match_c);
    });
    guest_c.detach();
    wait_for_matches(1, 10);
    if (check_match("guest_a", "", match_a)
            + check_match("guest_b", "", match_b)
            + check_match("guest_c", "", match_c)) {
        return;
    }

    std::cout << "guest_d arrives: my_sign 3, other_sign 1" << std::endl;
    std::thread guest_d([&party1, &match_d] {
        guest(&party1, "guest_d", 3, 1, &match_d);
    });
    guest_d.detach();
    wait_for_matches(2, 100);
    if (check_match("guest_a", "guest_d", match_a)
            + check_match("guest_d", "guest_a", match_d)) {
        return;
    }

    std::cout << "guest_e arrives: my_sign 2, other_sign 3" << std::endl;
    std::thread guest_e([&party1, &match_e] {
        guest(&party1, "guest_e", 2, 3, &match_e);
    });
    guest_e.detach();
    wait_for_matches(4, 100);
    if (check_match("guest_c", "guest_e", match_c)
            + check_match("guest_e", "guest_c", match_e)) {
        return;
    }

    std::cout << "guest_f arrives: my_sign 1, other_sign 2" << std::endl;
    std::thread guest_f([&party1, &match_f] {
        guest(&party1, "guest_f", 1, 2, &match_f);
    });
    guest_f.detach();
    wait_for_matches(6, 100);
    if (check_match("guest_b", "guest_f", match_b)
            + check_match("guest_f", "guest_b", match_f)) {
        return;
    }
}

void single_sign(void)
{
    // my_sign is the same as other_sign.
    
    Party party1;
    std::string match_a, match_b;

    matched = 0;
    std::cout << "guest_a arrives: my_sign 2, other_sign 2" << std::endl;
    std::thread guest_a([&party1, &match_a] {
        guest(&party1, "guest_a", 2, 2, &match_a);
    });
    guest_a.detach();
    std::cout << "guest_b arrives: my_sign 2, other_sign 2" << std::endl;
    std::thread guest_b([&party1, &match_b] {
        guest(&party1, "guest_b", 2, 2, &match_b);
    });
    guest_b.detach();
    wait_for_matches(2, 100);
    if (check_match("guest_a", "guest_b", match_a)
            + check_match("guest_b", "guest_a", match_b)) {
        return;
    }
}

void single_sign_many(void)
{
    // Many guests arrive, and my_sign and other_sign are the same
    // for all guests.
    
    Party party1;
    std::string matches[10];
    matched = 0;
    for (int i = 0; i < 10; i++) {
        std::cout << "guest " << i << " arrives: my_sign 2, other_sign 2"
                << std::endl;
        std::thread guest_x([&party1, &matches, i] {
                guest(&party1, std::to_string(i), 2, 2, &matches[i]);
        });
        guest_x.detach();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    wait_for_matches(10, 1000);
    for (int i = 0; i < 10; i+=2) {
        check_match(std::to_string(i), std::to_string(i+1), matches[i]);
        check_match(std::to_string(i+1), std::to_string(i), matches[i+1]);
    }
}

void same_name(void)
{
    // Guests arrive with the same name, but different preferences.
    // Make sure that each person matches correctly (even though
    // the names returned are all the same).
    
    Party party1;
    std::string match1;
    std::string match2;
    std::string match3;
    std::string match4;
    started = 0;
    matched = 0;
    std::cout << "Guest (clone 1) arrives: my_sign 4, other_sign 5" << std::endl;
    std::thread guest1([&party1, &match1] {
        guest(&party1, "Guest", 4, 5, &match1);
    });
    guest1.detach();
    while (started < 1) /* Do nothing */;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << "Guest (clone 2) arrives: my_sign 4, other_sign 5" << std::endl;
    std::thread guest2([&party1, &match2] {
        guest(&party1, "Guest", 4, 5, &match2);
    });
    guest2.detach();
    while (started < 2) /* Do nothing */;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << "Guest (clone 3) arrives: my_sign 5, other_sign 4" << std::endl;
    std::thread guest3([&party1, &match3] {
        guest(&party1, "Guest", 5, 4, &match3);
    });
    guest3.detach();
    wait_for_matches(2, 100);
    check_match("Guest (clone 1)", "Guest", match1);
    check_match("Guest (clone 3)", "Guest", match3);
    std::cout << "Guest (clone 4) arrives: my_sign 5, other_sign 4" << std::endl;
    std::thread guest4([&party1, &match4] {
        guest(&party1, "Guest", 5, 4, &match4);
    });
    guest4.detach();
    wait_for_matches(4, 100);
    check_match("Guest (clone 2)", "Guest", match2);
    check_match("Guest (clone 4)", "Guest", match4);
}

void randomized(int num_people, int max_distinct_signs)
{
    // Generate a random collection of guests, such that everyone can
    // eventually match properly. Then make sure they really do match
    // properly.
    
    // Chnage the constant below to change the size of the party
    // Number of distinct signs to use (fewer means more collisions).
    std::string matches[num_people];
    std::vector<std::pair<int,int>> random_list;
    std::vector<int> random_signs;
    Party party1;
    
    for (int i = 0; i < 12; i++) {\
        random_signs.push_back(i);
    }
    std::random_shuffle(random_signs.begin(), random_signs.end());
    for (int i = 0; i < num_people/2; i++) {
        int sign1 = random_signs[rand() % max_distinct_signs];
        int sign2 = random_signs[rand() % max_distinct_signs];
        random_list.push_back(std::make_pair(sign1, sign2));
        random_list.push_back(std::make_pair(sign2, sign1));
    }
    std::random_shuffle(random_list.begin(), random_list.end());
    for (int i = 0; i < num_people; i++) {
        int my_sign = random_list[i].first;
        int other_sign = random_list[i].second;
        std::cout << "guest " << i << " arrives: my_sign " << my_sign
                << ", other_sign " << other_sign << std::endl;
        std::thread guest_n([&party1, &matches, my_sign, other_sign, i] {
                guest(&party1, std::to_string(i), my_sign, other_sign,
                        &matches[i]);
        });
        guest_n.detach();
    }
    wait_for_matches(num_people, num_people*50);
    bool error = false;
    for (int i = 0; i < num_people; i++) {
        if (matches[i].empty()) {
            std::cout << "Error: guest " << i << " didn't match" << std::endl;
            error = true;
            continue;
        }
        int other;
        try {
            other = stoi(matches[i]);
        } catch (const std::invalid_argument& ia) {
            std::cout << "Error: invalid guest name \"" 
            << matches[i] << "\"" << std::endl; 
            error = true;
            continue;
        }

        if (matches[other].empty()) {
            std::cout << "Error: guest " << i << " matched to " << other
                    << ", but " << other << " didn't match" << std::endl;
            error = true;
        } else if (matches[other] != std::to_string(i)) {
            std::cout << "Error: guest " << i << " matched to " << other
                    << ", but " << other << " matched to "
                    << matches[other].c_str() << std::endl;
            error = true;
        }
        if (random_list[i].second != random_list[other].first) {
            std::cout << "Error: guest " << i << " mismatched to " << other
                    << ": wanted sign " << random_list[i].second << ", got "
                    << random_list[other].first << std::endl;
            error = true;
        }
    }
    if (!error) {
        std::cout << "All guests matched successfully" << std::endl;
    } else {
        std::cout << "Error(s) occurred during execution" << std::endl;
    }
}

int
main(int argc, char *argv[])
{
    srand(getpid() ^ time(NULL));

    // Add any new functions to map here
    std::unordered_map<std::string, std::function<void(void)>> testFns;
    testFns["two_guests_perfect_match"] = two_guests_perfect_match;
    testFns["return_in_order"] = return_in_order;
    testFns["sign_matching"] = sign_matching;
    testFns["single_sign"] = single_sign;
    testFns["single_sign_many"] = single_sign_many;
    testFns["same_name"] = same_name;
    testFns["cond_fifo"] = cond_fifo;
    // randomized is omitted, as it takes arguments

    if (argc == 1) {
        std::cout << "Available tests are:" << std::endl;
        for (const std::pair<const std::string&, const std::function<void(void)>&>& p : testFns) {
            std::cout << "\t" << p.first << std::endl;
        }
        std::cout << "\t" << "randomized [NUM_PASSENGERS] [MAX_TRAIN_SIZE]" << std::endl;
        return 0;
    }

    auto search = testFns.find(argv[1]);
    if (search != testFns.end()) {
        search->second();
    } else if (std::string(argv[1]) == "randomized") {
        int people = argc > 2 ? atoi(argv[2]) : 100;
        if (people % 2 != 0) std::cout << "number of guests must be even to ensure all match" << std::endl;
        else {
            int max_signs = argc > 3 ? atoi(argv[3]) : 4;
            if (max_signs > 12) std::cout << "max_signs must be between 1 and 12" << std::endl;
            else randomized(people, max_signs);
        }
    } else {
        std::cout << "No test named '" << argv[1] << "'" << std::endl;
    }
}
