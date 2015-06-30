#ifndef __TIMER_HEAP_H__
#define __TIMER_HEAP_H__

#define TIMERADD(tvp, uvp, vvp)                    \
    do {                                \
        (vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec;        \
        (vvp)->tv_nsec = (tvp)->tv_nsec + (uvp)->tv_nsec;       \
        if ((vvp)->tv_nsec >= 1000000000) {            \
            (vvp)->tv_sec++;                \
            (vvp)->tv_nsec -= 1000000000;            \
        }                            \
    } while (0)

#define    TIMERSUB(tvp, uvp, vvp)                    \
    do {                                \
        (vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec;        \
        (vvp)->tv_nsec = (tvp)->tv_nsec - (uvp)->tv_nsec;    \
        if ((vvp)->tv_nsec < 0) {                \
            (vvp)->tv_sec--;                \
            (vvp)->tv_nsec += 1000000000;            \
        }                            \
    } while (0)

/** Return true iff the tvp is related to uvp according to the relational
 * operator cmp.  Recognized values for cmp are ==, <=, <, >=, and >. */
#define    TIMERCMP(tvp, uvp, cmp)                    \
    (((tvp)->tv_sec == (uvp)->tv_sec) ?                \
     ((tvp)->tv_nsec cmp (uvp)->tv_nsec) :                \
     ((tvp)->tv_sec cmp (uvp)->tv_sec))

struct timeout_cb {
    void (*fn)(void *);
    void *arg;
};

#define TIMEOUT_INT 0x01 /* interval (repeating) timeout */
#define TIMEOUT_ABS 0x02 /* treat timeout values as absolute */

#define TIMEOUT_INITIALIZER(flags) { (flags) }

#define timeout_setcb(to, func, args) do { \
    (to)->callback.fn = (func);       \
    (to)->callback.arg = (args);     \
} while (0)

struct timeout {
    int flags;
    int min_heap_idx;
    struct timespec expires;
    struct timeout_cb callback;
};

void timer_heap_init(size_t count);
void timer_heap_add(struct timeout *to, struct timespec *delay);
void timer_heap_del(struct timeout *to);
/* Get first valid timer */
struct timeout *timer_heap_top();

#endif