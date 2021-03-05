#ifndef BIBLELOOKUPCLIENT_H
#define BIBLELOOKUPCLIENT_H

#include <string>
#include "fifo.h"
#include "Bible.h"
#include "Verse.h"
#include "Ref.h"

class BibleLookupClient {
private:
	Fifo pipe_request;
	Fifo pipe_reply;
	std::string bibleVersion;

	struct ServerReply {
		// Result. Other fields are only valid if this is SUCCESS.
		LookupResult result;
		// Reference returned.
		Ref ref;
		// Verse text (including leading Ref string), only valid if the request action was "lookup".
		std::string verseText;
	};

	// Send a request to the server for an action {lookup, next, prev} on the specified ref.
	// Will get back the server's processed reply.
	ServerReply request(std::string action, const Ref &ref);
public:
	// Connect to a Bible lookup server identified by the request and reply pipe IDs for the specified Bible version.
	BibleLookupClient(std::string pipe_request_id, std::string pipe_reply_id, std::string bibleVersion);

	// Try to get the verse identified by Ref. Record status of lookup in result.
	Verse lookup(const Ref &ref, LookupResult &result);

	// Try to get the ref after the specified ref. Record status of lookup in result.
	Ref next(const Ref &ref, LookupResult &result);

	// Try to get the ref before the specified ref. Record status of lookup in result.
	Ref prev(const Ref &ref, LookupResult &result);
};

#endif
