all: sample3D sample2D

sample3D: Sample_GL3_3D.cpp glad.c
	g++ Sample_GL3_3D.cpp glad.c -lGL -lglfw -ldl

sample2D: Sample_GL3_2D.cpp glad.c
	g++ Sample_GL3_2D.cpp glad.c -lGL -lglfw -ldl

clean:
	rm sample2D sample3D
