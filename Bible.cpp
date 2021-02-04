// Bible class function definitions
// Computer Science, MVNU

#include "Ref.h"
#include "Verse.h"
#include "Bible.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

Bible::Bible() { // Default constructor
	infile = "/home/class/csc3004/Bibles/web-complete";
	isOpen = hasScanned = hasPrevious = false;
}

// Constructor – pass bible filename
Bible::Bible(const string s) {
	infile = s;
	isOpen = hasScanned = hasPrevious = false;
}

bool Bible::resetFile() {
	// File reset, so reset scanned status.
	hasScanned = false;
	hasPrevious = false;

	if(isOpen) {
		// The file is already open, clear state and seek to the beginning.
		instream.clear();
		instream.seekg(0, ios::beg);
		return true;
	}
	else {
		// The file is not yet open, try opening it.
		instream.open(infile, ios::in);
		if(instream) {
			// File opened.
			isOpen = true;
			// Include whitespace in operations.
			instream.unsetf(ios::skipws);
			return true;
		}
		else {
			// Couldn't open. Log the error.
			cerr << "Error opening Bible file " << infile << endl;
			return false;
		}
	}
}

bool Bible::scanNext() {
	// Read the next line from the file.
	string buffer;
	getline(instream, buffer);

	// Check if we got something.
	bool ok = !instream.fail() && !buffer.empty();
	if(ok) {
		// We have a result.
		// Preserve previous verse.
		previousVerse = currentVerse;
		hasPrevious = hasScanned;
		// Update current verse.
		currentVerse = Verse(buffer);
		hasScanned = true;
	}
	return ok;
}

void Bible::scanTo(Ref ref, LookupResult& status) {
	// Check if the file is not open or the current state is past where we are going.
	if((!isOpen) || (hasScanned && getCurrentVerse().getRef() > ref)) {
		// If so, reset file state.
		if(!resetFile()) {
			// Unable to open file, fail immediately.
			status = OTHER;
			return;
		}
	}

	// If nothing has been scanned, feed in the first line.
	if(!hasScanned) {
		scanNext();
	}

	// Status of search on the wider parts of the Ref.
	bool foundBook = false;
	bool foundChapter = false;

	// Scan through the remaining lines...
	for(bool ok = hasScanned; ok; ok = scanNext()) {
		// The Ref of the line currently being scanned.
		Ref currentRef = getCurrentVerse().getRef();

		// Update wide search statuses.
		if(ref.getBook() == currentRef.getBook()) {
			// Book matches our search, we found the book.
			foundBook = true;
			if(ref.getChapter() == currentRef.getChapter()) {
				// Chapter also matches our search, we found it chapter too.
				foundChapter = true;
			}
		}

		// Check if this is the line we want.
		if(ref == currentRef) {
			// If it is, succeed immediately.
			status = SUCCESS;
			return;
		}
	}

	// Stopped scanning with nothing found, declare appropriate status by going down hierarchy of possibilities.
	if(!foundBook) {
		status = NO_BOOK;
	}
	else if(!foundChapter) {
		status = NO_CHAPTER;
	}
	else {
		status = NO_VERSE;
	}
}

const Verse Bible::lookup(Ref ref, LookupResult& status) {
	// Perform the scan and update status.
	scanTo(ref, status);
	// If verse found, return it, otherwise return a dummy verse.
	return status == SUCCESS ? getCurrentVerse() : Verse();
}

// Return the reference after the given ref
const Ref Bible::next(Ref ref, LookupResult& status) {
	scanTo(ref, status);

	if(status == SUCCESS) {
		// With the ref found, try scanning to the next ref, setting error status if it fails.
		if(!scanNext()) {
			status = NO_BOOK;
		}
	}

	// Return verse ref if found, otherwise dummy.
	return status == SUCCESS ? getCurrentVerse().getRef() : Ref();
}

// Return the reference before the given ref
const Ref Bible::prev(Ref ref, LookupResult& status) {
	scanTo(ref, status);

	// Set error status if there was no previous ref and the initial scan suceeded.
	if(status == SUCCESS && !hasPrevious) {
		status = NO_BOOK;
	}

	// Return verse ref if found, otherwise dummy.
	return status == SUCCESS ? getPreviousVerse().getRef() : Ref();
}

// Return an error message string to describe status
const string Bible::error(LookupResult status) {
	switch(status) {
		case SUCCESS:
			return "success";
		case NO_BOOK:
			return "no such book";
		case NO_CHAPTER:
			return "no such chapter";
		case NO_VERSE:
			return "no such verse";
		case OTHER:
		default:
			return "other error";
	}
}

void Bible::display() {
	cout << "Bible file: " << infile << endl;
}
