/* name: ParkNaHyeon, 20160233
assignment: 3
name of file: customer_manager2_hashexpansion.c */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "customer_manager.h"

#define UNIT_ARRAY_SIZE 1024
enum {HASH_MULTIPLIER = 65599};

/*------------------------------------------------------------------
 <struct UserInfo>
: contains char* name, char* id, int purchase, struct UserInfo* ptr_id,
struct UserInfo* ptr_name
------------------------------------------------------------------*/
struct UserInfo {
  char *name;                // customer name
  char *id;                  // customer id
  int purchase;              // purchase amount (> 0)
  struct UserInfo* ptr_id; // points next UserInfo in ID hash table
  struct UserInfo* ptr_name; // points next UserInfo in NAME hash table
};

/*------------------------------------------------------------------
 <struct DB>
: contains struct UserInfo** pArray_id, struct UserInfo** pArray_name,
int curArrSize, int count_id, int count_name, int flag, int dont_annoy
------------------------------------------------------------------*/
struct DB {
  struct UserInfo** pArray_id;// pointer to the hash table by ID
  struct UserInfo** pArray_name;//pointer to hash table by NAME
  struct UserInfo** old_id;// pointer to ID hash table before expansion
  struct UserInfo** old_name;// pointer to NAME hash table before expansion
  int curArrSize;// current array size (max # of elements)
  int count_id; // number of unempty ID hash table elements
  int count_name; // number of unempty NAME hash table elements
  int flag;	     // flag to know whether the array should be expanded
  int dont_annoy;    /* flag to prevent another hash table expansion while
                      rearranging the UserInfos with new hash_function 
		      results */
};
  
/*------------------------------------------------------------------
 <hash_function>

parameter: pcKey (type: const char*), iBucketCount (type: int)
return value: (int)(uiHash % (unsigned int)iBucketCount) (type:int)

This function returns a hash code for pcKey that is between 0
and iBucketCount-1.
------------------------------------------------------------------*/
static int hash_function(const char *pcKey, int iBucketCount)
{
   int i;
   unsigned int uiHash = 0U;
   for (i = 0; pcKey[i] != '\0'; i++)
      uiHash = uiHash * (unsigned int)HASH_MULTIPLIER
               + (unsigned int)pcKey[i];
   return (int)(uiHash % (unsigned int)iBucketCount);
}

/*------------------------------------------------------------------
<CreateCustomerDB>

parameter: receives nothing (type:void)
return value: d or NULL
d if it creates the DB successfully.
NULL if d is NULL or failed to calloc d->pArray_id or d->pArray_name

This function creates struct DB and sets the initial values needed.
------------------------------------------------------------------*/
DB_T
CreateCustomerDB(void)
{
  DB_T d;
  
  d = (DB_T) calloc(1, sizeof(struct DB));
  if (d == NULL) {
    fprintf(stderr, "Can't allocate a memory for DB_T\n");
    return NULL;
  }
  d->curArrSize = UNIT_ARRAY_SIZE; // start with 1024 elements
  d->pArray_id = (struct UserInfo**)calloc(d->curArrSize,
               sizeof(struct UserInfo*));
  d->pArray_name=(struct UserInfo**)calloc(d->curArrSize,
               sizeof(struct UserInfo*));
  d->count_id=0;
  d->count_name=0;
  d->flag=0;
  d->dont_annoy=0;
  if (d->pArray_id == NULL) {
    fprintf(stderr, "Can't allocate a memory for array of size %d\n",
	    d->curArrSize);   
    free(d);
    return NULL;
  }
  if (d->pArray_name == NULL) {
    fprintf(stderr, "Can't allocate a memory for array of size %d\n",
	    d->curArrSize);   
    free(d);
    return NULL;
  }
  return d;
}

/*------------------------------------------------------------------
 <UnlinkUserInfos>

parameter: p (type: struct UserInfo*)
return value: none (type: void)

This function is implemented to make DestroyCustomerDB simpler.
This function unlinks and frees the UserInfos in specific element of 
hash table(ID or NAME). 
-------------------------------------------------------------------*/
void
UnlinkUserInfos(struct UserInfo* p)
{
  struct UserInfo* nextp=p->ptr_id;
  for(;nextp!=NULL;p=nextp)
  {
    nextp=p->ptr_id;
    free(p);
  }
}

