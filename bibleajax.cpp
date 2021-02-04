/* Demo server program for Bible lookup using AJAX/CGI interface
* By James Skon, Febrary 10, 2011
* updated by Bob Kasper, January 2020
* Mount Vernon Nazarene University
* Benjamin Leskey (Feburary 2021).
*
* This sample program works using the cgicc AJAX library to
* allow live communication between a web page and a program running on the
* same server that hosts the web server.
*
* This program is run by a request from the associated html web document.
* A Javascript client function invokes an AJAX request,
* passing the input form data as the standard input string.
*
* The cgi.getElement function parses the input string, searching for the matching
* field name, and returing a "form_iterator" oject, which contains the actual
* string the user entered into the corresponding field. The actual values can be
* accessed by dereferencing the form iterator twice, e.g. **verse
*     refers to the actual string entered in the form's "verse" field.
*/

#include <iostream>
#include <string>
#include <limits>
#include <stdio.h>
#include <string.h>
using namespace std;

/* Required libraries for AJAX to function */
#include "/home/class/csc3004/cgicc/Cgicc.h"
#include "/home/class/csc3004/cgicc/HTTPHTMLHeader.h"
#include "/home/class/csc3004/cgicc/HTMLClasses.h"
using namespace cgicc;

#include "Bible.h"

// An incoming request.
// Wraps Cgicc.
class BibleCGIRequest {
public:
	// Construct the request object from the CGI request data.
	// Will set failed state if anything went wrong.
	BibleCGIRequest() : cgi() {
		// Clear failure.
		failed = false;
		errorMessage = "";

		// Get the CGI input data.
		// TODO: form_iterator st = cgi.getElement("search_type");
		form_iterator book = cgi.getElement("book");
		form_iterator chapter = cgi.getElement("chapter");
		form_iterator verse = cgi.getElement("verse");
		form_iterator nv = cgi.getElement("num_verse");

		// Construct the Ref from the input.
		ref = Ref(
			inputInteger<Ref::book_id>(book, "book", Ref::MIN_BOOK_ID, Ref::MAX_BOOK_ID),
			inputInteger<Ref::chapter_id>(chapter, "chapter", Ref::MIN_CHAPTER_ID, Ref::MAX_CHAPTER_ID),
			inputInteger<Ref::verse_id>(verse, "verse", Ref::MIN_VERSE_ID, Ref::MAX_VERSE_ID)
		);

		// Get the desired verse count.
		numberOfVerses = inputInteger<int>(nv, "verse count", 1, std::numeric_limits<int>::max());
	}

	// Check if the request processing failed.
	bool getFailed() { return failed; }
	// Get the failure error message.
	std::string getErrorMessage() { return errorMessage; }

	// Get the request reference. Only works after success.
	Ref getRef() { return ref; }
	// Get the desired number of verses. Only works after success.
	int getNumberOfVerses() { return numberOfVerses; }
private:
	// Create the Cgicc object within the Request.
	Cgicc cgi;

	Ref ref;
	int numberOfVerses;

	bool failed;
	std::string errorMessage;

	// Set the failure state on, with the specified error message.
	void fail(std::string message) {
		failed = true;
		errorMessage = message;
	}

	// Get an input integer of type T from an element with human-readable identifier name, pinned between min and max.
	// Will not operate in a failed state, and will update the failed state if the element does not fit the qualifications.
	template<typename T>
	T inputInteger(const form_iterator &element, std::string name, const T min, const T max) {
		// If this request already failed, do nothing.
		if(failed) {
			// Return default;
			return min;
		}

		// Check if the element doesn't exist or is empty.
		if(element == cgi.getElements().end() || element->getValue().empty()) {
			fail("the " + name + " was not specified");

			// Return default;
			return min;
		}
		else {
			// Get the integer and cast to the desired type.
			T result = static_cast<T>(element->getIntegerValue());

			// Range check.
			if(result < min) {
				fail("the specified " + name + " is below " + std::to_string(min));
			}
			else if(result > max) {
				fail("the specified " + name + " is above " + std::to_string(max));
			}

			// Return the result regardless of failure.
			return result;
		}
	}
};

int main() {
	// Send the required CGI content type header.
	// Plain text, we are only rendering part of a page.t
	cout << "Content-Type: text/plain\n\n";

	// Construct the request wrapper (it will create the Cgicc instance).
	BibleCGIRequest request;

	if(request.getFailed()) {
		// Output initial input error message upon failure.
		cout << "<p>Input error: <em>" << request.getErrorMessage() << "</em></p>";
	}
	else {
		// Valid request, Open the Bible.
		Bible bible("/home/class/csc3004/Bibles/web-complete");

		// Look up the first verse.
		LookupResult result;
		Verse verse = bible.lookup(request.getRef(), result);

		if(result == SUCCESS) {
			// Successful lookup, continue for all verses.

			// Current chapter being displayed, default to -1 to indicate display has not started.
			int currentChapter = -1;
			// Loop through possible verses until the end is reached (end of desired verses, end of initial book, or end of Bible).
			for(int i = 0; i < request.getNumberOfVerses() && verse.getRef().getBook() == request.getRef().getBook() && result == SUCCESS; i++) {
				// New chapter, print header.
				if(verse.getRef().getChapter() != currentChapter) {
					currentChapter = verse.getRef().getChapter();
					cout << "<h2>" << verse.getRef().getBookName() << " " << verse.getRef().getChapter() << "</h2>" << endl;
				}

				// Output verse.
				cout << "<p><em>" << verse.getRef().getVerse() << ".</em> " << verse.getVerse() << "</p>" << endl;

				// Find the next ref.
				Ref nextRef = bible.next(verse.getRef(), result);

				// If there is another verse, look it up for the next iteration.
				if(result == SUCCESS) {
					verse = bible.lookup(nextRef, result);
				}
			}
		}
		else {
			// Failed lookup, output error message.
			cout << "Lookup error: <em>" << bible.error(result);
			switch(result) {
				case NO_CHAPTER:
					cout << " in " << request.getRef().getBookName();
					break;
				case NO_VERSE:
					cout << " in " << request.getRef().getBookName() << " " << request.getRef().getChapter();
					break;
			}
			cout << "</em>" << endl;
		}
	}
}
