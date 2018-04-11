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


#ifndef PX_PHYSICS_SCB_SCENE
#define PX_PHYSICS_SCB_SCENE

#include "ScScene.h"

#include "ScbSceneBuffer.h"
#include "ScbType.h"
#include "PsFoundation.h"
#include "PsMutex.h"
#include "PsHashSet.h"

#ifdef PX_PS3
#include "ps3/PxPS3Config.h"
#endif

#include "pvd/PxVisualDebugger.h"
#if PX_SUPPORT_VISUAL_DEBUGGER
#include "PvdSceneVisualDebugger.h"
#define UPDATE_PVD_PROPERTIES_SCENE() { if(mSceneVisualDebugger.isConnected(true)) mSceneVisualDebugger.updatePvdProperties(); }
#else
namespace Pvd
{
	class SceneVisualDebugger;
}
#define UPDATE_PVD_PROPERTIES_SCENE() {}
#endif

namespace physx
{

class NpMaterial;

namespace Sc
{
	class BodyDesc;
}

namespace Scb
{
	class Base;
	class RigidObject;
	class RigidStatic;
	class Body;
	class Actor;
	class Shape;
	class Constraint;
	class Material;
	class ParticleSystem;
	class Articulation;
	class ArticulationJoint;
	class Aggregate;
	class Cloth;

	struct ShapeBuffer;

	/**
	\brief Helper class to track inserted/removed/updated buffering objects.
	*/
	class ObjectTracker
	{
	public:
		 ObjectTracker() {}
		 
		/**
		\brief An object has been inserted while the simulation was running -> track it for insertion at sync point
		*/
		void scheduleForInsert(Base& element);

		/**
		\brief An object has been removed while the simulation was running -> track it for removal at sync point
		*/
		void scheduleForRemove(Base& element);

		/**
		\brief An object has been changed while the simulation was running -> track it for update at sync point
		*/
		void scheduleForUpdate(Base& element);
		
		/**
		\brief Get the list of dirty objects that require processing at a sync point.
		*/
		Base*const *		getBuffered()				{ return mBuffered.getEntries();	}

		/**
		\brief Get number of dirty objects that require processing at a sync point.
		*/
		PxU32				getBufferedCount()	const	{ return mBuffered.size();	}
		
		/**
		\brief Cleanup dirty objects after sync point.

		\li Transition pending insertion objects from eINSERT_PENDING to eIN_SCENE.
		\li Transition pending removal objects from eREMOVE_PENDING to eNOT_IN_SCENE.
		\li Destroy objects marked as eIS_RELEASED.
		\li Clear dirty list.
		*/
		void clear();
	
		void insert(Base& element);
		void remove(Base& element);
		
	private:
		Ps::CoalescedHashSet<Base*> mBuffered;	
	};


	typedef ObjectTracker							ShapeManager;
	typedef ObjectTracker							RigidStaticManager;
	typedef ObjectTracker							BodyManager;
	typedef ObjectTracker							ParticleSystemManager;
	typedef ObjectTracker							ArticulationManager;
	typedef ObjectTracker							ConstraintManager;
	typedef ObjectTracker							ArticulationJointManager;
	typedef ObjectTracker							AggregateManager;
	typedef ObjectTracker							ClothManager;


	enum MATERIAL_EVENT
	{
		MATERIAL_ADD,
		MATERIAL_UPDATE,
		MATERIAL_REMOVE
	};

	class MaterialEvent
	{
	public:
		PxU32			mHandle;//handle to the master material table
		MATERIAL_EVENT	mType;
	};

	class Scene : public Ps::UserAllocated
	{
		PX_NOCOPY(Scene)
	public:
		enum BufferFlag
		{
			BF_GRAVITY					= (1 << 0),
			BF_BOUNCETHRESHOLDVELOCITY  = (1 << 1),
			BF_FLAGS					= (1 << 2),
			BF_DOMINANCE_PAIRS			= (1 << 3),
			BF_SOLVER_BATCH_SIZE		= (1 << 4),
			BF_CLIENT_BEHAVIOR_FLAGS	= (1 << 5),
			BF_VISUALIZATION			= (1 << 6),
			BF_SCENE_PARAMS				= (1 << 7)
			
		};

	public:
											Scene(const PxSceneDesc& desc, Cm::EventProfiler eventBuffer);
											~Scene() {}	//use release() plz.

		//---------------------------------------------------------------------------------
		// Wrapper for Sc::Scene interface
		//---------------------------------------------------------------------------------
		void								release();