/*------------------------------------------------------------------
<DestroyCustomerDB>

parameter: d (type: DB_T)
return value: none (type: void)

This function Destroys all structure and free all memorys that were
occupied by d.
------------------------------------------------------------------*/
void
DestroyCustomerDB(DB_T d)
{
  int i;
  struct UserInfo** p_id=d->pArray_id;
  struct UserInfo** p_name=d->pArray_name;
  if(d==NULL) return;
  for(i=0;i<=(d->curArrSize);i++)
  {
    if(p_id[i]==NULL) continue;
    UnlinkUserInfos(p_id[i]);
  }
  free(p_id);
  free(p_name);
  p_id=NULL;
  p_name=NULL;
  free(d);
  d=NULL;
  return;
}

/*------------------------------------------------------------------
 <Search_ID>

parameter: search_id (type: struct UserInfo*), id (type: const char*)
return value: NULL or prev (type: struct UserInfo*)
returns NULL if id is not found in the node that search_id is given.
returns prev(the previous pointer to searching UserInfo) if the Userinfo 
that this function is searching is found.

This function is implemented to make RegisterCustomer, UnregisterCustom
erbyID, GetPurchaseByID easier by searching id of each hast table element.
This function gets search_id and finds the UserInfo that has the id
given as parameter and returns previous pointer to UserInfo.
------------------------------------------------------------------*/
struct UserInfo*
Search_ID(struct UserInfo* search_id, const char* id)
{
  struct UserInfo* p;
  struct UserInfo* prev=search_id;
  
  for(p=prev->ptr_id;p!=NULL;p=p->ptr_id)
  {
    if(strcmp(p->id,id)==0) return prev;
    prev=p;
  }
  return NULL;
}

/*------------------------------------------------------------------
<Search_NAME>

parameter: search_name (type: struct UserInfo*), name (type: const char*)
return value: NULL or prev (type: struct UserInfo*)
returns NULL if name is not found in the node that search_name is given.
returns prev(the previous pointer to searching UserInfo) if the Userinfo 
that this function is searching is found.

This function is implemented to make RegisterCustomer, UnregisterCustom
erbyNAME, GetPurchaseByNAME easier by searching name of each hast table element.
This function gets search_name and finds the UserInfo that has the id
given as parameter and returns previous pointer of searched UserInfo.
------------------------------------------------------------------*/
struct UserInfo*
Search_NAME(struct UserInfo* search_name, const char* name)
{
  struct UserInfo* p;
  struct UserInfo* prev=search_name;
  
  for(p=prev->ptr_name;p!=NULL;p=p->ptr_name)
  {
    if(strcmp(p->name,name)==0) return prev;
    prev=p;   
  }
  return NULL;
}

/*------------------------------------------------------------------
<Hash_EXPANSION>

parameter: d (type: DB_T)
return value: -1 or 0 or 1 
-1 if calloc of d->old_id or d->old_id fails
0 if it successfully expands hash table
1 if d->curArrsize(size of current hash table) is 1048576 which
now cannot expand anymore

This function expands the hash table by calloc.
The reason that it does calloc to d->old_id and d->old_name although
it has to be d->pArray_id and d->pArray_name is to swap later.
If it doesn't then, we will lose the memory of d->pArray_id and 
d->pArray_name before expansion. So, I chose to swap these two
pointers in RegisterCustomer Function
------------------------------------------------------------------*/
int
Hash_EXPANSION(DB_T d)
{ 
  if((d->curArrSize)==1048576) return 1;
  d->curArrSize=(d->curArrSize)*2;
  d->old_id=(struct UserInfo**)calloc((d->curArrSize),sizeof(struct UserInfo*));
  d->old_name=(struct UserInfo**)calloc((d->curArrSize),sizeof(struct UserInfo*));
  (d->count_id)=0;
  (d->count_name)=0;
  
  if(((d->old_id)==NULL)||((d->old_name)==NULL)) return(-1);
  
  return 0;
}

