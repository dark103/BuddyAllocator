/**
 * Buddy Allocator
 *
 * For the list library usage, see http://www.mcs.anl.gov/~kazutomo/list/
 */

/**************************************************************************
 * Conditional Compilation Options
 **************************************************************************/
#define USE_DEBUG 0

/**************************************************************************
 * Included Files
 **************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "buddy.h"
#include "list.h"

/**************************************************************************
 * Public Definitions
 **************************************************************************/
#define MIN_ORDER 12
#define MAX_ORDER 20

#define PAGE_SIZE (1<<MIN_ORDER)
/* page index to address */
#define PAGE_TO_ADDR(page_idx) (void *)((page_idx*PAGE_SIZE) + g_memory)

/* address to page index */
#define ADDR_TO_PAGE(addr) ((unsigned long)((void *)addr - (void *)g_memory) / PAGE_SIZE)

/* find buddy address */
#define BUDDY_ADDR(addr, o) (void *)((((unsigned long)addr - (unsigned long)g_memory) ^ (1<<o)) \
									 + (unsigned long)g_memory)

#if USE_DEBUG == 1
#  define PDEBUG(fmt, ...) \
	fprintf(stderr, "%s(), %s:%d: " fmt,			\
		__func__, __FILE__, __LINE__, ##__VA_ARGS__)
#  define IFDEBUG(x) x
#else
#  define PDEBUG(fmt, ...)
#  define IFDEBUG(x)
#endif

/**************************************************************************
 * Public Types
 **************************************************************************/
struct page_t {
	struct list_head list;
	int order;
};

struct Node {
  Node* left;
  Node* right;
  Node* parent;

  page_t* page;

	int order;
};

/**************************************************************************
 * Global Variables
 **************************************************************************/
/* free lists*/
struct list_head free_area[MAX_ORDER+1];

//root of tree
Node* root;

/* memory area */
char g_memory[1<<MAX_ORDER];

/* page structures */
page_t g_pages[(1<<MAX_ORDER)/PAGE_SIZE];

/**************************************************************************
 * Public Function Prototypes
 **************************************************************************/

/**************************************************************************
 * Local Functions
 **************************************************************************/

/**
 * Initialize the buddy system
 */
void buddy_init()
{
	int i;
	int n_pages = (1<<MAX_ORDER) / PAGE_SIZE;
	for (i = 0; i < n_pages; i++) {
		g_pages[i].order = MIN_ORDER;
		INIT_LIST_HEAD(&(g_pages[i].list));
	}

	/* initialize freelist */
	for (i = MIN_ORDER; i <= MAX_ORDER; i++) {
		INIT_LIST_HEAD(&free_area[i]);
	}

	/* add the entire memory as a freeblock */
	list_add(&g_pages[0].list, &free_area[MAX_ORDER]);

	root->parent = 0;
	root->left = 0;
	root->right = 0;
	root->order = MAX_ORDER;
	root->page = &g_pages[0];
}

/**
 * Allocate a memory block.
 *
 * On a memory request, the allocator returns the head of a free-list of the
 * matching size (i.e., smallest block that satisfies the request). If the
 * free-list of the matching block size is empty, then a larger block size will
 * be selected. The selected (large) block is then splitted into two smaller
 * blocks. Among the two blocks, left block will be used for allocation or be
 * further splitted while the right block will be added to the appropriate
 * free-list.
 *
 * @param size size in bytes
 * @return memory block address
 */
void *buddy_alloc(int size)
{
	if(size > (1<<MAX_ORDER))
	{
		printf("Size too big for memory space\n");
		return NULL;
	}

	Node* temp = malloc(sizeof(Node));
	root->left = temp;



	// int index = MIN_ORDER;
	// while(size > (1<<index))
	// {
	// 	index++;
	// }
	// struct list_head head = free_area[index];
	// if(!list_empty(&head))
	// {
	// 	page_t* page = list_entry(head.next, page_t, list);
	// 	page->order = index;
	// 	return PAGE_TO_ADDR((unsigned long) (g_pages - page));
	// }
	// else
	// {
	// 	int splits = 1;
	// 	index++;
	// 	while(index <= MAX_ORDER && list_empty(&(free_area[index])))
	// 	{
	// 		splits++;
	// 		index++;
	// 	}
	// 	head = free_area[index];
	// 	page_t* page =  list_entry(head.next, page_t, list);
	// 	while(splits > 0)
	// 	{
	// 		page_t buddy = g_pages[ADDR_TO_PAGE(BUDDY_ADDR(PAGE_TO_ADDR((unsigned long) (page - g_pages)), (index-1)))];
	// 		buddy.order = index - 1;
	// 		list_add(&buddy.list, &free_area[index-1]);
	// 		splits--;
	// 		index--;
	// 	}
	// 	page->order = index;
	// 	return PAGE_TO_ADDR((unsigned long) (g_pages - page));
	// }

	return NULL;
}

/**
 * Free an allocated memory block.
 *
 * Whenever a block is freed, the allocator checks its buddy. If the buddy is
 * free as well, then the two buddies are combined to form a bigger block. This
 * process continues until one of the buddies is not free.
 *
 * @param addr memory block address to be freed
 */
void buddy_free(void *addr)
{
	/* TODO: IMPLEMENT THIS FUNCTION */
	// page_t* page = &g_pages[ADDR_TO_PAGE(addr)];
	// int index = page->order;
	// page_t* buddy = &g_pages[ADDR_TO_PAGE(BUDDY_ADDR(PAGE_TO_ADDR((unsigned long) (page - g_pages)), (index)))];
	// struct list_head* head = list_for_each_entry(buddy, &(buddy->list), list);
}

/**
 * Print the buddy system status---order oriented
 *
 * print free pages in each order.
 */
void buddy_dump()
{
	int o;
	for (o = MIN_ORDER; o <= MAX_ORDER; o++) {
		struct list_head *pos;
		int cnt = 0;
		list_for_each(pos, &free_area[o]) {
			cnt++;
		}
		printf("%d:%dK ", cnt, (1<<o)/1024);
	}
	printf("\n");
}
