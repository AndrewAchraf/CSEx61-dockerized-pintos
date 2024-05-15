#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
/***************************************/
#include "devices/shutdown.h" // halt
#include "threads/init.h"  // halt
#include "threads/synch.h"  //locks

#include "threads/vaddr.h" // exit syscall (use dto check validity of pointers)

static void syscall_handler (struct intr_frame *);

/* ######### Added  ##############*/
#define MAX_ARGS 3

struct lock file_lock;




/*  ######### Added  ############## */
/**
 * ensure that the pointers and stack pointers used in system calls are valid,
 * preventing the kernel from performing illegal memory accesses
 */
// Check validation of virtual memory :)
bool valid_in_virtual_memory(void *val);
// check validation of stack pointer
bool valid_esp(struct intr_frame *f);

void get_args (struct intr_frame *f, int *arg, int num_of_args);

void handle_sys_halt();
void handle_sys_exit(struct intr_frame *f);
void handle_sys_exec(struct intr_frame *f);
void handle_sys_wait(struct intr_frame *f);
void handle_sys_create(struct intr_frame *f);
void handle_sys_remove(struct intr_frame *f);
void handle_sys_open(struct intr_frame *f);
void handle_sys_filesize(struct intr_frame *f);
void handle_sys_read(struct intr_frame *f);
void handle_sys_seek(struct intr_frame *f);
void handle_sys_write(struct intr_frame *f);
void handle_sys_close(struct intr_frame *f);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");

  //initialize lock required for file synchronization
  lock_init(&file_lock);
}

static void
syscall_handler (struct intr_frame *f )
{
    // switch to system calls
    // check the system call number and call the appropriate function to handle it based on stack pointer esp

    int args[MAX_ARGS];
    switch (*(int *)f->esp)
    {
        case SYS_HALT:
        {
            //handle halt system call
            handle_sys_halt();
            break;
        }
        case SYS_EXIT:
        {
            //handle exit system call
            /** terminates the current user program returning status to the kernel
             */
            handle_sys_exit(f);
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
            handle_sys_create(f);
            break;
        }
        case SYS_REMOVE:
        {
            //handle remove system call
            handle_sys_remove(f);
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
/*
 * This function checks the validity of a given pointer with three major conditions:

Pointer is not NULL: It first checks if the pointer val is not null.
 A null pointer indicates that no valid data can be retrieved or stored, and dereferencing it would lead to a fault.

Pointer is in user address space: The is_user_vaddr(val) function checks if the pointer points to a user address space
 rather than kernel space. This is crucial because system calls should not inadvertently access or modify kernel memory,
 as doing so could compromise the kernel's integrity or security.

Pointer has a corresponding physical page: The pagedir_get_page(thread_current()->pagedir, val) != NULL part
 checks if the physical page corresponding to the provided virtual address (val) exists.
 In Pintos, the pagedir_get_page function looks up the page directory of the current thread to find
 if there is a physical page mapped to the given virtual address. If there's no such mapping, the pointer is considered invalid for dereferencing.
 */

bool valid_in_virtual_memory(void *ptr) {
    return ptr != NULL && is_user_vaddr(ptr) && pagedir_get_page(thread_current()->pagedir, ptr) != NULL;
}
// check validation of stack pointer
bool valid_esp(struct intr_frame *f)
{
    return valid_in_virtual_memory((int *)f->esp) || ((*(int *)f->esp) < 0) || (*(int *)f->esp) > 12;
}

void get_args (struct intr_frame *f, int *arg, int num_of_args)
{
    int *ptr;
    for (int i = 0; i < num_of_args; i++)
    {
        ptr = (int *)f->esp + i + 1;
        if (!valid_in_virtual_memory(ptr))
        {
            exit(-1);
        }
        arg[i] = *ptr;
    }
}
void handle_sys_halt(){
    shutdown_power_off();
}
void exit(int status){
    char *save_ptr;
    char *file_name = thread_current()->name;
    file_name = strtok_r(file_name, " ", &save_ptr);
    printf("%s: exit(%d)\n", file_name, status);
    thread_current()->exit_status = status;
    thread_exit();
}
/* it's job just passing the status to exit */
void handle_sys_exit(struct intr_frame *f){
    int status;
    // get the status from the stack and automatically the get args will check the validity of the pointer and exit if not valid
    get_args(f, &status, 1);
    f->eax = status;
    exit(status);
}

bool create_file(const char *file, unsigned initial_size)
{
    lock_acquire(&file_lock);
    bool success = filesys_create(file, initial_size);
    lock_release(&file_lock);
    return success;
}

/* it's job is checking validation in virtual memory if valid pass it to sys_create() */
void handle_sys_create(struct intr_frame *f)
{
    int arg[2];
    get_args(f, &arg, 2);
    char *file = (char *)(arg[0]);
    if (!valid_in_virtual_memory(file))
    {
        exit(-1);
    }
    unsigned initial_size = (unsigned int *)arg[1];
    f->eax = create_file(file, initial_size);
}

bool remove_file(const char *file)
{
    lock_acquire(&file_lock);
    bool success = filesys_remove(file);
    lock_release(&file_lock);
    return success;
}

void handle_sys_remove(struct intr_frame *f)
{
    int arg[1];
    get_args(f, &arg, 1);
    char *file = (char *)(arg[0]);
    if (!valid_in_virtual_memory(file))
    {
        exit(-1);
    }
    f->eax = remove_file(file);
}
