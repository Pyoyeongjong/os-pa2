/**********************************************************************
 * Copyright (c) 2019-2022
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

/* THIS FILE IS ALL YOURS; DO WHATEVER YOU WANT TO DO IN THIS FILE */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "types.h"
#include "list_head.h"

/**
 * The process which is currently running
 */
#include "process.h"
extern struct process *current;


/**
 * List head to hold the processes ready to run
 */
extern struct list_head readyqueue;


/**
 * Resources in the system.
 */
#include "resource.h"
extern struct resource resources[NR_RESOURCES];


/**
 * Monotonically increasing ticks. Do not modify it
 */
extern unsigned int ticks;


/**
 * Quiet mode. True if the program was started with -q option
 */
extern bool quiet;


/***********************************************************************
 * Default FCFS resource acquision function
 *
 * DESCRIPTION
 *   This is the default resource acquision function which is called back
 *   whenever the current process is to acquire resource @resource_id.
 *   The current implementation serves the resource in the requesting order
 *   without considering the priority. See the comments in sched.h
 ***********************************************************************/
bool fcfs_acquire(int resource_id)
{
	struct resource *r = resources + resource_id;

	if (!r->owner) {
		/* This resource is not owned by any one. Take it! */
		r->owner = current;
		return true;
	}

	/* OK, this resource is taken by @r->owner. */

	/* Update the current process state */
	current->status = PROCESS_WAIT;

	/* And append current to waitqueue */
	list_add_tail(&current->list, &r->waitqueue);

	/**
	 * And return false to indicate the resource is not available.
	 * The scheduler framework will soon call schedule() function to
	 * schedule out current and to pick the next process to run.
	 */
	return false;
}

/***********************************************************************
 * Default FCFS resource release function
 *
 * DESCRIPTION
 *   This is the default resource release function which is called back
 *   whenever the current process is to release resource @resource_id.
 *   The current implementation serves the resource in the requesting order
 *   without considering the priority. See the comments in sched.h
 ***********************************************************************/
void fcfs_release(int resource_id)
{
	struct resource *r = resources + resource_id;

	/* Ensure that the owner process is releasing the resource */
	assert(r->owner == current);

	/* Un-own this resource */
	r->owner = NULL;

	/* Let's wake up ONE waiter (if exists) that came first */
	if (!list_empty(&r->waitqueue)) {
		struct process *waiter =
				list_first_entry(&r->waitqueue, struct process, list);

		/**
		 * Ensure the waiter is in the wait status
		 */
		assert(waiter->status == PROCESS_WAIT);

		/**
		 * Take out the waiter from the waiting queue. Note we use
		 * list_del_init() over list_del() to maintain the list head tidy
		 * (otherwise, the framework will complain on the list head
		 * when the process exits).
		 */
		list_del_init(&waiter->list);

		/* Update the process status */
		waiter->status = PROCESS_READY;

		/**
		 * Put the waiter process into ready queue. The framework will
		 * do the rest.
		 */
		list_add_tail(&waiter->list, &readyqueue);
	}
}

// first process (if same)
struct process *get_largest_prio(struct list_head* head){
	struct process *pos;
	struct process *next = NULL;

	int priority = -1;
	list_for_each_entry(pos, head, list){
		if(priority < 0){
			priority = pos->prio;
			next = pos;
			continue;
		}
		if(pos->prio > priority){
			priority = pos->prio;
			next = pos;
		}
	}
	
	return next;
}


