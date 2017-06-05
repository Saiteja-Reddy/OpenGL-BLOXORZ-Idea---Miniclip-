all: sample2D

sample2D: Sample_GL3_2D.cpp glad.c
	g++ -o sample2D Sample_GL3_2D.cpp glad.c -framework OpenGL -lglfw -lSOIL -framework CoreFoundation -std=c++11

clean:
	rm sample2D
