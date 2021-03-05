#include <sstream>

#include "BibleLookupClient.h"
#include "Ref.h"

BibleLookupClient::BibleLookupClient(std::string pipe_request_id, std::string pipe_reply_id, std::string bibleVersion) : pipe_request(pipe_request_id), pipe_reply(pipe_reply_id), bibleVersion(bibleVersion) {}

BibleLookupClient::ServerReply BibleLookupClient::request(std::string action, const Ref &ref) {
	ServerReply reply;

	/* Construct the request and send it. */
	pipe_request.openwrite();
	std::stringstream out;
	out << bibleVersion << " " << action << " " << ref.toString();
	pipe_request.send(out.str());
	pipe_request.fifoclose();

	/* Receive the server's reply. */
	pipe_reply.openread();
	std::string replyText = pipe_reply.recv();
	pipe_reply.fifoclose();

	/* Split the reply. */
	std::string statusText = GetNextToken(replyText, " ");
	std::string verseText = replyText; // The rest of the reply is the verse line (including ref and text).

	/* Convert the reply. */
	reply.result = static_cast<LookupResult>(atoi(statusText.c_str()));
	reply.ref = Ref(verseText);
	reply.verseText = verseText;

	return reply;
}

/* Request wrapper functions for lookup, next, and prev. */
Verse BibleLookupClient::lookup(const Ref &ref, LookupResult &result) {
	ServerReply reply = request("lookup", ref);

	result = reply.result;
	return Verse(reply.verseText);
}

Ref BibleLookupClient::next(const Ref &ref, LookupResult &result) {
	ServerReply reply = request("next", ref);

	result = reply.result;
	return reply.ref;
}

Ref BibleLookupClient::prev(const Ref &ref, LookupResult &result) {
	ServerReply reply = request("prev", ref);

	result = reply.result;
	return reply.ref;
}
