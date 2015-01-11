xmlnode_HEADERS = \
	xmlnode.h \
	xmlparse.h 

xmlnode_OBJECTS = \
	pool.o \
	str.o \
	expat.o \
	xmlparse.o \
	xmlrole.o \
	xmltok.o \
	hashtable.o \
	xmlnode.o

all: $(xmlnode_OBJECTS) $(xmlnode_HEADERS)
	rm -f xmlnode.a
	ar cru xmlnode.a $(xmlnode_OBJECTS)
	ranlib xmlnode.a

$(xmlnode_OBJECTS): $(xmlnode_HEADERS)

clean:
	rm -f $(xmlnode_OBJECTS) xmlnode.a
