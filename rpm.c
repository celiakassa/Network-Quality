#include <stdlib.h>
#include <stdio.h>

#include "rpm.h"

Instan_aggregate init_instan_list(void){
   Instan_aggregate headI = (Instan_aggregate) malloc(sizeof(struct instan_aggregate_node));
   headI->next = NULL;
   headI->flag = -1;
   return headI;
}

Instan_aggregate addAggregate(long goodput, Instan_aggregate headI){
   if(headI->flag == -1){
      headI->flag = 0;
      headI->goodput = goodput;
      return headI;
   }
   Instan_aggregate n = (Instan_aggregate) malloc(sizeof(struct instan_aggregate_node));
   n->flag = 0;
   n->goodput = goodput;
   n->next = headI;
   headI = n;
   return headI;
}


Instan_aggregate delAggregate(Instan_aggregate headI){
  if(headI == NULL)
      return headI;
   Instan_aggregate n;
   Instan_aggregate next = headI->next;
   n = headI;
   headI = next;
   free(n);
   return headI;
}

long get_prev_goodput(Instan_aggregate ia){
    int i;
    int count = 0;
    long mva = 0;
    if(ia->flag == -1)
      return (long)-1;
    Instan_aggregate next = ia->next;
    mva+=ia->goodput;
    count++;
    while((next!=NULL) && (count < 4)){
       mva+=ia->goodput;
       next = next->next;
       count++;
    }
    return (long)(mva/count);
}
