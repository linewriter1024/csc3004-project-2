// Ref class function definitions
// Computer Science, MVNU

#include "Ref.h"
#include <iostream>
#include <string>
using namespace std;

// GetNextToken - general routine for getting next token from a string
// Parameters
//    str - string to search. search is destructive, i.e., token is removed.
//    delimiters - string, a list of characters to use a delimiters between tokens
// Returns: string token (removed from str)

string GetNextToken(string& str, const string& delimiters = " ") {
  // Skip delimiters at beginning
  string::size_type startPos = str.find_first_not_of(delimiters, 0);
  // Find position of delimiter at end of token
  string::size_type endPos = str.find_first_of(delimiters, startPos);

  // Found a token, remove it from string, and return it
  string next = str.substr(startPos, endPos - startPos);
  string rest = str.substr(endPos - startPos + 1, string::npos);
  str = rest;
  return(next);
}

// Ref member functions

Ref::Ref() {book = 0; chapter = 0; verse = 0;}  	// Default constructor

Ref::Ref(const string s) { // Parse constructor - receives a line "34:5:7 text"
    string rtext = s; // make local copy of string to avoid modifying parameter
    // parse the reference - notice, currently there is no error checking!
    // Get book number
    string strbook = GetNextToken(rtext,":");
    book = atoi(strbook.c_str());
    // Get the chapterter number
    string strchapter = GetNextToken(rtext,":");
    chapter = atoi(strchapter.c_str());
    // Get the verse number
    string strverse = GetNextToken(rtext," ");
    verse = atoi(strverse.c_str());
}

Ref::Ref(const Ref::book_id b, const Ref::chapter_id c, const Ref::verse_id v) { 	// Construct Ref from three ids
	book = b;
	chapter = c;
	verse = v;
}

// Accessors
Ref::book_id Ref::getBook() {return book;}	 // Access book number
Ref::chapter_id Ref::getChapter() {return chapter;}	 // Access chapterter number
Ref::verse_id Ref::getVerse() {return verse;}; // Access verse number

// Ref comparison operators.
bool Ref::operator==(const Ref &r) const {
	return book == r.book && chapter == r.chapter && verse == r.verse;
}

bool Ref::operator<(const Ref &r) const {
	return book < r.book || (book == r.book && chapter < r.chapter) || (book == r.book && chapter == r.chapter && verse < r.verse);
}

void Ref::display() { 	// Display Reference
     cout << getBookName() << " " << chapter << ":" << verse;
}

string Ref::getBookName() {
	static string book_names[66] = {
		"Genesis",
		"Exodus",
		"Leviticus",
		"Numbers",
		"Deuteronomy",
		"Joshua",
		"Judges",
		"Ruth",
		"1 Samuel",
		"2 Samuel",
		"1 Kings",
		"2 Kings",
		"1 Chronicles",
		"2 Chronicles",
		"Ezra",
		"Nehemiah",
		"Esther",
		"Job",
		"Psalms",
		"Proverbs",
		"Ecclesiastes",
		"Song of Solomon",
		"Isaiah",
		"Jeremiah",
		"Lamentations",
		"Ezekiel",
		"Daniel",
		"Hosea",
		"Joel",
		"Amos",
		"Obadiah",
		"Jonah",
		"Micah",
		"Nahum",
		"Habakkuk",
		"Zephaniah",
		"Haggai",
		"Zechariah",
		"Malachi",
		"Matthew",
		"Mark",
		"Luke",
		"John",
		"Acts",
		"Romans",
		"1 Corinthians",
		"2 Corinthians",
		"Galatians",
		"Ephesians",
		"Philippians",
		"Colossians",
		"1 Thessalonians",
		"2 Thessalonians",
		"1 Timothy",
		"2 Timothy",
		"Titus",
		"Philemon",
		"Hebrews",
		"James",
		"1 Peter",
		"2 Peter",
		"1 John",
		"2 John",
		"3 John",
		"Jude",
		"Revelation"
	};

	// Return book name as string if the book number is in range, otherwise an empty string.
	return (book >= MIN_BOOK_ID && book <= MAX_BOOK_ID) ? book_names[book - 1] : "";
}
