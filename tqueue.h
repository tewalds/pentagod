/*********************************
 * Templated Threaded Queue in C++
 * 
 * A threaded queue library for passing work between threads.
 *
 ********************************/



#ifndef _TQUEUE_H_
#define _TQUEUE_H_

#include <pthread.h>
#include <queue>
using std::queue;

template <class T> class tqueue {
private:
	queue <T *> q;        // The queue
	pthread_mutex_t lock; // The queue lock
	pthread_cond_t  cv;   // Lock conditional variable
	int             blck; // should pop() block by default

public:
	tqueue(){
		blck = 1;
		pthread_mutex_init(&lock, NULL);
		pthread_cond_init (&cv, NULL);
	}

/**
 * Pop an element off the queue
 * If wait is 0, return null if the queue is empty, otherwise wait until an item is placed in the queue
 * Potentially in the future:
 *  wait = -1 => block until an item is placed in the queue, and return it
 *  wait = 0  => don't block, return null if the queue is empty
 *  wait > 0  => block for <block> seconds  
 */
	T * pop(const int wait = -1){
		T * ret;
		pthread_mutex_lock(&lock);

		/**
		 * If the queue is empty
		 *   if we're in non-blocking mode, just return
		 *   otherwise wait on the condition to be triggered. The condition gets triggered
		 *     either when something is added, in which case the while condition fails and
		 *     the value gets returned, or the queue is changed into non-blocking mode and returns
		 **/

		while(q.empty()){
			if(wait == 0 || blck == 0){
				pthread_mutex_unlock(&lock);
				return NULL;
			}

			pthread_cond_wait(&cv, &lock);
		}

	//we know there is at least one item, so dequeue one and return
		ret = q.front();
		q.pop();

		pthread_mutex_unlock(&lock);

		return ret;
	}

//push on the queue
	void push( T * val ){
		pthread_mutex_lock(&lock);

		q.push(val);

		pthread_mutex_unlock(&lock);
		pthread_cond_signal(&cv);
	}

	unsigned int size(){
		pthread_mutex_lock(&lock);
		unsigned int ret = (unsigned int)q.size();
		pthread_mutex_unlock(&lock);
		return ret;
	}

	bool empty(){
		pthread_mutex_lock(&lock);
		bool ret = q.empty();
		pthread_mutex_unlock(&lock);
		return ret;
	}

//set in blocking mode, so a call to pop without args waits until something is added to the queue
	void block(){
		pthread_mutex_lock(&lock);
		blck = 1;
		pthread_mutex_unlock(&lock);
		pthread_cond_broadcast(&cv);
	}

//set in non-blocking mode, so a call to pop returns immediately, returning NULL if the queue is empty
//also tells all currently blocking pop calls to return immediately
	void nonblock(){
		pthread_mutex_lock(&lock);
		blck = 0;
		pthread_mutex_unlock(&lock);
		pthread_cond_broadcast(&cv);
	}
};

#endif

