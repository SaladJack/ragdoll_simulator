/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include "foundation/PxSimpleTypes.h"

namespace physx
{
	namespace SnippetUtils
	{

		/* Increment the specified location. Return the incremented value. */
		PxI32 atomicIncrement(volatile PxI32* val);

		/* Decrement the specified location. Return the decremented value. */
		PxI32 atomicDecrement(volatile PxI32* val);

		//******************************************************************************//
		
		/* Return the number of physical cores (does not include hyper-threaded cores), returns 0 on failure. */
		PxU32 getNbPhysicalCores();

		//******************************************************************************//

		/* Return the id of a thread. */
		PxU32 getThreadId();

		//******************************************************************************//

		/* Return the current time */
		PxU64 getCurrentTimeCounterValue();

		/* Convert to milliseconds an elapsed time computed from the difference of the times returned from two calls to getCurrentTimeCounterValue. */
		PxReal getElapsedTimeInMilliseconds(const PxU64 elapsedTime);

		/* Convert to microseconds an elapsed time computed from the difference of the times returned from two calls to getCurrentTimeCounterValue. */
		PxReal getElapsedTimeInMicroSeconds(const PxU64 elapsedTime);

		//******************************************************************************//

		struct Sync;

		/* Create a sync object. Returns a unique handle to the sync object so that it may be addressed through syncWait etc. */
		Sync* syncCreate();

		/* Wait indefinitely until the specified sync object is signaled. */
		void syncWait(Sync* sync);

		/* Signal the specified synchronization object, waking all threads waiting on it. */
		void syncSet(Sync* sync);

		/** Reset the specified synchronization object. */
		void syncReset(Sync* sync);

		/* Release the specified sync object so that it may be reused with syncCreate. */
		void syncRelease(Sync* sync);


		//******************************************************************************//

		struct Thread;

		/* Prototype of callback passed to threadCreate. */
		typedef void (*ThreadEntryPoint)(void*);

		/* Create a thread object and return a unique handle to the thread object so that it may be addressed through threadStart etc. 
		entryPoint implements ThreadEntryPoint and data will be passed as a function argument, POSIX-style. */
		Thread* threadCreate(ThreadEntryPoint entryPoint, void* data);

		/* Cleanly shut down the specified thread. Called in the context of the spawned thread. */
		void threadQuit(Thread* thread);

		/* Stop the specified thread. Signals the spawned thread that it should stop, so the 
		thread should check regularly. */
		void threadSignalQuit(Thread* thread);

		/* Wait for the specified thread to stop. Should be called in the context of the spawning
		thread. Returns false if the thread has not been started.*/
		bool threadWaitForQuit(Thread* thread);

		/* Check whether the thread is signalled to quit. Called in the context of the
		spawned thread. */
		bool threadQuitIsSignalled(Thread* thread);

		/* Release the specified thread object so that it may be reused with threadCreate. */
		void threadRelease(Thread* thread);

		//******************************************************************************//

		struct Mutex;

		/* Create a mutex object and return a unique handle to the mutex object so that it may be addressed through mutexLock etc. */
		Mutex* mutexCreate();

		/* Acquire (lock) the specified mutex. If the mutex is already locked by another thread, this method blocks until the mutex is unlocked.*/
		void mutexLock(Mutex* mutex);

		/* Release (unlock) the specified mutex, the calling thread must have previously called lock() or method will error. */
		void mutexUnlock(Mutex* mutex);

		/* Release the specified mutex so that it may be reused with mutexCreate. */
		void mutexRelease(Mutex* mutex);
	}
}
