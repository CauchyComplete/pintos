#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/thread.h"
#include "threads/interrupt.h"


#include <string.h>
#include "devices/shutdown.h"
#include "devices/input.h"
#include "lib/user/syscall.h"
#include "userprog/exception.h"
#include "userprog/process.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#define UADDR_BOTTOM ((void*)0x08048000)

void sys_halt(void);
int sys_read(int fd, void* buffer, size_t size);
int sys_write(int fd, const void* buffer, size_t size);
pid_t sys_exec(const char* cmd_line);
int sys_wait(pid_t pid);
int pibonacci(int n);
int sum_of_four_integers(int a,int b,int c,int d);
// Project 2-2
bool sys_create(const char *file, unsigned initial_size);
bool sys_remove(const char *file);
int sys_open(const char *file);
int sys_filesize(int fd);
void sys_seek(int fd, unsigned position);
unsigned sys_tell(int fd);
void sys_close(int fd);
static void syscall_handler (struct intr_frame *);

void 
get_args(struct intr_frame *f, void **args, int cnt);		/* get arguments the syscall need and store at args */

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/* f->esp has syscall number */

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
	void* args[4] = {NULL};
	// check if pointer is valid
	if(!address_validity(f->esp)) sys_exit(-1);
//	printf("SYSCALL_HANDLER %d\n",*(int*)f->esp);
	switch(*(int*)(f->esp)) {
		case SYS_HALT:
			{
				sys_halt();
				break;
			}
		case SYS_EXIT:
			{
				get_args(f,args,1);
				sys_exit(*(int*)args[0]);
				break;
			}
		case SYS_EXEC:
			{
				get_args(f,args,1);
				f->eax = sys_exec(*(const char**)args[0]);
				break;
			}
		case SYS_WAIT:
			{
				get_args(f,args,1);
				f->eax=sys_wait(*(pid_t*)args[0]);
				break;
			}
		case SYS_READ:
			{
				/* args[0] = fd , args[1] = buffer , args[2] = size */
//				hex_dump(0,f->esp,20,true);
				get_args(f,args,3);
				f->eax = sys_read(*(int*)args[0],*(void**)args[1],*(size_t*)args[2]);
				break;
			}
		case SYS_WRITE:
			{
				/* args[0] = fd , args[1] = buffer , args[2] = size */
				get_args(f,args,3);
				f->eax = sys_write(*(int*)args[0],*(const char**)args[1],*(size_t*)args[2]);
				break;
			}
		case SYS_PIB:
			{
				get_args(f,args,1);
				f->eax = pibonacci(*(int*)args[0]);
				break;
			}
		case SYS_SUM:
			{
				get_args(f,args,4);
				f->eax = sum_of_four_integers(*(int*)args[0],*(int*)args[1],*(int*)args[2],*(int*)args[3]);
				break;
			}
		case SYS_CREATE:
			{
				get_args(f,args,2);
				f->eax = sys_create(*(const char**)args[0],*(unsigned*)args[1]);
				break;
			}
		case SYS_REMOVE:
			{
				get_args(f,args,1);
				f->eax = sys_remove(*(const char**)args[0]);
				break;
			}
		case SYS_OPEN:
			{
				get_args(f,args,1);
				f->eax = sys_open(*(const char**)args[0]);
				break;
			}
		case SYS_FILESIZE:
			{
				get_args(f,args,1);
				f->eax = sys_filesize(*(int*)args[0]);
				break;
			}
		case SYS_SEEK:
			{
				get_args(f,args,2);
				sys_seek(*(int*)args[0],*(unsigned*)args[1]);
				break;
			}
		case SYS_TELL:
			{
				get_args(f,args,1);
				f->eax = sys_tell(*(int*)args[0]);
				break;
			}
		case SYS_CLOSE:
			{
				get_args(f,args,1);
				sys_close(*(int*)args[0]);
				break;
			}
		default:break;
	}
//    thread_exit ();
}

/* f->esp has syscall_num, +1, +2, +3, +4 has arguments' address */
void
get_args(struct intr_frame *f, void **args, int cnt) {
	int i;
	void *ptr = f->esp;
	for(i=0;i<cnt;i++) {
		ptr = ptr+4;
		args[i] = ptr;
		if(!address_validity(args[i]))
			sys_exit(-1);
	}
}

// shutdown pintos
void 
sys_halt(void) {
	shutdown_power_off();
}


