all:
	cc -std=c99 -Wall lispy.c mpc.c -ledit -lm -o lispy
clean:
	rm lispy
