#ifndef BUDDY_H
#define BUDDY_H

typedef struct Node Node;
typedef struct page_t page_t;

void buddy_init();
void *buddy_alloc(int size);
void buddy_free(void *addr);
void buddy_dump();

Node* find_order(int order);
Node* find_page(page_t page);

#endif // BUDDY_H
