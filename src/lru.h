#include "hashtable.h"
#include <pthread.h>
#ifndef __LRU_H
#define __LRU_H

struct lru_t
{
  //the queue contains key,value
  struct hash_queue_t *lru_queue;
  //the hash map contains key,queue_element
  struct hash_table_t *lru_hash;
  //max number of elements to keep in the queue
  size_t size;
  pthread_mutex_t lru_mutex;
};

int lru_init(struct lru_t *lru,size_t size);
//return element that has to be evicted the value swaped out or NULL //writer lock
void* lru_add(struct lru_t *lru,void *key,size_t key_size,void *value,size_t value_size);
//return the value if the element exists or NULL
void* lru_get(struct lru_t *lru,void *key,size_t key_size,size_t *item_len);
void* lru_remove(struct lru_t *lru,void *key,size_t key_size);
void lru_destroy(struct lru_t *lru);

static void* lru_evict(struct lru_t *lru);
#endif
