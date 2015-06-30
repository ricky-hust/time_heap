#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "timer_heap.h"

typedef struct min_heap
{
    struct timeout** p;
    unsigned n, a;
} min_heap_t;

static min_heap_t timeouts;

static void min_heap_ctor(min_heap_t* s) { s->p = 0; s->n = 0; s->a = 0; }
static void min_heap_dtor(min_heap_t* s) { if(s->p) free(s->p); }
static void min_heap_elem_init(struct timeout* e) { e->min_heap_idx = -1; }
static int min_heap_empty(min_heap_t* s) { return 0u == s->n; }
static unsigned min_heap_size(min_heap_t* s) { return s->n; }
static struct timeout* min_heap_top(min_heap_t* s) { return s->n ? *s->p : 0; }
static int min_heap_elem_greater(struct timeout *a, struct timeout *b)
{
    return TIMERCMP(&a->expires, &b->expires, >);
}

static int min_heap_reserve(min_heap_t* s, unsigned n)
{
    if(s->a < n)
    {
        struct timeout** p;
        unsigned a = s->a ? s->a * 2 : 8;
        if(a < n)
            a = n;
        if(!(p = (struct timeout**)realloc(s->p, a * sizeof *p)))
            return -1;
        s->p = p;
        s->a = a;
    }
    return 0;
}

static void min_heap_shift_up_(min_heap_t* s, unsigned hole_index, struct timeout* e)
{
    unsigned parent = (hole_index - 1) / 2;
    while(hole_index && min_heap_elem_greater(s->p[parent], e))
    {
        (s->p[hole_index] = s->p[parent])->min_heap_idx = hole_index;
        hole_index = parent;
        parent = (hole_index - 1) / 2;
    }
    (s->p[hole_index] = e)->min_heap_idx = hole_index;
}

static void min_heap_shift_down_(min_heap_t* s, unsigned hole_index, struct timeout* e)
{
    unsigned min_child = 2 * (hole_index + 1);
    while(min_child <= s->n)
    {
        min_child -= min_child == s->n || min_heap_elem_greater(s->p[min_child], s->p[min_child - 1]);
        if(!(min_heap_elem_greater(e, s->p[min_child])))
            break;
        (s->p[hole_index] = s->p[min_child])->min_heap_idx = hole_index;
        hole_index = min_child;
        min_child = 2 * (hole_index + 1);
    }
    //min_heap_shift_up_(s, hole_index,  e);
    (s->p[hole_index] = e)->min_heap_idx = hole_index;
}

static int min_heap_push(min_heap_t* s, struct timeout* e)
{
    if(min_heap_reserve(s, s->n + 1))
        return -1;
    min_heap_shift_up_(s, s->n++, e);
    return 0;
}

static struct timeout* min_heap_pop(min_heap_t* s)
{
    if(s->n)
    {
        struct timeout* e = *s->p;
        min_heap_shift_down_(s, 0u, s->p[--s->n]);
        e->min_heap_idx = -1;
        return e;
    }
    return NULL;
}

static int min_heap_erase(min_heap_t* s, struct timeout* e)
{
    if(-1 != e->min_heap_idx)
    {
        struct timeout *last = s->p[--s->n];
        unsigned parent = (e->min_heap_idx - 1) / 2;
        /* we replace e with the last element in the heap.  We might need to
           shift it upward if it is less than its parent, or downward if it is
           greater than one or both its children.*/
        if (e->min_heap_idx > 0 && min_heap_elem_greater(s->p[parent], last))
             min_heap_shift_up_(s, e->min_heap_idx, last);
        else
             min_heap_shift_down_(s, e->min_heap_idx, last);
        e->min_heap_idx = -1;
        return 0;
    }
    return -1;
}

void timer_heap_init(size_t count)
{
    min_heap_ctor(&timeouts);
    if (min_heap_reserve(&timeouts, count) < 0) {
		printf("realloc failed!!!!\n");
		exit(1);
    }
}

void timer_heap_destroy()
{
    min_heap_dtor(&timeouts);
}

void timer_heap_add(struct timeout *to, struct timespec *delay)
{
    struct timespec now;

    clock_gettime(CLOCK_REALTIME, &now);
    TIMERADD(&now, delay, &to->expires);
    if (min_heap_push(&timeouts, to) < 0) {
		printf("realloc failed!!!!\n");
		exit(1);
    }
}

void timer_heap_del(struct timeout *to)
{
    min_heap_erase(&timeouts, to);
}

int timer_heap_empty()
{
	min_heap_empty(&timeouts);
}

struct timeout *timer_heap_top()
{
    struct timeout *to;
    struct timespec now;

    clock_gettime(CLOCK_REALTIME, &now);
    while (NULL != (to = min_heap_top(&timeouts))) {
		if(TIMERCMP(&to->expires, &now, >=))
			return to;
		printf("timer out before callback fn get executed\n");
		min_heap_pop(&timeouts);
		to->callback.fn(to->callback.arg);	
    }

    return NULL;
}

struct timeout *timer_heap_pop()
{
	return min_heap_pop(&timeouts);
}

