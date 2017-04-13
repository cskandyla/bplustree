#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "ab-tree.h"

//we're doing nothing by default
struct record_t AB_Default_Alloc(void *value)
{
}

void AB_Default_Dealloc(struct record_t rec)
{
  //just asuming flat data structures and freeing
  free(rec.value.data);
  free(rec.key.data);
}


//At the very least a comparison function for keys has to be provided
struct ab_tree_traits AB_Default_Traits(data_compare compare)
{
  struct ab_tree_traits tree_traits;
  tree_traits.dealloc=AB_Default_Dealloc;
  tree_traits.alloc=AB_Default_Alloc;
  tree_traits.compare=compare;
  return tree_traits;
}

int AB_Create(struct ab_tree *tree,unsigned int a,unsigned int b,struct ab_tree_traits traits)
{
  tree->a=a;
  tree->b=b;
  tree->num_inner_nodes=0;
  tree->num_leaves=0;
  tree->traits=traits;
  //Set Comparison Function
  //Create and Set Root
  struct ab_inner_node *root=AB_Inner_Node_Create(tree);
  tree->root=root;

}

void AB_Destroy(struct ab_tree *tree)
{
  AB_Node_Destroy(tree,tree->root);
}



struct ab_inner_node* AB_Inner_Node_Create(struct ab_tree *tree)
{
  //Allocate memory for the node and its children and keys
  struct ab_inner_node *tree_node=malloc(sizeof(struct ab_inner_node));
  tree_node->children=malloc(sizeof(struct magic_id)*tree->b);
  tree_node->keys=malloc(sizeof(struct data_t)*(tree->b-1));
  tree_node->parent=NULL;
  //We added a new node update tree info
  tree->num_inner_nodes++;
  //Initialize node values
  tree_node->length=0;
  tree_node->BID=tree->num_inner_nodes+tree->num_leaves;
  return tree_node;
}

struct ab_leaf* AB_Leaf_Create(struct ab_tree *tree)
{
  struct ab_leaf* tree_leaf=malloc(sizeof(struct ab_leaf));
  tree_leaf->data=malloc(sizeof(struct record_t)*tree->b);
  tree_leaf->parent=NULL;
  //We added a new leaf update tree info
  tree->num_leaves++;
  //Initialize leaf values
  tree_leaf->length=0;
  tree_leaf->BID=tree->num_inner_nodes+tree->num_leaves;
  return tree_leaf;
}

void AB_Node_Destroy(struct ab_tree *tree,struct ab_inner_node *tn)
{
  //Destroy recursively
  for(int i=0;i<tn->length;i++)
    {
      if(tn->children[i].n_t==INNER)
	{
	  AB_Node_Destroy(tree,tn->children[i].address);
	}
      else if(tn->children[i].n_t==LEAF)
	{
	  AB_Leaf_Destroy(tree,tn->children[i].address);
	}
    }
  //free keys and node memory
  for(int i=0;i<tn->length-1;i++)
    {
      free(tn->keys[i].data);
    }
  AB_Node_Free(tn);
  free(tn);
  
  tree->num_inner_nodes--;
  
}

void AB_Leaf_Destroy(struct ab_tree *tree,struct ab_leaf *tl)
{
  
  for(int i=0;i<tl->length;i++)
    {
      tree->traits.dealloc(tl->data[i]);
    }

  AB_Leaf_Free(tl);
  free(tl);
  tree->num_leaves--;
}

void AB_Leaf_Free(struct  ab_leaf *leaf)
{
     free(leaf->data);
}

void AB_Node_Free(struct ab_inner_node *node)
{
  free(node->keys);
  free(node->children);
}


struct ab_inner_node* AB_Split_Node(struct ab_tree *tree,struct ab_inner_node *tn)
{
  //Create a new  inner node
  struct ab_inner_node *nn=AB_Inner_Node_Create(tree);
  nn->parent=tn->parent;  
  int j=0;
  for(int i=tn->length/2+1;i<tn->length;i++)
    {
      //update the node's parent
      if(tn->children[i].n_t==INNER)
	((struct ab_inner_node*)tn->children[i].address)->parent=nn;
      else
	((struct ab_leaf*)tn->children[i].address)->parent=nn;;
      //copy children over
      memcpy(&nn->children[j],&tn->children[i],sizeof(struct magic_id));
      //copy keys over
      if(i!=tn->length-1 )
	memcpy(&nn->keys[j],&tn->keys[i],sizeof(struct data_t));
      j++;
    }
  //update lengths
  nn->length+=j;
  tn->length-=j;
  return nn;
}

