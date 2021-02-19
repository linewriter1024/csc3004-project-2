// Bible class function definitions
// Computer Science, MVNU

#include "Ref.h"
#include "Verse.h"
#include "Bible.h"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

// Map of Bible version short names to files.
static std::map<std::string, std::string> bibleVersions = {
	{"kjv", "/home/class/csc3004/Bibles/kjv-complete"},
	{"web", "/home/class/csc3004/Bibles/web-complete"},
	{"dby", "/home/class/csc3004/Bibles/dby-complete"},
	{"webster", "/home/class/csc3004/Bibles/webster-complete"},
	{"ylt", "/home/class/csc3004/Bibles/ylt-complete"},
};

bool Bible::versionExists(std::string version) {
	return bibleVersions.count(version) > 0;
}

std::string Bible::versionToFile(std::string version) {
	return versionExists(version) ? bibleVersions.at(version) : "";
}

// Default constructor, just use the WEB version.
Bible::Bible() : Bible("/home/class/csc3004/Bibles/web-complete") {}

// Constructor – pass bible filename
Bible::Bible(const string s) : infile(s), isValid(false) {
	// Open the file and build the index if possible.
	instream.open(infile, ios::in);
	if(instream) {
		isValid = true;
		buildIndex();
	}
}

bool Bible::valid() {
	return isValid;
}

void Bible::buildIndex() {
	std::string buffer;
	std::streampos position;
	std::streampos last_valid_position = 0;
	do {
		// Record position and get the next line.
		position = instream.tellg();
		getline(instream, buffer);

		// If there's something here, parse the Ref and add it to the index.
		if(!buffer.empty()) {
			Ref ref(buffer);
			index[ref] = position;
			last_valid_position = position;
		}
	} while(!instream.fail());

	std::cerr << "Added " << index.size() << " references to the index." << endl;
	std::cerr << "The last verse is at offset " << last_valid_position << endl;
}

LookupResult Bible::getRefLookupStatus(Ref ref) {
	if(index.count(ref)) {
		// Ref exists.
		return SUCCESS;
	}
	else {
		/*
		 * Ref doesn't exist, figure out why not.
		 *
		 * First check if the book doesn't exist,
		 * then check if the chapter in the book doesn't exist,
		 * and if they both exist then it's the verse that doesn't exist.
		 */
		Ref bookTest(ref.getBook(), Ref::MIN_CHAPTER_ID, Ref::MIN_VERSE_ID);
		if(index.count(bookTest)) {
			Ref chapterTest(ref.getBook(), ref.getChapter(), Ref::MIN_VERSE_ID);
			if(index.count(chapterTest)) {
				return NO_VERSE;
			}
			else {
				return NO_CHAPTER;
			}
		}
		else {
			return NO_BOOK;
		}
	}
}

const Verse Bible::lookup(Ref ref, LookupResult& status) {
	// Check that the ref exists in the index.
	status = getRefLookupStatus(ref);
	if(status == SUCCESS) {
		// Reset and seek to the Ref's position in the file according to the index.
		instream.clear();
		instream.seekg(index[ref]);

		// Get the verse line.
		std::string buffer;
		getline(instream, buffer);

		// If we couldn't get anything, set failure status.
		if(buffer.empty()) {
			status = OTHER;
		}

		// Return the verse.
		return Verse(buffer);
	}
}

// Return the reference after the given ref
const Ref Bible::next(Ref ref, LookupResult& status) {
	// Ensure the initial Ref exists.
	status = getRefLookupStatus(ref);
	if(status != SUCCESS)
		return Ref();

	// Get an iterator to the Ref, increment it, and if that works, return the next key Ref.
	std::map<Ref, std::streampos>::iterator it = index.find(ref);
	if(++it != index.end()) {
		status = SUCCESS;
		return it->first;
	}
	else {
		// No next iterator, no next book.
		status = NO_BOOK;
		return Ref();
	}
}

// Return the reference before the given ref
const Ref Bible::prev(Ref ref, LookupResult& status) {
	// Ensure the initial Ref exists.
	status = getRefLookupStatus(ref);
	if(status != SUCCESS)
		return Ref();

	// Get an iterator to the Ref, decrement it, and if that works, return the prev key Ref.
	std::map<Ref, std::streampos>::iterator it = index.find(ref);
	if(--it != index.end()) {
		status = SUCCESS;
		return it->first;
	}
	else {
		// No previous iterator, no previous book.
		status = NO_BOOK;
		return Ref();
	}
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
