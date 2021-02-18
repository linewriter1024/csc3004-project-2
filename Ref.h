// Class Ref
// Computer Science, MVNU
//
// Ref is a class for representing a Bible reference consisting of
//    * integer book   - the book from 1 (Genesis) to 66 (Revalation)
//    * integer chap   - the chapter number >1
//    * integer verse  - he verse numner >1
//

#ifndef Ref_H
#define Ref_H
#include <string>
#include <stdlib.h>
using namespace std;

// GetNextToken returns a token from str, which contains all characters
// up to the first character from delimiters
string GetNextToken(string& str, const string& delimiters);

class Ref {
public:
	// ID types, for clarity.
	typedef short book_id;
	typedef short chapter_id;
	typedef short verse_id;

	// Limits for book and chapter IDs.
	static const book_id MIN_BOOK_ID = 1;
	static const book_id MAX_BOOK_ID = 66; // Revelation
	static const chapter_id MIN_CHAPTER_ID = 1;
	static const chapter_id MAX_CHAPTER_ID = 150; // Psalm 150
	static const verse_id MIN_VERSE_ID = 1;
	static const verse_id MAX_VERSE_ID = 176; // Psalm 119:176
private:
	book_id book;
	chapter_id chapter;
	verse_id verse;
public:
	Ref();  	// Default constructor
	Ref(string s); 	// Parse constructor - example parameter "43:3:16"
	Ref(const book_id, const chapter_id, const verse_id); // Construct from three ids
	// Accessors
	book_id getBook();	// Access book number
	chapter_id getChapter();	// Access chapter number
	verse_id getVerse();	// Access verse number

	// Get human-readable name of the book.
	string getBookName();

	// Comparison operators.
	bool operator==(const Ref &) const;
	bool operator<(const Ref &) const;
	// Define != in terms of ==.
	bool operator!=(const Ref &r) const { return !(*this == r); }
	// Define > in terms of == and <.
	bool operator>(const Ref &r) const { return !(*this == r || *this < r); }

	void display(); 	// Display the reference on cout, example output: John 3:16
};

#endif //Ref_H