struct ab_leaf* AB_Split_Leaf(struct ab_tree *tree,struct ab_leaf *tl)
{
  //Split leaf
  struct ab_leaf *nl=AB_Leaf_Create(tree);
  //Set parent
  nl->parent=tl->parent;
  int j=0;
  for(int i=tl->length/2;i<tl->length;i++)
    {
      //      nl->data[j].data=tl->data[i].data;
      //nl->data[j].size=tl->data[i].size;
      memcpy(&nl->data[j],&tl->data[i],sizeof(struct record_t));
      j++;
    }
  //update lengths
  nl->length+=j;
  tl->length-=j;
  return nl;
}



//Entry search function
struct record_t*  AB_Search(struct ab_tree *tree,struct data_t data)
{
  return AB_Node_Search(tree,tree->root,data);
}

struct record_t* AB_Node_Search(struct ab_tree *tree,struct ab_inner_node *tn,struct data_t data)
{
  int i=0;
  //Traverse through nodes and recurse or call Leaf Search
  for(i=0;i<(int)tn->length-1;i++)
    {
      if(tree->traits.compare(&data,&tn->keys[i])<0)
	{
	  break;
	}
    }
  if(tn->children[i].n_t==LEAF)
    {
      return AB_Leaf_Search(tree,tn->children[i].address,data);
    }
  else if(tn->children[i].n_t==INNER)
    {
      return AB_Node_Search(tree,tn->children[i].address,data);
    }
}


struct record_t* AB_Leaf_Search(struct ab_tree *tree,struct ab_leaf *tl,struct data_t data)
{
  //Just traverse through the data if we find what we're looking for return a pointer else return NULL
  int i=0;
  for(i=0;i<tl->length;i++)
    {      
      if(tree->traits.compare(&data,&tl->data[i].key)==0)
	{
	  return &tl->data[i];
	}
    }
  return NULL;
}

static int Slice(unsigned int B)
{
  if(B%2==0)
    return (B)/2;
  else
    return (B)/2+1;
  
}

//Exposed Insert function
int  AB_Insert(struct ab_tree *tree,struct record_t data)
{
  //Corner Case Tree is empty
  if(tree->num_leaves==0)
    {
      //Create a new leaf
      struct ab_leaf *new_leaf=AB_Leaf_Create(tree);
      new_leaf->parent=tree->root;
      //Update root info
      tree->root->length++;
      //Attach it to the root
      tree->root->children[0].r_d=RAM;
      tree->root->children[0].n_t=LEAF;
      tree->root->children[0].block_id=new_leaf->BID;
      tree->root->children[0].address=new_leaf;
      //Add the data to the leaf
      memcpy(&new_leaf->data[0],&data,sizeof(struct record_t));
      new_leaf->length++;
    }
  else
    {
      AB_Inner_Insert(tree,tree->root,NULL,data);
    }
}

int AB_Inner_Insert(struct ab_tree *tree,struct ab_inner_node *tn,struct ab_inner_node *tn_parent,struct record_t data)
{

  int i=0;
  //Find correct node/leaf to insert data
  for(i=0;i<tn->length-1;i++)
    {
      if(tree->traits.compare(&data.key,&tn->keys[i])<0)
	{
	  break;
	}
    }
  
  if(tn->children[i].n_t==LEAF)
    {
      AB_Leaf_Insert(tree,tn->children[i].address,tn,data,i);
    }
  else if(tn->children[i].n_t==INNER)
    {
      //Recursion
      AB_Inner_Insert(tree,tn->children[i].address,tn,data);
    }

}

