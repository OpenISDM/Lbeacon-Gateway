/*
  Copyright (c) 2016 Academia Sinica, Institute of Information Science

  License:

     GPL 3.0 : The content of this file is subject to the terms and cnditions
     defined in file 'COPYING.txt', which is part of this source code package.

  Project Name:

     BeDIS

  File Name:

     thpool.h

  File Description:

     This file the definitions and declarations of constants, structures, and
     functions used in the thpool.c file.

     Note: This code is forked from https://github.com/Pithikos/C-Thread-Pool
           Author: Johan Hanssen Seferidis
       	   License: MIT

  Version:

     2.0, 20190213

  Abstract:

     BeDIS uses LBeacons to deliver 3D coordinates and textual descriptions of
     their locations to users' devices. Basically, a LBeacon is an inexpensive,
     Bluetooth Smart Ready device. The 3D coordinates and location description
     of every LBeacon are retrieved from BeDIS (Building/environment Data and
     Information System) and stored locally during deployment and maintenance
     times. Once initialized, each LBeacon broadcasts its coordinates and
     location description to Bluetooth enabled user devices within its coverage
     area.

  Authors:

     Gary Xiao     , garyh0205@hotmail.com
     Holly Wang    , hollywang@iis.sinica.edu.tw

 */

#ifndef THPOOL_H
#define THPOOL_H

#define _GNU_SOURCE

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "Mempool.h"


/* The number of slots for the memory pool */
#define SLOTS_FOR_MEM_POOL 100

/* The size of a slot for the memory pool */
#define SIZE_FOR_MEM_POOL 100

#define err(str) fprintf(stderr, str)


/* ========================== STRUCTURES ============================ */


/* Binary semaphore */
typedef struct {
	pthread_mutex_t mutex;
	pthread_cond_t   cond;
	int v;
} bsem;


/* Job */
typedef struct job{
	struct job *prev;                    /* pointer to previous job   */
	void (*function)(void *arg);         /* function pointer          */
	void *arg;                           /* function's argument       */
	int priority;					     /* Priority of this job  	  */
} job;


/* Job queue */
typedef struct jobqueue{
	pthread_mutex_t rwmutex;             /* used for queue r/w access */
	job  *front;                         /* pointer to front of queue */
	job  *rear;                          /* pointer to rear  of queue */
	bsem *has_jobs;                      /* flag as binary semaphore  */
	int   len;                           /* number of jobs in queue   */
} jobqueue;


/* Thread */
typedef struct thread{
	int       id;                        /* friendly id               */
	pthread_t pthread;                   /* pointer to actual thread  */
	struct thpool_* thpool_p;            /* access to thpool          */
} thread;


/* Threadpool */
typedef struct thpool_{
	thread **threads;                    /* pointer to threads        */
	volatile int num_threads_alive;      /* threads currently alive   */
	volatile int num_threads_working;    /* threads currently working */
	pthread_mutex_t  thcount_lock;       /* used for thread count etc */
	pthread_cond_t  threads_all_idle;    /* signal to thpool_wait     */
	jobqueue  jobqueue;                  /* job queue                 */
	volatile int threads_keepalive;

	/* The memory pool for the allocation of all nodes in scanned_device_list
	   and tracked_object_list */
	Memory_Pool th_mempool;

} thpool_;

typedef thpool_ *Threadpool;

/* ========================== PROTOTYPES ============================ */


static int   thread_init(thpool_ *thpool_p, thread **thread_p, int id);
static void *thread_do(thread *thread_p);
static void  thread_destroy(thread *thread_p);

static int   jobqueue_init(Threadpool thpool_p, jobqueue *jobqueue_p);
static void  jobqueue_clear(Threadpool thpool_p, jobqueue *jobqueue_p);
static void  jobqueue_push(jobqueue *jobqueue_p, job *newjob_p);
static job  *jobqueue_pull(jobqueue *jobqueue_p);
static void  jobqueue_destroy(Threadpool thpool_p, jobqueue *jobqueue_p);

