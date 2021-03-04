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
#include <map>
#include <list>
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
   bool isValid;

   // The Ref -> position in file index.
   std::map<Ref, std::streampos> index;

   // Construct the index from an open input stream.
   void buildIndex();

   // Get the lookup status of a particular Ref in the index.
   LookupResult getRefLookupStatus(Ref ref);

 public:
   Bible();	// Default constructor
   Bible(const string s); // Constructor – pass name of bible file

   // Check if the Bible is valid after construction. Lookups can only be done if this is true.
   bool valid();

   // Look up a verse by ref in the Bible.
   // Sets status according to the result of the search, returns a dummy verse if the lookup was unsuccessful.
   const Verse lookup(Ref ref, LookupResult& status);
   // Return the reference after the given ref
   const Ref next(Ref ref, LookupResult& status);
   // Return the reference before the given ref
   const Ref prev(Ref ref, LookupResult& status);

   // Information functions
   // Return an error message string to describe status
   static const string error(LookupResult status);

   // Show the name of the bible file on cout
   void display();

   // Get the default Bible version.
   static std::string getDefaultVersion();

   // Does a version identifier (kjv, web, etc.) exist?
   static bool versionExists(std::string version);

   // Convert an existing version identifier to a file. Will return an empty string if the version identifier is not found.
   static std::string versionToFile(std::string version);

   // Get a list of all available Bible versions.
   static std::list<std::string> versionList();
};
#endif //Bible_H