//Inserting a new node/leaf into a node
int AB_Node_Insert(struct ab_tree *tree,struct ab_inner_node *tn,enum Node_Type n_t,void *node,struct data_t key)
{
  if(n_t==LEAF)
    {
      AB_Place_Leaf(tree,tn,node);
      free(key.data);
    }
  else if (n_t==INNER)
    {
      AB_Place_Node(tree,tn,node,key);
    }
  
  if(tn->length==tree->b)
    {
      //Split the node  
      struct data_t new_key;
      //key to move one level up after the split
      memcpy(&new_key,&tn->keys[tn->length/2],sizeof(struct data_t));
      
      struct ab_inner_node* nn=AB_Split_Node(tree,tn);
      
      //if we are the root well create a new one 
      if(tn->parent==NULL)
	{
	  //Create a new root;
	  struct ab_inner_node *root=AB_Inner_Node_Create(tree);
	  tree->root=root;
	  //Assign left child to root[0]
	  tree->root->children[0].r_d=RAM;
	  tree->root->children[0].n_t=INNER;
	  tree->root->children[0].block_id=tn->BID; 
	  tree->root->children[0].address=tn;
	  //Assign the right child to root[1]
	  tree->root->children[1].r_d=RAM;
	  tree->root->children[1].n_t=INNER;
	  tree->root->children[1].block_id=nn->BID; 
	  tree->root->children[1].address=nn;
	  //Set Up our new Root
	  memcpy(&tree->root->keys[0],&new_key,sizeof(struct data_t));
	  tree->root->length=2;
	  tn->parent=tree->root;
	  nn->parent=tree->root;
	  root->parent=NULL;
	}
      //We're not the root
      else
	{
	  //Update the parent insert the new node and key
	  AB_Node_Insert(tree,tn->parent,INNER,nn,new_key);
	}
    }
}

int AB_Leaf_Insert(struct ab_tree *tree,struct ab_leaf *tl,struct ab_inner_node *tl_parent,struct record_t data,int index)
{
  AB_Place_Data(tree,tl, data);
  if(tl->length==tree->b)
    {
      struct ab_leaf *nl=AB_Split_Leaf(tree,tl);
      //we need to allocate space  for the  new key
      struct data_t key;
      key.data=malloc(nl->data[0].key.size);
      key.size=nl->data[0].key.size;
      memcpy(key.data,nl->data[0].key.data,nl->data[0].key.size);
      AB_Node_Insert(tree,tl->parent,LEAF,nl,key);
    }
}

int AB_Place_Node(struct ab_tree *tree,struct ab_inner_node *tn,struct ab_inner_node *nn,struct data_t key)
{
  int  i=0;
  //find where to place the node
  for(i=0;i<tn->length-1;i++)
    {
      if(tree->traits.compare(&key,&tn->keys[i])<0)
	{
	  break;
	}
    }
  //Shift ellements to accomodate new node 
  for(int pos=tn->length;pos>i;pos--)
    {
      memcpy(&tn->children[pos],&tn->children[pos-1],sizeof(struct magic_id));
      if(pos!=tn->length)
	memcpy(&tn->keys[pos],&tn->keys[pos-1],sizeof(struct data_t));
    }
  //point to our new node
  tn->children[i+1].address=nn;
  tn->children[i+1].block_id=nn->BID;
  tn->children[i+1].n_t=INNER;
  tn->children[i+1].r_d=RAM;
  //Update key
  memcpy(&tn->keys[i],&key,sizeof(struct data_t));
  tn->length++;
  return i;
}

int AB_Place_Leaf(struct ab_tree *tree,struct ab_inner_node *tn,struct ab_leaf *tl)
{
  int i=0;
  for(i=0;i<tn->length-1;i++)
    {
      if(tree->traits.compare(&tl->data[0].key,&tn->keys[i])<0)
	{
	  break;
	}
    }
  //Shift elements to accomodate new leaf
  for(int pos=tn->length;pos>i;pos--)
    {
      memcpy(&tn->children[pos],&tn->children[pos-1],sizeof(struct magic_id));
      if(pos!=tn->length)
	memcpy(&tn->keys[pos],&tn->keys[pos-1],sizeof(struct data_t));
    }
  //point to our new leaf
  tn->children[i+1].address=tl;
  tn->children[i+1].block_id=tl->BID;
  tn->children[i+1].n_t=LEAF;
  tn->children[i+1].r_d=RAM;
  //Update key make a new copy malloc memory for it and copy int
  tn->keys[i].data=malloc(tl->data[0].key.size);
  tn->keys[i].size=tl->data[0].key.size;
  memcpy(tn->keys[i].data,tl->data[0].key.data,tl->data[0].key.size);
  tn->length++;
  return i;
}

