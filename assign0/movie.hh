#include <string>
#include <map>
#include <iostream>

// A Movie encapsulates information about a movie like its name and its cast.
class Movie {
public:
    
    // Construct a Movie by specifying the movie name
    Movie(const std::string& name);

    /* This method takes in the name of an actor and their role / part in the movie.
     * If this actor is not already in the movie, it stores this information internally
     * and returns true.  If this actor is already in the movie, it should not change
     * the internal state and instead should return false.  Assumes that the role / part
     * is not the empty string.
     */
    bool addActor(const std::string& actorName, const std::string& moviePart);

    /* This method takes in the name of an actor and removes them from the movie.
     * If the actor is not in the movie, this method does nothing.
     */
    void removeActor(const std::string& actorName);

    /* This method returns the part / role this actor has in this movie, or the empty
     * string ("") if they are not in the movie.  If they are not in the movie, no
     * internal state should be changed.
     */
    std::string getPartForActor(const std::string& actorName);

    // Returns the number of actors currently in the movie.
    int getNumActors() const;

    // Returns the name of the movie
    std::string getName() const;
    
private:
    // Add any necessary private instance variables here
    std::string name;
    std::map<std::string, std::string> actor;
};
