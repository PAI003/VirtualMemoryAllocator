#include "vma.h"

list_t* list_create(unsigned int data_size)
{
	list_t *list = malloc(sizeof(list_t));
	DIE(list == NULL, "could not allocate memory\n");

	list->data_size = data_size;
	list->l_size = 0;
	list->head = NULL;

	return list;
}

list_node_t* list_get_node(list_t* list, int n)
{
    if(list == NULL)
        printf("no list found\n");

    list_node_t *node;
    node = list->head;

    n = n % list->l_size;

    for(int i = 0; i < n; i++)
        node = node->next;

    return node;
}

void add_nth_node(list_t* list, unsigned int n, const void* new_data)
{
    if(list == NULL)
        printf("no list found\n");

    if(n >= list->l_size)
        n = list->l_size;

    list_node_t *new;
    new = malloc(sizeof(list_node_t));
    DIE(new == NULL, "could not allocate memory\n");

    new->data = malloc(list->data_size);
    DIE(new->data == NULL, "could not allocate memory\n");
    memcpy(new->data, new_data, list->data_size);
    new->next = NULL;

    list_node_t *node;

    if(list->l_size == 0) {
        list->head = new;
        new->prev = NULL;
        new->next = NULL;
        list->l_size++;

        return;
    }

    if(n == 0) {
        node = list->head;
        new->next = node;
        new->prev = NULL;
        node->prev = new;
        list->head = new;
        list->l_size++;

        return;
    }

    node = list->head;

    for(unsigned int i = 0; i < n - 1; i++)
        node = node->next;

    new->prev = node;
    new->next = node->next;
    node->next->prev = new;
    node->next = new;
    list->l_size++;

    return;
    
}

void* remove_nth_node(list_t* list, unsigned int n)
{
    if(list == NULL || list->l_size == 0)
        printf("no list found\n");

    if(n >= list->l_size - 1)
        n = list->l_size;

    list_node_t *node, *removed_node;

    if(list->l_size == 0) {
        list->head = NULL;
    }

    else if(n == 0) {
            removed_node = list->head;
            node = list->head;
            node->next->prev = NULL;
            list->head = node->next;
            list->l_size--;
            free(removed_node->data);
            free(removed_node);

        
        }

    else if(n < list->l_size) {
            node = list->head;

            for(unsigned int i = 0; i < n - 1; i++)
                node = node->next;

            removed_node = node->next;
            node->next = node->next->next;
            node->next->prev = node;
            list->l_size--;
            free(removed_node->data);
            free(removed_node);
        }

    else {
        node = list->head;

        for(unsigned int i = 0; i < n - 1; i++)
            node = node->next;
    
        removed_node = node;
        node->next = NULL;
        list->l_size--;
        free(removed_node->data);
        free(removed_node);
    }
}

void list_free(list_t **pp_list)
{
    if(pp_list == NULL)
        return;

    list_node_t *node;

    while(((*pp_list)->l_size) != 0) {
        remove_nth_node(*pp_list, 0);

        free(node->data);
        free(node);
    }

    free(*pp_list);
    *pp_list = NULL;
}

void alloc_miniblock(miniblock_t *miniblock, uint64_t address, uint64_t size) {
    
   // miniblock = malloc(sizeof(miniblock_t));
    //DIE(miniblock == NULL, "could not allocate memory\n");
    miniblock->start_address = address;
    miniblock->size = size;
}

arena_t *alloc_arena(uint64_t size)
{
    arena_t *arena = malloc(sizeof(arena_t));
    DIE(arena == NULL, "could not allocate memory\n");
    arena->arena_size = size;
    arena->alloc_list = list_create(sizeof(block_t));

    return arena;

}



void dealloc_arena(arena_t *arena)
{
    list_node_t *node_block;
    node_block = arena->alloc_list->head;

    while(node_block != NULL) {
        block_t *block;
        block = node_block->data;

        list_free(&block->miniblock_list);

        node_block = node_block->next;
    }

    list_free(&arena->alloc_list);
    free(arena);

    return;
}

