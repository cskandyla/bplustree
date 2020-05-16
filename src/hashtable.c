#include "hashtable.h"
#include <string.h>
#include <endian.h>
#include <stdlib.h>
#include <time.h>

//murmurhash3 Based on MurmurHash3 by Austing Appleby:
//using the 32 bit version since the 128bit version produces different results in x86 and x64
uint32_t getblock(uint8_t *p,int i)
{
  uint32_t r;
  memcpy(&r,&p[i*sizeof(uint32_t)],sizeof(uint32_t));
  return le32toh(r);
}

uint32_t rotl(uint32_t x,uint8_t r)
{
  return (x << r) | (x >> (32 - r));
}

uint32_t fmix(uint32_t h)
{
  h^= h >> 16;
  h*= 0x85ebca6b;
  h^= h >> 13;
  h*= 0xc2b2ae35;
  h^= h >> 16;
  return h;
}

uint32_t  MurmurHash3_32(void *key,int len,uint32_t seed)
{
  uint8_t *blocks=(uint8_t*)key;
  int num_blocks=len/4;
  uint32_t h1=seed;
  uint32_t c1 = 0xcc9e2d51;
  uint32_t c2 = 0x1b873593;
  uint32_t k1;
  for(int i=0;i<num_blocks;i++)
    {
      k1=getblock(blocks,i);
      k1*=c1;
      k1=rotl(k1,15);
      k1*=c2;
      h1^=k1;
      h1=rotl(h1,13);
      h1=h1*5+0xe6546b64;
    }
  uint8_t *tail=(uint8_t*)key+num_blocks*4;
  k1=0;
  switch(len & 3)
    {
    case 3:
      k1^=tail[2]<<16;
    case 2:
      k1^= tail[1]<<8;
    case 1:
      k1^=tail[0];
      k1*=c1;
      k1=rotl(k1,15);
      k1*=c2;
      h1^=k1;
    }
  h1^=len;
  h1=fmix(h1);
  return h1;
}

//hash_entry functions
struct hash_entry_t* hash_entry_create(void *key,size_t key_len,
				       void *value,size_t value_len)
{
  struct hash_entry_t *entry=malloc(sizeof(struct hash_entry_t));
  entry->key=key;
  entry->key_len=key_len;
  entry->value=value;
  entry->value_len=value_len;
  entry->prev=entry->next=NULL;
}
void hash_entry_destroy(struct hash_entry_t *entry)
{
  free(entry);
}

//hash_queue functions
void hash_queue_init(struct hash_queue_t *queue)
{
  queue->head=queue->tail=NULL;
  queue->size=0;
}

void hash_queue_push_front(struct hash_queue_t *queue,void *key,size_t key_len,
			   void *value,size_t value_len)
{
  struct hash_entry_t *entry=hash_entry_create(key,key_len,value,value_len);
  entry->prev=NULL;
  entry->next=queue->head;
  if(queue->head==NULL)
    {
      queue->head=queue->tail=entry;
    }
  else
    {
      queue->head->prev=entry;
      queue->head=entry; 
    }
  queue->size++;
}

void hash_queue_push_back(struct hash_queue_t *queue,void *key,size_t key_len,
			  void *value,size_t value_len)
{
  struct hash_entry_t *entry=hash_entry_create(key,key_len,value,value_len);
  entry->prev=queue->tail;
  entry->next=NULL;
  if(queue->tail==NULL)
    {
      queue->head=queue->tail=entry;
    }
  else
    {
      queue->tail->next=entry;
      queue->tail=entry;
      
    }
  queue->size++;
}

struct hash_entry_t* hash_queue_pop_front(struct hash_queue_t *queue)
{
  if(queue->head)
    {
      struct hash_entry_t* entry=queue->head;
      queue->head=queue->head->next;
      queue->head->prev=NULL;
      queue->size--;
      return entry;
    }
  return NULL; 
}

struct hash_entry_t* hash_queue_pop_back(struct hash_queue_t *queue)
{
  if(queue->tail)
    {
      struct hash_entry_t* entry=queue->tail;
      queue->tail=queue->tail->prev;
      queue->tail->next=NULL;
      queue->size--;
      return entry;
    }
  return NULL;
}