		PxScene*							getPxScene(); 

		PX_INLINE void						setGravity(const PxVec3& gravity);
		PX_INLINE PxVec3					getGravity() const;

		PX_INLINE void						setBounceThresholdVelocity(const PxReal t);
		PX_INLINE PxReal					getBounceThresholdVelocity() const;

		PX_INLINE void						setFlags(PxSceneFlags flags);
		PX_INLINE PxSceneFlags				getFlags() const;

		PX_INLINE void						setFrictionType(PxFrictionType::Enum);
		PX_INLINE PxFrictionType::Enum		getFrictionType() const;

		void 								addRigidStatic(Scb::RigidStatic&, bool noSim);
		void 								removeRigidStatic(Scb::RigidStatic&, bool wakeOnLostTouch, bool noSim);

		void 								addRigidBody(Scb::Body&, bool noSim);
		void 								removeRigidBody(Scb::Body&, bool wakeOnLostTouch, bool noSim);

		void								addConstraint(Scb::Constraint&);
		void								removeConstraint(Scb::Constraint&);

		void								addArticulation(Scb::Articulation&);
		void								removeArticulation(Scb::Articulation&);

		void								addArticulationJoint(Scb::ArticulationJoint&);
		void								removeArticulationJoint(Scb::ArticulationJoint&);

		void								addAggregate(Scb::Aggregate&);
		void								removeAggregate(Scb::Aggregate&);

	#if PX_USE_PARTICLE_SYSTEM_API
		void								addParticleSystem(Scb::ParticleSystem&);
		void								removeParticleSystem(Scb::ParticleSystem&, bool isRelease);
	#endif

	#if PX_USE_CLOTH_API
		void								addCloth(Scb::Cloth&);
		void								removeCloth(Scb::Cloth&);
	#endif

#ifdef PX_PS3
		void								setSceneParamInt(PxPS3ConfigParam::Enum param, PxU32 value);
		PxU32								getSceneParamInt(PxPS3ConfigParam::Enum param);
#endif

		PX_FORCE_INLINE	Cm::EventProfiler&	getEventProfiler()	{ return mScene.getEventProfiler();	}

		bool								addMaterial(Sc::MaterialCore& mat);
		void								updateMaterial(Sc::MaterialCore& mat);//(PxU32 index, Sc::MaterialCore& material);
		void								removeMaterial(Sc::MaterialCore& mat);//(PxU32 index, PxMaterial*);  // The second parameter is only used for an integrity check
		void								updateLowLevelMaterial(NpMaterial** masterMaterials);
		// These methods are only to be called at fetchResults!
		PX_INLINE void						initActiveBodiesIterator(Sc::BodyIterator&);

		PX_INLINE PxSimulationEventCallback*	getSimulationEventCallback(PxClientID client) const;
		PX_INLINE void						setSimulationEventCallback(PxSimulationEventCallback* callback, PxClientID client);
		PX_INLINE PxContactModifyCallback*	getContactModifyCallback() const;
		PX_INLINE void						setContactModifyCallback(PxContactModifyCallback* callback);
		PX_INLINE PxCCDContactModifyCallback*	getCCDContactModifyCallback() const;
		PX_INLINE void						setCCDContactModifyCallback(PxCCDContactModifyCallback* callback);
		PX_INLINE PxU32						getCCDMaxPasses() const;
		PX_INLINE void						setCCDMaxPasses(PxU32 ccdMaxPasses);
		PX_INLINE	void					setBroadPhaseCallback(PxBroadPhaseCallback* callback, PxClientID client);
		PX_INLINE	PxBroadPhaseCallback*	getBroadPhaseCallback(PxClientID client)		const;
					PxBroadPhaseType::Enum	getBroadPhaseType()																				const;
					bool					getBroadPhaseCaps(PxBroadPhaseCaps& caps)														const;
					PxU32					getNbBroadPhaseRegions()																		const;
					PxU32					getBroadPhaseRegions(PxBroadPhaseRegionInfo* userBuffer, PxU32 bufferSize, PxU32 startIndex)	const;
					PxU32					addBroadPhaseRegion(const PxBroadPhaseRegion& region, bool populateRegion);
					bool					removeBroadPhaseRegion(PxU32 handle);

		// Collision filtering
		PX_INLINE const void*				getFilterShaderData() const;
		PX_INLINE PxU32						getFilterShaderDataSize() const;
		PX_INLINE PxSimulationFilterShader	getFilterShader() const;
		PX_INLINE PxSimulationFilterCallback* getFilterCallback() const;

