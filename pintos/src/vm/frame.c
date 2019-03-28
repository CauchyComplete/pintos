#include "vm/frame.h"
#include "threads/synch.h"
#include "userprog/pagedir.h"
#include "threads/thread.h"
#include "threads/interrupt.h"
#include "threads/vaddr.h"

struct list page_frame;
struct lock frame_lock;
struct list_elem *frame_pos;

void frame_init() {
	list_init(&page_frame);
	lock_init(&frame_lock);
	frame_pos = NULL;
}

void insert_page_to_frame(struct page *pg) {
	lock_acquire(&frame_lock);
	list_push_back(&page_frame, &pg->frame_elem);
	lock_release(&frame_lock);
}

struct page* find_page(void *kaddr) {
	lock_acquire(&frame_lock);
	struct list_elem *e;
	for(e = list_begin(&page_frame);e != list_end(&page_frame);e = list_next(e)) {
		struct page *p = list_entry(e, struct page, frame_elem);
		if(p->kaddr == kaddr)
			return p;
	}
	lock_release(&frame_lock);
	return NULL;
}

void delete_from_frame(struct page* p) {
	lock_acquire(&frame_lock);
	if(frame_pos == p->frame_elem)
		frame_pos = list_remove(frame_pos);
	else
		list_remove(p->frame_elem);
	lock_release(&frame_lock);
}
