all: run
lispy:
	cc -std=c99 -Wall lispy.c mpc.c -ledit -lm -o lispy
run: lispy
	./lispy
clean:
	rm lispy