// return status to the kernel or parent
// need to close file_list
void 
sys_exit(int status) {
    // written by Kwon Myung Joon
	struct thread *cur = thread_current();
	// need to yield while parent's cur_child is not cur_thread
	struct thread *parent = cur->parent;
	while((parent->cur_child != cur->tid) || (parent->status != THREAD_BLOCKED)) thread_yield();
	// delete cur thread's file_list
	struct list_elem *e;
	
	for(e=list_begin(&(cur->file_list));e!=list_end(&(cur->file_list));){
		struct my_file *tmp = list_entry(e,struct my_file,elem);
		e = list_next(e);
		if(!tmp) continue;
		file_close(tmp->file);
		list_remove(&(tmp->elem));
		free(tmp);
	}

    // delete this thread from parent->child_list
	parent->child_status = status;
    for(e=list_begin(&(cur->parent->child_list)); e != list_end(&(cur->parent->child_list)); )
   	{
		struct child_process *cp = list_entry(e,struct child_process, elem);
		e = list_next(e);
//		printf("%s's child : %s %d\n",parent->name,cp->child->name,cp->tid);		
        if( cp->tid == cur->tid ){
           	list_remove(&(cp->elem));
			free(cp);
       	    break;
   	    }
    }
//	printf("Sema Up in sys_exit\n");
	sema_up( &(cur->parent->sema) );			// added by JHS
	printf("%s: exit(%d)\n",cur->name,status);
	if(cur->cur_file){
		file_close(cur->cur_file);
	}
	thread_exit();
}

pid_t
sys_exec(const char *cmd_line) {
	/* process_execute -> thread_create -> kernel_thread -> start_process -> load */
	/* Funcs above need to be modified */
	// TO-DO : 
	//printf("SYS_EXEC %s\n",cmd_line);
	if(!address_validity(cmd_line))sys_exit(-1);
	return process_execute(cmd_line);
}

int
sys_wait(pid_t pid) {
	return process_wait(pid);
}
/* it works only if fd == 0(stdin) */
int 
sys_read(int fd, void *buffer, unsigned size) {
	int i;
	char input;
	if(!address_validity(buffer)) sys_exit(-1);
	if(fd == 0) {
		for(i=0;i<size;i++) {
			input = input_getc();
			memset((char*)buffer+i,input,1);
		}
		return i;
	}
	else if(fd == 1)
		return -1;
	else {
		struct my_file *tmp = searchFileList(&(thread_current()->file_list),fd);
		if(!tmp)
			return -1;
		return file_read(tmp->file,buffer,size);
	}
}
/* it works only if fd == 1(stdout) */
int 
sys_write(int fd, const void *buffer, unsigned size) {
	unsigned i;
	if(!address_validity(buffer)) sys_exit(-1);
	if(fd == 0)
		return -1;
	else if(fd == 1) {
		for(i=0;i<size;i++)
			if(*(char*)(buffer+i) == '\0') break;
		putbuf(buffer, i);
		return i;
	}
	else {
		struct my_file *tmp = searchFileList(&(thread_current()->file_list),fd);
		if(!tmp)
			return -1;
		return file_write(tmp->file, buffer, size);
	}
}

int
pibonacci(int n) {
	int prev = 0, now = 1, temp, i;

	for(i=1;i<n;i++) {
		temp = prev + now;
		prev = now;
		now = temp;
	}

	return now;
}

int
sum_of_four_integers(int a, int b, int c, int d) {
	return a+b+c+d;
}


/* Modified by Jeon Hae Seong - Start*/
/* create, remove, filesize, tell, seek have no need to modify.
 * There may be synch issues.
 */
bool sys_create(const char *file,unsigned initial_size) {
	if(!address_validity(file))
		sys_exit(-1);
	return filesys_create(file,initial_size);
}

bool sys_remove(const char *file) {
	if(!address_validity(file))
		sys_exit(-1);
	return filesys_remove(file);
}

int sys_open(const char *file) {
	if(!address_validity(file)) 
		sys_exit(-1);
	int cur = 2;
	struct file* fp = filesys_open(file);
	if(!fp)
		return -1;
	struct list_elem* e;
	struct my_file *new = (struct my_file*)malloc(sizeof(struct my_file));
	struct list *l = &(thread_current()->file_list);
	// get empty fd, 0 and 1 are reserved
	// traverse file_list, then get empty fd
	for(e=list_begin(l);e!=list_end(l);e=list_next(e)){
		struct my_file *tmp = list_entry(e,struct my_file,elem);
		if(tmp->fd == cur) {
			cur++;continue;
		}
		break;
	}
//	printf("New FD : %d\n",cur);
	new->fd = cur;
	new->file = fp;
	list_push_back(l,&(new->elem));
	return cur;
}
// seek, tell want struct file pointer, So need to find file related to fd
void sys_seek(int fd, unsigned pos) {
	struct my_file *tmp = searchFileList(&(thread_current()->file_list),fd);
	if(!tmp)
		return ;
	file_seek(tmp->file,pos);
//	file_seek(fd,pos);
}

unsigned sys_tell(int fd) {
	struct my_file *tmp = searchFileList(&(thread_current()->file_list),fd);
	if(!tmp)
		return -1;
	return file_tell(tmp->file);
//	return file_tell(fd);
}

int sys_filesize(int fd) {
	struct my_file *tmp = searchFileList(&(thread_current()->file_list),fd);
	if(!tmp)
		return -1;
	return file_length(tmp->file);
}

void sys_close(int fd) {
	struct my_file *tmp = searchFileList(&(thread_current()->file_list),fd);
	if(!tmp)
		return ;
	file_close(tmp->file);
	list_remove(&(tmp->elem));
	free(tmp);
}
