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
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "lib/kernel/list.h"

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
    if(!valid_esp(f))
    {
        call_exit(-1);
    }
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
            handle_sys_exec(f);
            break;
        }
        case SYS_WAIT:
        {
            //handle wait system call
            handle_sys_wait(f);
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
            handle_sys_open(f);
            break;
        }
        case SYS_FILESIZE:
        {
            //handle filesize system call
            handle_sys_filesize(f);
            break;
        }
        case SYS_READ:
        {
            //handle read system call
            handle_sys_read(f);
            break;
        }
        case SYS_WRITE:
        {
            //handle write system call
            handle_sys_write(f);
            break;
        }
        case SYS_SEEK:
        {
            //handle seek system call
            handle_sys_seek(f);
            break;
        }
        case SYS_TELL:
        {
            //handle tell system call
            handle_sys_tell(f);
            break;
        }
        case SYS_CLOSE:
        {
            //handle close system call
            handle_sys_close(f);
            break;
        }
        default:
        {
            printf ("system call!\n");
        }
    }

  //thread_exit ();
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
            call_exit(-1);
        }
        arg[i] = *ptr;
    }
}
void handle_sys_halt(){
    shutdown_power_off();
}
void call_exit(int status){
    char * name = thread_current()->name;
    char * save_ptr;
    char * executable = strtok_r (name, " ", &save_ptr);
    thread_current()->exit_status = status;
    printf("%s: exit(%d)\n",executable,status);
    thread_exit();
}
/* it's job just passing the status to exit */
void handle_sys_exit(struct intr_frame *f){
    int status ;
    get_args(f,&status,1);
    if (!is_user_vaddr(status))
    {
        // not user virtual address, then exit with failure
        f->eax = -1;
        call_exit(-1);
    }
    f->eax = status;
    call_exit(status);
}

void handle_sys_exec(struct intr_frame *f)
{
    char *file_name = (char *)(*((int *)f->esp + 1));
    f->eax = process_execute(file_name);
}

tid_t  call_wait(tid_t tid)
{
    return process_wait(tid);
}

void handle_sys_wait(struct intr_frame *f)
{
    if (!valid_in_virtual_memory((int *)f->esp + 1))
        call_exit(-1);
    tid_t tid = *((int *)f->esp + 1);
    f->eax = call_wait(tid);
}

int call_create(const char *file, unsigned initial_size)
{
    lock_acquire(&file_lock);
    int success = filesys_create(file, initial_size);
    lock_release(&file_lock);
    return success;
}

/* it's job is checking validation in virtual memory if valid pass it to sys_create() */
void handle_sys_create(struct intr_frame *f)
{
    char *file = (char *)*((int *)f->esp + 1);
    if (!valid_in_virtual_memory(file))
    {
        call_exit(-1);
    }
    unsigned initial_size = (unsigned)*((int *)f->esp + 2);
    f->eax = call_create(file, initial_size);
}

int call_remove(const char *file)
{
    lock_acquire(&file_lock);
    int success = filesys_remove(file);
    lock_release(&file_lock);
    return success;
}

void handle_sys_remove(struct intr_frame *f)
{
    char *file = (char *)(*((int *)f->esp + 1));
    if (!valid_in_virtual_memory(file))
    {
        call_exit(-1);
    }
    f->eax = call_remove(file);
}

int call_open(const char *file)
{
    static unsigned long current_fd = 2;// 0 and 1 are reserved for stdin and stdout
    lock_acquire(&file_lock);
    struct file *f = filesys_open(file);
    lock_release(&file_lock);
    if (f == NULL)
    {
        // if file not found || failed to open
        return -1;
    }
    else{
        struct open_file *new_file = (struct open_file *)malloc(sizeof(struct open_file));
        int file_fd = current_fd;
        new_file->fd = current_fd;
        new_file->file = f;
        lock_acquire(&file_lock);
        current_fd++;
        lock_release(&file_lock);
        list_push_back(&(thread_current()->list_of_open_file), &(new_file->elem));
        return file_fd;
    }


}
void handle_sys_open(struct intr_frame *f)
{
    char *file = (char *)(*((int *)f->esp + 1));
    if (!valid_in_virtual_memory(file))
    {
        call_exit(-1);
    }
    f->eax = call_open(file);
}

