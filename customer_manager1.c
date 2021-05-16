/* Name: 20160233 ParkNaHyeon 
 assignment number: 3  
file: customer_manager2.c*/

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "customer_manager.h"

#define UNIT_ARRAY_SIZE 1024 

/*------------------------------------------------------------------
 <struct UserInfo>
: contains char* name, char* id, int purchase
------------------------------------------------------------------*/
struct UserInfo {
  char *name;   // customer name
  char *id;   // customer id
  int purchase;   // purchase amount (> 0)
};

/*------------------------------------------------------------------
<struct DB>
: contains struct UserInfo *pArray, int curArrSize, int numItems
------------------------------------------------------------------*/
struct DB {
  struct UserInfo *pArray;   // pointer to the array of UserInfos
  int curArrSize;   // current array size (max # of elements)
  int numItems;   // # of stored items(users)
};

/*------------------------------------------------------------------
<CreateCustomerDB>

parameter: receives nothing (type: void)
return value: d (type: DB_T)
              However, when d or d->pArray is NULL, returns NULL

This function creates DB_T d and sets d->curArrSize, d->pArray,
d->numItems. If d is NULL, or d->pArray is NULL it prints to standard 
error stream and returns NULL. 
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
  d->pArray = (struct UserInfo *)calloc(d->curArrSize,
               sizeof(struct UserInfo));
  d->numItems=0;
  if (d->pArray == NULL) {
    fprintf(stderr, "Can't allocate a memory for array of size %d\n",
	    d->curArrSize);   
    free(d);
    return NULL;
  }
  return d;
}

/*------------------------------------------------------------------
<DestroyCustomerDB>

parameter: d (type: DB_T)
return value: returns nothing (type: void)
              NULL if d is NULL

This function frees every UserInfo's name, id, and frees the UserInfo.
Then, it frees d->pArray and eventuallt frees d theirby destroying
all the structure.
------------------------------------------------------------------*/
void
DestroyCustomerDB(DB_T d)
{
  int i;
  struct UserInfo *ptr=d->pArray;
  
  if(d==NULL) return;
  for(i=0;i<(d->numItems);i++)
  {
    free(ptr->name);
    free(ptr->id);
    ptr++;
  }
  ptr=NULL;
  free(d->pArray);
  d->pArray=NULL;
  free(d);
  d=NULL;
  
  return;
}

/*------------------------------------------------------------------
<RegisterCustomer>

parameter: d (type: DB_T), id (type: const char*), name (type: const
char*), purchase (type: const int)
return value: 0 or -1 (type: int)
returns 0 when new UserInfo is registered successfully
returns -1 when d or id or name is NULL, purchase is equal or smaller
than zero, reallocation when expansion fails, when their already
exists UserInfo that has same id or name.

This function registers new UserInfo and sets id, name, purchase.
------------------------------------------------------------------*/
int
RegisterCustomer(DB_T d, const char *id,
		 const char *name, const int purchase)
{
  struct UserInfo* new=d->pArray;
  int flag=0;

  if(d==NULL) return (-1);
  if(id==NULL) return (-1);
  if(name==NULL) return (-1);
  if(purchase<=0) return (-1);

  while(1){
    if((new->name)==NULL){
      break;
    }
    else{
      if((d->numItems)>=(d->curArrSize)){
       	d->curArrSize=(d->numItems)+UNIT_ARRAY_SIZE;
	d->pArray=realloc(d->pArray,(d->curArrSize)*sizeof(struct UserInfo));
	if(d->pArray==NULL) return(-1);
	flag=1;
	break;
      }
      if(strcmp(new->name,name)==0) return(-1);
      if(strcmp(new->id,id)==0) return(-1);
      new++;
    }
  }
  if(flag==0){
    new->name=strdup(name);
    new->id=strdup(id);
    new->purchase=purchase;
  }
  else{
    new=(d->pArray)+(d->numItems);
    new->name=strdup(name);
    new->id=strdup(id);
    new->purchase=purchase;
  }
  (d->numItems)++;
  return (0);
}

