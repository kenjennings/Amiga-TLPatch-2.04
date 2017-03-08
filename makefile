
all: TLPatch204.o TLPatch204.c Translate204.o Translate204.c
	slink FROM LIB:c.o+TLPatch204.o TO TLPatch204 LIB LIB:sc.lib+LIB:amiga.lib
	slink FROM LIB:c.o+Translate204.o TO Translate204 LIB LIB:sc.lib+LIB:amiga.lib