		// Groups
		PX_INLINE void						setDominanceGroupPair(PxDominanceGroup group1, PxDominanceGroup group2, const PxDominanceGroupPair& dominance);
		PX_INLINE PxDominanceGroupPair		getDominanceGroupPair(PxDominanceGroup group1, PxDominanceGroup group2) const;

		PX_INLINE void						setSolverBatchSize(PxU32 solverBatchSize);
		PX_INLINE PxU32						getSolverBatchSize() const;

		PX_INLINE void						simulate(PxReal timeStep, PxBaseTask* continuation);
		PX_INLINE void						collide(PxReal timeStep, PxBaseTask* continuation);
		PX_INLINE void						solve(PxReal timeStep, PxBaseTask* continuation);
		PX_INLINE void						stepSetupCollide();
		PX_INLINE void						endSimulation();
		PX_INLINE void						flush(bool sendPendingReports);
		PX_INLINE void						fireCallBacksPreSync();						//callbacks that are fired on the core side, before the buffers get synced
		PX_INLINE void						fireCallBacksPostSync();					//callbacks that are fired on the core side, after the buffers get synced
		PX_FORCE_INLINE void				postCallbacksPreSync() { mScene.postCallbacksPreSync(); }  //cleanup tasks after the pre-sync callbacks have fired
		PX_INLINE void						postReportsCleanup();

		PX_INLINE PxReal					getMeshContactMargin() const;
		PX_INLINE const PxSceneLimits&		getLimits() const;
		PX_INLINE void						setLimits(const PxSceneLimits& limits);

		PX_INLINE void						getStats(PxSimulationStatistics& stats) const;

		PX_INLINE void						buildActiveTransforms(); // build the list of active transforms
		PX_INLINE PxActiveTransform*		getActiveTransforms(PxU32& nbTransformsOut, PxClientID client);

		PX_INLINE PxClientID				createClient();
		PX_INLINE void						setClientBehaviorFlags(PxClientID client, PxClientBehaviorFlags clientBehaviorFlags);
		PX_INLINE PxClientBehaviorFlags		getClientBehaviorFlags(PxClientID client) const;

#if PX_USE_CLOTH_API
		PX_INLINE void						setClothInterCollisionDistance(PxF32 distance);
		PX_INLINE PxF32						getClothInterCollisionDistance() const;
		PX_INLINE void						setClothInterCollisionStiffness(PxF32 stiffness); 
		PX_INLINE PxF32						getClothInterCollisionStiffness() const;
		PX_INLINE void						setClothInterCollisionNbIterations(PxU32 nbIterations);
		PX_INLINE PxU32						getClothInterCollisionNbIterations() const;
#endif

		PX_INLINE void						setVisualizationParameter(PxVisualizationParameter::Enum param, PxReal value);
		PX_INLINE PxReal					getVisualizationParameter(PxVisualizationParameter::Enum param) const;

		PX_INLINE void						setVisualizationCullingBox(const PxBounds3& box);
		PX_INLINE const PxBounds3&			getVisualizationCullingBox() const;

		void								addStaticAndShapesToPvd(Scb::RigidStatic& s);
		void								addBodyAndShapesToPvd(Scb::Body& b);
		void								addShapeToPvd(Scb::Shape& shape, PxActor& owner);
		void								removeShapeFromPvd(Scb::Shape& shape, PxActor& owner);
		void								addAggregateToPvd(Scb::Aggregate& a);
		void								removeAggregateFromPvd(Scb::Aggregate& a);

		void								shiftOrigin(const PxVec3& shift);

		//---------------------------------------------------------------------------------
		// Data synchronization
		//---------------------------------------------------------------------------------
	public:
		void								syncEntireScene(PxU32* error);
		void								processPendingRemove();

		PX_FORCE_INLINE	PxU16*				allocShapeMaterialBuffer(PxU32 count, PxU32& startIdx) { return allocArrayBuffer(mShapeMaterialBuffer, count, startIdx); }
		PX_FORCE_INLINE	const PxU16*		getShapeMaterialBuffer(PxU32 startIdx) const { return &mShapeMaterialBuffer[startIdx]; }
		PX_FORCE_INLINE	Scb::Shape**		allocShapeBuffer(PxU32 count, PxU32& startIdx) { return allocArrayBuffer(mShapePtrBuffer, count, startIdx); }
		PX_FORCE_INLINE	Scb::Shape**		getShapeBuffer(PxU32 startIdx) { return &mShapePtrBuffer[startIdx]; }
		PX_FORCE_INLINE	Scb::Actor**		allocActorBuffer(PxU32 count, PxU32& startIdx) { return allocArrayBuffer(mActorPtrBuffer, count, startIdx); }
		PX_FORCE_INLINE	Scb::Actor**		getActorBuffer(PxU32 startIdx) { return &mActorPtrBuffer[startIdx]; }


