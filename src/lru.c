#include "lru.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int lru_init(struct lru_t *lru,size_t size)
{
  lru->lru_hash=malloc(sizeof(struct hash_table_t));  
  hash_table_init(lru->lru_hash,size);
  lru->lru_queue=malloc(sizeof(struct hash_queue_t));
  hash_queue_init(lru->lru_queue);
  lru->size=size;
  pthread_mutex_init(&lru->lru_mutex, NULL);
}

void* lru_add(struct lru_t *lru,void *key,size_t key_size,void *value,size_t value_size)
{
  void *evicted=NULL;
  pthread_mutex_lock(&lru->lru_mutex);
  //check if the elem is contained in the hash_table
  struct hash_entry_t *present=hash_table_entry(lru->lru_hash,key,key_size);
  if(present)
    {   //assuming flat structures for keys
      if(memcmp(present->key,lru->lru_queue->head->key,key_size)!=0)
	{
	  struct hash_entry_t *queue_entry=present->value;
	  //in case we want to free the swaped memory 
	  evicted=hash_queue_remove(lru->lru_queue,queue_entry);
	  hash_entry_destroy(queue_entry);
	  hash_queue_push_front(lru->lru_queue,key,key_size,value,value_size);
	  hash_table_insert(lru->lru_hash,key,key_size,lru->lru_queue->head,sizeof(struct hash_entry_t*));
	}
    }
  else
    {
      //check if our cache is full
      if(lru->lru_queue->size==lru->size)
	{
	  evicted=lru_evict(lru);
	}
      hash_queue_push_front(lru->lru_queue,key,key_size,value,value_size);
      hash_table_insert(lru->lru_hash,key,key_size,lru->lru_queue->head,sizeof(struct hash_entry_t*));
    }
  pthread_mutex_unlock(&lru->lru_mutex);
  return evicted;
}

void* lru_evict(struct lru_t *lru)
{
  void *value=NULL;
  pthread_mutex_unlock(&lru->lru_mutex);
  if(lru->lru_queue->tail!=NULL)
    {
      hash_table_remove(lru->lru_hash,lru->lru_queue->tail->key,lru->lru_queue->tail->key_len);
      //remove the tail element from the queue
      struct hash_entry_t *entry=hash_queue_pop_back(lru->lru_queue);;
      value=entry->value;
      hash_entry_destroy(entry);
    }
  pthread_mutex_unlock(&lru->lru_mutex);
  return value;
}

void* lru_get(struct lru_t *lru,void *key,size_t key_size,size_t *item_len)
{
  pthread_mutex_lock(&lru->lru_mutex);
 void *element=((struct hash_entry_t *)hash_table_get(lru->lru_hash,key,key_size,item_len))->value;
  pthread_mutex_unlock(&lru->lru_mutex);
  return element;  
}

void* lru_remove(struct lru_t *lru,void *key,size_t key_size)
{
  pthread_mutex_lock(&lru->lru_mutex);
  void *element=NULL;
  struct hash_entry_t *present=hash_table_entry(lru->lru_hash,key,key_size);
  if(present)
    {
      struct hash_entry_t *queue_entry=present->value;
      element =hash_queue_remove(lru->lru_queue,queue_entry);
      hash_table_remove(lru->lru_hash,key,key_size);
      hash_entry_destroy(queue_entry);
    }
  pthread_mutex_unlock(&lru->lru_mutex);
  return element;  
}
void lru_destroy(struct lru_t *lru)
{
  pthread_mutex_lock(&lru->lru_mutex);
  hash_table_destroy(lru->lru_hash);
  free(lru->lru_hash);
  hash_queue_destroy(lru->lru_queue);
  free(lru->lru_queue);
  pthread_mutex_unlock(&lru->lru_mutex);
  pthread_mutex_destroy(&lru->lru_mutex);
}

/*
int main(int argc,char *argv[])
{
   srand(time(NULL));
  struct lru_t lru;
  lru_init(&lru,10);
  int lru_d[150];
  int a=99,b=1,c=2,d=3,e=4;
  lru_add(&lru,&a,sizeof(int),&a,sizeof(int));
  lru_add(&lru,&b,sizeof(int),&b,sizeof(int));
  lru_add(&lru,&c,sizeof(int),&c,sizeof(int));
  lru_add(&lru,&d,sizeof(int),&d,sizeof(int));
  lru_add(&lru,&e,sizeof(int),&e,sizeof(int));
  lru_add(&lru,&a,sizeof(int),&a,sizeof(int));
  lru_add(&lru,&c,sizeof(int),&c,sizeof(int));
  struct   hash_entry_t *entr=lru.lru_queue->head;
  while(entr)
    {
      printf("%d ",*(int*)entr->key);
      entr=entr->next;
    }
    printf("\n");
  size_t len;
  printf("\na?:%d\n",*(int*)lru_get(&lru,&a,sizeof(int),&len));
  lru_remove(&lru,&a,sizeof(int));
  lru_remove(&lru,&c,sizeof(int));
  lru_destroy(&lru);
  
  return 0;
}
*/
