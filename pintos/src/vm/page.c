#include "threads/thread.h"
#include "vm/frame.h"
#include "userprog/pagedir.h"
#include "filesys/file.h"

static unsigned vm_table_hash_func(const struct hash_elem *, void * UNUSED);
static bool vm_table_less_func(const struct hash_elem *, const struct hash_elem *, void * UNUSED);
static void vm_table_destroy_func(struct hash_elem *, void * UNUSED);

void vm_table_init(struct hash *h) {
	ASSERT (h != NULL);
	hash_init(h, vm_table_hash_func, vm_table_less_func, NULL);
}

void vm_table_destroy(struct hash *h) {
	ASSERT (h != NULL);
	hash_destroy (h , vm_table_destroy_func);
}

static unsigned vm_table_hash_func(const struct hash_elem *e, void *aux UNUSED) {
	ASSERT (e != NULL);
	struct page_entry *pe = hash_entry(e,struct page_entry, elem);
	return hash_int(pe->vaddr);
}

static bool vm_table_less_func(const struct hase_elem *a, const struct hash_elem *b, void *aux UNUSED) {
	ASSERT (a != NULL);
	ASSERT (a != NULL);
	struct page_entry *pa = hash_entry(a, struct page_entry, elem);
	struct page_entry *pb = hash_entry(b, struct page_entry, elem);
	return a->vaddr < b->vaddr;
}

void free_page_virtual(void *vaddr) {

}

static void vm_table_destroy_func(struct hash_elem *e, void *aux UNUSED) {
	ASSERT (e !- NULL);
	struct page_entry *pe = hash_entry(e, struct page_entry, elem);
	// TODO : need to make free page function with virtual address
	free_page_virtual(pe->vaddr);
	// TODO : need to make managing swap table function
	
	free(pe);
}
