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
#include <sstream>
using namespace std;

/* Required libraries for AJAX to function */
#include "/home/class/csc3004/cgicc/Cgicc.h"
#include "/home/class/csc3004/cgicc/HTTPHTMLHeader.h"
#include "/home/class/csc3004/cgicc/HTMLClasses.h"
using namespace cgicc;

#include "Bible.h"
#include "fifo.h"

// Including the logging system.
#define logging
#define LOG_FILENAME "/tmp/benleskey-bibleajax.log"
#include "logfile.h"

// Check if a string represents an integer.
static bool stringIsInteger(std::string s) {
	// Possible digits (and negative sign).
	static const std::string integer_chars = "-0123456789";
	// s is an integer if it has characters and has no character that is not a digit/negative
	return !s.empty() && s.find_first_not_of(integer_chars) == std::string::npos;
}

// An incoming request.
// Wraps Cgicc.
class BibleCGIRequest {
public:
	// Construct the request object from the CGI request data.
	// Will set failed state if anything went wrong.
	BibleCGIRequest() : cgi(), failed(false), errorMessage("") {
		// Get the CGI input data.
		form_iterator bible = cgi.getElement("bible");
		form_iterator book = cgi.getElement("book");
		form_iterator chapter = cgi.getElement("chapter");
		form_iterator verse = cgi.getElement("verse");
		form_iterator nv = cgi.getElement("num_verse");

		// Get the bible version.
		bibleVersion = inputToBibleVersion(bible, "bible version");

		// Construct the Ref from the input.
		ref = Ref(
			inputToInteger<Ref::book_id>(book, "book", Ref::MIN_BOOK_ID, Ref::MAX_BOOK_ID),
			inputToInteger<Ref::chapter_id>(chapter, "chapter", Ref::MIN_CHAPTER_ID, Ref::MAX_CHAPTER_ID),
			inputToInteger<Ref::verse_id>(verse, "verse", Ref::MIN_VERSE_ID, Ref::MAX_VERSE_ID)
		);

		// Get the desired verse count.
		numberOfVerses = inputToInteger<int>(nv, "verse count", 1, std::numeric_limits<int>::max());
	}

	// Check if the request processing failed.
	bool getFailed() { return failed; }
	// Get the failure error message.
	std::string getErrorMessage() { return errorMessage; }

	// Get the request reference. Only works after success.
	Ref getRef() { return ref; }
	// Get the desired number of verses. Only works after success.
	int getNumberOfVerses() { return numberOfVerses; }
	// Get the desired Bible version. Only works after success.
	std::string getBibleVersion() { return bibleVersion; };
private:
	// Create the Cgicc object within the Request.
	Cgicc cgi;

	std::string bibleVersion;
	Ref ref;
	int numberOfVerses;

	bool failed;
	std::string errorMessage;

	// Set the failure state on, with the specified error message.
	void fail(std::string message) {
		failed = true;
		errorMessage = message;
	}

	// Returns true if an element exists and is non-empty, false otherwise.
	bool elementSpecified(const form_iterator &element) {
		return element != cgi.getElements().end() && !element->getValue().empty();
	}

	// Get an input string representing a valid Bible version from an element with human-readable identifier name.
	// Will update the failed state if the element does not refer to a valid version.
	// If the fail is set or becomes set, the return value will be invalid.
	std::string inputToBibleVersion(const form_iterator &element, std::string name) {
		std::string result;

		// Only try anything if we've not already failed.
		if(!failed) {
			// Check if the element is not specified.
			if(!elementSpecified(element)) {
				fail("the " + name + " was not specified");
			}
			// Ensure the bible version is valid.
			else if(!Bible::versionExists(element->getValue())) {
				fail("the specified " + name + " is not a recognized bible version");
			}
			else {
				// Valid version, set result.
				result = element->getValue();
			}
		}

		// Return result, will be invalid data on failure.
		return result;
	}

