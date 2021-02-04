# AJAX-based Bible Reader Program
# Based on version by Dr. Robert Kasper, MVNU Computer Science (January 2020)
# Version by Benjamin Leskey (February 2021).

# Deployment user.
USER= benleskey

# Deployment targets
PutCGI= /var/www/html/class/csc3004/$(USER)/cgi-bin/bibleajax.cgi
PUTHTML= /var/www/html/class/csc3004/$(USER)/cgi-bin/bibleajax.html

# Use GNU C++ compiler with C++11 standard
CC= g++
CFLAGS= -g -std=c++11

# Default target deploys to web server.
all: $(PutCGI) $(PutHTML)

bibleajax.cgi: bibleajax.o Ref.o Verse.o Bible.o
	$(CC) $(CFLAGS) -o bibleajax.cgi bibleajax.o Ref.o Verse.o Bible.o -lcgicc

bibleajax.o: bibleajax.cpp Ref.h Verse.h Bible.h
	$(CC) $(CFLAGS) -c bibleajax.cpp

Ref.o : Ref.h Ref.cpp
	$(CC) $(CFLAGS) -c Ref.cpp

Verse.o : Ref.h Verse.h Verse.cpp
	$(CC) $(CFLAGS) -c Verse.cpp

Bible.o : Ref.h Verse.h Bible.h Bible.cpp
	$(CC) $(CFLAGS) -c Bible.cpp

# Program deployment.
$(PutCGI): bibleajax.cgi
	cp bibleajax.cgi $(PutCGI)
	chmod 755 $(PutCGI)

# HTML interface deployment.
$(PutHTML): bibleajax.html
	cp bibleajax.html $(PutHTML)

clean:
	rm -f *.o core bibleajax.cgi
