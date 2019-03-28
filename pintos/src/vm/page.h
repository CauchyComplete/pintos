#ifndef VM_PAGE_H
#define VM_PAGE_H
#include <hash.h>
#include <list.h>
#include <stdint.h>

extern struct lock frame_lock;

// structure used in page frame
struct page {
	void *kaddr;												// physical address (kernel address matches)
	struct page_entry *entry;									// virtual page entry 
	struct thread *t;
	struct list_elem frame_elem;
};

// structure used in virtual memory table per process
// this structure can be changed if it needs to be.
struct page_entry (
	void *vaddr;												// virtual address
	bool writable;												// is this page writable?
	bool pinned;												// is this page pinned? (if pinned, it does not be victims)
	bool loaded;												// is this page in memory?
	struct file* file;											// file opened now.
	size_t offset;
	size_t read_bytes;
	size_t zero_bytes;											// these three variables are basic variables from pintos system.
	size_t swap_pos;
	struct hash_elem elem;										// virtual memory table per process is hash table.
)

void vm_table_init(struct hash *);
void vm_table_destroy(struct hash *);

struct page_entry *find_entry (void *vaddr);
bool insert_page_entry(struct hash *,struct page_entry *);
bool delete_page_entry(struct hash *,struct page_entry *);

#endif