		static			void				bufferCreationError(const char* object);
						void				scheduleForUpdate(Scb::Base& object);
						PxU8*				getStream(ScbType::Enum type);

		PX_FORCE_INLINE void				removeShapeFromPendingUpdateList(Scb::Base& shape) { mShapeManager.remove(shape); }

		PX_FORCE_INLINE	const Sc::Scene&	getScScene()					const	{ return mScene;						}
		PX_FORCE_INLINE	Sc::Scene&			getScScene()							{ return mScene;						}
		PX_FORCE_INLINE void				prepareOutOfBoundsCallbacks()			{ mScene.prepareOutOfBoundsCallbacks();	}

	private:
						void				syncState();
		PX_FORCE_INLINE	Ps::IntBool			isBuffered(BufferFlag f)	const	{ return Ps::IntBool(mBufferFlags& f);	}
		PX_FORCE_INLINE	void				markUpdated(BufferFlag f)			{ mBufferFlags |= f;		}

		//---------------------------------------------------------------------------------
		// Miscellaneous
		//---------------------------------------------------------------------------------
	public:
		PX_FORCE_INLINE bool				isPhysicsRunning()				const	{ return mSimulationRunning;			}
		PX_FORCE_INLINE	void				setPhysicsRunning(bool running)			{ mSimulationRunning = running;			}

		PX_FORCE_INLINE bool				isPhysicsBuffering()			const	{ return mIsBuffering;					}
		PX_FORCE_INLINE void				setPhysicsBuffering(bool buffering)		{ mIsBuffering = buffering;				}

		void								preSimulateUpdateAppThread(PxReal timeStep);	// Data updates that need to be handled in the application thread before the simulation potentially switches																					// to its own thread.

		PX_FORCE_INLINE bool				isValid()								const	{ return mScene.isValid();				}

		PX_FORCE_INLINE PxReal				getWakeCounterResetValue() const { return mWakeCounterResetValue; }

		void								switchRigidToNoSim(Scb::RigidObject&, bool isDynamic);
		void								switchRigidFromNoSim(Scb::RigidObject&, bool isDynamic);

		static size_t						getScOffset()	{ return reinterpret_cast<size_t>(&reinterpret_cast<Scene*>(0)->mScene); }

#if PX_SUPPORT_VISUAL_DEBUGGER
		PX_FORCE_INLINE	Pvd::SceneVisualDebugger&		getSceneVisualDebugger()			{ return mSceneVisualDebugger; }
		PX_FORCE_INLINE	const Pvd::SceneVisualDebugger&	getSceneVisualDebugger() const		{ return mSceneVisualDebugger; }
#endif

	private:
						void				addShapeInternal(Scb::Shape&);
						void				addShapesInternal(PxU32 nbShapes, PxShape** PX_RESTRICT shapes, size_t scbOffset, PxActor** PX_RESTRICT owners, PxU32 offsetNpToCore, bool isDynamic);

		template<typename T> T*				allocArrayBuffer(Ps::Array<T>& buffer, PxU32 count, PxU32& startIdx);

	private:


	template<typename T, bool addToPvd>		void add(T& v, ObjectTracker &tracker);
	template<typename T, bool removeFromPvd>void remove(T& v, ObjectTracker &tracker, bool wakeOnLostTouch = false);

	template<typename T, bool TIsDynamic, bool addToPvd>		void addRigidNoSim(T& v, ObjectTracker &tracker);
	template<typename T, bool TIsDynamic, bool removeFromPvd>	void removeRigidNoSim(T& v, ObjectTracker &tracker);

	template<typename T, typename S, bool pvdUpdates>
											void processSimUpdates(S*const * scObjects, PxU32 nbObjects, bool pvdConnected);

	template<typename T, bool pvdInserts, bool pvdUpdates>
											void processUserUpdates(ObjectTracker& tracker, bool pvdIsConnected);