/*------------------------------------------------------------------
<RegisterCustomer>

parameter: d (type: DB_T), id (type: const char*), name (type: const
char*), purchase (type: int)
return value: 0 or -1
0 when it registers successfully.
-1 when d or id or name is NULL, purchase is equal or smaller than zero,
when there already exists UserInfo that has same name or id given.

This function registers the new customer by forming new struct UserInfo*
and setting each element in UserInfo.

--HASH TABLE EXPANSION----------------------------------------------
prints out to standard error stream if the result of hash_EXPANSION
which is expand (type: int) is not zero (1 or -1)

if d->flag is 1 and d->dont_annoy is 0, it does hash table expansion
by calling the function 'hash_EXPANSION'.

------------------------------------------------------------------*/
int
RegisterCustomer(DB_T d, const char *id,
		 const char *name, const int purchase)
{
  struct UserInfo* new=(struct UserInfo*)calloc(1,sizeof(struct UserInfo));
  struct UserInfo** search_id=d->pArray_id;
  struct UserInfo** search_name=d->pArray_name;
  struct UserInfo** temp_id; // Swaps d->old_id and d->pArray_id
  struct UserInfo** temp_name; // Swaps d->old_name and d->pArray_name
  struct UserInfo* p_id;
  struct UserInfo* next_p_id;
  int h_id=hash_function(id,d->curArrSize);
  int h_name=hash_function(name,d->curArrSize);
  int i;
  int expand;
  int save; // to save the d->curArrSize before expansion
  
  if(d==NULL) return (-1);
  if(id==NULL) return (-1);
  if(name==NULL) return (-1);
  if((purchase==0)||(purchase<0)) return (-1);
  
  /*HASH TABLE EXPANSION*/
  if(((d->flag)==1)&&(d->dont_annoy)==0){
    save=(d->curArrSize);
    if((d->dont_annoy)==0){
      expand=Hash_EXPANSION(d);
      if(expand==1) fprintf(stderr,"Cannot Expand anymore: maxinum size\n");
      if(expand==(-1)) fprintf(stderr,"Calloc failed\n");
      d->dont_annoy=1;
    }
    d->flag=0;
    
    /*swap*/
    temp_id=(d->old_id);
    d->old_id=(d->pArray_id);
    d->pArray_id= temp_id;
    
    temp_name=(d->old_name);
    d->old_name=(d->pArray_name);
    d->pArray_name=temp_name;
    temp_id=NULL;
    temp_name=NULL;
    
    /*Register again with new hash function result*/
    for(i=0;i<save;i++){
      if(d->old_id[i]==NULL) continue;
      p_id=d->old_id[i];
      next_p_id=p_id->ptr_id;
      if(next_p_id==NULL){
	RegisterCustomer(d,p_id->id,p_id->name,p_id->purchase);
	free(p_id);
	continue;
      }
      while(next_p_id!=NULL){
	RegisterCustomer(d,p_id->id,p_id->name,p_id->purchase);
	free(p_id);
	p_id=next_p_id;
	next_p_id=next_p_id->ptr_id;
      }
      RegisterCustomer(d,p_id->id,p_id->name,p_id->purchase);
      free(p_id);
    }
    /* Make d->dont_annoy to zero and free d->old_id and d->old_name*/
    (d->dont_annoy)=0;
    free(d->old_id);
    free(d->old_name);
  }
  
  /*Initialize again (because of hash table expansion)*/
  search_id=d->pArray_id;
  search_name=d->pArray_name;
  h_id=hash_function(id,d->curArrSize);
  h_name=hash_function(name,d->curArrSize);
    
  /*search if same id/name already exists*/
  for(i=0;i<(d->curArrSize);i++){    
    if(search_id[i]==NULL) continue;
    if(strcmp(search_id[i]->id,id)==0) return (-1);
    if(Search_ID(search_id[i],id)!=NULL) return (-1);
  }
 
  for(i=0;i<(d->curArrSize);i++){
    if(search_name[i]==NULL) continue;
    if(strcmp(search_name[i]->name,name)==0) return (-1);
    if(Search_NAME(search_name[i],name)!=NULL) return (-1);
  }

  /*fill in name,id,purchase,ptr_id,ptr_name*/
  new->name=strdup(name);
  new->id=strdup(id);
  new->purchase=purchase;
  new->ptr_id=d->pArray_id[h_id];
  new->ptr_name=d->pArray_name[h_name]; 
  
  /*implement in hash table*/
  d->pArray_id[h_id]=new;
  d->pArray_name[h_name]=new;

  /*check hash_expansion*/
  if(new->ptr_id==NULL) (d->count_id)++;
  if(new->ptr_name==NULL) (d->count_name)++;
  if(((d->count_id)>=(d->curArrSize)*0.75)||((d->count_name)>=(d->curArrSize)*0.75)){
      d->flag=1;   
  }
  return 0;
}

