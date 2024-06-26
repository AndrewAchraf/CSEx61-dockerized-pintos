/* This file is derived from source code for the Nachos
   instructional operating system.  The Nachos copyright notice
   is reproduced in full below. */

/* Copyright (c) 1992-1996 The Regents of the University of California.
   All rights reserved.

   Permission to use, copy, modify, and distribute this software
   and its documentation for any purpose, without fee, and
   without written agreement is hereby granted, provided that the
   above copyright notice and the following two paragraphs appear
   in all copies of this software.

   IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO
   ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
   CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE
   AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA
   HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
   BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
   PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
   MODIFICATIONS.
*/

#include "threads/synch.h"
#include <stdio.h>
#include <string.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

/* Initializes semaphore SEMA to VALUE.  A semaphore is a
   nonnegative integer along with two atomic operators for
   manipulating it:

   - down or "P": wait for the value to become positive, then
     decrement it.

   - up or "V": increment the value (and wake up one waiting
     thread, if any). */
void
sema_init (struct semaphore *sema, unsigned value) 
{
  ASSERT (sema != NULL);

  sema->value = value;
  list_init (&sema->waiters);
}

/* Down or "P" operation on a semaphore.  Waits for SEMA's value
   to become positive and then atomically decrements it.

   This function may sleep, so it must not be called within an
   interrupt handler.  This function may be called with
   interrupts disabled, but if it sleeps then the next scheduled
   thread will probably turn interrupts back on. */
void
sema_down (struct semaphore *sema) 
{
  enum intr_level old_level;

  ASSERT (sema != NULL);
  ASSERT (!intr_context ());

  old_level = intr_disable ();
  while (sema->value == 0) 
    {
      //list_push_back (&sema->waiters, &thread_current ()->elem);
      list_insert_ordered(&(sema->waiters), &thread_current()->elem, compare_Priority, NULL);
      list_sort(&sema->waiters, compare_Priority, NULL);
      if(!thread_mlfqs){
          broadcastChangeInPriority(thread_current());
      }
      thread_block ();
    }
  sema->value--;
  intr_set_level (old_level);
}

/* Down or "P" operation on a semaphore, but only if the
   semaphore is not already 0.  Returns true if the semaphore is
   decremented, false otherwise.

   This function may be called from an interrupt handler. */
bool
sema_try_down (struct semaphore *sema) 
{
  enum intr_level old_level;
  bool success;

  ASSERT (sema != NULL);

  old_level = intr_disable ();
  if (sema->value > 0) 
    {
      sema->value--;
      success = true; 
    }
  else
    success = false;
  intr_set_level (old_level);

  return success;
}

/* Up or "V" operation on a semaphore.  Increments SEMA's value
   and wakes up one thread of those waiting for SEMA, if any.

   This function may be called from an interrupt handler. */
void
sema_up (struct semaphore *sema) 
{
  enum intr_level old_level;

  ASSERT (sema != NULL);

  old_level = intr_disable ();
  if (!list_empty (&sema->waiters)){
      list_sort(&sema->waiters, compare_Priority, NULL);
      struct thread *t = list_entry(list_pop_front(&sema->waiters), struct thread, elem);
      thread_unblock(t);
      if(t->priority > thread_current()->priority){
          sema->value++;
          intr_set_level (old_level);
          thread_yield();
          return;

      }
  }
  sema->value++;
  intr_set_level (old_level);
}

static void sema_test_helper (void *sema_);

/* Self-test for semaphores that makes control "ping-pong"
   between a pair of threads.  Insert calls to printf() to see
   what's going on. */
void
sema_self_test (void) 
{
  struct semaphore sema[2];
  int i;

  printf ("Testing semaphores...");
  sema_init (&sema[0], 0);
  sema_init (&sema[1], 0);
  thread_create ("sema-test", PRI_DEFAULT, sema_test_helper, &sema);
  for (i = 0; i < 10; i++) 
    {
      sema_up (&sema[0]);
      sema_down (&sema[1]);
    }
  printf ("done.\n");
}

/* Thread function used by sema_self_test(). */
static void
sema_test_helper (void *sema_) 
{
  struct semaphore *sema = sema_;
  int i;

  for (i = 0; i < 10; i++) 
    {
      sema_down (&sema[0]);
      sema_up (&sema[1]);
    }
}

/* Initializes LOCK.  A lock can be held by at most a single
   thread at any given time.  Our locks are not "recursive", that
   is, it is an error for the thread currently holding a lock to
   try to acquire that lock.

   A lock is a specialization of a semaphore with an initial
   value of 1.  The difference between a lock and such a
   semaphore is twofold.  First, a semaphore can have a value
   greater than 1, but a lock can only be owned by a single
   thread at a time.  Second, a semaphore does not have an owner,
   meaning that one thread can "down" the semaphore and then
   another one "up" it, but with a lock the same thread must both
   acquire and release it.  When these restrictions prove
   onerous, it's a good sign that a semaphore should be used,
   instead of a lock. */
