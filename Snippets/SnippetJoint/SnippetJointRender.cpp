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

#ifdef RENDER_SNIPPET

#include <vector>

#include "PxPhysicsAPI.h"

#include "../SnippetRender/SnippetRender.h"
#include "../SnippetRender/SnippetCamera.h"

using namespace physx;
#pragma warning(disable: 4100)

extern void initPhysics(bool interactive);
extern void stepPhysics(bool interactive);
extern void cleanupPhysics(bool interactive);
extern void keyPress(const char key, const PxTransform& camera);

namespace
{
	Snippets::Camera*	sCamera;

	void motionCallback(int x, int y)
	{
		sCamera->handleMotion(x, y);
	}

	void SpecialKeyboardCallback(int key, int x, int y)
	{
		//arrowKeyPress(key);
	}

	void keyboardCallback(unsigned char key, int x, int y)
	{
		if (key == 27)
			exit(0);

		if (!sCamera->handleKey(key, x, y, 0.1f))
			//keyPress(key, sCamera->getTransform());
			keyPress(key, PxTransform::createIdentity());
	}

	void mouseCallback(int button, int state, int x, int y)
	{
		sCamera->handleMouse(button, state, x, y);
	}

	void timerCallback(int val)
	{
		//glutPostRedisplay();
		//stepPhysics(true);
		glutTimerFunc(500, timerCallback, 1);
	}
	
	void idleCallback()
	{
		glutPostRedisplay();
		//glutTimerFunc(1.f / 100.f * 1000, timerCallback, 1);
	}



	void renderCallback()
	{
		stepPhysics(true);

		Snippets::startRender(sCamera->getEye(), sCamera->getDir());

		PxScene* scene;
		PxGetPhysics().getScenes(&scene, 1);
		PxU32 nbActors = scene->getNbActors(PxActorTypeSelectionFlag::eRIGID_DYNAMIC | PxActorTypeSelectionFlag::eRIGID_STATIC);
		if (nbActors)
		{
			std::vector<PxRigidActor*> actors(nbActors);
			scene->getActors(PxActorTypeSelectionFlag::eRIGID_DYNAMIC | PxActorTypeSelectionFlag::eRIGID_STATIC, (PxActor**)&actors[0], nbActors);
			Snippets::renderActors(&actors[0], (PxU32)actors.size(), true);
		}

		const PxRenderBuffer& rb = scene->getRenderBuffer();
		for (PxU32 i = 0; i < rb.getNbLines(); i++)
		{
			const PxDebugLine& line = rb.getLines()[i];
			Snippets::renderLine(line.pos0, line.pos1, PxVec3(line.color0 & 0xff, (line.color0 >> 8) & 0xff, (line.color0 >> 16) & 0xff));
		}

		Snippets::finishRender();
	}

	void exitCallback(void)
	{
		delete sCamera;
		cleanupPhysics(true);
	}
}


void renderLoop()
{
	//sCamera = new Snippets::Camera(PxVec3(50.0f, 50.0f, 50.0f), PxVec3(-0.6f,-0.2f,-0.7f));

	//-50.406940, 60.818699, 85.356842
	//sCamera = new Snippets::Camera(PxVec3(0.0f,5.0f,50.0f), PxVec3(0.0f,0.0f,-1.0f));
	PxVec3 eye = 0.05 * PxVec3(-50.406940, 60.818699, 85.356842);
	sCamera = new Snippets::Camera(eye, -eye);

	Snippets::setupDefaultWindow("Ragdoll");
	Snippets::setupDefaultRenderState();


	glutIdleFunc(idleCallback);
	glutDisplayFunc(renderCallback);

	glutSpecialFunc(SpecialKeyboardCallback);
	glutKeyboardFunc(keyboardCallback);
	glutMouseFunc(mouseCallback);
	glutMotionFunc(motionCallback);
	motionCallback(0, 0);
	

	atexit(exitCallback);

	initPhysics(true);
	//glutTimerFunc(1.f / 60.f * 1000.f, timerCallback, 1);
	
	glutMainLoop();

}

void updateCameraDsc(const PxTransform& pos)
{
	sCamera->mEye = PxVec3(pos.p.x, pos.p.y+2.0, pos.p.z + 5.0f);
	sCamera->mDir = PxVec3(0.0f, -0.5f, -1.0f);
}
#endif
