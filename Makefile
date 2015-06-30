all:test_timer

test_timer: test_timer.c timer_heap.c timer_heap.h
	gcc -o test_timer test_timer.c timer_heap.c

clean:
	rm test_timer