int AB_Place_Data(struct ab_tree *tree,struct ab_leaf *tl,struct record_t data)
{

  int i=0;
  //Find where to place the data
  for(i=0;i<tl->length;i++)
    {
      if(tree->traits.compare(&data.key,&tl->data[i].key)<0)
	{
	  break;
	}
    }
  for(int pos=tl->length;pos>i;pos--)
    {
      memcpy(&tl->data[pos],&tl->data[pos-1],sizeof(struct record_t));
    }  
  memcpy(&tl->data[i],&data,sizeof(struct record_t));
  tl->length++;
  return i;
}


int  AB_Delete(struct ab_tree *tree,struct data_t data)
{
  AB_Inner_Delete(tree,tree->root,NULL,data);
}

int AB_Inner_Delete(struct ab_tree *tree,struct ab_inner_node *tn,struct ab_inner_node *tn_parent,struct data_t data)
{
  int i=0;
  //Find correct node/leaf to insert data in
  for(i=0;i<tn->length-1;i++)
    {      
      if(tree->traits.compare(&data,&tn->keys[i])<0)
	{
	  break;
	}
    }
  if(tn->children[i].n_t==LEAF)
    {      
      AB_Leaf_Delete(tree,tn->children[i].address,tn,data,i);
    }
  else if(tn->children[i].n_t==INNER)
    {
      //Recursion
      AB_Inner_Delete(tree,tn->children[i].address,tn,data);
    }
}

int AB_Node_Position(struct ab_inner_node *tn,void *address)
{
 
  for(int i=0;i<tn->length;i++)
    {
      //possibly cast to uniptr_t just to be safe
      if(tn->children[i].address==address)
	return i;
    }
  return -1;
}

int AB_Node_Delete(struct ab_tree *tree,struct ab_inner_node *tn,int pos)
{
  
  if(tn->children[pos].n_t==LEAF)
    {
      AB_Remove_Leaf(tn,pos);
      tree->num_leaves--;
    }
  else if (tn->children[pos].n_t==INNER)
    {
      AB_Remove_Node(tn,pos);
      tree->num_inner_nodes--;
    }
  //We need to merge
  if(tn->length<tree->a)
    {
      if(tn->parent!=NULL)
	{
	  int index=AB_Node_Position(tn->parent,tn);
	  struct ab_inner_node *merged=NULL;
	  struct data_t merge_key;

	  //find the left node //we have to load to check the length hmm maybe transfer info to the parent
	  if( index-1>=0 && (tn->length+((struct ab_inner_node*)tn->parent->children[index-1].address)->length)<=tree->b)
	    { 
	      //printf("Merging with left node index:%d\n",index);
	      struct ab_inner_node *left=tn->parent->children[index-1].address;
	     
	      memcpy(&merge_key,&tn->parent->keys[index-1],sizeof(struct data_t));
	      void *addr=tn->parent->keys[index-1].data;
	      merged=AB_Merge_Node(left,tn,merge_key);
	      AB_Node_Delete(tree,tn->parent,index-1);
	      free(addr);
	      //removing index
	      
	    }
	  //find the right node
	  else if(index+1<tn->parent->length && (tn->length+((struct ab_inner_node*)tn->parent->children[index+1].address)->length)<=tree->b)
	    {	      
	      // printf("Merging with right node\n"); 
	      struct ab_inner_node *right=tn->parent->children[index+1].address;
	     
	      memcpy(&merge_key,&tn->parent->keys[index],sizeof(struct data_t));
	      //right is index+1
	      void *addr=tn->parent->keys[index].data;
	      merged=AB_Merge_Node(tn,right,merge_key);
	      AB_Node_Delete(tree,tn->parent,index);
	      free(addr);

	    } 
	  //We need to split again:)
	  if(merged!=NULL && merged->length==tree->b)
	    {	
	      
	      struct data_t new_key;
	      memcpy(&new_key,&merged->keys[merged->length/2],sizeof(struct data_t));
	      struct ab_inner_node *node=AB_Split_Node(tree,merged);

	      AB_Node_Insert(tree,merged->parent,INNER,node,new_key);
	    }
	  //if we reached the root's children make the merged node our current root and free the previous one
	   if(merged && merged->parent && merged->parent==tree->root && merged->parent->length==1)
	    {
	      AB_Node_Free(tree->root);
	      free(tree->root);
	      tree->root=merged;
	      tree->num_inner_nodes--;
	      merged->parent=NULL;
	    }
	}
    }
}