void alloc_block(arena_t *arena, uint64_t address, uint64_t size)
{
    if(address >= arena->arena_size) {
        printf("The allocated address is outside the size of arena\n");
        return;
    }

    if(address + size >= arena->arena_size) {
        printf("The end address is past the size of the arena\n");
        return;
    }

    if(arena == NULL || size == 0)
        return;

    int ok = 0;

    miniblock_t *miniblock = NULL;
    miniblock = malloc(sizeof(miniblock_t));
    DIE(miniblock == NULL, "could not allocate memory\n");
    alloc_miniblock(miniblock, address, size);

    list_node_t *node;
    node = arena->alloc_list->head;

    while(node != NULL) {
        block_t *aux_block = node->data;

        if(aux_block->start_address + aux_block->size == miniblock->start_address) {
            add_nth_node(aux_block->miniblock_list, aux_block->miniblock_list->l_size - 1, miniblock);
            free(miniblock);
            return;
        }

        if(miniblock->start_address + miniblock->size == aux_block->start_address) {
            add_nth_node(aux_block->miniblock_list, 0, miniblock);
            free(miniblock);
            return;
        }

        node = node->next;
    }

    if(arena->alloc_list->l_size == 0 || ok == 0) {
        
        block_t *block;
        block = malloc(sizeof(block_t));
        DIE(block == NULL, "could not allocate memory\n");
        block->start_address = address;
        block->size = size;
        //block->miniblock_list = malloc(sizeof(list_t));
        //DIE(block->miniblock_list == NULL, "could not allocate memory\n");
        block->miniblock_list = list_create(sizeof(miniblock_t));
	
        
        add_nth_node(block->miniblock_list, 0, miniblock);
        add_nth_node(arena->alloc_list, 0, block);
        free(miniblock);
		free(block);

        return;

    }

}
void free_block(arena_t *arena, uint64_t address)
{
    list_node_t *node_block;
    node_block = arena->alloc_list->head;
    
    int count = 0;

    while(node_block != NULL) {
        block_t *block;
        block = node_block->data;

        list_node_t *first_node_miniblock;
        first_node_miniblock = block->miniblock_list->head;

        miniblock_t *miniblock1;
        miniblock1 = first_node_miniblock->data;
        
        list_node_t *last_node_miniblock = block->miniblock_list->head;

        for(unsigned int i = 0; i < block->miniblock_list->l_size - 1; i++)
            last_node_miniblock = last_node_miniblock->next;

        miniblock_t *miniblock2;
        miniblock2 = last_node_miniblock->data;

        if(miniblock1->start_address == address && block->miniblock_list->l_size == 1) {
            remove_nth_node(block->miniblock_list, 0);
            remove_nth_node(arena->alloc_list, 0);

            return;
        }

        if(miniblock1->start_address == address) {
            remove_nth_node(block->miniblock_list, 0);

            return;
        }

        if(miniblock2->start_address == address) {
            remove_nth_node(block->miniblock_list, block->miniblock_list->l_size - 1);

            return;
        }

        count++;

        node_block = node_block->next;
    }

    if(count != 0) {
        printf("Invalid address for free.\n");
        return;
    }
}

void pmap(arena_t *arena)
{
    list_node_t *node1;
    node1 = arena->alloc_list->head;
    
    uint64_t mem_allocated = 0;
    unsigned int nr_miniblocks = 0;

    while(node1 != NULL) {
        block_t *block1;
        block1 = node1->data;

        nr_miniblocks = nr_miniblocks + block1->miniblock_list->l_size;
        mem_allocated = mem_allocated + (block1->start_address + block1->size);

        node1 = node1->next;
    }

    printf("Total memory: 0x%lu bytes\n", arena->arena_size);
    printf("Free memory: 0x%lu bytes\n", arena->arena_size - mem_allocated);
    printf("Number of allocated blocks: %x\n", arena->alloc_list->l_size);
    printf("Number of allocated miniblocks: %x\n", nr_miniblocks);
	printf("\n");

    list_node_t *node2;
    node2 = arena->alloc_list->head;

    unsigned int contor_blocks = 1;

    while(node2 != NULL) {
        block_t *block2;
        block2 = node2->data;

		unsigned int contor_miniblocks = 1;

        printf("Block %x begin\n", contor_blocks);
        printf("Zone: 0x%lu - 0x%lu\n", block2->start_address, block2->start_address + block2->size);

        list_node_t *node3;
        node3 = block2->miniblock_list->head;

        while(node3 != NULL) {
            miniblock_t *miniblock;
            miniblock = node3->data;

            printf("Miniblock %x:		0x%lu		-		0x%lu		| RW-\n", contor_miniblocks, miniblock->start_address, miniblock->start_address + miniblock->size);
			contor_miniblocks++;

			node3 = node3->next;
		}


		printf("Block %x end\n", contor_blocks);
		
		contor_blocks++;

		node2 = node2->next;
	}
}



//void read(arena_t *arena, uint64_t address, uint64_t size)
//{

//}

//void write(arena_t *arena, const uint64_t address, const uint64_t size, int8_t *data)
//{

//void mprotect(arena_t *arena, uint64_t address, int8_t *permission)
//{

//}

int main(void)
{
    arena_t *arena = NULL;
    char command[25];
    
    while(scanf("%s", command)) {

        if(strncmp(command, "ALLOC_ARENA", 12) == 0) {
            uint64_t size;
            scanf("%lu", &size);
            
            arena = alloc_arena(size);
        }

        if(strncmp(command, "ALLOC_BLOCK", 12) == 0) {
            uint64_t address;
            uint64_t size;
            scanf("%lu", &address);
            scanf("%lu", &size);

            alloc_block(arena, address, size);
        }

        if(strncmp(command, "FREE_BLOCK", 11) == 0) {
            uint64_t address;
            scanf("%lu", &address);

            free_block(arena, address);
        }

		if(strncmp(command, "PMAP", 5) == 0)
			pmap(arena);

        if(strncmp(command, "DEALLOC_ARENA", 14) == 0) {
            dealloc_arena(arena);

            exit(0);
		}
    }

    return 0;
}