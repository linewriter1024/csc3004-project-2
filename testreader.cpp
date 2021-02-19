// biblereader.cpp
// main function for Project 1

#include "Ref.h"
#include "Verse.h"
#include "Bible.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

int main (int argc, char **argv) {
	// Create Bible object to process the raw text file
	Bible bible;

	if(!bible.valid()) {
		std::cerr << "Error: invalid bible file" << endl;
		return EXIT_FAILURE;
	}

	// Book, chapter, verse.
	int b, c, v;
	// Number of verses to fetch.
	int length = 1;

	// Check for too few arguments and output an approriate error message upon failure.
	switch(argc) {
		case 0:
		case 1:
			cerr << "Error: book number is missing" << endl;
			return EXIT_FAILURE;
		case 2:
			cerr << "Error: chapter number is missing" << endl;
			return EXIT_FAILURE;
		case 3:
			cerr << "Error: verse number is missing" << endl;
			return EXIT_FAILURE;
		default:
			break;
	}

	// Get the ref arguments as integers.
	b = atoi(argv[1]);
	c = atoi(argv[2]);
	v = atoi(argv[3]);

	// Get the length argument if possible.
	if(argc >= 5) {
		length = atoi(argv[4]);
	}

	// Create a reference from the numbers
	Ref ref(b, c, v);

	// Look up the initial reference.
	LookupResult result;
	Verse verse = bible.lookup(Ref(b, c, v), result);

	if(result == SUCCESS) {
		// Initial fetch succeeded, begin displaying verses.

		// Current chapter being displayed, default to -1 to indicate display has not started.
		int currentChapter = -1;
		for(int i = 0; i < length && result == SUCCESS; i++) {
			if(verse.getRef().getChapter() != currentChapter) {
				currentChapter = verse.getRef().getChapter();
				cout << verse.getRef().getBookName() << " " << verse.getRef().getChapter() << endl;
			}

			cout << " " << verse.getRef().getVerse() << ". " << verse.getVerse() << endl;

			Ref nextRef = bible.next(verse.getRef(), result);
			if(nextRef.getBook() != ref.getBook()) {
				break;
			}
			if(result == SUCCESS) {
				verse = bible.lookup(nextRef, result);
			}
		}
	}
	else {
		// Initial fetch failed, tell the user what happened.
		cerr << "Error: " << bible.error(result);
		switch(result) {
			case NO_CHAPTER:
				cerr << " in " << ref.getBookName();
				break;
			case NO_VERSE:
				cerr << " in " << ref.getBookName() << " " << c;
				break;
		}
		cerr << endl;
	}
}
