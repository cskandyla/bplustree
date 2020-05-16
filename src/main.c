#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "ab-tree.h"

//bad array shuffle function used for testing
void shuffle(int *array, int array_size, int shuff_size)
{   
  if (array_size > 1)  
    {   
      size_t i;
      for (i = 0; i < shuff_size - 1; i++) 
	{   
	  size_t j = i + rand() / (RAND_MAX / (array_size - i) + 1); 
	  int t = array[j];
	  array[j] = array[i];
	  array[i] = t;
	}   
    }   
}   

//string comparator
int str_compare(struct data_t *str1,struct data_t *str2)
{
  return strcmp(str1->data,str2->data);
}

struct record_t* search_str(struct ab_tree *tree,int value)
{
  char *str=malloc(20);
  sprintf(str,"%d\0",value);
  struct data_t data;
  data.data=str;
  data.size=20;
  struct record_t* ret=AB_Search(tree,data);
  free(str);
  return ret;
}

struct foo
{
  int num;
  char *str;
  void *buff;
  
};

void free_foo(struct record_t foo_rec)
{
  //ee(foo_rec.key.data);
  struct foo *fooz=foo_rec.value.data;
  free(fooz->str);
  if(fooz->buff!=NULL)
    {
      free(fooz->buff);
    }
  free(foo_rec.value.data);
  
}

void Insert_foo(struct ab_tree *tree,struct foo *fooz)
{
  struct record_t record;
  record.value.data=malloc(sizeof(struct foo));
  memcpy(record.value.data,fooz,sizeof(struct foo));;
  record.value.size=sizeof(struct foo);

  // record.key.data=malloc(20);
  // memcpy(record.key.data,fooz->str,20);
  // record.key.data=malloc(sizeof(int));
  //memcpy(&record.key.data,&fooz->num,sizeof(int));
  record.key.data=fooz->str;
  record.key.size=20;
    //printf("FOOZ STR %s len %d\n",fooz->str,strlen(fooz->str));
  AB_Insert(tree,record);
}
void build_foo(struct ab_tree *tree,int max)
{
    int *arr=malloc(max*sizeof(int));
  for(int i=0;i<max;i++)
    {
      arr[i]=i;
    }
  
  shuffle(arr,max-1,max-1);
  for(int i=0;i<max;i++)
   {
     //insert_int(tree,i,i);
     struct foo fooz;
     fooz.num=i;
     
       fooz.str=malloc(20);
       sprintf(fooz.str,"%d\0",i);
       fooz.buff=NULL;
     Insert_foo(tree,&fooz);
      printf("Inserting:%d\r",i);
    }
  printf("\n");
}

int validate_foo(struct ab_tree *tree,int max)
{
 
  int count_fails=0;
  for(int i=0;i<max;i++)
    {
      printf("Searching:%d\r",i);
      struct record_t *rec=search_str(tree,i);
      if(rec==NULL)
	{ 
	  printf("\nFAILED TO FIND %d %d\n",i,++count_fails);
	  // return 0;
	}
      else
	{
	  struct foo *fooz=rec->value.data;
	  //printf("\nGOT:%d %s \n",fooz->num,fooz->str);
	}
    }
  printf("\n");
  return 1;
}

int delete_test_foo()
{
  

}


void insert_str(struct ab_tree *tree,int key, int val)
{
  struct record_t data;
  data.value.data=malloc(20);
  data.key.data=malloc(20);
  data.value.size=20;
  data.key.size=20;
  sprintf(data.value.data,"%d\0",val);
  sprintf(data.key.data,"%d\0",key);
  
  AB_Insert(tree,data);
}



void delete_str(struct ab_tree *tree,int value)
{
  char *str=malloc(20);
  sprintf(str,"%d\0",value);
  struct data_t data;
  data.data=str;
  data.size=20;
  AB_Delete(tree,data);
  free(str);
}


int build_str(struct ab_tree *tree,int max)
{
  int arr[max];
  for(int i=0;i<max;i++)
    {
      arr[i]=i;
    }
  
  shuffle(arr,max-1,max-1);
  for(int i=0;i<max;i++)
   {
     //insert_int(tree,i,i);
     insert_str(tree,i,i);
      printf("Inserting:%d\r",i);
    }
  printf("\n");
}

int deletion_test_str(struct ab_tree *tree,int max)
{
  int arr[max];
  for(int i=0;i<max;i++)
    {
      arr[i]=i;
    }
  
  shuffle(arr,max-1,max-1);

  for(int i=0;i<max;i++)
    {
      delete_str(tree,arr[i]);
      printf("Deleting:%d\r",i);
    }
  printf("\n");

}