int AB_Leaf_Delete(struct ab_tree *tree,struct ab_leaf *tl,struct ab_inner_node *tl_parent,struct data_t data,int index)
{ 
  if(AB_Remove_Data(tree,tl, data)!=-1)
    {
      //We need to merge
      if(tl->length<tree->a)
	{
	  struct ab_leaf *merged=NULL;
	  //left leaf is index -1 // store the length in parent?
	  if( index-1>=0 && (tl->length+((struct ab_leaf*)tl->parent->children[index-1].address)->length)<=tree->b)
	    {
	      struct ab_leaf *left=tl->parent->children[index-1].address;
	      //We have  to delete the inbetween key
	      void *addr=tl->parent->keys[index-1].data;
	      merged=AB_Merge_Leaf(left,tl);
	      AB_Node_Delete(tree,tl->parent,index-1);
	      free(addr); 
	    }
	  else if(index<tl->parent->length-1 && (tl->length+((struct ab_leaf*)tl->parent->children[index+1].address)->length)<=tree->b)
	    {
	      struct ab_leaf *right=tl->parent->children[index+1].address;
	      //We have  to delete the inbetween key
	      void *addr=tl->parent->keys[index].data;
	      merged=AB_Merge_Leaf(tl,right);
	      AB_Node_Delete(tree,tl->parent,index);
	      free(addr);
	    }
	  else
	    {
	      //we're the last remaining leaf we've got no left or right
	      if(tl->length==0)
		{  
		  AB_Remove_Leaf(tl->parent,0); 
		  tree->num_leaves--;
		}
	    }
	  if(merged!=NULL)
	    {
	      if(merged->length==tree->b)
		{
		  //we need to split the leaf again :)
		  struct ab_leaf *leaf=AB_Split_Leaf(tree,merged);
		  struct data_t key;
		  key.data=malloc(leaf->data[0].key.size);
		  key.size=leaf->data[0].key.size;
		  memcpy(key.data,leaf->data[0].key.data,key.size);
		  AB_Node_Insert(tree,merged->parent,LEAF,leaf,key);
		}
	    }
	}
      
    }
}

int AB_Remove_Node(struct ab_inner_node *tn,int pos)
{
  AB_Node_Free(tn->children[pos].address);
  free(tn->children[pos].address);
   for(int i=pos;i<tn->length-1;i++)
    {
      memcpy(&tn->children[i],&tn->children[i+1],sizeof(struct magic_id));
      if(i!=tn->length-2)
	{
	  memcpy(&tn->keys[i],&tn->keys[i+1],sizeof(struct data_t));
	}
    }  
  tn->length--;
}

int AB_Remove_Leaf(struct ab_inner_node *tn,int pos)
{ 
   AB_Leaf_Free(tn->children[pos].address);
   free(tn->children[pos].address);

  for(int i=pos;i<tn->length-1;i++)
    {
      memcpy(&tn->children[i],&tn->children[i+1],sizeof(struct magic_id));
      if(i!=tn->length-2)
	memcpy(&tn->keys[i],&tn->keys[i+1],sizeof(struct data_t));
    }

  tn->length--;
}

int AB_Remove_Data(struct ab_tree *tree,struct ab_leaf *tl,struct data_t data)
{
  int i=0;
  //find the position of data to remove if any
  for(i=0;i<tl->length;i++)
    {
      if(tree->traits.compare(&data,&tl->data[i].key)==0)
	{
	  //let the caller deallocate return a pointer to the record 	  
	  tree->traits.dealloc(tl->data[i]);
	  break;
	}
    }
  
  //if there is no item return -1 making no changes
  if(i>=tl->length)
    return -1;
  
  // shift elements left
  for(int pos=i;pos<tl->length-1;pos++)
    {
      memcpy(&tl->data[pos],&tl->data[pos+1],sizeof(struct record_t));
    }
  tl->length--;
  //printf("Final Leaf Length:%d\n",tl->length);
  return 0;
}

struct ab_leaf* AB_Merge_Leaf(struct ab_leaf *left,struct ab_leaf *right)
{
  //Move all elements to right;
  for(int i=right->length-1;i>=0;i--)
    {
      memcpy(&right->data[left->length+i],&right->data[i],sizeof(struct record_t));
    }
  for(int i=0;i<left->length;i++)
    {
      memcpy(&right->data[i],&left->data[i],sizeof(struct record_t));
    }
  
