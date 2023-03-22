#ifndef CALTRAIN_H
#define CALTRAIN_H

/** Class: Station
 * This class models a Caltrain station that coordinates trains and loading
 * passengers.  Trains can arrive at the station to be loaded, and passengers
 * wait for trains before boarding, and indicate once they are boarded.
 */

#include <condition_variable>
#include <mutex>

class Station {
public:
    Station();

    /* Called when a train arrives in the station and has opened its doors. 
     * available indicates how many seats are available on the train. 
     * This method does not return until the train is satisfactorily loaded 
     * (all passengers are in their seats, and either the train is full or 
     * all waiting passengers have boarded).
     */
    void load_train(int count);

    /* Invoked when a passenger arrives in the station. This method does
     * not return until a train is in the station (i.e., a call to load_train
     * is in progress) and there are enough free seats on the train for this 
     * passenger to sit down. Once this method returns, the passenger
     * can begin boarding. 
     */
    void wait_for_train();

    /* Invoked once a passenger is boarded (in a seat). 
     * Any train mustn't leave the station until all of the admitted 
     * passengers are safely in their seats.
     */
    void boarded();
    
private:
    // Synchronizes access to all information in this object.
    std::mutex mutex_;

    // TODO: declare any additional instance variables here
    // (and initialize them in the .cc constructor if needed)
    size_t seat_available = 0;
    size_t num_waiting = 0;
    size_t wait_boarded = 0;
    std:: condition_variable_any done_waiting;
    std:: condition_variable_any done_boarded;
};

#endif /* CALTRAIN_H */