#include "party.hh"

using namespace std;

string Party::meet(string &my_name, int my_sign, int other_sign)
{
    // You need to implement this
    unique_lock<mutex> lock(mutex_);
    string name;
    if (has_matching_waiting_guest(my_sign, other_sign))
    {   // Found the matching guest in waitlist
        WaitingGuest &match = get_matching_waiting_guest(my_sign, other_sign);
        name = match.name;
        match.guest_name = my_name;
        match.is_matched = true;
        match.find_match.notify_all();
    } 
    else 
    {   // Stay in the waiting list and wait for the match
        WaitingGuest guest;
        guest.name = my_name;
        add_waiting_guest(my_sign, other_sign, guest);
        guest.find_match.wait(lock, [&guest]() -> bool {
            return guest.is_matched == true;
        });
        name = guest.guest_name; // get the matching guest name
    }
    return name;
}

//  You can ignore the method implementations below

bool Party::has_matching_waiting_guest(int my_sign, int other_sign) {
    return !waiting_guests_[my_sign][other_sign].empty();
}

Party::WaitingGuest& Party::get_matching_waiting_guest(int my_sign, int other_sign) {
    WaitingGuest *match = waiting_guests_[my_sign][other_sign].front();
    waiting_guests_[my_sign][other_sign].pop();
    return *match;
}

void Party::add_waiting_guest(int my_sign, int other_sign, WaitingGuest& guest) {
    waiting_guests_[other_sign][my_sign].push(&guest);
}