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

  int pageIndex;

	int order;
	int free;
};

/**************************************************************************
 * Global Variables
 **************************************************************************/
/* free lists*/
struct list_head free_area[MAX_ORDER+1];

int count[MAX_ORDER + 1];

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

	root = malloc(sizeof(Node));
	root->parent = 0;
	root->left = 0;
	root->right = 0;
	root->order = MAX_ORDER;
	root->free = 1;
	root->pageIndex = 0;
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

	int newOrder = MIN_ORDER;
	while(size > (1<<newOrder))
	{
		newOrder++;
	}
	// printf("Received request of order: %d\n", newOrder);

	int splits = 0;
	Node* temp = find_order(newOrder);
	while(temp == 0)
	{
		newOrder++;
		temp = find_order(newOrder);
		splits++;
	}
	if(splits == 0)
	{
		temp->free = 0;
		return PAGE_TO_ADDR(temp->pageIndex);
	}
	while(splits > 0)
	{
		// printf("Splitting node of order: %d\n", temp->order);
		Node* tempLeft = init_node(temp, temp->pageIndex);
		int pageRight = ADDR_TO_PAGE(BUDDY_ADDR(PAGE_TO_ADDR((unsigned long)temp->pageIndex), (temp->order-1)));
		Node* tempRight = init_node(temp, pageRight);
		temp->left = tempLeft;
		temp->right = tempRight;
		temp->free = 0;
		temp = temp->left;
		splits--;
	}
	temp->free = 0;
	return PAGE_TO_ADDR(temp->pageIndex);



	/*old code
	int index = MIN_ORDER;
	while(size > (1<<index))
	{
		index++;
	}
	struct list_head head = free_area[index];
	if(!list_empty(&head))
	{
		page_t* page = list_entry(head.next, page_t, list);
		page->order = index;
		return PAGE_TO_ADDR((unsigned long) (g_pages - page));
	}
	else
	{
		int splits = 1;
		index++;
		while(index <= MAX_ORDER && list_empty(&(free_area[index])))
		{
			splits++;
			index++;
		}
		head = free_area[index];
		page_t* page =  list_entry(head.next, page_t, list);
		while(splits > 0)
		{
			page_t buddy = g_pages[ADDR_TO_PAGE(BUDDY_ADDR(PAGE_TO_ADDR((unsigned long) (page - g_pages)), (index-1)))];
			buddy.order = index - 1;
			list_add(&buddy.list, &free_area[index-1]);
			splits--;
			index--;
		}
		page->order = index;
		return PAGE_TO_ADDR((unsigned long) (g_pages - page));
	}*/

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
	int page = ADDR_TO_PAGE(addr);
	Node* node = find_page(page);
	node->free = 1;
	Node* parent = node->parent;
	while(parent != 0 && (parent->left->free + parent->right->free == 2))
	{
		free(parent->left);
		free(parent->right);
		page = parent->pageIndex;
		node = parent;
		parent = node->parent;
		node->free = 1;
	}



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
	fillCount();
	int o;
	for (o = MIN_ORDER; o <= MAX_ORDER; o++) {
		/*struct list_head *pos;
		int cnt = 0;
		list_for_each(pos, &free_area[o]) {
			cnt++;
		}*/
		printf("%d:%dK  ", count[o], (1<<o)/1024);
	}
	printf("\n");
}

void fillCount()
{
	int i = 0;
	for(i = 0; i <= MAX_ORDER; i++)
	{
		count[i] = 0;
	}
	countNode(root);
}

void countNode(Node* node)
{
	if(node->left == 0 && node->right == 0)
	{
		if(node->free == 1)
			count[node->order]++;
		return;
	}

	if(node->left != 0)
		countNode(node->left);

	if(node->right != 0)
		countNode(node->right);
}

Node* find_order(int order)
{
	if(root->order == order && root->free == 1)
		return root;
	else
		return find_order_recursive(root, order);
}

Node* find_order_recursive(Node* current, int order)
{
	Node* left = current->left;
	if(left != 0)
	{
		if(left->order == order && left->free == 1)
		{
			return left;
		}
		Node* temp = find_order_recursive(left, order);
		if(temp != 0)
		{
			return temp;
		}
	}
	Node* right = current->right;
	if(right != 0)
	{
		if(right->order == order && right->free == 1)
		{
			return right;
		}
		Node* temp = find_order_recursive(right, order);
		if(temp != 0)
		{
			return temp;
		}
	}
	return 0;
}

Node* find_page(int page)
{
	if(root->pageIndex == page)
		return root;
	else
		return find_page_recursive(root, page);
}

Node* find_page_recursive(Node* current, int page)
{
	Node* left = current->left;
	if(left != 0)
	{
		if(left->pageIndex == page)
		{
			return left;
		}
		Node* temp = find_page_recursive(left, page);
		if(temp != 0)
		{
			return temp;
		}
	}
	Node* right = current->right;
	if(right != 0)
	{
		if(right->pageIndex == page)
		{
			return right;
		}
		Node* temp = find_page_recursive(right, page);
		if(temp != 0)
		{
			return temp;
		}
	}
	return 0;
}

Node* init_node(Node* parent, int page)
{
	Node* temp = malloc(sizeof(Node));
	temp->parent = parent;
	temp->left = 0;
	temp->right = 0;
	temp->order = parent->order - 1;
	temp->free = 1;
	temp->pageIndex = page;
	return temp;
}
