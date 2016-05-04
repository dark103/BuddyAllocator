#ifndef BUDDY_H
#define BUDDY_H

typedef struct Node Node;
typedef struct page_t page_t;

void buddy_init();
void *buddy_alloc(int size);
void buddy_free(void *addr);
void buddy_dump();

void fillCount();
void countNode(Node* node);
Node* find_order(int order);
Node* find_order_recursive(Node* current, int order);
Node* find_page(int page);
Node* find_page_recursive(Node* current, int page);
Node* init_node(Node* parent, int page);

#endif // BUDDY_H