	// Get an input integer of type T from an element with human-readable identifier name, pinned between min and max.
	// Will update the failed state if the element does not fit the qualifications.
	// If the fail is set or becomes set, the return value will be invalid.
	template<typename T>
	T inputToInteger(const form_iterator &element, std::string name, const T min, const T max) {
		// Result, default to min.
		T result = min;

		// Only try anything if we've not already failed.
		if(!failed) {
			// Check if the element is not specified.
			if(!elementSpecified(element)) {
				fail("the " + name + " was not specified");
			}
			// Ensure the parameter is actually an integer.
			else if(!stringIsInteger(element->getValue())) {
				fail("the specified " + name + " is not an integer");
			}
			else {
				long value = element->getIntegerValue();

				// Range check on original value.
				if(value < min) {
					fail("the specified " + name + " is below " + std::to_string(min));
				}
				else if(value > max) {
					fail("the specified " + name + " is above " + std::to_string(max));
				}

				// Assign (or cast) result from long value.
				result = value;
			}
		}

		// Return result, will be invalid data if anything failed.
		return result;
	}
};

/* Communication pipe identifiers. */
static const std::string pipe_id_receive = "bible_reply";
static const std::string pipe_id_send = "bible_request";

int main() {
	// Begin logging.
	#ifdef logging
		logFile.open(logFilename.c_str(),ios::out);
	#endif

	// Send the required CGI content type header.
	// Plain text, we are only rendering part of a page.
	cout << "Content-Type: text/plain\n\n";

	// Construct the request wrapper (it will create the Cgicc instance).
	BibleCGIRequest request;

	if(request.getFailed()) {
		// Output initial input error message upon failure.
		cout << "<p>Input error: <em>" << request.getErrorMessage() << "</em></p>";
		log("request itself was invalid: " + request.getErrorMessage());
	}
	else {
		/* Open Communication */
		Fifo pipe_receive(pipe_id_receive);
		Fifo pipe_send(pipe_id_send);

		/* Prepare for request. */
		pipe_send.openwrite();
		log("opened request pipe");

		/* Send request to the server with the parameters from the CGI request. */
		std::stringstream ss;
		ss << request.getBibleVersion() << " " << request.getRef().getBook() << ":" << request.getRef().getChapter() << ":" << request.getRef().getVerse() << " " << request.getNumberOfVerses();
		pipe_send.send(ss.str());

		log("sending request: " + ss.str());

		/* Begin reading reply. */
		pipe_receive.openread();
		log("opened reply pipe");

		LookupResult status = OTHER; // Overall status of the request.
		bool gotStatus = false; // Have we received the first (status) line yet?
		int currentChapter = -1; // Current chapter being displayed, -1 means nothing yet.

		// Loop through each line in the reply.
		for(;;) {
			// Get the next line in the reply.
			std::string line = pipe_receive.recv();
			if(gotStatus) {
				// If we've already got the first (status) line, process.
				if(line == "$end") {
					// End means we're done.
					log("request fulfilled");
					break;
				}
				else if(status == SUCCESS) {
					// There is a verse, get the verse from this line.
					Verse verse(line);

					// Output the chapter heading if this is a new chapter.
					if(currentChapter != verse.getRef().getChapter()) {
						currentChapter = verse.getRef().getChapter();
						std::cout << "<h2>" <<  verse.getRef().getBookName() << " " << verse.getRef().getChapter() << "</h2>" << std::endl;
					}

					// Output the verse.
					std::cout << "<p><em>" << verse.getRef().getVerse() << ".</em> " << verse.getVerse() << "</p>" << std::endl;
				}
				else {
					// No verse, just output the error message.
					std::cout << "Lookup error: <em>" << line << "</em>" << std::endl;
				}
			}
			else {
				// This is the first line, set the status.
				status = static_cast<LookupResult>(atoi(line.c_str()));
				gotStatus = true;
				log("got status line: " + line + ": " + Bible::error(status));
			}
		}

		/* Close Communication */
		pipe_receive.fifoclose();
		pipe_send.fifoclose();
		log("closed pipes");
	}
}
