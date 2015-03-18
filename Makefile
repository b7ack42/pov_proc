all: pov_proc
pov_proc: pov_proc.cpp tinyxml2.cpp tinyxml2.h
clean:
	rm -f *.o pov_proc
