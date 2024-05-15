#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

// added headers
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "stdbool.h"
// end of added headers

void syscall_init(void);
// added methods

// system calls _____________________________________________________________________________

/* Terminates the current user program, returning status to the kernel. If the process’s parent
waits for it, this is the status that will be returned. Conventionally, a status of 0 indicates success
and nonzero values indicate errors*/
void call_exit(int status);
/* Waits for a child process pid and retrieves the child’s exit status.
If pid is still alive, waits until it terminates. Then, returns the status that pid passed to exit.
If pid did not call exit(), but was terminated by the kernel (e.g. killed due to an exception),
wait(pid) must return -1. It is perfectly legal for a parent process to wait for child processes that have already
terminated by the time the parent calls wait,
but the kernel must still allow the parent to retrieve its child’s exit status,
or learn that the child was terminated by the kernel. */
tid_t  call_wait(tid_t tid);
/* Creates a new file called file initially initial_size bytes in size. Returns true if successful,
false otherwise. Creating a new file does not open it: opening the new file is a separate
operation which would require a open system call. */
bool call_create(const char *file, unsigned initial_size);
/*Deletes the file called file. Returns true if successful, false otherwise. A file may be
removed regardless of whether it is open or closed, and removing an open file
does not close it.*/
bool call_remove(const char* file);
/* Opens the file called file. Returns a nonnegative integer handle called a "file descriptor"
(fd), or -1 if the file could not be opened. */
int call_open(const char* file);
/* Returns the file opened to help setting the file size. */
struct open_file* sys_file(int fd);
/* Reads size bytes from the file open as fd into buffer. Returns the number of bytes actually
read (0 at end of file), or -1 if the file could not be read (due to a condition other than end of
file). Fd 0 reads from the keyboard using input_getc(). */
int call_read(int fd, void* buffer, unsigned size);
/* Writes size bytes from buffer to the open file fd.
Returns the number of bytes actually written, which may be less than size if some bytes could not be written.
Fd 1 writes to the console. Your code to write to the console should write all of buffer in
one call to putbuf(), at least as long as size is not bigger than a few hundred bytes. (It is
reasonable to break up larger buffers.) Otherwise, lines of text output by different processes
may end up interleaved on the console, confusing both human readers and our grading scripts. */
int call_write(int fd, const void* buffer, unsigned size);
/* Changes the next byte to be read or written in open file fd to position, expressed in bytes
from the beginning of the file. (Thus, a position of 0 is the file’s start.) */
void call_seek(struct intr_frame *f);
/* Returns the position of the next byte to be read or written in open file fd,
expressed in bytes from the beginning of the file. */
void call_tell(struct intr_frame *f);
/* Closes file descriptor fd. Exiting or terminating a process implicitly closes all
its open file descriptors, as if by calling this function for each one */
int call_close(int fd);
// end of added methods  ___________________________________________________________________________


/* Terminates Pintos by calling shutdown_power_off()
This should be seldom used, because you lose some information about possible deadlock situations, etc. */
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
void handle_sys_tell(struct intr_frame *f);
void handle_sys_write(struct intr_frame *f);
void handle_sys_close(struct intr_frame *f);


#endif /* userprog/syscall.h */
