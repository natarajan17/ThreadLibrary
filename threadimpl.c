/*
 * Description : To create a thread library .
 *  Author: Natarajan Ramesh (200018549) (nramesh)
 *
*/

#include<stdio.h>
#include<ucontext.h>
#include "mythread.h"
#include<stdlib.h>


#define QUEUESIZE  1000

#define debug 0



/*  my thread structure 

Thread contains the following items:

1. Thread context
2. 


*/
typedef struct mythread thread;
struct mythread
{
ucontext_t threadcontext;
//ucontext_t parentcontext;
int id;
int status;
int parentid;
int isjoinall;
int joinid;
};

static int p=0;

// Structure of queue to hold the different contexts.
typedef struct node queue;
struct node{
         thread  *mycontext[QUEUESIZE];             /* body of queue */
        int first;                      /* position of first element */
        int last;                       /* position of last element */
        int count;                      /* number of queue elements */
};


typedef struct semaphore S;
struct semaphore
{
int svalue;
queue *semblocked;
};



static int id=0;

// Initialize the queue.
init_queue(queue *q)
{
	int i =0;
	for(i=0;i<QUEUESIZE;i++)
	{
		q->mycontext[i] = (thread*)malloc(sizeof(thread));
	}	
        q->first = 0;
        q->last = -1;
        q->count = 0;
}


//Enqueue the thread into the given queue.
enqueue(queue *q, thread *x)
{
        if (q->count >= QUEUESIZE)
                printf("Warning: queue overflow enqueue x=%d\n",x);
        else {
                q->last = (q->last+1);
                q->mycontext[ q->last ] = x;
                q->count = q->count + 1;

        }
}



// Dequeue the first element from the given queue.
thread* dequeue(queue *q)
{

       thread *x;

        if (q->count <= 0) printf("Warning: empty queue dequeue.\n");
        else {
                x = q->mycontext[ q->first ];
                q->first = (q->first+1);
                q->count = q->count - 1;
                }

        return(x);

}

ucontext_t parent; // mainthread is the first thread that is created in init, parent is the kernel thread.
queue *mylist, *running, *blocked; // mylist is the readyQ , and running is the running queue.
thread *mainthread; // mainthrad is thfirst thread.

//Find the parentid in the blocked Q.
thread* findParentInBlockedQ(int id)
{
	thread *x;
	int index=0;
	int i=blocked->first;
        if (blocked->count > 0)
	{
	while(index<blocked->count)
	{
                if( blocked->mycontext[i]->id == id)
		{	
                	return blocked->mycontext[i];
		}
		i++;
       		index++;
        }
	return NULL;
	}
}

// Find the thread with the same parent id in Ready Queue.
int findThreadWithSameParentInReadyQ(int id)
{
	int i=mylist->first;
	int index =0;
	if(mylist->count!=0)
	{
		while(index < mylist->count)
		{
			if(mylist->mycontext[i]->parentid == id)
			{
				return 1;
				break;
			}
			 i++;
			index++;
		}

	}
	return 0;

}

// Find the thread with same parent in Blocked Q.
int findThreadWithSameParentinBlockedQ(int id)
{
        int i= blocked->first;
        int index =0;
        if(blocked->count!=0)
        {
                while(index < blocked->count)
                {
                        if(blocked->mycontext[i]->parentid == id)
                        {
                                return 1;
                                break;
                        }
                         i++;
                        index++;
                }

        }
        return 0;

}


// Remove the given pid element from blocked q.
void removeFromBlocked(int pid)
{
	//thread *x;
	int found =0;
	int i=blocked->first;
	int index =0;
	while(index<blocked->count)
	{	
		if(blocked->mycontext[i]->id == pid)
		{

	//		x = blocked[i]->mycontext;
			if(i == blocked->first)
			{
				blocked->first = ((blocked->first)+1);
				blocked->count--;
			}
			else if(i==blocked->last)
			{
				blocked->last --;
				blocked->count --;
				break;
				
			}
			else
			{
				blocked->mycontext[i]= blocked->mycontext[i+1];	
			}
			found =1;
		}
		else if(found)
		{ 
			if(blocked->last == i)
			{
				//free(blocked[blocked->last]);
				blocked->last --;
				blocked->count --;
				break;
	
			}
			blocked->mycontext[i] = blocked->mycontext[i+1];

		}
		i++;
		index++;
	}

//return(x);
}


// Get the first item from Q.
thread*  getItemFromQ (queue *q)
{
       thread *x;

        if (q->count <= 0) return NULL;
        else {
	  x = q->mycontext[q->first];
        }

        return(x);
}

// Check if Q is empty.
int isQEmpty(queue *q)
{
	if(q->count <=0) return 1;
	else return 0;
}
//
// // ****** CALLS ONLY FOR UNIX PROCESS ****** 
// // Create and run the "main" thread
 void MyThreadInit(void(*start_funct)(void *), void *args)
{
running =(queue*)malloc(sizeof(queue));
mylist = (queue*)malloc(sizeof(queue));
blocked = (queue*)malloc(sizeof(queue));
init_queue(mylist);
mainthread =(thread*)malloc(sizeof(thread));
init_queue(running);
init_queue(blocked);

getcontext(&mainthread->threadcontext);
mainthread->threadcontext.uc_stack.ss_sp = malloc(1024*16);
mainthread->threadcontext.uc_stack.ss_size = 1024*16;
mainthread->id = id++;
mainthread->parentid = -1;

makecontext(&mainthread->threadcontext,(void (*)(void))start_funct,1,args);
enqueue(running, mainthread);

swapcontext(&parent,&mainthread->threadcontext);
free(&mainthread->threadcontext);
free(running);
free(mylist);
free(blocked);
id = 0;
#if debug 
printf("\n\ndone");
#endif
}

