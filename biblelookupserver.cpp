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
	for(auto version : Bible::getVersionList()) {
		std::cout << "Loading and indexing Bible version: " << version << std::endl;
		bibles[version] = std::make_shared<Bible>(Bible::getVersionFile(version));

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

	for(;;) {
		/* Get the next request. */
		pipe_receive.openread();
		std::string request = pipe_receive.recv();
		pipe_receive.fifoclose();

		std::cout << "Got request: " << request << std::endl;

		/* Split into pieces. */
		std::string version = GetNextToken(request, " ");
		std::string requestType = GetNextToken(request, " ");
		std::string refText = GetNextToken(request, " ");

		LookupResult result;
		Ref ref(refText);

		std::cout << "Parsed request, now processing..." << std::endl;

		/* Begin writing reply. */
		pipe_send.openwrite();

		std::stringstream out;

		/* First check for error conditions, then do the actual lookup. */
		if(bibles.count(version) == 0) {
			result = OTHER;
			out << result;
		}
		else {
			/* Access the appropriate bible. */
			std::shared_ptr<Bible> bible = bibles[version];

			/* Perform requested operation and return results. */
			if(requestType == "lookup") {
				Verse verse = bible->lookup(ref, result);
				out << result << " " << ref.toString() << " " << verse.getVerse();
			}
			else if(requestType == "next") {
				Ref nextRef = bible->next(ref, result);
				out << result << " " << nextRef.toString();
			}
			else if(requestType == "prev") {
				Ref prevRef = bible->prev(ref, result);
				out << result << " " << prevRef.toString();
			}
			else {
				result = OTHER;
				out << result;
			}
		}

		/* Write and close. */
		pipe_send.send(out.str());
		pipe_send.fifoclose();

		std::cout << "Request complete, status: " << Bible::error(result) << endl;
	}
}
