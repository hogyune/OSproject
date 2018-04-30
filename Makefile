#
# To compile, type "make" or make "all"
# To remove files, type "make clean"
#
OBJS = server.o request.o stems.o clientGet.o clientPost.o Mysql.o
TARGET = server

CC = gcc
CFLAGS = -g -Wall

LIBS = -lpthread -L /usr/lib/mysql -l mysqlclient -lm

.SUFFIXES: .c .o 

all: server clientPost clientGet dataGet.cgi dataPost.cgi alarm.cgi pushClient.cgi pushServer.cgi

server: server.o request.o stems.o
	$(CC) $(CFLAGS) -o server server.o request.o stems.o $(LIBS)

clientGet: clientGet.o stems.o
	$(CC) $(CFLAGS) -o clientGet clientGet.o stems.o

clientPost: clientPost.o stems.o
	$(CC) $(CFLAGS) -o clientPost clientPost.o stems.o $(LIBS)

dataGet.cgi: dataGet.c stems.h Mysql.o
	$(CC) $(CFLAGS) -o dataGet.cgi dataGet.c stems.o Mysql.o $(LIBS)

dataPost.cgi: dataPost.c stems.h Mysql.o
	$(CC) $(CFLAGS) -o dataPost.cgi dataPost.c stems.o Mysql.o $(LIBS)

alarm.cgi: push.c stems.h
	$(CC) $(CFLAGS) -o alarm.cgi push.c stems.o $(LIBS)

pushServer.cgi: pushServer.c stems.h
	$(CC) $(CFLAGS) -o pushServer.cgi pushServer.c request.o stems.o $(LIBS)

pushClient.cgi: pushClient.c stems.h
	$(CC) $(CFLAGS) -o pushClient.cgi pushClient.c request.o stems.o $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

server.o: stems.h request.h
clientGet.o: stems.h
clientPost.o: stems.h
Mysql.o : Mysql.h

clean:
	-rm -f $(OBJS) server clientPost clientGet dataGet.cgi dataPost.cgi alarm.cgi pushClient.cgi pushServer.cgi
