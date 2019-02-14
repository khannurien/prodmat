DIRS=bin obj

all: obj/writer.o obj/main.o
	gcc -o bin/prodmat obj/writer.o obj/main.o

obj/writer.o: src/writer.c src/writer.h src/prodmat.h
	gcc -c src/writer.c -o obj/writer.o

obj/main.o: src/main.c src/writer.h src/prodmat.h
	gcc -c src/main.c -o obj/main.o

clean:
	rm bin/* obj/*.o; \
	rmdir $(DIRS)

$(info $(shell mkdir -p $(DIRS)))