bool prio_acquire(int resource_id)
{
	struct resource *r = resources + resource_id;

	if (!r->owner) {
		/* This resource is not owned by any one. Take it! */
		r->owner = current;
		return true;
	}

	/* OK, this resource is taken by @r->owner. */

	/* Update the current process state */
	current->status = PROCESS_WAIT;

	/* And append current to waitqueue */
	list_add_tail(&current->list, &r->waitqueue);

	/**
	 * And return false to indicate the resource is not available.
	 * The scheduler framework will soon call schedule() function to
	 * schedule out current and to pick the next process to run.
	 */
	return false;
}
void prio_release(int resource_id)
{
	struct resource *r = resources + resource_id;

	assert(r->owner == current);

	r->owner = NULL;

	if (!list_empty(&r->waitqueue)) {
		struct process *waiter =
				get_largest_prio(&r->waitqueue);

		/**
		 * Ensure the waiter is in the wait status
		 */
		assert(waiter->status == PROCESS_WAIT);

		/**
		 * Take out the waiter from the waiting queue. Note we use
		 * list_del_init() over list_del() to maintain the list head tidy
		 * (otherwise, the framework will complain on the list head
		 * when the process exits).
		 */
		list_del_init(&waiter->list);

		/* Update the process status */
		waiter->status = PROCESS_READY;

		/**
		 * Put the waiter process into ready queue. The framework will
		 * do the rest.
		 */
		list_add_tail(&waiter->list, &readyqueue);
	}
}

bool pcp_acquire(int resource_id)
{
	struct resource *r = resources + resource_id;

	if (!r->owner) {
		/* This resource is not owned by any one. Take it! */
		r->owner = current;
		current->prio = MAX_PRIO;
		return true;
	}

	/* OK, this resource is taken by @r->owner. */

	/* Update the current process state */
	current->status = PROCESS_WAIT;

	/* And append current to waitqueue */
	list_add_tail(&current->list, &r->waitqueue);

	/**
	 * And return false to indicate the resource is not available.
	 * The scheduler framework will soon call schedule() function to
	 * schedule out current and to pick the next process to run.
	 */
	return false;
}
void pcp_release(int resource_id)
{
	struct resource *r = resources + resource_id;

	assert(r->owner == current);

	current->prio = current->prio_orig;
	r->owner = NULL;

	if (!list_empty(&r->waitqueue)) {
		struct process *waiter =
				get_largest_prio(&r->waitqueue);

		/**
		 * Ensure the waiter is in the wait status
		 */
		assert(waiter->status == PROCESS_WAIT);

		/**
		 * Take out the waiter from the waiting queue. Note we use
		 * list_del_init() over list_del() to maintain the list head tidy
		 * (otherwise, the framework will complain on the list head
		 * when the process exits).
		 */
		list_del_init(&waiter->list);

		/* Update the process status */
		waiter->status = PROCESS_READY;

		/**
		 * Put the waiter process into ready queue. The framework will
		 * do the rest.
		 */
		list_add_tail(&waiter->list, &readyqueue);
	}
}
bool pip_acquire(int resource_id)
{
	struct resource *r = resources + resource_id;

	if (!r->owner) {
		/* This resource is not owned by any one. Take it! */
		r->owner = current;
		return true;
	}

	/* OK, this resource is taken by @r->owner. */

	/* Update the current process state */
	current->status = PROCESS_WAIT;
	r->owner->prio = current->prio_orig;
	/* And append current to waitqueue */
	list_add_tail(&current->list, &r->waitqueue);

	/**
	 * And return false to indicate the resource is not available.
	 * The scheduler framework will soon call schedule() function to
	 * schedule out current and to pick the next process to run.
	 */
	return false;
}
void pip_release(int resource_id)
{
	struct resource *r = resources + resource_id;

	assert(r->owner == current);

	current->prio = current->prio_orig;
	r->owner = NULL;

	if (!list_empty(&r->waitqueue)) {
		struct process *waiter =
				get_largest_prio(&r->waitqueue);

		/**
		 * Ensure the waiter is in the wait status
		 */
		assert(waiter->status == PROCESS_WAIT);

		/**
		 * Take out the waiter from the waiting queue. Note we use
		 * list_del_init() over list_del() to maintain the list head tidy
		 * (otherwise, the framework will complain on the list head
		 * when the process exits).
		 */
		list_del_init(&waiter->list);

		/* Update the process status */
		waiter->status = PROCESS_READY;

		/**
		 * Put the waiter process into ready queue. The framework will
		 * do the rest.
		 */
		list_add_tail(&waiter->list, &readyqueue);
	}
}
#include "sched.h"

