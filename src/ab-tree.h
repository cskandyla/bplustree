#include <stdio.h>
#include "record.h"
#include "lru.h"
#ifndef __AB_TREE_H
#define __AB_TREE_H


struct ab_tree_traits
{
  data_compare   compare;
  data_free      dealloc;
  data_alloc     alloc;
  data_serialize store;
  data_load      load;
};

struct ab_node_meta_t
{
  enum Node_Type n_t;
  enum Ram_Disk r_d;
  unsigned int block_id;
  //union for location on ram/disk
  //union node_loc location;maybe we just need the disk location?
  void  *address;
};

struct ab_inner_node
{
  struct ab_inner_node *parent;
  struct ab_node_meta_t *children;
  struct data_t *keys;//b-1 keys for each node--careful with that
  unsigned int BID;
  int length;
};

struct ab_leaf
{
  struct ab_inner_node *parent;
  struct record_t      *data;
  int                  length;
  unsigned int         BID;
};



struct ab_tree
{
  struct ab_inner_node    *root;
  unsigned long           height;
  unsigned long           num_inner_nodes;
  unsigned long           num_leaves;
  unsigned int            a;
  unsigned int            b;
  struct ab_tree_traits   traits;
  //store elements by block id
  struct lru_t            lru;
  FILE                    *tree_file;
  //queue holding next offsets to write to
  struct hash_queue_t     disk_queue;
  long                    next_disk_write;
};

//Exported Functions 
int                   AB_Create(struct ab_tree *tree,unsigned int a,unsigned int b,struct ab_tree_traits traits,char *filename);
int                   AB_Insert(struct ab_tree *tree,struct record_t data);
int                   AB_Delete(struct ab_tree *tree,struct data_t data);       
struct                record_t*  AB_Search(struct ab_tree *tree,struct data_t data);
void                  AB_Destroy(struct ab_tree *tree);
void                  TraverseTree(struct ab_tree *tree,data_describe data_desc);


struct ab_tree_traits AB_Default_Traits();
//Default Trait Functions
struct record_t AB_Default_Alloc(void *value);
void            AB_Default_Dealloc(struct record_t rec);
void            AB_Default_Serialize(struct record_t rec,FILE *fp);
struct record_t AB_Default_Load(FILE *fp);


/*
Static functions  to force scope to file
*/
//Struct Creation
static struct ab_inner_node* AB_Inner_Node_Create(struct ab_tree *tree);
static struct ab_leaf*       AB_Leaf_Create(struct ab_tree *tree);
//Struct Deletion
static void AB_Node_Destroy(struct ab_tree *tree,struct ab_inner_node *tn);
static void AB_Leaf_Destroy(struct ab_tree *tree,struct ab_leaf *tl);
static void AB_Leaf_Free(struct ab_leaf *leaf);
static void AB_Node_Free(struct ab_inner_node *node);
//Slice
static int Slice(unsigned int B);
//Insert Helpers
static int AB_Inner_Insert(struct ab_tree *tree,struct ab_inner_node *tn,struct ab_inner_node *tn_parent,struct record_t data);
static int AB_Leaf_Insert(struct ab_tree *tree,struct ab_leaf *tl,struct ab_inner_node *tl_parent,struct record_t data,int index);
static int AB_Node_Insert(struct ab_tree *tree,struct ab_inner_node *tn,enum Node_Type n_t,void *node,struct data_t key);
static int AB_Place_Node(struct ab_tree *tree,struct ab_inner_node *tn,struct ab_inner_node *nn,struct data_t key);
static int AB_Place_Leaf(struct ab_tree *tree,struct ab_inner_node *tn,struct ab_leaf *tl);
static int AB_Place_Data(struct ab_tree *tree,struct ab_leaf *tl,struct record_t data);
//Delete Helpers
static int AB_Inner_Delete(struct ab_tree *tree,struct ab_inner_node *tn,struct ab_inner_node *tn_parent,struct data_t data);
static int AB_Leaf_Delete(struct ab_tree *tree,struct ab_leaf *tl,struct ab_inner_node *tl_parent,struct data_t data,int index);
static int AB_Node_Delete(struct ab_tree *tree,struct ab_inner_node *tn,int pos);
static int AB_Remove_Node(struct ab_inner_node *tn,int pos);
static int AB_Remove_Leaf(struct ab_inner_node *tn,int pos);
static int AB_Remove_Data(struct ab_tree *tree,struct ab_leaf *tl,struct data_t data);
//Splits-Merges
static struct ab_leaf*       AB_Split_Leaf(struct ab_tree *tree,struct ab_leaf *tl);
static struct ab_inner_node* AB_Split_Node(struct ab_tree *tree,struct ab_inner_node *tn);
struct ab_leaf*              AB_Merge_Leaf(struct ab_leaf *left,struct ab_leaf *right);
struct ab_inner_node*        AB_Merge_Node(struct ab_inner_node *left,struct ab_inner_node *right,struct data_t key);
//Search Helpers
static struct record_t* AB_Node_Search(struct ab_tree *tree,struct ab_inner_node *tn,struct data_t data);
static struct record_t* AB_Leaf_Search(struct ab_tree *tree,struct ab_leaf *tl,struct data_t data);
//PrintHelpers
 void print_leaf_stats(struct ab_leaf *tl,data_describe data_desc);
static void print_node_stats(struct ab_inner_node *tn);
static void print_tree_stats(struct ab_tree *tree,char traverse,data_describe data_desc);
static void TraverseNode(struct ab_inner_node *tn,data_describe data_desc);
static void minimal_node_stats(struct ab_inner_node *tn);

//Leaf Load/Store make static 
struct ab_leaf* AB_Leaf_Load(struct ab_tree *tree,struct ab_inner_node *parent,int pos);
void            AB_Leaf_Store(struct ab_tree *tree,struct ab_leaf *leaf);

//Leaf Grab: return it from the lru or add it to the lru and return it, also store the leaf returned by the lru 
struct ab_leaf* AB_Leaf_Grab(struct ab_tree *tree,struct ab_inner_node *parent,int pos);

#endif
