/* File: movietest.cc
 * This file contains test functions for the Movie class.  Run it with the test
 * number (1-6) you'd like to run.
 */

#include <iostream>
#include <string>
#include "movie.hh"

using namespace std;

// This test creates a Movie and tries to retrieve its name
void test1() {
    string name = "MovieName";
    Movie myMovie(name);
    string storedName = myMovie.getName();
    if (storedName != name) {
        cout << "test1 failed - incorrect name '" << storedName << "'" << endl;
    } else {
        cout << "test1 passed!" << endl;
    }
}

// This test creates a Movie with one actor and fetches that actor's info
void test2() {
    Movie myMovie("MovieName");
    string name = "ActorName";
    string role = "Role1";

    bool success = myMovie.addActor(name, role);
    if (!success) {
        cout << "test2 failed - addActor returned false" << endl;
        return;
    }

    string storedRole = myMovie.getPartForActor(name);
    if (storedRole != role) {
        cout << "test2 failed - incorrect role '" << storedRole << "'" << endl;
    } else {
        cout << "test2 passed!" << endl;
    }
}

// This test tries adding the same actor twice to a movie
void test3() {
    Movie myMovie("MovieName");
    string name = "ActorName";
    string role = "Role1";

    bool success = myMovie.addActor(name, role);
    if (!success) {
        cout << "test3 failed - addActor returned false" << endl;
        return;
    }

    success = myMovie.addActor(name, role);
    if (success) {
        cout << "test3 failed - addActor returned true when adding duplicate" << endl;
        return;
    }

    cout << "test3 passed!" << endl;
}

// This test adds an actor, then removes them, then re-adds them (which should work)
void test4() {
    Movie myMovie("MovieName");
    string name = "ActorName";
    string role = "Role1";

    bool success = myMovie.addActor(name, role);
    if (!success) {
        cout << "test4 failed - addActor returned false" << endl;
        return;
    }

    myMovie.removeActor(name);

    success = myMovie.addActor(name, role);
    if (!success) {
        cout << "test4 failed - addActor returned false" << endl;
        return;
    }

    cout << "test4 passed!" << endl;
}

// This test ensures the number of actors reported is correct
void test5() {
    Movie myMovie("MovieName");

    bool success = myMovie.addActor("ActorName", "Role1");
    if (!success) {
        cout << "test5 failed - addActor returned false" << endl;
        return;
    }

    int numActors = myMovie.getNumActors();
    if (numActors != 1) {
        cout << "test5 failed - reported " << numActors << " actors." << endl;
        return;
    }

    // should return empty
    string result = myMovie.getPartForActor("Other Actor");
    if (result != "") {
        cout << "test5 failed - getPartForActor gave part for nonexistent actor" << endl;
    }

    // size should be the same
    numActors = myMovie.getNumActors();
    if (numActors != 1) {
        cout << "test5 failed - reported " << numActors << " actors." << endl;
        return;
    }

    cout << "test5 passed!" << endl;
}

// This test adds an actor, and remove it twice, then return the number of actors.
void yourTest() {
    Movie myMovie("Movie 6");
    string name = "ActorName";
    string role = "Role1";

    bool success = myMovie.addActor(name, role);
    if (!success) {
        cout << "test4 failed - addActor returned false" << endl;
        return;
    }
    myMovie.removeActor(name);
    myMovie.removeActor(name);

    int num = myMovie.getNumActors();
    if (num != 0) {
        cout << "test6 failed, actor number is " << num;
        return;
    }
    cout << "test6 passed!" << endl;
}
int main(int argc, char const *argv[]) {
    if (argc == 1) {
        cout << "ERROR: please specify the test number (1-6) to run." << endl;
        return 1;
    }

    int testNum = atoi(argv[1]);
    if (testNum == 1) {
        test1();
    } else if (testNum == 2) {
        test2();
    } else if (testNum == 3) {
        test3();
    } else if (testNum == 4) {
        test4();
    } else if (testNum == 5) {
        test5();
    } else if (testNum == 6) {
        yourTest();
    }

    return 0;
}
