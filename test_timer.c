#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/timerfd.h>
#include <errno.h>
#include "timer_heap.h"

int create_timerfd()
{
    int sfd;
    sfd = timerfd_create(CLOCK_REALTIME, 0);
    if (sfd == -1) {
        fprintf(stderr, "Failed in timerfd_create (%d)\n", errno);
        exit(1);
    }
    return sfd;
}

int set_timer(int sfd, struct timespec *expires)
{
    struct itimerspec its;
    struct timespec now;
    
    clock_gettime(CLOCK_REALTIME, &now);
    if(TIMERCMP(expires, &now, <)) {
        printf("set_timer\n");
        return -1;
    }
    /* Start the timer */
    its.it_value = *expires;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
    if (timerfd_settime(sfd, TFD_TIMER_ABSTIME, &its, NULL) == -1) {
        fprintf(stderr, "Failed in timer_settime (%d)\n", errno);
        exit(1);
    }
    return 0;
}

void timeout_cb_fn(void *arg)
{
    struct timespec delay = {1, 0};
    struct timeout *t = arg;

    printf("timeout callback\n");
    timer_heap_del(t);
    timer_heap_add(t, &delay);
}

static void my_cb(void *arg)
{
	int i=(int)arg;
	printf("i=%d\n",i);
}

int main()
{
    struct timeout t[50];
    struct timeout *top;
    struct timespec delay = {1, 0};
    int sfd, count = 0;

    sfd = create_timerfd();
    timer_heap_init(1);
	srand(time(NULL));
	for(count=0;count<50;count++) {
		timeout_setcb(t+count,my_cb,(void *)count);
		delay.tv_sec=random()%10;
		printf("tv_sec:%d\n",delay.tv_sec);
		timer_heap_add(t+count,&delay);
	}

	for(count=0;count<50;count++) {
		unsigned long long exp;
		if(NULL ==  (top=timer_heap_top())) {
			fprintf(stderr,"there is no timer in heap!\n");
			break;
		}
		set_timer(sfd,&top->expires);
		if(read(sfd,&exp,sizeof(exp)) != sizeof(exp)) {
			fprintf(stderr,"error in read timerfd\n");
			exit(1);
		}
		printf("in for\n");
		top->callback.fn(top->callback.arg);
		timer_heap_del(top);
	}
do{
    struct timeout t;
	timeout_setcb(&t, timeout_cb_fn, &t);
	delay.tv_sec=1;
    timer_heap_add(&t, &delay);

	count=0;
    while(count < 10) {
        unsigned long long exp;
        ssize_t res;

        if(NULL == (top = timer_heap_top())) {
            fprintf(stderr, "there is no timer in heap!!!\n");
            break;
        }
        set_timer(sfd, &top->expires);
        res = read(sfd, &exp, sizeof(exp));
        if ((res < 0) || (res != sizeof(exp))) {
            fprintf(stderr, "Failed in read (%d)\n", errno);
            exit(1);
        }
        top->callback.fn(top->callback.arg);
        count++;
    }
}while(0);
    timer_heap_destroy();
    return 0;
}