	template<typename T, bool syncOnRemove, bool wakeOnLostTouchCheck, bool releasePvdInstance> 
											void processRemoves(ObjectTracker& tracker);

	template<typename T>					void processShapeRemoves(ObjectTracker& tracker);


						bool				isUpdatingPvd();
						Pvd::SceneVisualDebugger*	getPvd();

					Sc::Scene							mScene;
 
					Ps::Array<MaterialEvent>			mSceneMaterialBuffer;
					Ps::Mutex							mSceneMaterialBufferLock;
				
					bool								mSimulationRunning;
					bool								mIsBuffering;

					Cm::FlushPool						mStream;  // Pool for temporarily buffering user changes on objects
					ShapeManager						mShapeManager;
					Ps::Array<PxU16>					mShapeMaterialBuffer;  // Buffered setMaterial() call might need to track list of materials (for multi material shapes)
					Ps::Array<Scb::Shape*>				mShapePtrBuffer;  // List of shape pointers to track buffered calls to resetFiltering(), for example
					Ps::Array<Scb::Actor*>				mActorPtrBuffer;
					RigidStaticManager					mRigidStaticManager;
					BodyManager							mBodyManager;
#if PX_USE_PARTICLE_SYSTEM_API
					ParticleSystemManager				mParticleSystemManager;
#endif
					ConstraintManager					mConstraintManager;
					ArticulationManager					mArticulationManager;
					ArticulationJointManager			mArticulationJointManager;
					AggregateManager					mAggregateManager;
#if PX_USE_CLOTH_API
					ClothManager						mClothManager;
#endif
#if PX_SUPPORT_VISUAL_DEBUGGER
					Pvd::SceneVisualDebugger			mSceneVisualDebugger;
#endif

					PxReal								mWakeCounterResetValue;

					// note: If deletion of rigid objects is done before the sync of the simulation data then we
					//       might wanna consider having a separate list for deleted rigid objects (for performance
					//       reasons)

