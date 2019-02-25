DIRS=bin obj

all: obj/writer.o obj/reader.o obj/product.o obj/main.o
	gcc -g -o bin/prodmat obj/writer.o obj/reader.o obj/product.o obj/main.o -pthread

obj/writer.o: src/writer.c src/writer.h src/prodmat.h
	gcc -g -c src/writer.c -o obj/writer.o

obj/reader.o: src/reader.c src/reader.h src/prodmat.h
	gcc -g -c src/reader.c -o obj/reader.o

obj/product.o: src/product.c src/product.h src/prodmat.h
	gcc -g -c src/product.c -o obj/product.o

obj/main.o: src/main.c src/writer.h src/reader.h src/prodmat.h src/product.h
	gcc -g -c src/main.c -o obj/main.o

clean:
	rm bin/* obj/*.o; \
	rmdir $(DIRS)

$(info $(shell mkdir -p $(DIRS)))