void
lock_init (struct lock *lock)
{
  ASSERT (lock != NULL);

  lock->holder = NULL;
  sema_init (&lock->semaphore, 1);
}

/* Acquires LOCK, sleeping until it becomes available if
   necessary.  The lock must not already be held by the current
   thread.

   This function may sleep, so it must not be called within an
   interrupt handler.  This function may be called with
   interrupts disabled, but interrupts will be turned back on if
   we need to sleep. */
void
lock_acquire (struct lock *lock)
{
    ASSERT (lock != NULL);
    ASSERT (!intr_context ());
    ASSERT (!lock_held_by_current_thread (lock));

    struct thread *current_thread = thread_current();
    /*make it waiting bec we are not sure it will take it or not*/
    current_thread->lock_waiting = lock;
    sema_down (&lock->semaphore);
    lock->holder = thread_current ();
    //see if any one else in the list of threads waiting for the same lock has higher priority
    if(!thread_mlfqs){
        /*-------  Priority Schedule and Donation    --------*/

        /* the thread no longer waits for the lock.
         * it acquired the lock */
        current_thread -> lock_waiting = NULL;
        if(!list_empty(&lock->semaphore.waiters)){
            /* if there is other threads waiting for the same lock check priority
             * make the max priority with the priority of thread with max priority in waiters*/
            lock->maxPriority = list_entry(list_front(&lock->semaphore.waiters), struct thread, elem)->priority;
        }
        else{
            /*no one is waiting for the same lock and its aquired now by the current thread
             * so set max priority for lock to MIN_PRI which is actually 0.*/
            lock->maxPriority = PRI_MIN;
        }
        //add the lock to the list of acquired locks of the current thread in descending order according to the priority holder.
        list_insert_ordered(&lock->holder->locks_held, &lock->elem, cmp_locks_priority, NULL);
    }



}

/* Tries to acquires LOCK and returns true if successful or false
   on failure.  The lock must not already be held by the current
   thread.

   This function will not sleep, so it may be called within an
   interrupt handler. */
bool
lock_try_acquire (struct lock *lock)
{
    bool success;

    ASSERT (lock != NULL);
    ASSERT (!lock_held_by_current_thread (lock));

    success = sema_try_down (&lock->semaphore);
    if (success)
        lock->holder = thread_current ();
    return success;
}

/* Releases LOCK, which must be owned by the current thread.

   An interrupt handler cannot acquire a lock, so it does not
   make sense to try to release a lock within an interrupt
   handler. */

void
lock_release (struct lock *lock) 
{
    ASSERT (lock != NULL);
    ASSERT (lock_held_by_current_thread (lock));
    if(!thread_mlfqs){
        //remove the lock from the list of acquired locks of the current thread.
        list_remove(&lock->elem);
        //the thread holding the lock now left t so make threads know this and check priority
        broadcastChangeInPriority(lock->holder);

        lock->maxPriority = PRI_MIN;
    }
    /*now lock isn't held*/
    lock->holder = NULL;
    sema_up (&lock->semaphore);
}

/* Returns true if the current thread holds LOCK, false
   otherwise.  (Note that testing whether some other thread holds
   a lock would be racy.) */
bool
lock_held_by_current_thread (const struct lock *lock) 
{
  ASSERT (lock != NULL);

  return lock->holder == thread_current ();
}

/* One semaphore in a list. */
struct semaphore_elem 
  {
    struct list_elem elem;              /* List element. */
    struct semaphore semaphore;         /* This semaphore. */
  };

/* Initializes condition variable COND.  A condition variable
   allows one piece of code to signal a condition and cooperating
   code to receive the signal and act upon it. */
void
cond_init (struct condition *cond)
{
  ASSERT (cond != NULL);

  list_init (&cond->waiters);
}

/* Atomically releases LOCK and waits for COND to be signaled by
   some other piece of code.  After COND is signaled, LOCK is
   reacquired before returning.  LOCK must be held before calling
   this function.

   The monitor implemented by this function is "Mesa" style, not
   "Hoare" style, that is, sending and receiving a signal are not
   an atomic operation.  Thus, typically the caller must recheck
   the condition after the wait completes and, if necessary, wait
   again.

   A given condition variable is associated with only a single
   lock, but one lock may be associated with any number of
   condition variables.  That is, there is a one-to-many mapping
   from locks to condition variables.

   This function may sleep, so it must not be called within an
   interrupt handler.  This function may be called with
   interrupts disabled, but interrupts will be turned back on if
   we need to sleep. */
