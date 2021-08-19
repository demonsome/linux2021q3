#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <stdalign.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <string.h>
#include <threads.h>

struct entry_s {
	int *key;
	atomic_uintptr_t *value;
	struct entry_s *next;
};

typedef struct entry_s entry_t;

struct hashtable_s {
	int size;
	struct entry_s **table;	
};

typedef struct hashtable_s hashtable_t;


/* Create a new hashtable. */
hashtable_t *ht_create( int size ) {

	hashtable_t *hashtable = NULL;
	int i;

	if( size < 1 ) return NULL;

	/* Allocate the table itself. */
	if( ( hashtable = malloc( sizeof( hashtable_t ) ) ) == NULL ) {
		return NULL;
	}

	/* Allocate pointers to the head nodes. */
	if( ( hashtable->table = malloc( sizeof( entry_t * ) * size ) ) == NULL ) {
		return NULL;
	}
	for( i = 0; i < size; i++ ) {
		hashtable->table[i] = NULL;
	}

	hashtable->size = size;

	return hashtable;	
}

/* Hash a string for a particular hash table. */
int ht_hash( hashtable_t *hashtable, int *key ) {
	return *key % hashtable->size;
}

/* Create a key-value pair. */
entry_t *ht_newpair( int *key, atomic_uintptr_t *value ) {
	entry_t *newpair;

	if( ( newpair = malloc( sizeof( entry_t ) ) ) == NULL ) {
		return NULL;
	}

	if( ( newpair->key = key ) == NULL ) {
		return NULL;
	}

	if( ( newpair->value = value ) == NULL ) {
		return NULL;
	}

	newpair->next = NULL;

	return newpair;
}

/* Insert a key-value pair into a hash table. */
void ht_set( hashtable_t *hashtable, int *key, atomic_uintptr_t *value ) {
	int bin = 0;
	entry_t *newpair = NULL;
	entry_t *next = NULL;
	entry_t *last = NULL;

	bin = ht_hash( hashtable, key );

	next = hashtable->table[ bin ];

	while( next != NULL && next->key != NULL &&  (*key > *next->key)) {
		last = next;
		next = next->next;
		printf("add in %d",bin);
	}

	/* There's already a pair.  Let's replace that string. */
	if( next != NULL && next->key != NULL && (*key == *next->key)) {

		free( next->value );
		next->value = value ;

	/* Nope, could't find it.  Time to grow a pair. */
	} else {
		newpair = ht_newpair( key, value );

		/* We're at the start of the linked list in this bin. */
		if( next == hashtable->table[ bin ] ) {
			newpair->next = next;
			hashtable->table[ bin ] = newpair;
	
		/* We're at the end of the linked list in this bin. */
		} else if ( next == NULL ) {
			last->next = newpair;
	
		/* We're in the middle of the list. */
		} else  {
			newpair->next = next;
			last->next = newpair;
		}
	}
}

/* Retrieve a key-value pair from a hash table. */
atomic_uintptr_t *ht_get( hashtable_t *hashtable, int *key ) {
	int bin = 0;
	entry_t *pair;
	
	bin = ht_hash( hashtable, key );

	/* Step through the bin, looking for our value. */
	pair = hashtable->table[ bin ];
	while( pair != NULL && pair->key != NULL && (*key > *pair->key) ) {
		pair = pair->next;
		
	}
	
	/* Did we actually find anything? */
	if( pair == NULL || pair->key == NULL || (*key != *pair->key )) {
		printf("Find Nothing!\n");
		return NULL;

	} else {
		return pair->value;
	}
	
	
}

/* Delete a key-value pair from a hash table. */
void ht_del( hashtable_t *hashtable, int *key ) {
	int bin = 0;
	entry_t *pair;
	entry_t *last;
	
	
	bin = ht_hash( hashtable, key );

	/* Step through the bin, looking for our value. */
	last = pair = hashtable->table[ bin ];
	
	while( pair != NULL && pair->key != NULL && (*key > *pair->key) ) {
		last = pair;
		pair = pair->next;
	}
	
	/* Key founded. */
	if( pair != NULL && pair->key != NULL && (*key == *pair->key)) {
		/* We're at the start of the linked list in this bin. */
		if( pair == hashtable->table[ bin ] ) {
			
			hashtable->table[ bin ] = pair->next;
			free(pair);
			printf("start\n");
	
		/* We're at the end of the linked list in this bin. */
		} else if ( pair == NULL ) {
			printf("end\n");
			return;
			
		/* We're in the middle of the list. */
		} else  {
			printf("middle\n");
			last->next = pair->next;
			free(pair);	
		}

}


