// Verse class function definitions
// Computer Science, MVNU

#include "Verse.h"
#include "Ref.h"
#include <iostream>
using namespace std;

Verse::Verse() { // Default constructor
    verseText = "Uninitialized Verse!";
    verseRef = Ref();
}

Verse::Verse(const string s) {
	// Copy the verse and split it into Ref and verse text portions.
	string buffer = s;
	// Split on the first whitespace.
	string refToken = GetNextToken(buffer, " ");
	// Initialize Ref from first token.
	verseRef = Ref(refToken);
	// Initialize text from remaining buffer.
	verseText = buffer;
}

string Verse::getVerse() {
	return verseText;
}

Ref Verse::getRef() {
	return verseRef;
}

// display reference and verse
void Verse::display() {
    verseRef.display();
    cout << " " << verseText;
 }
