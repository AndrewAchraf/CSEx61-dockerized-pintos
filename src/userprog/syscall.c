#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
/***************************************/
#include "devices/shutdown.h" // halt
#include "threads/init.h"  // halt
#include "threads/synch.h"  //locks

struct lock file_lock;
static void syscall_handler (struct intr_frame *);

/* ######### Added  ##############*/
void sys_halt();

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  //initialize lock required for file synchronization
    lock_init(&file_lock);
}

static void
syscall_handler (struct intr_frame *f UNUSED)
{
    // switch to system calls
    switch (*(int *)f->esp)
    {
        case SYS_HALT:
        {
            sys_halt();
            break;
        }
        case SYS_EXIT:
        {
            //handle exit system call

            break;
        }
        case SYS_EXEC:
        {
            //handle exec system call

            break;
        }
        case SYS_WAIT:
        {
            //handle wait system call

            break;
        }
        case SYS_CREATE:
        {
            //handle create system call

            break;
        }
        case SYS_REMOVE:
        {
            //handle remove system call

             break;
        }
        case SYS_OPEN:
        {
            //handle open system call

            break;
        }
        case SYS_FILESIZE:
        {
            //handle filesize system call

            break;
        }
        case SYS_READ:
        {
            //handle read system call

            break;
        }
        case SYS_WRITE:
        {
            //handle write system call

            break;
        }
        case SYS_SEEK:
        {
            //handle seek system call

            break;
        }
        case SYS_TELL:
        {
            //handle tell system call

            break;
        }
        case SYS_CLOSE:
        {
            //handle close system call
            
            break;
        }
        default:
        {
            printf ("system call!\n");
        }
    }

  thread_exit ();
}

void sys_halt(){
    shutdown_power_off();
}