/*------------------------------------------------------------------
<UnregisterCustomerByID>

parameter: d (type: DB_T), id (type: const char*)
return value: 0 or -1 (type: int)
0 when it successfully unregisters UserInfo.
-1 when d or id is NULL, flag is zero.

This function unregisters UserInfo by ID by freeing pointer.
------------------------------------------------------------------*/
int
UnregisterCustomerByID(DB_T d, const char *id)
{
  struct UserInfo* delete=d->pArray;
  int i=0;
  int flag=0; // 1 when it finds the UserInfo, 0 when it doesn't
  
  if(d==NULL) return(-1);
  if(id==NULL) return(-1);
  for(i=0;i<d->curArrSize;i++){
    if(delete->id==NULL){
      delete++;
      continue;
    }
    if(strcmp((delete->id),id)==0){
      flag=1;
      break;
    }
    delete++;
  }
  if(flag==0) return(-1);
  free(delete->name);
  free(delete->id);
  delete->name=NULL;
  delete->id=NULL;
  delete->purchase=0;
  (d->numItems)--;
  return 0;
}

/*------------------------------------------------------------------
<UnregisterCustomerByName>

parameter: d (type: DB_T), name (type: const char*)
return value: 0 or -1 (type: int)
0 when it successfully unregisters UserInfo.
-1 when d or id is NULL, flag is zero.

This function unregisters UserInfo by Name by freeing pointer.
------------------------------------------------------------------*/
int
UnregisterCustomerByName(DB_T d, const char *name)
{
  struct UserInfo* delete=d->pArray;
  int i=0;
  int flag=0; // 1 when it finds the UserInfo, 0 when it doesn't
  
  for(i=0;i<d->curArrSize;i++){
    if(delete->name==NULL){
      delete++;
      continue;
    }
    if(strcmp((delete->name),name)==0){
      flag=1;
      break;
    }
    delete++;
  }
  if(flag==0) return(-1);
  free(delete->name);
  free(delete->id);
  delete->name=NULL;
  delete->id=NULL;
  delete->purchase=0;
  (d->numItems)--;
  return 0;
}

/*------------------------------------------------------------------
<GetPurchaseByID>

parameter: d (type: DB_T), id (type: const char*)
return value: get->purchase or -1 (type: int)
-1 when d or id is NULL or get->id is NULL.
get->purchase when it finds the purchase by ID successfully.

This function searches purchase amount of UserInfo and returns it by
receiving ID. 
------------------------------------------------------------------*/
int
GetPurchaseByID(DB_T d, const char* id)
{
  struct UserInfo* get=d->pArray;

  if(d==NULL) return(-1);
  if(id==NULL) return(-1);
  while(1){
    if(strcmp((get->id),id)==0) break;
    get++;
    if(get->id==NULL) return (-1);
  }
  return (get->purchase);
}

/*------------------------------------------------------------------
<GetPurchaseByName>

parameter: d (type: DB_T), name (type: const char*)
return value: get->purchase or -1 (type: int)
-1 when d or name is NULL or get->name is NULL.
get->purchase when it finds the purchase by Name successfully.

This function searches purchase amount of UserInfo and returns it by
receiving Name. 
------------------------------------------------------------------*/
int
GetPurchaseByName(DB_T d, const char* name)
{
  struct UserInfo* get=d->pArray;
  
  if(d==NULL) return(-1);
  if(name==NULL) return(-1);
  while(1){
    if(strcmp((get->name),name)==0) break;
    get++;
    if(get->name==NULL) return (-1);
  }
  
  return (get->purchase);
}

/*------------------------------------------------------------------
<GetSumCustomerPurchase>

parameter: d (type: DB_T), fp (type: FUNCPTR)
return value: -1 or sum (type: int)
-1 when d or fp is NULL
sum when it gets the sum of the result that function pointer produces

This function earns the sum of whole result by receiving specific
function pointer.
------------------------------------------------------------------*/
int
GetSumCustomerPurchase(DB_T d, FUNCPTR_T fp)
{
  struct UserInfo* ptr=d->pArray;
  int sum=0;
  int i=0;
  int count=0;
  
  if(d==NULL) return (-1);
  if(fp==NULL) return(-1);
  
  for(i=0;i<(d->curArrSize);i++){
    if(ptr->id==NULL) continue;
    count=fp(ptr->id,ptr->name,ptr->purchase);
    ptr++;
    sum=sum+count;
  }
  return (sum);
}