/*------------------------------------------------------------------
 <UnregisterCustomerByID>

parameter: d (type: DB_T), id (type: const char*)
return value: 0 or -1 (type: int)
0 when it successfully unregisters UserInfo.
-1 when d or id is NULL, when no UserInfo of given id is found.

This function unregisters the UserInfo which has give id by freeing
all the elements that UserInfo contains and links the previous and
next UserInfo of each hash table if it exists.
------------------------------------------------------------------*/
int
UnregisterCustomerByID(DB_T d, const char *id)
{
  struct UserInfo** delete_id=d->pArray_id;
  struct UserInfo** delete_name=d->pArray_name;
  struct UserInfo* prev_p;
  struct UserInfo* p;
  struct UserInfo* next_p;
  struct UserInfo* prev_t;
  struct UserInfo* t;
  struct UserInfo* save;
  int i;
  int h_name;
  
  if(d==NULL) return(-1);
  if(id==NULL) return(-1);

  for(i=0;i<(d->curArrSize);i++){
    
    if(delete_id[i]==NULL) continue;
    /*if there is only one UserInfo in one hash element*/
    if(strcmp(delete_id[i]->id,id)==0){
      /*find hash-name for same id useritem and link again*/
      h_name=hash_function(delete_id[i]->name,d->curArrSize);
      if(strcmp(delete_name[h_name]->name,delete_id[i]->name)==0){
	delete_name[h_name]=delete_name[h_name]->ptr_name;
	/*DECREASE d->count_id TO CHECK HASH EXPANSION LATER*/
	if(delete_name[h_name]==NULL) (d->count_name)--;
      }
      else{
	prev_t=Search_NAME(delete_name[h_name],delete_id[i]->name);
	t=prev_t->ptr_name;
        prev_t->ptr_name=t->ptr_name;
      }
      
      /*delete id hash*/
      free(delete_id[i]->id);
      free(delete_id[i]->name);
      save=delete_id[i];
      delete_id[i]=save->ptr_id;
      
      /*DECREASE d->count_id TO CHECK HASH EXPANSION LATER*/
      if(delete_id[i]==NULL) (d->count_id)--;
      free(save);
      return 0;
    }
    /*if there are multiple UserInfo per hash*/
    prev_p=Search_ID(delete_id[i],id);
    if(prev_p!=NULL) break;
  }
  if(i==(d->curArrSize)) return(-1);

  /*set p,next_p*/
  p=prev_p->ptr_id;
  next_p=p->ptr_id;

  /*link name*/
  h_name=hash_function(p->name,d->curArrSize);
  if(strcmp(delete_name[h_name]->name,p->name)==0){
	delete_name[h_name]=NULL;
	/*DECREASE d->count_id TO CHECK HASH EXPANSION LATER*/
	(d->count_name)--;
  }
  else{
    prev_t=Search_NAME(delete_name[h_name],p->name);
    t=prev_t->ptr_name;
    prev_t->ptr_name=t->ptr_name;
  }

  /*free id*/
  if(next_p==NULL){
    free(p->id);
    free(p->name);
    free(p);
    prev_p->ptr_id=NULL;
  }
  else{
    free(p->id);
    free(p->name);
    prev_p->ptr_id=p->ptr_id;
    free(p);
  }
  return (0);
}

/*------------------------------------------------------------------
 <UnregisterCustomerByNAME>

parameter: d (type: DB_T), name (type: const char*)
return value: 0 or -1 (type: int)
0 when it successfully unregisters UserInfo.
-1 when d or name is NULL, when no UserInfo of given id is found.

This function unregisters the UserInfo which has give name by freeing
all the elements that UserInfo contains and links the previous and
next UserInfo of each hash table if it exists.
------------------------------------------------------------------*/
int
UnregisterCustomerByName(DB_T d, const char *name)
{
  struct UserInfo** delete_id=d->pArray_id;
  struct UserInfo** delete_name=d->pArray_name;
  struct UserInfo* prev_p;
  struct UserInfo* p;
  struct UserInfo* next_p;
  struct UserInfo* prev_t;
  struct UserInfo* t;
  struct UserInfo* save;
  int i;
  int h_id;
  
  if(d==NULL) return(-1);
  if(name==NULL) return(-1);
  for(i=0;i<(d->curArrSize);i++){
    if(delete_name[i]==NULL) continue;
    /*if there is only one useritem per one hash element*/
    if(strcmp(delete_name[i]->name,name)==0){
      /*find hash-id for same name useritem and link again*/
      h_id=hash_function(delete_name[i]->id,d->curArrSize);
      
      if(strcmp(delete_id[h_id]->id,delete_name[i]->id)==0){
	delete_id[h_id]=delete_id[h_id]->ptr_id;
	/*DECREASE d->count_name TO CHECK HASH EXPANSION LATER*/
	if(delete_id[h_id]==NULL) (d->count_id)--;
      }
      else{
	prev_t=Search_ID(delete_id[h_id],delete_name[i]->id);
	t=prev_t->ptr_id;
        prev_t->ptr_id=t->ptr_id;
      }
      /*delete name hash*/
      free(delete_name[i]->name);
      free(delete_name[i]->id);
      save=delete_name[i];
      delete_name[i]=save->ptr_name;
      /*DECREASE d->count_name TO CHECK HASH EXPANSION LATER*/
      if(delete_name[i]==NULL) (d->count_name)--;
      free(save);
      return 0;
    }
    /*if multiple useritem per hash*/
    prev_p=Search_NAME(delete_name[i],name);
    
    if(prev_p!=NULL) break;
  }
  if(i==(d->curArrSize)) return(-1);
  /*set p,next_p*/
  p=prev_p->ptr_name;
  next_p=p->ptr_name;
  
  /*link id*/
  h_id=hash_function(p->id,d->curArrSize);
  if(strcmp(delete_id[h_id]->id,p->id)==0){
	delete_id[h_id]=NULL;
	/*DECREASE d->count_name TO CHECK HASH EXPANSION LATER*/
	(d->count_id)--;
  }
  else{
    prev_t=Search_ID(delete_id[h_id],p->id);
    t=prev_t->ptr_id;
    prev_t->ptr_id=t->ptr_id;
  }

  /*Delete UserInfo*/
  if(next_p==NULL){
    free(p->id);
    free(p->name);
    free(p);
    prev_p->ptr_name=NULL;
  }
  else{
    free(p->id);
    free(p->name);
    prev_p->ptr_name=p->ptr_name;
    free(p);
  }
  return 0;
}

