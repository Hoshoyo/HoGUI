all: build
	gcc -DHHU_USE_GLFW -g -I include src/*.c lib/hogui.a lib/libfreetype.a -lGL -lglfw -lm -o bin/hogui
build:
	cd bin; rm *.o; gcc -DGM_STATIC -DHHU_USE_GLFW -I ../include -g -c ../src/hogui/*.c
	cd bin; ar rcs ../lib/hogui.a *.o