#ifndef PARTY_H
#define PARTY_H

/** Class: Party
 * This class models a Party where guests arrive to meet other guests.
 * Guests can arrive and indicate the Zodiac sign of a guest they'd like to
 * meet, and this class matches guests according to these Zodiac signs.
 */

#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>

// The total number of Zodiac signs
static const int NUM_SIGNS = 12;

class Party {
public:

    /* Indicates that a guest has arrived at the party, specifying their
     * name (assumed to be a nonempty string), their
     * Zodiac sign and the Zodiac sign of another guest that this guest
     * would like to talk to.  This method does not  return until a guest
     * has arrived with matching my_sign and other_sign, and this method then
     * returns the name of the matching guest.
     */
    std::string meet(std::string &my_name, int my_sign, int other_sign);

private:
    // Synchronizes access to all information in this object.
    std::mutex mutex_;

    // Represents one guest for whom we are looking for a match.
    typedef struct WaitingGuest {
        /* TODO: add fields here to store for each guest
         * Each waiting guest will have its own copy of these fields
         * Initialize them here as well if you need to set a default value.
         */
         std:: condition_variable_any find_match;
         bool is_matched = false;
         std:: string name;
         std:: string guest_name;
         
    } WaitingGuest;
    
    /* Provided internal helper function that checks the list
     * of waiting guests to see if there is anyone with
     * other_sign looking for someone with my_sign.  Returns true
     * if yes, false if no.  Assumes caller has mutex_.
     */
    bool has_matching_waiting_guest(int my_sign, int other_sign);

    /* Provided internal helper function that returns a WaitingGuest
     * struct reference containing information about a matching
     * guest with other_sign looking for someone with my_sign.
     * It will respect the ordering in which guests were added
     * via add_candidate below; in other words, if there are
     * multiple potential pairings, this function returns the
     * guest who was added first.  This also removes this guest
     * from the list of waiting guests. Assumes caller has mutex_.
     */
    WaitingGuest& get_matching_waiting_guest(int my_sign, int other_sign);

    /* Provided internal helper function that adds a WaitingGuest
     * to the list of waiting guests looking for a match.
     * my_sign is the sign of this guest, and other_sign is the
     * sign of the person they are interested in talking to.
     * Assumes caller has mutex_.
     */
    void add_waiting_guest(int my_sign, int other_sign, WaitingGuest& guest);

    /* You can ignore this - this is the data structure
     * internally used by the helper functions above - 
     * it is a 2D array of queues.  first index is
     * sign of person waiting; second index is sign of desired match.
     */
    std::queue<WaitingGuest *> waiting_guests_[NUM_SIGNS][NUM_SIGNS];
};

#endif /* PARTY_H */