/* loop on all files opened by thread and return the one with fd required*/
struct open_file *sys_file(int fd)
{
    struct open_file *ans=NULL;
    struct list *list_of_files = &(thread_current()->list_of_open_file);
    for (struct list_elem *cur = list_begin(list_of_files); cur != list_end(list_of_files); cur = list_next(cur))
    {
        struct open_file *cur_file = list_entry(cur, struct open_file, elem);
        if ((cur_file->fd) == fd)
        {
            return cur_file;
        }
    }
    return NULL;
}

void handle_sys_filesize(struct intr_frame *f)
{
    int arg[1];
    get_args(f, &arg, 1);
    int fd = arg[0];
    struct open_file *file = sys_file(fd);
    if (file == NULL)
    {
        f->eax = -1;
        return;
    }
    lock_acquire(&file_lock);
    f->eax = file_length(file->file);
    lock_release(&file_lock);
}

int call_read(int fd, void *buffer, unsigned size)
{
    int res = size;
    if (fd == 0)
    { // stdin .. 0 at end of file
        while (size--)
        {
            lock_acquire(&file_lock);
            char ch = input_getc();
            lock_release(&file_lock);
            buffer += ch;
        }
        return res;
    }

    struct open_file *user_file = sys_file(fd);
    if (user_file == NULL)
    { // fail
        return -1;
    }
    else
    {
        struct file *file = user_file->file;
        lock_acquire(&file_lock);
        size = file_read(file, buffer, size);
        lock_release(&file_lock);
        return size;
    }
}

void handle_sys_read(struct intr_frame *f) {
    int fd = (int)(*((int *)f->esp + 1));
    char *buffer = (char *)(*((int *)f->esp + 2));
    if (fd == 1 || !valid_in_virtual_memory(buffer))
    { // fail if fd is 1 means (stdout) or in valid in virtual memory
        call_exit(-1);
    }
    unsigned size = *((unsigned *)f->esp + 3);
    f->eax = call_read(fd, buffer, size);
}

int call_write(int fd, const void *buffer, unsigned size)
{
    if (fd == 1)
    {
        // write to console
        lock_acquire(&file_lock);
        putbuf(buffer, size);
        lock_release(&file_lock);
        return size;
    }
    struct open_file *file = sys_file(fd);
    if (file == NULL)
    {
        return -1;
    }
    lock_acquire(&file_lock);
    int bytes_written = file_write(file->file, buffer, size);
    lock_release(&file_lock);
    return bytes_written;
}

void handle_sys_write(struct intr_frame *f)
{
    int fd = *((int *)f->esp + 1);
    char *buffer = (char *)(*((int *)f->esp + 2));
    if (fd == 0 || !valid_in_virtual_memory(buffer))
    { // fail, if fd is 0 (stdin), or its virtual memory
        call_exit(-1);
    }
    unsigned size = (unsigned)(*((int *)f->esp + 3));
    f->eax = call_write(fd, buffer, size);
}

void call_seek(struct intr_frame *f)
{
    int fd = (int)(*((int *)f->esp + 1));
    unsigned position = (unsigned)(*((int *)f->esp + 2));
    struct open_file *file = sys_file(fd);
    if (file == NULL)
    {
        f->eax = -1;
        return;
    }
    lock_acquire(&file_lock);
    file_seek(file->file, position);
    f->eax = position;
    lock_release(&file_lock);
}

void handle_sys_seek(struct intr_frame *f)
{
    call_seek(f);
}

void call_tell(struct intr_frame *f)
{
    int fd = (int)(*((int *)f->esp + 1));
    struct open_file *file = sys_file(fd);
    if (file == NULL)
    {
        f->eax = -1;
        return;
    }
    lock_acquire(&file_lock);
    f->eax = file_tell(file->file);
    lock_release(&file_lock);
}

void handle_sys_tell(struct intr_frame *f)
{
    call_tell(f);
}

int call_close(int fd)
{
    struct open_file *file = sys_file(fd);
    if (file == NULL)
    {
        return -1;
    }
    lock_acquire(&file_lock);
    file_close(file->file);
    lock_release(&file_lock);
    list_remove(&(file->elem));
    return 1;
}

void handle_sys_close(struct intr_frame *f)
{
    int fd = (int)(*((int *)f->esp + 1));
    if (fd < 2)
    { // fail, the fd is stdin or stdout
        call_exit(-1);
    }
    f->eax = call_close(fd);
}
