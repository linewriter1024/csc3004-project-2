/*
 * biblelookupserver.cpp: Server providing access to the Bible.
 * Author: Benjamin Leskey
 * Date: March 2021
 */

#include "Bible.h"
#include "Ref.h"
#include "fifo.h"

#include <sstream>
#include <iostream>
#include <memory>

/* Communication pipe identifiers. */
static const std::string pipe_id_receive = "bible_request";
static const std::string pipe_id_send = "bible_reply";

/*
 * Load all possible Bible versions.
 * Returns a map of version identifiers to Bible objects.
 *
 * (Uses std::shared_ptr to avoid both copying Bibles and leaking memory.)
 */
std::map<std::string, std::shared_ptr<Bible>> loadAllBibles() {
	std::map<std::string, std::shared_ptr<Bible>> bibles;

	/* Try all versions. */
	for(auto version : Bible::versionList()) {
		std::cout << "Loading and indexing Bible version: " << version << std::endl;
		bibles[version] = std::make_shared<Bible>(Bible::versionToFile(version));

		/* If the Bible was not valid, remove this version from the map. */
		if(!bibles[version]->valid()) {
			std::cout << "Could not open Bible version: " << version << std::endl;
			bibles.erase(version);
		}
	}

	return bibles;
}

int main() {
	/* Load all Bible versions. */
	std::map<std::string, std::shared_ptr<Bible>> bibles = loadAllBibles();

	/* Open communication. */
	Fifo pipe_receive(pipe_id_receive);
	Fifo pipe_send(pipe_id_send);

	std::cout << "Opening pipes and waiting for requests..." << std::endl;

	/* Begin reading requests. */
	pipe_receive.openread();

	for(;;) {
		/* Get the next request. */
		std::string request = pipe_receive.recv();

		std::cout << "Got request: " << request << std::endl;

		/* Split into pieces. */
		std::string version = GetNextToken(request, " ");
		std::string refText = GetNextToken(request, " ");
		std::string numberofVersesText = GetNextToken(request, " ");

		LookupResult result = OTHER;
		Ref ref(refText);
		int numberOfVerses = std::stoi(numberofVersesText);

		std::cout << "Parsed request, now processing..." << std::endl;

		/* Begin writing reply. */
		pipe_send.openwrite();

		/* First check for error conditions, then do the actual lookup. */
		if(bibles.count(version) == 0) {
			result = OTHER;
			pipe_send.send(std::to_string(result));
			pipe_send.send("the specified Bible version is unavailable");
		}
		else if(numberOfVerses < 0) {
			result = OTHER;
			pipe_send.send(std::to_string(result));
			pipe_send.send("the specified number of verses is invalid");
		}
		else {
			/* Access the appropriate bible and get the initial verse. */
			std::shared_ptr<Bible> bible = bibles[version];
			Verse verse = bible->lookup(ref, result);

			/* Send the status immediately. */
			pipe_send.send(std::to_string(result));

			if(result == SUCCESS) {
				// Keep local result just for this loop.
				LookupResult lresult = result;
				/* On success, output the Ref and verse, and repeat for as many verses as were requested, going to the next verse each time, or until the end of the book is reached. */
				for(int i = 0; i < numberOfVerses && lresult == SUCCESS; i++) {
					std::stringstream ss;
					ss << verse.getRef().getBook() << ":" << verse.getRef().getChapter() << ":" << verse.getRef().getVerse() << " " << verse.getVerse();
					pipe_send.send(ss.str());

					/* Go to next verse if possible and if it's in the same book. */
					Ref nextRef = bible->next(verse.getRef(), lresult);
					if(nextRef.getBook() != ref.getBook()) {
						break;
					}
					if(lresult == SUCCESS) {
						verse = bible->lookup(nextRef, lresult);
					}
				}
			}
			else {
				/* Error condition, output the message with added detail. */
				std::stringstream ss;
				ss << bible->error(result);
				switch(result) {
					case NO_CHAPTER:
						ss << " in " << ref.getBookName();
						break;
					case NO_VERSE:
						ss << " in " << ref.getBookName() << " " << ref.getChapter();
						break;
					default:
						break;
				}
				pipe_send.send(ss.str());
			}
		}

		/* Done writing, tell the client this is the end. */
		pipe_send.send("$end");
		pipe_send.fifoclose();

		std::cout << "Request complete, status: " << Bible::error(result) << endl;
	}
}