/***********************************************************************
 * FIFO scheduler
 ***********************************************************************/
static int fifo_initialize(void)
{
	return 0;
}

static void fifo_finalize(void)
{
}

static struct process *fifo_schedule(void)
{
	struct process *next = NULL;

	/* You may inspect the situation by calling dump_status() at any time */
	// dump_status();

	/**
	 * When there was no process to run in the previous tick (so does
	 * in the very beginning of the simulation), there will be
	 * no @current process. In this case, pick the next without examining
	 * the current process. Also, when the current process is blocked
	 * while acquiring a resource, @current is (supposed to be) attached
	 * to the waitqueue of the corresponding resource. In this case just
	 * pick the next as well.
	 */
	if (!current || current->status == PROCESS_WAIT) {
		goto pick_next;
	}

	/* The current process has remaining lifetime. Schedule it again */
	if (current->age < current->lifespan) {
		return current;
	}

pick_next:
	/* Let's pick a new process to run next */

	if (!list_empty(&readyqueue)) {
		/**
		 * If the ready queue is not empty, pick the first process
		 * in the ready queue
		 */
		next = list_first_entry(&readyqueue, struct process, list);

		/**
		 * Detach the process from the ready queue. Note we use list_del_init()
		 * instead of list_del() to maintain the list head tidy. Otherwise,
		 * the framework will complain (assert) on process exit.
		 */
		list_del_init(&next->list);
	}

	/* Return the process to run next */
	return next;
}

struct scheduler fifo_scheduler = {
	.name = "FIFO",
	.acquire = fcfs_acquire,
	.release = fcfs_release,
	.initialize = fifo_initialize,
	.finalize = fifo_finalize,
	.schedule = fifo_schedule,
};


/***********************************************************************
 * SJF scheduler
 ***********************************************************************/
static int sjf_initialize(void)
{
	return 0;
}
static void sjf_finalize(void)
{

}



static struct process *sjf_schedule(void)
{
	struct process *next = NULL;
	/**
	 * Implement your own SJF scheduler here.
	 */

	if(!current || current->status == PROCESS_WAIT){
		goto pick_next;
	}

	if (current->age < current->lifespan) {
		return current;
	}

pick_next:

	if(!list_empty(&readyqueue))
	{
		struct process *pos;
		unsigned int min_lifespan=0;

		list_for_each_entry(pos, &readyqueue, list){
			if(min_lifespan == 0){
				min_lifespan = pos->lifespan;
				next = pos;
				continue;
			}
			if(pos->lifespan < min_lifespan){
				min_lifespan = pos->lifespan;
				next = pos;
			}
		}
		list_del_init(&next->list);
	}

	return next;
}

struct scheduler sjf_scheduler = {
	.name = "Shortest-Job First",
	.acquire = fcfs_acquire, /* Use the default FCFS acquire() */
	.release = fcfs_release, /* Use the default FCFS release() */
	.schedule = sjf_schedule,/* TODO: Assign sjf_schedule()
								to this function pointer to activate
								SJF in the simulation system */
	.initialize = sjf_initialize,
	.finalize = sjf_finalize,
};


/***********************************************************************
 * SRTF scheduler
 ***********************************************************************/
static struct process *srtf_schedule(void){

	struct process *next = NULL;

	if(current && current->lifespan>current->age && current->status != PROCESS_WAIT){
		list_add_tail(&current->list, &readyqueue);
	}

	if(!list_empty(&readyqueue))
	{
		struct process *pos;
		unsigned int min_remain = 0;

		list_for_each_entry(pos, &readyqueue, list){
			if(min_remain == 0){
				min_remain = pos->lifespan - pos->age;
				next = pos;
				continue;
			}
			if(pos->lifespan-pos->age < min_remain){
				min_remain = pos->lifespan-pos->age;
				next = pos;
			}
		}
		list_del_init(&next->list);
	}

