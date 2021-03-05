# AJAX-based Bible Reader Program
# Based on version by Dr. Robert Kasper, MVNU Computer Science (January 2020)
# Version by Benjamin Leskey (February 2021).

# Deployment user.
USER= benleskey

# Deployment targets
PutCGI= /var/www/html/class/csc3004/$(USER)/cgi-bin/bibleajax.cgi
PutHTML= /var/www/html/class/csc3004/$(USER)/bibleajax.html

# Use GNU C++ compiler with C++11 standard
CC= g++
CFLAGS= -g -std=c++11 -Werror -Wall -Og

# Default target deploys to web server.
all: $(PutCGI) $(PutHTML) testreader biblelookupserver

biblelookupserver: biblelookupserver.o fifo.o Ref.o Verse.o Bible.o
	$(CC) $(CFLAGS) -o $@ $^

bibleajax.cgi: bibleajax.o Ref.o Verse.o Bible.o fifo.o BibleLookupClient.o
	$(CC) $(CFLAGS) -o $@ $^ -lcgicc

testreader: testreader.o Ref.o Verse.o Bible.o fifo.o BibleLookupClient.o
	$(CC) $(CFLAGS) -o $@ $^

biblelookupserver.o: biblelookupserver.cpp fifo.h Ref.h Verse.h Bible.h
	$(CC) $(CFLAGS) -c -o $@ $<

bibleajax.o: bibleajax.cpp Ref.h Verse.h Bible.h logfile.h BibleLookupClient.h
	$(CC) $(CFLAGS) -c -o $@ $<

testreader.o: testreader.cpp Ref.h Verse.h Bible.h BibleLookupClient.h
	$(CC) $(CFLAGS) -c -o $@ $<

fifo.o: fifo.cpp fifo.h
	$(CC) $(CFLAGS) -c -o $@ $<

BibleLookupClient.o: BibleLookupClient.cpp BibleLookupClient.h Bible.h Verse.h Ref.h fifo.h
	$(CC) $(CFLAGS) -c -o $@ $<

Ref.o : Ref.cpp Ref.h
	$(CC) $(CFLAGS) -c -o $@ $<

Verse.o : Verse.cpp Ref.h Verse.h
	$(CC) $(CFLAGS) -c -o $@ $<

Bible.o : Bible.cpp Ref.h Verse.h Bible.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Program deployment.
$(PutCGI): bibleajax.cgi
	rm -f $(PutCGI)
	cp bibleajax.cgi $(PutCGI)
	chmod 755 $(PutCGI)

# HTML interface deployment.
$(PutHTML): bibleajax.html
	cp bibleajax.html $(PutHTML)

clean:
	rm -f *.o core bibleajax.cgi testreader biblelookupserver
