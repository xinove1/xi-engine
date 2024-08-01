#ifndef XI_ARENAS_H_
# define XI_ARENAS_H_
# include <stdlib.h>
# include <string.h>
# include <assert.h>
# include "types.h"

# ifndef ARENA_BLOCK_SIZE
#  define ARENA_BLOCK_SIZE 1024
# endif

// NOTE from untangling lifetimes, dont really remember why i got them
/* # define PushArray(arena, type, count) (type *)ArenaPush((arena), sizeof(type)*(count)) */
/* # define PushArrayZero(arena, type, count) (type *)ArenaPushZero((arena), sizeof(type)*(count)) */
/* # define PushStruct(arena, type) PushArray((arena), (type), 1) */
/* # define PushStructZero(arena, type) PushArrayZero((arena), (type), 1) */

typedef struct List List;
struct List
{
	void *content;
	List *next;
};

typedef struct
{
	byte   *mem_begin;
	byte   *mem_end;
	byte   *pos;
	size_t chunk_sz;
	List   *freed;
} Arena;

typedef struct
{

} ArenaCtx;

	// ArenaMem	*mem;
	// void (*clean)(ArenaMem *arena);
// Arena
Arena *arena_create(size_t chunk_size);
Arena *arena_create_sized(size_t chunk_size, size_t mem_size_scalar);
void arena_destroy(Arena *arena);
void *linear_alloc(Arena *arena, size_t size); // Linear
void *pool_alloc(Arena *arena); // Pool
void pool_free(Arena *arena, void *mem);
void arena_resize(Arena *arena);
void arena_clean(Arena *arena);

#endif 

#ifdef XI_ARENAS_IMPLEMENTATION
// NOLINTBEGIN(misc-definitions-in-headers)

static void arena_push_freed(Arena *arena);

Arena *arena_create(size_t chunk_size)
{
	Arena *arena = (Arena*) malloc(sizeof(Arena));

	arena->mem_begin = (byte *) malloc(ARENA_BLOCK_SIZE);
	arena->mem_end = arena->mem_begin + ARENA_BLOCK_SIZE;
	arena->pos = arena->mem_begin;
	arena->chunk_sz = chunk_size;
	arena->freed = NULL;
	if (chunk_size != 0)
	{
		assert(chunk_size >= sizeof(List));
		arena_push_freed(arena);
	}
	return (arena);
}

Arena *arena_create_sized(size_t chunk_size, size_t mem_size_scalar)
{
	Arena *arena = (Arena*) malloc(sizeof(Arena));

	arena->mem_begin = (byte *) malloc(ARENA_BLOCK_SIZE * mem_size_scalar);
	arena->mem_end = arena->mem_begin + ARENA_BLOCK_SIZE * mem_size_scalar;
	arena->pos = arena->mem_begin;
	arena->chunk_sz = chunk_size;
	arena->freed = NULL;
	if (chunk_size != 0)
	{
		assert(chunk_size >= sizeof(List));
		arena_push_freed(arena);
	}
	return (arena);
}

void arena_destroy(Arena *arena)
{
	free(arena->mem_begin);
	free(arena);
}

void *linear_alloc(Arena *arena, size_t size)
{
	assert(!arena->chunk_sz);
	void   *mem = NULL;
	size_t size_alloc;

	size_alloc = size;

	// TODO change to add another arena
	if (arena->pos + size_alloc > arena->mem_end) {
		arena_resize(arena);
	}
	mem = arena->pos;
	memset(mem, 0, size_alloc);
	arena->pos += size_alloc;
	return (mem);
}

void *pool_alloc(Arena *arena)
{
	assert(arena->chunk_sz);
	void *mem = NULL;

	mem = arena->freed;
	assert(mem != NULL);
	arena->freed = arena->freed->next;
	return (mem);
}

void pool_free(Arena *arena, void *mem)
{
	assert((byte *) mem >= arena->mem_begin && (byte *) mem <= arena->mem_end);

	memset(mem, 0, arena->chunk_sz);
	List	*node = (List *) mem;
	node->next = arena->freed;
	arena->freed = node;
}

void arena_clean(Arena *arena)
{
	memset(arena->mem_begin, 0, arena->pos - arena->mem_begin);
	arena->pos = arena->mem_begin;
	if (arena->chunk_sz)
		arena_push_freed(arena);
}

static void arena_push_freed(Arena *arena)
{
	i32 size = (arena->mem_end - arena->mem_begin ) / arena->chunk_sz;

	for (i32 i = 0; i < size; i++)
	{
		List *node = (List *)&arena->mem_begin[i * arena->chunk_sz];
		node->next = arena->freed;
		arena->freed = node;
	}
}

void arena_resize(Arena *arena)
{
	assert(0 == 1);
	TraceLog(LOG_INFO, "Arena being resized\n");
	arena->mem_begin = (byte *)realloc(arena->mem_begin, arena->mem_end - arena->mem_begin + ARENA_BLOCK_SIZE);
}
// NOLINTEND(misc-definitions-in-headers)
#endif