  right->length+=left->length;
  return right;
}

struct ab_inner_node* AB_Merge_Node(struct ab_inner_node *left,struct ab_inner_node *right,struct data_t key)
{
  
  for(int i=right->length-1;i>=0;i--)
    {      
      memcpy(&right->children[left->length+i],&right->children[i],sizeof(struct magic_id));
      if(i!=right->length-1)
	{
	  
	  memcpy(&right->keys[left->length+i],&right->keys[i],sizeof(struct data_t));
	}
    }
  for(int i=0;i<left->length;i++)
    {
      //set up new parrent
      if(left->children[i].n_t==LEAF)
	{
	  ((struct ab_leaf*)left->children[i].address)->parent=right;
	}
      else
	{
	  ((struct ab_inner_node*)left->children[i].address)->parent=right;
	}

      memcpy(&right->children[i],&left->children[i],sizeof(struct magic_id));
      if(i!=left->length-1)
	{
	  
	  memcpy(&right->keys[i],&left->keys[i],sizeof(struct data_t));
	}
    }

  right->keys[left->length-1].data=malloc(key.size);
  right->keys[left->length-1].size=key.size;
  memcpy(right->keys[left->length-1].data,key.data,key.size);
  right->length+=left->length;
  return right;
}

void print_leaf_stats(struct ab_leaf *tl,data_describe data_desc)
{
  printf("Node Address: %p\n",tl);
  printf( "Node Block ID: %zu\n",tl->BID);
  printf("Node Num Elements: %zu\n",tl->length);

  for(int i=0;i<tl->length;i++)
    {
      char *desc=data_desc(tl->data[i]);
      printf("%dth description: %s\n",i,desc);
      free(desc);
   }
}

void print_node_stats(struct ab_inner_node *tn)
{
  printf("Node Address: %p\n",tn);
  printf( "Node Block ID: %zu\n",tn->BID);
  if(tn->parent!=NULL)
    printf( "Parent Block ID %zu\n",tn->parent->BID);
  printf("Node Num Elements: %zu\n",tn->length);
  for(int i=0;i<tn->length;i++)
    {
      printf("Child %zu Block ID: %zu\n",i,tn->children[i].block_id);
      if(tn->children[i].r_d==RAM)
	printf("Child %zu Location: RAM\n",i);
      else	
	printf("Child %zu Location: DISK\n",i);
      if(tn->children[i].n_t==INNER)
	printf("Child %zu Node Type:INNER\n",i);
      else
	printf("Child %zu Node Type:LEAF\n",i);
      if(i<tn->length-1)
	printf("Child %zu Description:%s\n",i,tn->keys[i].data);
      else
	printf("Child %zu Description: larger than the rest\n",i);
      printf("Child %zu Adress: %p\n",i,tn->children[i].address);
    }
}

void minimal_node_stats(struct ab_inner_node *tn)
{
  printf( "Node Block ID: %zu\n",tn->BID);
  for(int i=0;i<tn->length;i++)
    {
      printf("Child %zu\n",i);
      printf("Block ID: %zu\n",tn->children[i].block_id);
      if(i<tn->length-1)
	printf("Description:%d\n",*(int*)tn->keys[i].data);
      else
	printf("Description: larger than the rest\n");
    }
}

void TraverseTree(struct ab_tree *tree,data_describe data_desc)
{
  printf("Tree Root Address: %p\n",tree->root);
  //    unsigned long height;TAKE NOTE 
  printf("Num Inner Nodes: %zu\n",tree->num_inner_nodes);
  printf("Num Leaves: %zu\n",tree->num_leaves);
  printf("A:%u,B:%u\n",tree->a,tree->b);
  TraverseNode(tree->root,data_desc);

}

void TraverseNode(struct ab_inner_node *tn,data_describe data_desc)
{
  print_node_stats(tn);
  for(int i=0;i<tn->length;i++)
    {
      if(tn->children[i].n_t==INNER)
	TraverseNode(tn->children[i].address,data_desc);
      else
	{
	  print_leaf_stats(tn->children[i].address,data_desc);
	}
    }
}