int validate_str(struct ab_tree *tree,int max)
{
 
  int count_fails=0;
  for(int i=0;i<max;i++)
    {
      printf("Searching:%d\r",i);
      struct record_t *rec=search_str(tree,i);
      if(rec==NULL)
	{ 
	  printf("\nFAILED TO FIND %d %d\n",i,++count_fails);
	  // return 0;
	}
      else
	{
	  char *str=rec->value.data;
	  printf("\nGOT:%s\n",str);
	}
    }
  printf("\n");
  return 1;
}



int generic_insert(struct ab_tree *tree,void *value,int value_length,void *key,int key_length)
{
  struct record_t data;
  data.value.data=malloc(value_length);
  data.key.data=malloc(key_length);
  data.value.size=value_length;
  data.key.size=key_length;
  memcpy(data.value.data,value,value_length);
  memcpy(data.key.data,key,key_length);
  AB_Insert(tree,data);
}

int int_compare(struct data_t *int1,struct data_t *int2)
{

  return *(int*)int1->data-*(int*)int2->data;
}

char* int_ptr_description(struct record_t data)
{
  char *alloc=malloc(100);
  sprintf(alloc,"Int Value is %d pointer:%p",*(int*)data.value.data,data.value.data);
  return alloc;
}

char* char_description(struct record_t data)
{
  char *alloc=malloc(100);
  sprintf(alloc,"Str:%s pointer:%p",data.value.data,data.value.data);
  return alloc;
}

char* foo_desc(struct record_t foo_rec)
{
  struct foo *fooz=foo_rec.value.data;
  char *alloc=malloc(100);
  sprintf(alloc,"Foo Int:%d pointer:%p",fooz->num,&fooz->num);
  return alloc;
}

void insert_int(struct ab_tree *tree,int key, int val)
{
  struct record_t data;
  data.value.data=malloc(sizeof(int));
  data.key.data=malloc(sizeof(int));
  data.value.size=sizeof(int);
  data.key.size=sizeof(int);
  memcpy(data.value.data,&val,sizeof(int));
  memcpy(data.key.data,&key,sizeof(int));
  AB_Insert(tree,data);
}

void delete_int_ptr(struct ab_tree *tree,int value)
{
  int *val_ptr=malloc(sizeof(int));
  *val_ptr=value;
  struct data_t data;
  data.data=val_ptr;
  data.size=sizeof(int);
  AB_Delete(tree,data);
  free(val_ptr);
}

struct record_t* search_int_ptr(struct ab_tree *tree,int value)
{
  int *val_ptr=malloc(sizeof(int));
  *val_ptr=value;
  struct data_t data;
  data.data=val_ptr;
  data.size=sizeof(int);
  struct record_t* ret=AB_Search(tree,data);
  free(val_ptr);
  return ret;
}

int build(struct ab_tree *tree,size_t max)
{
  
    int *arr=malloc(max*sizeof(int));
    for(int i=0;i<max;i++)
    {
	arr[i]=(int)i;
    }
    
    shuffle(arr,max-1,max-1);
    for(int i=0;i<max;i++)
    {
	//insert_int(tree,i,i);
	char string[20];
	sprintf(string,"Value%d",i);
	generic_insert(tree,string,20,&i,sizeof(int));
	printf("Inserting:%d\r",arr[i]);
    }
    printf("\n");
    free(arr);
}

int deletion_test(struct ab_tree *tree,int max)
{
  int *arr=malloc(max*sizeof(int));
  for(int i=0;i<max;i++)
    {
      arr[i]=i;
    }
  
  shuffle(arr,max-1,max-1);

  for(int i=0;i<max;i++)
    {
      delete_int_ptr(tree,arr[i]);
      printf("Deleting:%d\r",i);
    }
  printf("\n");
  free(arr);
}

int validate(struct ab_tree *tree,int max)
{
 
  int count_fails=0;
  for(int i=0;i<max;i++)
    {
      printf("Searching:%d\r",i);
      struct record_t *rec=search_int_ptr(tree,i);
      if(rec==NULL)
	{ 
	  printf("\nFAILED TO FIND %d %d\n",i,++count_fails);
	  // return 0;
	}
      else
	{
	}
    }
  printf("\n");
  return 1;
}

int main(int argc, char *argv[])
{
  srand(time(NULL));
  struct ab_tree tree;
  struct ab_tree_traits tree_traits;
  AB_Create(&tree,2,5,AB_Default_Traits(int_compare),"tree");
  int max=1000000;
  build(&tree,max);
  validate(&tree,max);
  deletion_test(&tree,max);
  AB_Destroy(&tree);
  return 0;
}

