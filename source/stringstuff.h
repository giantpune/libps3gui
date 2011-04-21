#ifndef STRINGSTUFF_H
#define STRINGSTUFF_H


#include <string>
#include <vector>

bool HasEnding( std::string const &fullString, std::string const &ending );

void PrintStringList( std::vector<std::string> &list );
bool StartsWith( std::string const &fullString, std::string const &beginning );


std::vector<std::string> Split( const std::string &s, char delim, bool skipEmpty = true );
std::vector<std::string> &Split( const std::string &s, char delim, std::vector<std::string> &elems, bool skipEmpty );

std::string ToString(const float& number );

//remove "?<>\:*|”^" from a string
//! removes leading and trailing spaces, and truncates to 255 characters
//! input & output are utf8 encoded strings
std::string ToFatName( const std::string &str );

//replace characters like "áüç" with "auc"
std::string Utf8ToAnsi( const std::string &utf8 );

//check if a list of strings contains a given string
bool ListContains( const std::vector<std::string>& list, const std::string &text );

#endif // STRINGSTUFF_H
