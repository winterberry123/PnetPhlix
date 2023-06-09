#include "Recommender.h"
#include "UserDatabase.h"
#include "MovieDatabase.h"
#include "User.h"
#include "Movie.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
using namespace std;

bool alphabeticallyEarlier(const Movie* m1, const Movie* m2);

bool hasHigherRating(const Movie* m1, const Movie* m2);

bool hasHigherCompatibility(const MovieAndRank& m1, const MovieAndRank& m2);

Recommender::Recommender(const UserDatabase& user_database,
                         const MovieDatabase& movie_database)
{
    m_user_database = &user_database;
    m_movie_database = &movie_database;
 }

vector<MovieAndRank> Recommender::recommend_movies(const string& user_email, int movie_count) const
{
    vector<MovieAndRank> final_movie_ranks;
    unordered_map<Movie*, int> rating_map;
    
    User* user = m_user_database->get_user_from_email(user_email);
    vector<string> movieIDs = user->get_watch_history();
    
    vector<string>::iterator it;
    for(it = movieIDs.begin(); it != movieIDs.end(); it++){
        Movie* currMovie = m_movie_database->get_movie_from_id(*it);
        //movies with same director +20
        vector<string> directors = currMovie->get_directors();
        //traversing through everyone who directed the current movie
        for(vector<string>::iterator j = directors.begin(); j != directors.end(); j++){
            //list of movies in the database with same director
            vector<Movie*> movies = m_movie_database->get_movies_with_director(*j);
            for(vector<Movie*>::iterator k = movies.begin(); k != movies.end(); k++){
                rating_map[*k] += 20;
            }
        }
        //movies with same actor +30
        vector<string> actors = currMovie->get_actors();
        for(vector<string>::iterator j = actors.begin(); j != actors.end(); j++){
            //list of movies in the database with same actor
            vector<Movie*> movies = m_movie_database->get_movies_with_actor(*j);
            //iterate through these movies
            for(vector<Movie*>::iterator k = movies.begin(); k != movies.end(); k++){
                rating_map[*k] += 30;
            }
        }
        //movies with same genre +1
        vector<string> genres = currMovie->get_genres();
        for(vector<string>::iterator j = genres.begin(); j != genres.end(); j++){
            //list of movies in the database with same genre
            vector<Movie*> movies = m_movie_database->get_movies_with_genre(*j);
            //iterate through these movies
            for(vector<Movie*>::iterator k = movies.begin(); k != movies.end(); k++){
                if(*k != currMovie){
                    rating_map[*k]++;
                }
            }
        }
    }
    
    //filter out movies already watched
    for(vector<string>::iterator it = movieIDs.begin(); it != movieIDs.end(); it++){
        Movie* u = m_movie_database->get_movie_from_id(*it);
        if(rating_map.find(u) != rating_map.end()) rating_map.erase(u);
    }
    
    //create vector of movies pointers (for all movies with scores >=1)
    vector<Movie*> all_movies;
    for (auto i : rating_map){
        all_movies.push_back(i.first);
    }
    
    //sort alphabetically first, then by rating
    sort(all_movies.begin(), all_movies.end(), alphabeticallyEarlier);
    stable_sort(all_movies.begin(), all_movies.end(), hasHigherRating);
    
    vector<MovieAndRank> movie_ranks;
    for(vector<Movie*>::iterator it = all_movies.begin(); it != all_movies.end(); it++){
        string id = (*it)->get_id();
        int score = rating_map[*it];
        MovieAndRank temp(id, score);
        movie_ranks.push_back(temp);
    }
    
    //stable sort the compatibility last (previous sorts will remain in order)
    stable_sort(movie_ranks.begin(), movie_ranks.end(), hasHigherCompatibility);
    
    //check if # of requested movie reccs <= movies w/ score of at least 1
    long count = movie_count >= movie_ranks.size() ? movie_ranks.size() : movie_count;
    for(vector<MovieAndRank>::iterator it = movie_ranks.begin(); it != movie_ranks.begin() + count; it++){
        final_movie_ranks.push_back(*it);
    }
    return final_movie_ranks;
}

//checks which Movie's title is alphabetically earlier
bool alphabeticallyEarlier(const Movie* m1, const Movie* m2){
    return m1->get_title() < m2->get_title();
}

//checks which Movie has a higher rating
bool hasHigherRating(const Movie* m1, const Movie* m2){
    return m1->get_rating() > m2->get_rating();
}

//checks which Movie has a higher compatibility score
bool hasHigherCompatibility(const MovieAndRank& m1, const MovieAndRank& m2){
    return m1.compatibility_score > m2.compatibility_score;
}