	return next;
	
}
static void srtf_exiting(struct process *a){
	
}
struct scheduler srtf_scheduler = {
	.name = "Shortest Remaining Time First",
	.acquire = fcfs_acquire, /* Use the default FCFS acquire() */
	.release = fcfs_release, /* Use the default FCFS release() */
	.exiting = srtf_exiting,
	.schedule = srtf_schedule,

	/* Obviously, you should implement srtf_schedule() and attach it here */
};


/***********************************************************************
 * Round-robin scheduler
 ***********************************************************************/
static struct process *rr_schedule(void)
{
	struct process *next = NULL;

	if(current && current->lifespan>current->age && current->status != PROCESS_WAIT){
		list_add_tail(&current->list, &readyqueue);
	}

	struct process *pos;
	if(!list_empty(&readyqueue)){

		pos = list_first_entry(&readyqueue, struct process, list);
		next = pos;
		list_del_init(&next->list);
		
	}

	return next;
}
struct scheduler rr_scheduler = {
	.name = "Round-Robin",
	.acquire = fcfs_acquire, /* Use the default FCFS acquire() */
	.release = fcfs_release, /* Use the default FCFS release() */
	/* Obviously, you should implement rr_schedule() and attach it here */
	.schedule = rr_schedule,
};


/***********************************************************************
 * Priority scheduler
 ***********************************************************************/
static struct process *prio_schedule(void)
{	

	struct process *next = NULL;

	if(current && current->lifespan>current->age && current->status != PROCESS_WAIT){
		list_add_tail(&current->list, &readyqueue);
	}

	struct process *pos;
	if(!list_empty(&readyqueue)){

		pos = get_largest_prio(&readyqueue);
		next = pos;
		list_del_init(&next->list);
		
	}

	return next;
}
struct scheduler prio_scheduler = {
	.name = "Priority",
	.acquire = prio_acquire,
	.release = prio_release,
	.schedule = prio_schedule,

	/**
	 * Implement your own acquire/release function to make the priority
	 * scheduler correct.
	 */
	/* Implement your own prio_schedule() and attach it here */
};


/***********************************************************************
 * Priority scheduler with aging
 ***********************************************************************/
static struct process* pa_schedule(void){

	struct process *next = NULL;

	if(current && current->lifespan>current->age && current->status != PROCESS_WAIT){
		list_add_tail(&current->list, &readyqueue);
	}

	struct process *pos;
	if(!list_empty(&readyqueue)){

		pos = get_largest_prio(&readyqueue);
		next = pos;
		next->prio = next->prio_orig;
		list_del_init(&next->list);
		
	}
	struct process *remain;
	if(!list_empty(&readyqueue)){
		list_for_each_entry(remain, &readyqueue, list){
			remain->prio += 1;
		}
	}
	
	return next;

}
struct scheduler pa_scheduler = {
	.name = "Priority + aging",
	.acquire = prio_acquire,
	.release = prio_release,
	.schedule = pa_schedule,
	/**
	 * Ditto
	 */
};


/***********************************************************************
 * Priority scheduler with priority ceiling protocol
 ***********************************************************************/
struct scheduler pcp_scheduler = {
	.name = "Priority + PCP Protocol",
	.schedule = prio_schedule,
	.acquire = pcp_acquire,
	.release = pcp_release,

	/**
	 * Ditto
	 */
};


/***********************************************************************
 * Priority scheduler with priority inheritance protocol
 ***********************************************************************/
struct scheduler pip_scheduler = {
	.name = "Priority + PIP Protocol",
	.schedule = prio_schedule,
	.acquire = pip_acquire,
	.release = pip_release,
	/**
	 * Ditto
	 */
};
