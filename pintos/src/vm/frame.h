#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "page.h"

// page frame header
// use list to manage frame_list
// replecement strategy : second chance

void frame_init(void) ;
void insert_page_to_frame(struct *page);
struct page *find_page(void *vaddr);				// returns page 
void delelte_from_frame(void);

struct page *find_victim(void) ;

#endif