void
cond_wait (struct condition *cond, struct lock *lock) 
{
  struct semaphore_elem waiter;

  ASSERT (cond != NULL);
  ASSERT (lock != NULL);
  ASSERT (!intr_context ());
  ASSERT (lock_held_by_current_thread (lock));
  
  sema_init (&waiter.semaphore, 0);
  list_push_back (&cond->waiters, &waiter.elem);
//  list_insert_ordered(&cond->waiters, &waiter.elem, cmp_cond_priority, NULL);
  lock_release (lock);
  sema_down (&waiter.semaphore);
  lock_acquire (lock);
}

/* If any threads are waiting on COND (protected by LOCK), then
   this function signals one of them to wake up from its wait.
   LOCK must be held before calling this function.

   An interrupt handler cannot acquire a lock, so it does not
   make sense to try to signal a condition variable within an
   interrupt handler. */
void
cond_signal (struct condition *cond, struct lock *lock UNUSED) 
{
  ASSERT (cond != NULL);
  ASSERT (lock != NULL);
  ASSERT (!intr_context ());
  ASSERT (lock_held_by_current_thread (lock));

  if (!list_empty (&cond->waiters)) {
    /* compares the priority of the threads in the waiters list of the semaphore
     * bec the list of cond inserts semaphore elements which contains the list of waiters so in each element of list
     * in cond waiting list the element is a semaphore element so we need to get the first element in the semaphore list
     * which is definitely with higher priority than the rest of the elements in the semaphore list bec it is sorted */
      list_sort(&cond->waiters, cmp_cond_priority, NULL);
      sema_up (&list_entry (list_pop_front (&cond->waiters),
      struct semaphore_elem, elem)->semaphore);
  }

}

/* Wakes up all threads, if any, waiting on COND (protected by
   LOCK).  LOCK must be held before calling this function.

   An interrupt handler cannot acquire a lock, so it does not
   make sense to try to signal a condition variable within an
   interrupt handler. */
void
cond_broadcast (struct condition *cond, struct lock *lock) 
{
  ASSERT (cond != NULL);
  ASSERT (lock != NULL);

  while (!list_empty (&cond->waiters))
    cond_signal (cond, lock);
}

/*compares the priority of the threads in the waiters list of the semaphore
 * bec the list of cond inserts semaphore elements which contains the list of waiters so in each element of list
 * in cond waiting list the element is a semaphore element so we need to get the first element in the semaphore list
 * which is definitely with higher priority than the rest of the elements in the semaphore list bec it is sorted */

bool cmp_cond_priority(struct list_elem *first, struct list_elem *second, void *aux)
{
    struct semaphore_elem *fsem = list_entry (first, struct semaphore_elem, elem);
    struct semaphore_elem *ssem = list_entry (second, struct semaphore_elem, elem);

    return list_entry(list_front(&fsem->semaphore.waiters), struct thread, elem)->priority > list_entry (list_front(&ssem->semaphore.waiters), struct thread, elem)->priority;

}
/* sorts the list of locks aquired by the current thread for retrieval in descending order */
bool cmp_locks_priority(struct list_elem *first, struct list_elem *second, void *aux)
{
    struct lock *flock = list_entry (first, struct lock, elem);
    struct lock *slock = list_entry (second, struct lock, elem);

    return flock->maxPriority > slock->maxPriority;

}
/*
 * Called by a lock to notify its holder thread of a change in priority,
 * so the thread check the new priority and chooses the appropriate new priority.
 */
void broadcastChangeInPriority(struct  thread* t){
    if(!list_empty(&(t->locks_held))){
        /* if the thread holds another locks it sets its priority to the max bet its actual priority or
         * the max in the list of the held threads
         * */
        int maxPriorityInWaiters = (list_entry(list_front(&(t->locks_held)),
        struct lock, elem))->maxPriority;
        if (t->actual_priority > maxPriorityInWaiters)
            t->priority = t ->actual_priority;
        else
            t->priority = maxPriorityInWaiters;
    }
    else{
        //if not holding any thread so it rebase to its actual priority
        t->priority = t->actual_priority;
    }
    handleNestedDonation(t);
}
// Handles the nested donation procedure
void
handleNestedDonation(struct thread* t){
    if (t->lock_waiting == NULL)
        //thread not waiting for anything
        return;

    list_remove(&t->elem);
    list_insert_ordered(&t->lock_waiting->semaphore.waiters, &t->elem, compare_Priority, NULL);

    if (t->lock_waiting->maxPriority < t->priority)
        t->lock_waiting->maxPriority = t->priority;

    if (t->lock_waiting->holder != NULL) {
        int maxPriority = list_entry(list_front(&t->lock_waiting->semaphore.waiters), struct thread, elem)->priority;
        if (t->lock_waiting->holder->priority >= maxPriority)
            return;

        t->lock_waiting->holder->priority = maxPriority;
        handleNestedDonation(t->lock_waiting->holder);
    }
}

