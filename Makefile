DIRS=bin obj

all: bin/prodmat bin/writemat

bin/prodmat: obj/writer.o obj/reader.o obj/product.o obj/prodmat.o
	gcc -g -o bin/prodmat obj/writer.o obj/reader.o obj/product.o obj/prodmat.o -pthread

bin/writemat: obj/writer.o obj/reader.o obj/writemat.o
	gcc -g -o bin/writemat obj/writer.o obj/reader.o obj/writemat.o

obj/writer.o: src/writer.c src/writer.h src/prodmat.h
	gcc -g -c src/writer.c -o obj/writer.o

obj/reader.o: src/reader.c src/reader.h src/prodmat.h
	gcc -g -c src/reader.c -o obj/reader.o

obj/product.o: src/product.c src/product.h src/prodmat.h
	gcc -g -c src/product.c -o obj/product.o

obj/prodmat.o: src/prodmat.c src/writer.h src/reader.h src/prodmat.h src/product.h
	gcc -g -c src/prodmat.c -o obj/prodmat.o

obj/writemat.o: src/writemat.c src/writer.h src/reader.h src/prodmat.h
	gcc -g -c src/writemat.c -o obj/writemat.o

clean:
	rm bin/* obj/*.o; \
	rmdir $(DIRS)

$(info $(shell mkdir -p $(DIRS)))
