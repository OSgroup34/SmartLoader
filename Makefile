all:
	gcc  -m32 -no-pie -nostdlib -o fib fib.c
	gcc -m32 -o SmartLoader SmartLoader.c

clean:
	-@rm -f fib SmartLoader