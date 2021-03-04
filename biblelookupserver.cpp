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

/* Map of version identifiers to Bible objects. */
static std::map<std::string, std::shared_ptr<Bible>> bibles;

/* Communication pipe identifiers. */
static const std::string pipe_id_receive = "bible_request";
static const std::string pipe_id_send = "bible_reply";

int main() {
	/* Load all Bible versions. */
	for(auto version : Bible::versionList()) {
		std::cout << "Loading and indexing Bible version: " << version << std::endl;
		bibles[version] = std::make_shared<Bible>(Bible::versionToFile(version));

		/* If the Bible was not valid, remove this version from the map. */
		if(!bibles[version]->valid()) {
			std::cout << "Could not open Bible version: " << version << std::endl;
			bibles.erase(version);
		}
	}

	Fifo pipe_receive(pipe_id_receive);
	Fifo pipe_send(pipe_id_send);

	pipe_receive.openread();

	for(;;) {
		std::string request = pipe_receive.recv();

		std::cout << "Got request: " << request << endl;

		std::string version = GetNextToken(request, " ");
		std::string refText = GetNextToken(request, " ");
		std::string numberofVersesText = GetNextToken(request, " ");

		LookupResult result = OTHER;
		Ref ref(refText);
		int numberOfVerses = std::stoi(numberofVersesText);

		pipe_send.openwrite();

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
			std::shared_ptr<Bible> bible = bibles[version];
			Verse verse = bible->lookup(ref, result);

			pipe_send.send(std::to_string(result));

			if(result == SUCCESS) {
				for(int i = 0; i < numberOfVerses && result == SUCCESS; i++) {
					std::stringstream ss;
					ss << verse.getRef().getBook() << ":" << verse.getRef().getChapter() << ":" << verse.getRef().getVerse() << " " << verse.getVerse();
					pipe_send.send(ss.str());

					/* Go to next verse if possible and if it's in the same book. */
					Ref nextRef = bible->next(verse.getRef(), result);
					if(nextRef.getBook() != ref.getBook()) {
						break;
					}
					if(result == SUCCESS) {
						verse = bible->lookup(nextRef, result);
					}
				}
			}
			else {
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

		pipe_send.send("$end");
		pipe_send.fifoclose();

		std::cout << "Request complete, status: " << Bible::error(result) << endl;
	}
}