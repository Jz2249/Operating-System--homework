/* CLASS: NewsManager
 * This class encapsulates various functionality regarding fetching
 * and searching news articles.  It is initialized with the path to the
 * file containing RSS Feeds, and it can get titles/URLs of all the articles 
 * in those feeds, fetch the contents of a single article, and display a
 * prompt to the user to search the contents of articles.
 */

#include <map>
#include <string>
#include <vector>
#include "article.h"

class NewsManager {
public:
	/* Initialize a NewsManager with the path to the RSS Feed list file.
	 * Also initializes XML library for article parsing.
	 */
	NewsManager(const std::string& feedListFilename);

	// Destructor that uninitializes the XML library for article parsing.
	~NewsManager();

	/* Returns a vector of Articles referred to by the feed list.  An
	 * Article is a struct combination of title and URL.  This function
	 * fetches contents from the web (the contents of the RSS Feeds), and
	 * blocks while waiting for the download.
	 */
	std::vector<Article> getArticleTitlesAndURLs() const;

	/* Returns a vector of tokens (words) in order for this article.  This
	 * function fetches the contents from the web, and blocks while waiting
	 * for the download.  Thread-safe.
	 */
	std::vector<std::string> fetchArticleContents(const Article& article) const;

	/* Continually prompts the user for search queries and goes through
	 * the database (assumed to be a map from article to article tokens)
	 * to find any matches for their query, printing them out each time.
	 */
	void handleUserQueries(std::map<Article, std::vector<std::string>> database) const;
private:
	std::string feedListFilename;
};