static void  bsem_init(bsem *bsem_p, int value);
static void  bsem_reset(bsem *bsem_p);
static void  bsem_post(bsem *bsem_p);
static void  bsem_post_all(bsem *bsem_p);
static void  bsem_wait(bsem *bsem_p);

/* ================================= API ==================================== */

/**
 * @brief  Initialize threadpool
 *
 * Initializes a threadpool. This function will not return untill all
 * threads have initialized successfully.
 *
 * @example
 *
 *    ..
 *    threadpool thpool;                    //First we declare a threadpool
 *    thpool = thpool_init(4);              //then we initialize it to 4 threads
 *    ..
 *
 * @param  num_threads   number of threads to be created in the threadpool
 * @return threadpool    created threadpool on success,
 *                       NULL on error
 */
Threadpool thpool_init(int num_threads);


/**
 * @brief Add work to the job queue
 *
 * Takes an action and its argument and adds it to the threadpool's job queue.
 * If you want to add to work a function with more than one arguments then
 * a way to implement this is by passing a pointer to a structure.
 *
 * NOTICE: You have to cast both the function and argument to not get warnings.
 *
 * @example
 *
 *    void print_num(int num){
 *       printf("%d\n", num);
 *    }
 *
 *    int main() {
 *       ..
 *       int a = 10;
 *       thpool_add_work(thpool, (void*)print_num, (void*)a);
 *       ..
 *    }
 *
 * @param  threadpool    threadpool to which the work will be added
 * @param  function_p    pointer to function to add as work
 * @param  arg_p         pointer to an argument
 * @param  priority      priority of this work
 * @return 0 on successs, -1 otherwise.
 */
int thpool_add_work(Threadpool threadpool, void (*function_p)(void *),
					void *arg_p, int priority);


/**
 * @brief Wait for all queued jobs to finish
 *
 * Will wait for all jobs - both queued and currently running to finish.
 * Once the queue is empty and all work has completed, the calling thread
 * (probably the main program) will continue.
 *
 * Smart polling is used in wait. The polling is initially 0 - meaning that
 * there is virtually no polling at all. If after 1 seconds the threads haven't
 * finished, the polling interval starts growing exponentially untill it reaches
 * max_secs seconds. Then it jumps down to a maximum polling interval assuming
 * that heavy processing is being used in the threadpool.
 *
 * @example
 *
 *    ..
 *    threadpool thpool = thpool_init(4);
 *    ..
 *    // Add a bunch of work
 *    ..
 *    thpool_wait(thpool);
 *    puts("All added work has finished");
 *    ..
 *
 * @param threadpool     the threadpool to wait for
 * @return nothing
 */
void thpool_wait(Threadpool thpool_p);


/**
 * @brief Destroy the threadpool
 *
 * This will wait for the currently active threads to finish and then 'kill'
 * the whole threadpool to free up memory.
 *
 * @example
 * int main() {
 *    threadpool thpool1 = thpool_init(2);
 *    threadpool thpool2 = thpool_init(2);
 *    ..
 *    thpool_destroy(thpool1);
 *    ..
 *    return 0;
 * }
 *
 * @param threadpool     the threadpool to destroy
 * @return nothing
 */
void thpool_destroy(Threadpool thpool_p);


/**
 * @brief Show currently working threads
 *
 * Working threads are the threads that are performing work (not idle).
 *
 * @example
 * int main() {
 *    threadpool thpool1 = thpool_init(2);
 *    threadpool thpool2 = thpool_init(2);
 *    ..
 *    printf("Working threads: %d\n", thpool_num_threads_working(thpool1));
 *    ..
 *    return 0;
 * }
 *
 * @param threadpool     the threadpool of interest
 * @return integer       number of threads working
 */
int thpool_num_threads_working(Threadpool thpool_p);


#endif
