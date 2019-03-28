#ifndef DEVICES_TIMER_H
#define DEVICES_TIMER_H

#include <round.h>
#include <stdint.h>
#include <list.h>
/* Number of timer interrupts per second. */
#define TIMER_FREQ 100

void timer_init (void);
void timer_calibrate (void);

int64_t timer_ticks (void);
int64_t timer_elapsed (int64_t);

/* Sleep and yield the CPU to other threads. */
void timer_sleep (int64_t ticks);
void timer_msleep (int64_t milliseconds);
void timer_usleep (int64_t microseconds);
void timer_nsleep (int64_t nanoseconds);

/* Busy waits. */
void timer_mdelay (int64_t milliseconds);
void timer_udelay (int64_t microseconds);
void timer_ndelay (int64_t nanoseconds);

void timer_print_stats (void);

/* Added by JHS Project3 */
/* The list should be managed with non-decreasing order */
struct list* sleep_list;

struct slept {
	int64_t wakeup;
	struct thread* hold;
	struct list_elem elem;
};

void sleep_list_init(void);
void sleep_list_destroy(void);
void sleep_list_wakeup(void);
void sleep_list_insert(int64_t);
bool sleep_list_less_func(const struct list_elem *, const struct list_elem *,void *aux);

#endif /* devices/timer.h */
