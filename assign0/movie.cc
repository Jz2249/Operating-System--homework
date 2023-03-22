#include "movie.hh"

using namespace std;

Movie::Movie(const string& name) {
    // TODO: your code here
    this->name = name;
}

bool Movie::addActor(const string& actorName, const string& moviePart) {
    // TODO: Remove the placeholder code below and add your implementation.
    if (actor.find(actorName) == actor.end()) {
        actor[actorName] = moviePart;
        return true;
    }
    return false;
}

void Movie::removeActor(const string& actorName) {
    // TODO: your code here
    if (actor.find(actorName) != actor.end()) {
        actor.erase(actorName);
    }
}

string Movie::getPartForActor(const string& actorName) {
    // TODO: Remove the placeholder code below and add your implementation.
    if (actor.find(actorName) != actor.end()) {
        return actor[actorName];
    }
    return "";
}

int Movie::getNumActors() const {
    // TODO: remove the placeholder code below and add your implementation.
    return actor.size();
}

string Movie::getName() const {
    // TODO: Remove the placeholder code below and add your implementation.
    return name;
}


