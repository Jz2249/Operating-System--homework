/* Program: news-aggregator
 * This program can be run with a specified file of RSS Feeds, and
 * will download all the articles in those feeds and allow the user
 * to do keyword searches through them.  The current version of the
 * program downloads each article sequentially, but we can use
 * multithreading to have each article downloaded in its own thread,
 * dramatically improving performance.
 */
#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <mutex>

#include "news-manager.hh"
#include "article.h"
#include "ostreamlock.h"

using namespace std;

// This function downloads the contents of a single article and adds it to the database map.
void fetchArticle(const Article& article, const NewsManager& nm, map<Article, vector<string>>& database, mutex& locker) {
    cout << oslock << "Fetching " << article.url << "..." << endl << osunlock;
    vector<string> articleContents = nm.fetchArticleContents(article);
    locker.lock();
    database[article] = articleContents;
    locker.unlock();
}

int main(int argc, char *argv[]) {
    // The user must specify one argument, the name of the RSSFeedList file
    if (argc != 2) {
        cerr << "ERROR: please specify an RSSFeedList" << endl;
        return 1;
    }

    // Construct our collection of article -> article words ("tokens")
    map<Article, vector<string>> database;


    // A NewsManager can get a list of articles, download a single article, and search articles
    NewsManager nm(argv[1]);

    // An article is a struct that has a title and url field
    vector<Article> articles = nm.getArticleTitlesAndURLs();

    mutex dblock;

    vector<thread> my_threads;

    for (int i = 0; i < articles.size(); i++) {
        my_threads.push_back(thread(fetchArticle, ref(articles[i]), ref(nm), ref(database), ref(dblock)));
        //fetchArticle(articles[i], nm, database);
    }

    for (thread& t: my_threads) {
        t.join();
    }

    nm.handleUserQueries(database);
    return 0;
}
