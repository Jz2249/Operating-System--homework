/* File: moviecredits.cc
 * ----------------------
 * This file constructs and prints out a map from movie names to their cast.
 * You run it by specifying the dataset size ("small" or "large") to use for
 * testing, and the program will print out the movies in the dataset along
 * with their cast, as well as the total number of movies in the dataset.
 */

#include <iostream>
#include <vector>
#include <map>
#include "moviecredits-util.cc"

using namespace std;

const string kSmallDBName = "small";
const string kLargeDBName = "large";


/* This function takes in a vector of Movie structs and an empty movieMap,
 * and fills in the movieMap with entries where the key is the name of the movie,
 * and the value is a vector of its cast members.  The map is populated in order
 * of the movies in the provided vector - if we later encounter a movie with the
 * same name as one we already saw, we add it to the map with its year 
 * as part of its name (e.g. "Movie (Year)") to disambiguate.
 */
void buildMovieMap(const vector<Movie>& movies, map<string, vector<string>>& movieMap) {
    // TODO: your code here
    int sz = movies.size();
    for (int i = 0; i < sz; i++) {
        string title = movies[i].title;
        if (movieMap.find(title) != movieMap.end()) { // find duplicate name
            title = title + " (" + to_string(movies[i].year) + ")";
        }
        movieMap[title] = movies[i].cast;
    }
    for (auto& it : movieMap) {
        cout << "'" << it.first << "':";
        int actor_sz = it.second.size();
        for (int i = 0; i < actor_sz; i++) {
            cout << " "<< it.second[i];
        }
        cout << endl;
    }
}

/* This function loads a made-up test dataset of movies and cast members, including
 * some movies with the same name but different release years.
 */
void loadSmallDB(vector<Movie>& movies) {
    movies.push_back({"Movie1", 2000, {"Actor1", "Actor2", "Actor3", "Actor4"}});
    movies.push_back({"Movie2", 1977, {"Actor5", "Actor6", "Actor7", "Actor8"}});
    movies.push_back({"Movie3", 2016, {"Actor3", "Actor4"}});
    movies.push_back({"Movie4", 2001, {"Actor1", "Actor5", "Actor8"}});
    movies.push_back({"Movie2", 2002, {"Actor9", "Actor1", "Actor3", "Actor6", "Actor10"}});
    movies.push_back({"Movie3", 2022, {"Actor10", "Actor2"}});
    movies.push_back({"Movie5", 1965, {"Actor6", "Actor2", "Actor10", "Actor4"}});
}

int main(int argc, char *argv[]) {
    /* If the user didn't specify an argument, or they specified something
     * other than "small" or "large", print an error and exit.
     */
    if (argc == 1 || (string(argv[1]) != kSmallDBName && string(argv[1]) != kLargeDBName)) {
        cout << "ERROR: must specify either '" << kSmallDBName 
             << "' or '" << kLargeDBName << "' for test dataset size" << endl;
        return 1;
    }

    vector<Movie> movies;
    if (string(argv[1]) == kSmallDBName) {
        loadSmallDB(movies);
    } else if (string(argv[1]) == kLargeDBName) {
        loadLargeDB(movies);
    }

    map<string, vector<string>> movieMap;
    buildMovieMap(movies, movieMap);
    cout << "Constructed map with " << movieMap.size() << " movie entries" << endl;
    return 0;
}
