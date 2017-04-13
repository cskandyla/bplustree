#include <stddef.h>
#ifndef __RECORD_H
#define __RECORD_H

//rename to record.h ?

//simplest possible generic type struct--blob of memory
struct data_t
{
  void *data;
  unsigned int size;
};

//Simplistic key/value data type
struct record_t
{
  struct data_t key;
  struct data_t value;
};

//function pointers needed for record used in the tree traits to provide generic functionality
typedef struct record_t (*data_alloc)(void *value);
typedef void (*data_free)(struct record_t record);
typedef char* (*data_describe)(struct record_t data);
typedef int  (*data_compare)(struct data_t *a,struct data_t *b);//Somewhat Compatible with qsort
typedef void (*data_serialize)(struct record_t data);
typedef void (*data_deserialize)(struct record_t data);

enum Ram_Disk { RAM=0,DISK};
enum Node_Type {LEAF=0,INNER};

//remove Ram_Disk ?
//rename to node_meta_t
struct magic_id
{
  enum Ram_Disk r_d;
  enum Node_Type n_t;
  size_t block_id;
  //union for location 
  void  *address;
};

/*
address/file_offset depending on what we're pointing to
union node_loc
{
  void *address;
  long file_offset;
};
*/


#endif