/*------------------------------------------------------------------
 <GetPurchaseByID>

parameter: d (type: DB_T), id (type: const char*)
return value: (prev_ptr->ptr_id)->purchase or -1
0 when it successfully finds purchase amount of id given.
-1 if d or id is NULL, when there is no such UserInfo that contains
given id.

This function finds the purchase amount of UserInfo of given id.
It gets previous pointer of UserInfo that it is searching by using
the Search_ID function stated above.
------------------------------------------------------------------*/
int
GetPurchaseByID(DB_T d, const char* id)
{
  int i;
  struct UserInfo** search_id=d->pArray_id;
  struct UserInfo* prev_ptr;
  
  if(d==NULL) return (-1);
  if(id==NULL) return (-1);
  
  for(i=0;i<(d->curArrSize);i++)
  {
    if(search_id[i]==NULL) continue;
    if(strcmp(search_id[i]->id,id)==0) return (search_id[i]->purchase);
    prev_ptr=Search_ID(search_id[i],id);
    if(prev_ptr!=NULL) return ((prev_ptr->ptr_id)->purchase);

  }
 
  return (-1);
}

/*------------------------------------------------------------------
 <GetPurchaseByNAME>

parameter: d (type: DB_T), name (type: const char*)
return value: (prev_ptr->ptr_name)->purchase or -1
0 when it successfully finds purchase amount of name given.
-1 if d or id is NULL, when there is no such UserInfo that contains
given name.

This function finds the purchase amount of UserInfo of given name.
It gets previous pointer of UserInfo that it is searching by using
the Search_NAME function stated above.
------------------------------------------------------------------*/
int
GetPurchaseByName(DB_T d, const char* name)
{
  int i;
  struct UserInfo** search_name=d->pArray_name;
  struct UserInfo* prev_ptr;
  
  if(d==NULL) return (-1);
  if(name==NULL) return (-1);

  for(i=0;i<(d->curArrSize);i++){
    
    if(search_name[i]==NULL) continue;
    if(strcmp(search_name[i]->name,name)==0) return (search_name[i]->purchase);
    prev_ptr=Search_NAME(search_name[i],name);
    if(prev_ptr!=NULL) return ((prev_ptr->ptr_name)->purchase);
  }
  return (-1);
}

/*------------------------------------------------------------------
 <GETSumCustomerPurchase>

parameter: d (type: DB_T), fp (type: FUNCPTR_T)
return value: sum or -1 (type: int)
returns -1 when d or fp id NULL
returns sum if when sum is obtained.

This function adds all the results of fp and returns the sum of them.
------------------------------------------------------------------*/
int
GetSumCustomerPurchase(DB_T d, FUNCPTR_T fp)
{
  struct UserInfo** get_id=d->pArray_id;
  struct UserInfo* ptr;
  int i;
  int sum=0;
  /* fill out this function */
  if(d==NULL) return (-1);
  if(fp==NULL) return (-1);
  
  for(i=0;i<(d->curArrSize);i++){
    
    if(get_id[i]==NULL) continue;
    for(ptr=get_id[i];ptr!=NULL;ptr=ptr->ptr_id){
      
      sum+=fp(ptr->id,ptr->name,ptr->purchase);
    }
  }
  
  return (sum);
}


