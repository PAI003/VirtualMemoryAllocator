#pragma once
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);				        \
		}							\
	} while (0)


typedef struct list_node_t list_node_t;
struct list_node_t
{
    void* data; 
    list_node_t *prev, *next;
};

typedef struct list_t list_t;
struct list_t
{
    list_node_t* head;
    unsigned int data_size;
    unsigned int l_size;
};  

typedef struct {
   uint64_t start_address; 
   size_t size; 
   list_t* miniblock_list;
} block_t;

typedef struct {
   uint64_t start_address;
   size_t size;
   uint8_t perm;
   void* rw_buffer;
} miniblock_t;

typedef struct {
	uint64_t arena_size;
	list_t *alloc_list;
} arena_t;

list_t* list_create(unsigned int data_size);
list_node_t* list_get_node(list_t* list, int n);
void add_nth_node(list_t* list, unsigned int n, const void* new_data);
void* remove_nth_node(list_t* list, unsigned int n);
void list_free(list_t **pp_list);

arena_t* alloc_arena(uint64_t size);
void dealloc_arena(arena_t* arena);

void alloc_block(arena_t* arena, uint64_t address, uint64_t size);
void free_block(arena_t* arena, uint64_t address);

void read(arena_t* arena, uint64_t address, uint64_t size);
void write(arena_t* arena, const uint64_t address,  const uint64_t size, int8_t *data);
void pmap(arena_t* arena);
void mprotect(arena_t* arena, uint64_t address, int8_t *permission);