					//---------------------------------------------------------------------------------
					// On demand buffered data (simulation read-only data)
					//---------------------------------------------------------------------------------
					Scb::SceneBuffer					mBufferedData;
					PxU32								mBufferFlags;
	};

}  // namespace Scb


template<typename T>
T* Scb::Scene::allocArrayBuffer(Ps::Array<T>& buffer, PxU32 count, PxU32& startIdx)
{
	PxU32 oldSize = buffer.size();
	buffer.resize(oldSize + count);
	startIdx = oldSize;
	return &buffer[oldSize];
}


		
PX_INLINE void Scb::Scene::setGravity(const PxVec3& gravity)
{
	if (!isPhysicsRunning())
	{
		mScene.setGravity(gravity);
		UPDATE_PVD_PROPERTIES_SCENE()
	}
	else
	{
		mBufferedData.gravity = gravity;
		markUpdated(BF_GRAVITY);
	}
}

PX_INLINE PxVec3 Scb::Scene::getGravity() const
{
	if (isBuffered(BF_GRAVITY))
		return mBufferedData.gravity;
	else
		return mScene.getGravity();
}

void Scb::Scene::setBounceThresholdVelocity(const PxReal t)
{
	if (!isPhysicsRunning())
	{
		mScene.setBounceThresholdVelocity(t);
		UPDATE_PVD_PROPERTIES_SCENE()
	}
	else
	{
		mBufferedData.bounceThresholdVelocity = t;
		markUpdated(BF_BOUNCETHRESHOLDVELOCITY);
	}
}

PxReal Scb::Scene::getBounceThresholdVelocity() const
{
	if (isBuffered(BF_BOUNCETHRESHOLDVELOCITY))
		return mBufferedData.bounceThresholdVelocity;
	else
		return mScene.getBounceThresholdVelocity();
}

PX_INLINE void Scb::Scene::setFrictionType(PxFrictionType::Enum frictionType)
{
	mScene.setFrictionType(frictionType);
}

PX_INLINE PxFrictionType::Enum Scb::Scene::getFrictionType() const
{
	return mScene.getFrictionType();
}


PX_INLINE void Scb::Scene::setFlags(PxSceneFlags flags)
{
	if (!isPhysicsBuffering())
	{
		mScene.setPublicFlags(flags);
		const bool pcm = (flags & PxSceneFlag::eENABLE_PCM);
		mScene.setPCM(pcm);
		const bool contactCache = !(flags & PxSceneFlag::eDISABLE_CONTACT_CACHE);
		mScene.setContactCache(contactCache);
		UPDATE_PVD_PROPERTIES_SCENE()
	}
	else
	{
		mBufferedData.flags = flags;
		markUpdated(BF_FLAGS);
	}
}


PX_INLINE PxSceneFlags Scb::Scene::getFlags() const
{
	if (isBuffered(BF_FLAGS))
		return mBufferedData.flags;
	else
		return mScene.getPublicFlags();
}

PX_INLINE void Scb::Scene::initActiveBodiesIterator(Sc::BodyIterator& bodyIterator)
{
	PX_ASSERT(!mSimulationRunning);
	mScene.initActiveBodiesIterator(bodyIterator);
}

///////////////////////////////////////////////////////////////////////////////

PX_INLINE PxSimulationEventCallback* Scb::Scene::getSimulationEventCallback(PxClientID client) const
{
	return mScene.getSimulationEventCallback(client);
}

PX_INLINE void Scb::Scene::setSimulationEventCallback(PxSimulationEventCallback* callback, PxClientID client)
{
	if(!isPhysicsBuffering())
		mScene.setSimulationEventCallback(callback, client);
	else
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxScene::setSimulationEventCallback() not allowed while simulation is running. Call will be ignored.");
}

PX_INLINE PxContactModifyCallback* Scb::Scene::getContactModifyCallback() const
{
	return mScene.getContactModifyCallback();
}

PX_INLINE void Scb::Scene::setContactModifyCallback(PxContactModifyCallback* callback)
{
	if(!isPhysicsBuffering())
		mScene.setContactModifyCallback(callback);
	else
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxScene::setContactModifyCallback() not allowed while simulation is running. Call will be ignored.");
}

PX_INLINE PxCCDContactModifyCallback* Scb::Scene::getCCDContactModifyCallback() const
{
	return mScene.getCCDContactModifyCallback();
}

PX_INLINE void Scb::Scene::setCCDContactModifyCallback(PxCCDContactModifyCallback* callback)
{
	if(!isPhysicsBuffering())
		mScene.setCCDContactModifyCallback(callback);
	else
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxScene::setContactModifyCallback() not allowed while simulation is running. Call will be ignored.");
}

PX_INLINE PxU32 Scb::Scene::getCCDMaxPasses() const
{
	return mScene.getCCDMaxPasses();
}

PX_INLINE void Scb::Scene::setCCDMaxPasses(PxU32 ccdMaxPasses)
{
	if(!isPhysicsBuffering())
		mScene.setCCDMaxPasses(ccdMaxPasses);
	else
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxScene::setCCDMaxPasses() not allowed while simulation is running. Call will be ignored.");
}

PX_INLINE PxBroadPhaseCallback* Scb::Scene::getBroadPhaseCallback(PxClientID client) const
{
	return mScene.getBroadPhaseCallback(client);
}

PX_INLINE void Scb::Scene::setBroadPhaseCallback(PxBroadPhaseCallback* callback, PxClientID client)
{
	if(!isPhysicsBuffering())
		mScene.setBroadPhaseCallback(callback, client);
	else
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxScene::setBroadPhaseCallback() not allowed while simulation is running. Call will be ignored.");
}

///////////////////////////////////////////////////////////////////////////////

PX_INLINE const void* Scb::Scene::getFilterShaderData() const
{
	return mScene.getFilterShaderDataFast();
}


PX_INLINE PxU32 Scb::Scene::getFilterShaderDataSize() const
{
	return mScene.getFilterShaderDataSizeFast();
}


PX_INLINE PxSimulationFilterShader	Scb::Scene::getFilterShader() const
{
	return mScene.getFilterShaderFast();
}


PX_INLINE PxSimulationFilterCallback* Scb::Scene::getFilterCallback() const
{
	return mScene.getFilterCallbackFast();
}


PX_INLINE void Scb::Scene::simulate(PxReal timeStep, PxBaseTask* continuation)
{
	mScene.simulate(timeStep, continuation);
}

PX_INLINE void Scb::Scene::solve(PxReal timeStep, PxBaseTask* continuation)
{
	mScene.solve(timeStep, continuation);
}

PX_INLINE void Scb::Scene::collide(PxReal timeStep, PxBaseTask* continuation)
{
	mScene.collide(timeStep, continuation);
}

PX_INLINE void Scb::Scene::stepSetupCollide()
{
	mScene.stepSetupCollide();
}

PX_INLINE void Scb::Scene::endSimulation()
{
	mScene.endSimulation();
}


PX_INLINE void Scb::Scene::flush(bool sendPendingReports)
{
	PX_ASSERT(!isPhysicsBuffering());

	mShapeMaterialBuffer.reset();
	mShapePtrBuffer.reset();
	mActorPtrBuffer.reset();

	//!!! TODO: Clear all buffers used for double buffering changes (see ObjectTracker::mBufferPool)

	mScene.flush(sendPendingReports);
}


PX_INLINE void Scb::Scene::fireCallBacksPreSync()
{
	mScene.fireCallBacksPreSync();
}


PX_INLINE void Scb::Scene::fireCallBacksPostSync()
{
	mScene.fireCallBacksPostSync();
}


PX_INLINE void Scb::Scene::postReportsCleanup()
{
	PX_ASSERT(!isPhysicsBuffering());
	mScene.postReportsCleanup();
}


PX_INLINE PxReal Scb::Scene::getMeshContactMargin() const
{
	return mScene.getMeshContactMargin();
}


PX_INLINE const PxSceneLimits& Scb::Scene::getLimits() const
{
	return mScene.getLimits();
}

PX_INLINE void Scb::Scene::setLimits(const PxSceneLimits& limits)
{
	mScene.setLimits(limits);
}

PX_INLINE void Scb::Scene::setDominanceGroupPair(PxDominanceGroup group1, PxDominanceGroup group2, const PxDominanceGroupPair& dominance)
{
	if (!isPhysicsBuffering())
	{
		mScene.setDominanceGroupPair(group1, group2, dominance);
		UPDATE_PVD_PROPERTIES_SCENE()
	}
	else
	{
		mBufferedData.setDominancePair(group1, group2, dominance);
		markUpdated(BF_DOMINANCE_PAIRS);
	}
}


PX_INLINE PxDominanceGroupPair Scb::Scene::getDominanceGroupPair(PxDominanceGroup group1, PxDominanceGroup group2) const
{
	if (isBuffered(BF_DOMINANCE_PAIRS))
	{
		PxDominanceGroupPair dominance(0.0f, 0.0f);
		if (mBufferedData.getDominancePair(group1, group2, dominance))
			return dominance;
	}

	return mScene.getDominanceGroupPair(group1, group2);
}


PX_INLINE void Scb::Scene::setSolverBatchSize(PxU32 solverBatchSize)
{
	if (!isPhysicsBuffering())
	{
		mScene.setSolverBatchSize(solverBatchSize);
		UPDATE_PVD_PROPERTIES_SCENE()
	}
	else
	{
		mBufferedData.solverBatchSize = solverBatchSize;
		markUpdated(BF_SOLVER_BATCH_SIZE);
	}
}

PX_INLINE PxU32 Scb::Scene::getSolverBatchSize() const
{
	if (isBuffered(BF_SOLVER_BATCH_SIZE))
		return mBufferedData.solverBatchSize;
	else
		return mScene.getSolverBatchSize();
}


PX_INLINE void Scb::Scene::getStats(PxSimulationStatistics& stats) const
{
	PX_ASSERT(!isPhysicsBuffering());

	mScene.getStats(stats);
}


PX_INLINE void Scb::Scene::buildActiveTransforms()
{
	PX_ASSERT(!isPhysicsBuffering());

	mScene.buildActiveTransforms();
}


PX_INLINE PxActiveTransform* Scb::Scene::getActiveTransforms(PxU32& nbTransformsOut, PxClientID client)
{
	if (!isPhysicsRunning())
	{
		return mScene.getActiveTransforms(nbTransformsOut, client);
	}
	else
	{
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxScene::getActiveTransforms() not allowed while simulation is running. Call will be ignored.");
		nbTransformsOut = 0;
		return NULL;
	}
}

PX_INLINE PxClientID Scb::Scene::createClient()
{
	mBufferedData.clientBehaviorFlags.pushBack(PxClientBehaviorFlag_eNOT_BUFFERED);		//PxClientBehaviorFlag_eNOT_BUFFERED means its not storing anything.  Do this either way to make sure this buffer is big enough for behavior bit set/gets later.

	if (!isPhysicsBuffering())
	{
		PxClientID i = mScene.createClient();
		PX_ASSERT(mBufferedData.clientBehaviorFlags.size()-1 == i);
		return i;
	}
	else
	{
		mBufferedData.numClientsCreated++;
		return PxClientID(mBufferedData.clientBehaviorFlags.size()-1);	//mScene.createClient();
	}
}

PX_INLINE void Scb::Scene::setClientBehaviorFlags(PxClientID client, PxClientBehaviorFlags clientBehaviorFlags)
{
	if (!isPhysicsBuffering())
	{
		mScene.setClientBehaviorFlags(client, clientBehaviorFlags);
		UPDATE_PVD_PROPERTIES_SCENE()
	}
	else
	{
		PX_ASSERT(mBufferedData.clientBehaviorFlags.size() > client);
		mBufferedData.clientBehaviorFlags[client] = clientBehaviorFlags;
		markUpdated(BF_CLIENT_BEHAVIOR_FLAGS);
	}
}

PX_INLINE PxClientBehaviorFlags Scb::Scene::getClientBehaviorFlags(PxClientID client) const
{
	PX_ASSERT(mBufferedData.clientBehaviorFlags.size() > client);
	if (isBuffered(BF_CLIENT_BEHAVIOR_FLAGS) && (mBufferedData.clientBehaviorFlags[client] != PxClientBehaviorFlag_eNOT_BUFFERED))
		return mBufferedData.clientBehaviorFlags[client];
	else
		return mScene.getClientBehaviorFlags(client);
}

#if PX_USE_CLOTH_API

PX_INLINE void Scb::Scene::setClothInterCollisionDistance(PxF32 distance)
{
	if (!isPhysicsBuffering())
	{
		mScene.setClothInterCollisionDistance(distance);
	}
	else
	{
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxScene::setClothInterCollisionDistance() not allowed while simulation is running. Call will be ignored.");
	}	
}

PX_INLINE PxF32	Scb::Scene::getClothInterCollisionDistance() const
{
	return mScene.getClothInterCollisionDistance();
}

PX_INLINE void Scb::Scene::setClothInterCollisionStiffness(PxF32 stiffness)
{
	if (!isPhysicsBuffering())
	{
		mScene.setClothInterCollisionStiffness(stiffness);
	}
	else
	{
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxScene::setClothInterCollisionStiffness() not allowed while simulation is running. Call will be ignored.");
	}	
}

PX_INLINE PxF32 Scb::Scene::getClothInterCollisionStiffness() const
{
	return mScene.getClothInterCollisionStiffness();
}

PX_INLINE void Scb::Scene::setClothInterCollisionNbIterations(PxU32 nbIterations)
{
	if (!isPhysicsBuffering())
	{
		mScene.setClothInterCollisionNbIterations(nbIterations);
	}
	else
	{
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxScene::setClothInterCollisionNbIterations() not allowed while simulation is running. Call will be ignored.");
	}	
}

PX_INLINE PxU32 Scb::Scene::getClothInterCollisionNbIterations() const
{
	return mScene.getClothInterCollisionNbIterations();
}

#endif


PX_INLINE void Scb::Scene::setVisualizationParameter(PxVisualizationParameter::Enum param, PxReal value)
{
	if (!isPhysicsBuffering())
		mScene.setVisualizationParameter(param, value);
	else
	{
		PX_ASSERT(param < PxVisualizationParameter::eNUM_VALUES);
		mBufferedData.visualizationParamChanged[param] = 1;
		mBufferedData.visualizationParam[param] = value;
		markUpdated(BF_VISUALIZATION);
	}
}

PX_INLINE PxReal Scb::Scene::getVisualizationParameter(PxVisualizationParameter::Enum param) const
{
	PX_ASSERT(param < PxVisualizationParameter::eNUM_VALUES);

	if (isBuffered(BF_VISUALIZATION) && mBufferedData.visualizationParamChanged[param])
		return mBufferedData.visualizationParam[param];
	else
		return mScene.getVisualizationParameter(param);
}

PX_INLINE void Scb::Scene::setVisualizationCullingBox(const PxBounds3& box)
{
	if (!isPhysicsBuffering())
		mScene.setVisualizationCullingBox(box);
	else
	{
		mBufferedData.visualizationCullingBoxChanged = 1;
		mBufferedData.visualizationCullingBox = box;
		markUpdated(BF_VISUALIZATION);
	}
}

PX_INLINE const PxBounds3& Scb::Scene::getVisualizationCullingBox() const
{
	if (isBuffered(BF_VISUALIZATION) && mBufferedData.visualizationCullingBoxChanged)
		return mBufferedData.visualizationCullingBox;
	else
		return mScene.getVisualizationCullingBox();
}

}

#endif
