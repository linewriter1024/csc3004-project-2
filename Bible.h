// Class Bible
// Computer Science, MVNU
//
// A Bible object represents a particular version of the Bible
// A Bible object is constructed by giving it a file reference containing
// the entire text of the version.

#ifndef Bible_H
#define Bible_H

#include "Ref.h"
#include "Verse.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

// status codes to be returned when looking up a reference
enum LookupResult { SUCCESS, NO_BOOK, NO_CHAPTER, NO_VERSE, OTHER };

class Bible {	// A class to represent a version of the bible
 private:
   string infile;		// file path name
   ifstream instream;	// input stream, used when file is open
   bool isOpen;			// true if file is open

   // Scan status variables.
   bool hasScanned;
   bool hasPrevious;

   // Scan result variables.
   Verse currentVerse;
   Verse previousVerse;

   // Reset or open the input file for fresh scanning, returns false if unable to open the file.
   bool resetFile();

   // Scan the input file to the supplied Ref. Sets status according to the result of the search.
   void scanTo(Ref ref, LookupResult &status);

   // Scan to the next verse. Returns true if there was a next verse, otherwise false.
   bool scanNext();

   // Get the currently scanned verse.
   // Can only be called after a successful scanTo() or scanNext(), when hasScanned == true.
   Verse getCurrentVerse() { return currentVerse; };

   // Get the currently scanned verse.
   // Can only be called after a successful scanTo() or scanNext() calls, when hasPrevious == true.
   Verse getPreviousVerse() { return previousVerse; };

 public:
   Bible();	// Default constructor
   Bible(const string s); // Constructor – pass name of bible file

   // Look up a verse by ref in the Bible.
   // Sets status according to the result of the search, returns a dummy verse if the lookup was unsuccessful.
   const Verse lookup(Ref ref, LookupResult& status);
   // Return the reference after the given ref
   const Ref next(Ref ref, LookupResult& status);
   // Return the reference before the given ref
   const Ref prev(Ref ref, LookupResult& status);

   // Information functions
   // Return an error message string to describe status
   const string error(LookupResult status);

   // Show the name of the bible file on cout
   void display();
};
#endif //Bible_H