int hash_queue_find(struct hash_queue_t *queue,struct hash_entry_t *entry,
		    int (*cmpfunc)(void *elem1,void *elem2))
{
  if(!entry)
    return -1;
  struct hash_entry_t *elem=queue->head;
  while(elem)
    {
      if(cmpfunc(entry,elem)==0)
	{
	  return 0;
	}
      elem=elem->next;
    }
  return 1;
}

void* hash_queue_remove(struct hash_queue_t *queue,
				       struct hash_entry_t *entry)
{
  if(entry->prev)
    entry->prev->next=entry->next;
  if(entry->next)
    entry->next->prev=entry->prev;
  if(entry==queue->head)
    queue->head=entry->next;
  if(entry==queue->tail)
    queue->tail=entry->prev;
  void *value=entry->value;
  //hash_entry_destroy(entry);
  queue->size--;
  return value;
}

int hash_queue_destroy(struct hash_queue_t *queue)
{
  struct hash_entry_t *elem=queue->head;
  while(elem)
    {
      struct hash_entry_t *tmp=elem;
      elem=elem->next;
      hash_entry_destroy(tmp);
    }
}

//hash_table functions
int hash_table_init(struct hash_table_t *hashtable,size_t size)
{
  hashtable->table=malloc(size*sizeof(struct hash_queue_t*));
  hashtable->size=size;
  hashtable->num_elements=0;
  hashtable->seed=time(NULL);
  //initilize all queues
  for(size_t i=0;i<hashtable->size;i++)
    {
      hashtable->table[i]=malloc(sizeof(struct hash_queue_t));
      hash_queue_init(hashtable->table[i]);
    }
}

void hash_table_insert(struct hash_table_t *hashtable,void *key,size_t key_len,
		       void *value,size_t value_len)
{
  //check if the key allready exists
  struct hash_entry_t *entry=hash_table_entry(hashtable,key,key_len);
  if(entry)
    {
      entry->value=value;
      entry->value_len=value_len;
      //memcpy(&entry_value,&value,len);
    }
  else
    {
      //create a new entry and push it to the correct queue
      size_t idx=MurmurHash3_32(key,key_len,hashtable->seed)%hashtable->size;
      hash_queue_push_back(hashtable->table[idx],key,key_len,value,value_len);
      hashtable->num_elements++;
    }
}

void* hash_table_entry(struct hash_table_t *hashtable,void *key,size_t key_len)
{
 size_t idx=MurmurHash3_32(key,key_len,hashtable->seed)%hashtable->size;
      struct hash_entry_t *entry=hashtable->table[idx]->head;
      while(entry)
	{
	  if(memcmp(key,entry->key,key_len)==0)
	    {
	      return entry;
	    }
	  entry=entry->next;
	}
  return NULL;
}

void* hash_table_get(struct hash_table_t *hashtable,void *key,size_t key_len,
		     size_t* value_len)
{
  struct hash_entry_t *entry=hash_table_entry(hashtable,key,key_len);
  if(entry)
    {
      if(value_len)
      *value_len=entry->value_len;
      return entry->value;
    }
    
  return NULL;
}

void* hash_table_remove(struct hash_table_t *hashtable,void *key,size_t key_len)
{
  size_t idx=MurmurHash3_32(key,key_len,hashtable->seed)%hashtable->size;
  struct hash_entry_t *entry=hashtable->table[idx]->head;
  while(entry)
    {  
      if(entry->key && memcmp(key,entry->key,key_len)==0)
	{
	  void *value=hash_queue_remove(hashtable->table[idx],entry);
	  hash_entry_destroy(entry);
	  hashtable->num_elements--;
	  return value;
	}
      entry=entry->next;
    }
  return NULL;
}

int hash_table_destroy(struct hash_table_t *hashtable)
{
  //destroy all queues
  for(size_t i=0;i<hashtable->size;i++)
    {
      hash_queue_destroy(hashtable->table[i]);
      free(hashtable->table[i]);
    }
  free(hashtable->table);
}