MyThread MyThreadCreate(void(*func)(void *),void *args)
{

// First create a new item  and allocate memory to it

thread *newitem = (thread*)malloc(sizeof(thread)); 

// get the parent from 

// Get its context 
getcontext(&newitem->threadcontext);
// assign stack

newitem->threadcontext.uc_stack.ss_sp = malloc(1024*16);
newitem->threadcontext.uc_stack.ss_size = 1024*16;
newitem->id = id++;
newitem->parentid = running->mycontext[running->first]->id;
// assign the given function to it 
makecontext(&newitem->threadcontext,(void (*)(void)) func,1,args);
//enqueue  it in reeady queue
enqueue(mylist,newitem);
return newitem;
}

// Yield invoking thread
 void MyThreadYield(void)
{
thread *runningthread = dequeue(running);
thread *firstthread = dequeue(mylist);
enqueue(running,firstthread);
enqueue(mylist, runningthread);
swapcontext(&runningthread->threadcontext,&firstthread->threadcontext);
}



// // Join with a child thread
int MyThreadJoin(MyThread t)
{

thread  *readythread = getItemFromQ(mylist);
thread *runningthread= getItemFromQ(running);
thread *childthread = (thread*)t;

if(childthread->status == 1)
{
return 0;
}

else if(childthread->parentid != runningthread->id)
{
return -1;
}
readythread = dequeue(mylist);
runningthread = dequeue(running);
runningthread->joinid = childthread->id;
enqueue(blocked,runningthread);
enqueue(running, readythread);
swapcontext(&runningthread->threadcontext,&readythread->threadcontext);
}
//
// // Join with all children

void MyThreadJoinAll(void)
{

thread *runningthread  = getItemFromQ(running);
thread *readythread = getItemFromQ(mylist);
int i=mylist->first;
int index = 0;
int childPresent =0;
while(index<mylist->count) 
{
	if(runningthread->id == mylist->mycontext[i]->parentid)
	{
		childPresent =1;
		break;
	}
	i++;	
	index++;
}
if(childPresent)
{
	readythread = dequeue(mylist);
	runningthread =  dequeue(running);
	runningthread->isjoinall = 1;
	enqueue(blocked,runningthread);
	enqueue(running,readythread);
	swapcontext(&runningthread->threadcontext,&readythread->threadcontext);
}




}

//
// // Terminate invoking thread
void MyThreadExit(void)
{
	thread *readythread=NULL,*runningthread=NULL;
	if(!isQEmpty(running))
		 runningthread = dequeue(running);
	int isJoinAll;
	// chck if the present thread has any parent which is in the block queue

	thread *parentFound = findParentInBlockedQ(runningthread->parentid);

	// for join all.. Check if there is any child which is present in ready q.
	if(parentFound!=NULL)
	{
		if(parentFound->isjoinall) // It is a join all
		{	
			isJoinAll = findThreadWithSameParentInReadyQ(parentFound->id) || findThreadWithSameParentinBlockedQ(parentFound->id);
			if(!isJoinAll)
			{
				removeFromBlocked(parentFound->id);	
				enqueue(mylist,parentFound);
			}
		}	
		else // It is just a Join
		{
			if(parentFound->joinid == runningthread->id)
			{
			   removeFromBlocked(parentFound->id);
			   enqueue(mylist,parentFound);
			}

		}
		
	}
        if(!isQEmpty(mylist))
                 readythread = dequeue(mylist);
	
	runningthread->status = 1;	
	
	if(readythread != NULL)
	{
		enqueue(running,readythread);
		swapcontext(&runningthread->threadcontext,&readythread->threadcontext);
	}
	else
	{
		swapcontext(&runningthread->threadcontext,&parent);

	}
}



//
// // ****** SEMAPHORE OPERATIONS ****** 
// // Create a semaphore
 MySemaphore MySemaphoreInit(int initialValue)
{
S *newsem = (S*)malloc(sizeof(S));
newsem->svalue = initialValue;
newsem->semblocked = (queue*)malloc(sizeof(queue));
init_queue(newsem->semblocked);
return newsem;
}



//
// // Signal a semaphore
 void MySemaphoreSignal(MySemaphore sem)
{
	S *s = (S*)sem;
	s->svalue++;
 	if(s->svalue<=0)
	{
			
		thread *readythread = dequeue(s->semblocked);
		enqueue(mylist,readythread);
	}	
	

}
//
// // Wait on a semaphore
 void MySemaphoreWait(MySemaphore sem)
{
      S *s = (S*)sem;
        thread *readythread =NULL;
        thread *runningthread;
        s->svalue--;
        if(s->svalue <0)
        {
                runningthread = dequeue(running);
                enqueue(s->semblocked , runningthread);
		if(!isQEmpty(mylist))
			readythread = dequeue(mylist);
		if(readythread==NULL)
		{
			swapcontext(&runningthread->threadcontext,&parent);
		}
		enqueue(running,readythread);
                swapcontext(&runningthread->threadcontext,&readythread->threadcontext);
        }


}
//
// // Destroy on a semaphore
int MySemaphoreDestroy(MySemaphore sem)
{

S *dest = (S*) sem;
if(dest->semblocked->count == 0)
{
free (dest);
return 0;
}
return -1;
}

