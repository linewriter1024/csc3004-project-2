// biblereader.cpp
// main function for Project 1

#include "Bible.h"
#include "Ref.h"
#include "fifo.h"

#include <sstream>
#include <iostream>
#include <cstdlib>

/* Communication pipe identifiers. */
static const std::string pipe_id_receive = "bible_reply";
static const std::string pipe_id_send = "bible_request";

int main (int argc, char **argv) {
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

	/* Open Communication */
	Fifo pipe_receive(pipe_id_receive);
	Fifo pipe_send(pipe_id_send);

	/* Prepare for request. */
	pipe_send.openwrite();

	/* Send request with the default version specified. */
	std::stringstream ss;
	ss << Bible::getDefaultVersion() << " " << b << ":" << c << ":" << v << " " << length;
	pipe_send.send(ss.str());

	/* Begin reading reply. */
	pipe_receive.openread();

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
				break;
			}
			else if(status == SUCCESS) {
				// There is a verse, get the verse from this line.
				Verse verse(line);

				// Output the chapter heading if this is a new chapter.
				if(currentChapter != verse.getRef().getChapter()) {
					currentChapter = verse.getRef().getChapter();
					std::cout << verse.getRef().getBookName() << " " << verse.getRef().getChapter() << std::endl;
				}

				// Output the verse.
				std::cout << " " << verse.getRef().getVerse() << ". " << verse.getVerse() << std::endl;
			}
			else {
				// No verse, just output the error message.
				std::cout << "Error: " << line << std::endl;
			}
		}
		else {
			// This is the first line, set the status.
			status = static_cast<LookupResult>(atoi(line.c_str()));
			gotStatus = true;
		}
	}

	/* Close Communication */
	pipe_receive.fifoclose();
	pipe_send.fifoclose();
}
