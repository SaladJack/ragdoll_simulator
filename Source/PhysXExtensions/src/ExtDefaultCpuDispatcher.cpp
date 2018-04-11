/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#include "ExtDefaultCpuDispatcher.h"
#include "ExtCpuWorkerThread.h"
#include "ExtTaskQueueHelper.h"
#include "PxTask.h"
#include "PsString.h"

using namespace physx;

namespace physx
{
	PxDefaultCpuDispatcher* PxDefaultCpuDispatcherCreate(PxU32 numThreads, PxU32* affinityMasks);
}

PxDefaultCpuDispatcher* physx::PxDefaultCpuDispatcherCreate(PxU32 numThreads, PxU32* affinityMasks)
{
	return PX_NEW(Ext::DefaultCpuDispatcher)(numThreads, affinityMasks);
}


#if !defined(PX_X360) && !defined(PX_WIIU) && !defined(PX_PSP2)
void Ext::DefaultCpuDispatcher::getAffinityMasks(PxU32* affinityMasks, PxU32 threadCount)
{
	for(PxU32 i=0; i < threadCount; i++)
	{
		affinityMasks[i] = 0;
	}
}
#endif


Ext::DefaultCpuDispatcher::DefaultCpuDispatcher(PxU32 numThreads, PxU32* affinityMasks)
	: mQueueEntryPool(EXT_TASK_QUEUE_ENTRY_POOL_SIZE, "QueueEntryPool"), mNumThreads(numThreads), mShuttingDown(false)
#ifdef PX_PROFILE
	,mRunProfiled(true)
#else
	,mRunProfiled(false)
#endif
{
	PxU32* defaultAffinityMasks = NULL;

	if(!affinityMasks)
	{
		defaultAffinityMasks = (PxU32*)PX_ALLOC(numThreads * sizeof(PxU32), PX_DEBUG_EXP("ThreadAffinityMasks"));
		getAffinityMasks(defaultAffinityMasks, numThreads);
		affinityMasks = defaultAffinityMasks;
	}
	 
	// initialize threads first, then start

	mWorkerThreads = reinterpret_cast<CpuWorkerThread*>(PX_ALLOC(numThreads * sizeof(CpuWorkerThread), PX_DEBUG_EXP("CpuWorkerThread")));
	const PxU32 nameLength = 32;
	mThreadNames = reinterpret_cast<PxU8*>(PX_ALLOC(nameLength * numThreads, PX_DEBUG_EXP("CpuWorkerThreadName")));

	if (mWorkerThreads)
	{
		for(PxU32 i = 0; i < numThreads; ++i)
		{
			PX_PLACEMENT_NEW(mWorkerThreads+i, CpuWorkerThread)();
			mWorkerThreads[i].initialize(this);
		}

		for(PxU32 i = 0; i < numThreads; ++i)
		{
			mWorkerThreads[i].setAffinityMask(affinityMasks[i]);
			mWorkerThreads[i].start(Ps::Thread::getDefaultStackSize());

			if (mThreadNames)
			{
				char* threadName = reinterpret_cast<char*>(mThreadNames + (i*nameLength));
				string::sprintf_s(threadName, nameLength, "PxWorker%02d", i);
				mWorkerThreads[i].setName(threadName);
			}
		}

		if (defaultAffinityMasks)
			PX_FREE(defaultAffinityMasks);
	}
	else
	{
		mNumThreads = 0;
	}
}


Ext::DefaultCpuDispatcher::~DefaultCpuDispatcher()
{
	for(PxU32 i = 0; i < mNumThreads; ++i)
		mWorkerThreads[i].signalQuit();

	mShuttingDown = true;
	mWorkReady.set();
	for(PxU32 i = 0; i < mNumThreads; ++i)
		mWorkerThreads[i].waitForQuit();

	for(PxU32 i = 0; i < mNumThreads; ++i)
		mWorkerThreads[i].~CpuWorkerThread();

	PX_FREE(mWorkerThreads);

	if (mThreadNames)
		PX_FREE(mThreadNames);
}


void Ext::DefaultCpuDispatcher::submitTask(PxBaseTask& task)
{
	Ps::Thread::Id currentThread = Ps::Thread::getId();
	if(!mNumThreads)
	{
		// no worker threads, run directly
		if(mRunProfiled)
			task.runProfiled(static_cast<PxU32>(currentThread));
		else
			task.run();
		task.release();
		return;
	}	

	// TODO: Could use TLS to make this more efficient
	for(PxU32 i = 0; i < mNumThreads; ++i)
	{
		if(mWorkerThreads[i].tryAcceptJobToLocalQueue(task, currentThread))
			return mWorkReady.set();
	}

	SharedQueueEntry* entry = mQueueEntryPool.getEntry(&task);
	if (entry)
	{
		mJobList.push(*entry);
		mWorkReady.set();
	}
}

PxBaseTask* Ext::DefaultCpuDispatcher::fetchNextTask()
{
	PxBaseTask* task = getJob();

	if(!task)
		task = stealJob();

	return task;
}

void Ext::DefaultCpuDispatcher::runTask(PxBaseTask& task)
{
	if(mRunProfiled)
	{
		const PxU32 threadId = static_cast<PxU32>(Ps::Thread::getId());
		task.runProfiled(threadId);
	}
	else
		task.run();
}

PxU32 Ext::DefaultCpuDispatcher::getWorkerCount() const
{
	return mNumThreads;	
}

void Ext::DefaultCpuDispatcher::release()
{
	PX_DELETE(this);
}


PxBaseTask* Ext::DefaultCpuDispatcher::getJob(void)
{
	return TaskQueueHelper::fetchTask(mJobList, mQueueEntryPool);
}


PxBaseTask* Ext::DefaultCpuDispatcher::stealJob()
{
	PxBaseTask* ret = NULL;

	for(PxU32 i = 0; i < mNumThreads; ++i)
	{
		ret = mWorkerThreads[i].giveUpJob();

		if(ret != NULL)
			break;
	}

	return ret;
}


void Ext::DefaultCpuDispatcher::resetWakeSignal()
{
	mWorkReady.reset();
	
	// The code below is necessary to avoid deadlocks on shut down.
	// A thread usually loops as follows:
	// while quit is not signaled
	// 1)  reset wake signal
	// 2)  fetch work
	// 3)  if work -> process
	// 4)  else -> wait for wake signal
	//
	// If a thread reaches 1) after the thread pool signaled wake up,
	// the wake up sync gets reset and all other threads which have not
	// passed 4) already will wait forever.
	// The code below makes sure that on shutdown, the wake up signal gets
	// sent again after it was reset
	//
	if (mShuttingDown)
		mWorkReady.set();
}
