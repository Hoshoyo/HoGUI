all:
	mkdir -p bin
	gcc -g -Iinclude *.c renderer/*.c font/*.c hogui/*.c -o bin/hogui -lglfw -lGL -lm lib/libfreetype.so.6.16.1