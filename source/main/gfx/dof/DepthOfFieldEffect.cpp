/*
This source file is part of Rigs of Rods
Copyright 2005-2012 Pierre-Michel Ricordel
Copyright 2007-2012 Thomas Fischer

For more information, see http://www.rigsofrods.com/

Rigs of Rods is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3, as
published by the Free Software Foundation.

Rigs of Rods is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rigs of Rods.  If not, see <http://www.gnu.org/licenses/>.
*/
// "Depth of Field" demo for Ogre
// Copyright (C) 2006  Christian Lindequist Larsen
//
// This code is in the public domain. You may do whatever you want with it.

#include "DepthOfFieldEffect.h"

#include "Lens.h"
#include "Ogre.h"
#include "Settings.h"

using namespace Ogre;

const int DepthOfFieldEffect::BLUR_DIVISOR = 2;

DepthOfFieldEffect::DepthOfFieldEffect(Ogre::Viewport *v, Ogre::Camera *cam) :
	  mCamera(cam)
	, mCompositor(NULL)
	, mDepthTarget(NULL)
	, mDepthTechnique(NULL)
	, mDepthViewport(NULL)
	, mFarBlurCutoff(1.0)
	, mFarDepth(190.0)
	, mFocalDepth(100.0)
	, mNearDepth(10.0)
	, mViewport(v)
{
	mWidth = mViewport->getActualWidth();
	mHeight = mViewport->getActualHeight();
	
	mDepthTexture.setNull();
	mDepthMaterial.setNull();

	createDepthRenderTexture();
	addCompositor();
}

DepthOfFieldEffect::~DepthOfFieldEffect()
{
	removeCompositor();
	destroyDepthRenderTexture();
}

void DepthOfFieldEffect::setFocalDepths(float nearDepth, float focalDepth, float farDepth)
{
	mNearDepth = nearDepth;
	mFocalDepth = focalDepth;
	mFarDepth = farDepth;
}

void DepthOfFieldEffect::setFarBlurCutoff(float cutoff)
{
	mFarBlurCutoff = cutoff;
}

bool DepthOfFieldEffect::getEnabled() const
{
	return mCompositor->getEnabled();
}

void DepthOfFieldEffect::setEnabled(bool value)
{
	mCompositor->setEnabled(value);
}

void DepthOfFieldEffect::createDepthRenderTexture()
{
	// Create the depth render texture
	mDepthTexture = TextureManager::getSingleton().createManual(
		"DoF_Depth",ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		TEX_TYPE_2D, mWidth, mHeight,
		0, PF_L8, TU_RENDERTARGET);

	// Get its render target and add a viewport to it
	mDepthTarget = mDepthTexture->getBuffer()->getRenderTarget();
	mDepthViewport = mDepthTarget->addViewport(mCamera);

	// Register 'this' as a render target listener
	mDepthTarget->addListener(this);

	// Get the technique to use when rendering the depth render texture
	mDepthMaterial = MaterialManager::getSingleton().getByName("DoF_Depth");
	mDepthMaterial->load(); // needs to be loaded manually
	mDepthTechnique = mDepthMaterial->getBestTechnique();

	// Create a custom render queue invocation sequence for the depth render texture
    RenderQueueInvocationSequence* invocationSequence = Root::getSingleton().createRenderQueueInvocationSequence("DoF_Depth");

	// Add a render queue invocation to the sequence, and disable shadows for it
	/*
	RenderQueueInvocation* invocation = invocationSequence->add(RENDER_QUEUE_MAIN, "main");
    invocation->setSuppressShadows(true);
	//invocation->setSuppressRenderStateChanges(true);
	invocation->setSolidsOrganisation(QueuedRenderableCollection::OM_SORT_ASCENDING);
	// Set the render queue invocation sequence for the depth render texture viewport
    mDepthViewport->setRenderQueueInvocationSequenceName("DoF_Depth");
	*/

	mDepthViewport->setShadowsEnabled(false);
	mDepthViewport->setOverlaysEnabled(false);
	//mDepthViewport->setSkiesEnabled(false);

	mDepthViewport->setVisibilityMask(~DEPTHMAP_DISABLED);

	//re-set texture "DoF_Depth"
	MaterialPtr p = MaterialManager::getSingleton().getByName("DoF_DepthOfField");
	p->load();
	p->getBestTechnique()->getPass(0)->getTextureUnitState("depth")->setTextureName("DoF_Depth");
	p->unload();
}

void DepthOfFieldEffect::destroyDepthRenderTexture()
{
	mDepthViewport->setRenderQueueInvocationSequenceName("");
	Root::getSingleton().destroyRenderQueueInvocationSequence("DoF_Depth");

	mDepthMaterial->unload();

	mDepthTarget->removeAllListeners();
	mDepthTarget->removeAllViewports();
	//TextureManager::getSingleton().unload("DoF_Depth");
	TextureManager::getSingleton().remove("DoF_Depth");
}

void DepthOfFieldEffect::addCompositor()
{
	mCompositor = CompositorManager::getSingleton().addCompositor(mViewport, "DoF_Compositor_test");
	mCompositor->addListener(this);

	mCompositor->setEnabled(true);
}

void DepthOfFieldEffect::removeCompositor()
{
	mCompositor->setEnabled(false);

	mCompositor->removeListener(this);
	CompositorManager::getSingleton().removeCompositor(mViewport, "DoF_Compositor_test");
}

void DepthOfFieldEffect::notifyMaterialSetup(uint32 passId, MaterialPtr& material)
{
	switch (passId)
	{
	case BlurPass:
		{
			//float pixelSize[2] = {
			//	1.0f / (mViewport->getActualWidth() / BLUR_DIVISOR),
			//	1.0f / (mViewport->getActualHeight() / BLUR_DIVISOR)};

			// Adjust fragment program parameters
			Ogre::Vector3 ps = Ogre::Vector3(1.0f / (mWidth / BLUR_DIVISOR),1.0f / (mHeight / BLUR_DIVISOR), 1.0f);
			float pixelSize[3] = { ps.x, ps.y, ps.z };
			GpuProgramParametersSharedPtr fragParams = material->getBestTechnique()->getPass(0)->getFragmentProgramParameters();
			if((!fragParams.isNull())&&(fragParams->_findNamedConstantDefinition("pixelSize")))
				fragParams->setNamedConstant("pixelSize", pixelSize, 1, 3);

			break;
		}

	case OutputPass:
		{
			float pixelSizeScene[3] = {
				1.0f / mWidth,
				1.0f / mHeight,
				0};

			float pixelSizeBlur[3] = {
				1.0f / (mWidth / BLUR_DIVISOR),
				1.0f / (mHeight / BLUR_DIVISOR),
				0};

			// Adjust fragment program parameters
			GpuProgramParametersSharedPtr fragParams =
				material->getBestTechnique()->getPass(0)->getFragmentProgramParameters();
			if((!fragParams.isNull())&&(fragParams->_findNamedConstantDefinition("pixelSizeScene")))
				fragParams->setNamedConstant("pixelSizeScene", pixelSizeScene,1,3);
			if((!fragParams.isNull())&&(fragParams->_findNamedConstantDefinition("pixelSizeBlur")))
				fragParams->setNamedConstant("pixelSizeBlur", pixelSizeBlur,1,3);

			break;
		}
	}
}

void DepthOfFieldEffect::preViewportUpdate(const Ogre::RenderTargetViewportEvent& evt)
{

	float dofParams[4] = {mNearDepth, mFocalDepth, mFarDepth, mFarBlurCutoff};

	// Adjust fragment program parameters for depth pass
	GpuProgramParametersSharedPtr fragParams =
		mDepthTechnique->getPass(0)->getFragmentProgramParameters();
	if((!fragParams.isNull())&&(fragParams->_findNamedConstantDefinition("dofParams")))
		fragParams->setNamedConstant("dofParams", dofParams,1,4);		

	evt.source->setVisibilityMask(~DEPTHMAP_DISABLED);

	// Add 'this' as a RenderableListener to replace the technique for all renderables
	RenderQueue* queue = evt.source->getCamera()->getSceneManager()->getRenderQueue();
	queue->setRenderableListener(this);
}

void DepthOfFieldEffect::postViewportUpdate(const Ogre::RenderTargetViewportEvent& evt)
{
	// Reset the RenderableListener
	RenderQueue* queue = evt.source->getCamera()->getSceneManager()->getRenderQueue();
	queue->setRenderableListener(0);
}

bool DepthOfFieldEffect::renderableQueued(Ogre::Renderable* rend, Ogre::uint8 groupID,
				Ogre::ushort priority, Ogre::Technique** ppTech, Ogre::RenderQueue* pQueue)
{
	// Replace the technique of all renderables
	*ppTech = mDepthTechnique;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
DOFManager::DOFManager(Ogre::SceneManager *m, Ogre::Viewport *mViewport, Ogre::Root *root, Ogre::Camera *cam) : mRoot(root), mCamera(cam), mSceneMgr(m)
{
	mFocusMode = Manual;
	mAutoSpeed = 30;
	mAutoTime = 0.5f;
	targetFocalDistance = 5;

	mDepthOfFieldEffect = new DepthOfFieldEffect(mViewport, mCamera);
	mLens = new Lens(mCamera->getFOVy(), 1);
	mLens->setFocalDistance(5);
	//mLens->setFStop(10);
//	mDepthOfFieldEffect->setEnabled(false);
	mRoot->addFrameListener(this);

	mRaySceneQuery = mSceneMgr->createRayQuery(Ogre::Ray());
	mRaySceneQuery->setSortByDistance(true);
	//mRaySceneQuery->setQueryMask(queryMask);

	debugNode = 0;
	if (BSETTING("DOFDebug", false))
	{
		MaterialPtr material = MaterialManager::getSingleton().getByName("DoF_DepthDebug");
		material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName("DoF_Depth");
		Overlay* overlay = OverlayManager::getSingleton().getByName("DoF_DepthDebugOverlay");
		overlay->show();

		debugNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		Entity *ent = mSceneMgr->createEntity("sphere.mesh");
		debugNode->attachObject(ent);
		debugNode->setScale(0.5, 0.5, 0.5);
	}
}

DOFManager::~DOFManager()
{
	cleanup();
}

void DOFManager::cleanup()
{
	mRoot->removeFrameListener(this);

	delete mLens;
	mLens = NULL;

	delete mDepthOfFieldEffect;
	mDepthOfFieldEffect = NULL;
}

void DOFManager::setAutoSpeed(float f)
{
	mAutoSpeed = f;
}

void DOFManager::setEnabled(bool enabled)
{
	mDepthOfFieldEffect->setEnabled(enabled);
	/*
	// crashes for some reason
	if(enabled && !mDepthOfFieldEffect->getEnabled())
	{
		// turn on
		mDepthOfFieldEffect->setEnabled(true);
		mRoot->addFrameListener(this);
	} else if(!enabled && mDepthOfFieldEffect->getEnabled())
	{
		// turn off
		mDepthOfFieldEffect->setEnabled(false);
		mRoot->removeFrameListener(this);
	}
	*/
}

bool DOFManager::getEnabled()
{
	return mDepthOfFieldEffect->getEnabled();
}

void DOFManager::zoomView(float delta)
{
	Real fieldOfView = mLens->getFieldOfView().valueRadians();
	fieldOfView += delta;
	fieldOfView = std::max<Real>(0.1, std::min<Real>(fieldOfView, 2.0));
	mLens->setFieldOfView(Radian(fieldOfView));
	mCamera->setFOVy(Radian(fieldOfView));
}

void DOFManager::Aperture(float delta)
{
	if (mFocusMode == Pinhole)
		return;
	Real fStop = mLens->getFStop();
	fStop += delta;
	fStop = std::max<Real>(0.5, std::min<Real>(fStop, 12.0));
	mLens->setFStop(fStop);
}

void DOFManager::moveFocus(float delta)
{
	mLens->setFocalDistance(mLens->getFocalDistance() + delta);
}

void  DOFManager::setZoom(float f)
{
	Real fieldOfView = Degree(Real(f)).valueRadians();
	fieldOfView = std::max<Real>(0.1, std::min<Real>(fieldOfView, 2.0));
	mLens->setFieldOfView(Radian(fieldOfView));
	mCamera->setFOVy(Radian(fieldOfView));
}

void  DOFManager::setLensFOV(Radian fov)
{
	mLens->setFieldOfView(fov);
}

void  DOFManager::setAperture(float f)
{
	if (mFocusMode == Pinhole)
		return;
	Real fStop = f;
	fStop = std::max<Real>(0.5, std::min<Real>(fStop, 12.0));
	mLens->setFStop(fStop);
}

void  DOFManager::setFocus(float f)
{
	mLens->setFocalDistance(f);
}

bool DOFManager::frameStarted(const FrameEvent& evt)
{
	Camera *camera = mCamera;
	// Focusing
	switch (mFocusMode)
	{
		case Auto:
		{
			// TODO: Replace with accurate ray/triangle collision detection
			Real currentFocalDistance = mLens->getFocalDistance();

			// TODO: Continous AF / triggered
			mAutoTime -= evt.timeSinceLastFrame;
			if(mAutoTime <= 0.0f)
			{
				mAutoTime = 0.5f;

				targetFocalDistance = currentFocalDistance;

				// Ryan Booker's (eyevee99) ray scene query auto focus
				Ray focusRay;
				focusRay.setOrigin(camera->getDerivedPosition());
				focusRay.setDirection(camera->getDerivedDirection());
				Vector3 v;
				Vector3 vn;
				
				mRaySceneQuery->setRay(focusRay);
				Ogre::RaySceneQueryResult &qryResult = mRaySceneQuery->execute();
				for (Ogre::RaySceneQueryResult::iterator it = qryResult.begin();it != qryResult.end(); it++)
				{
					if(it->worldFragment)
					{
						if(debugNode) debugNode->setPosition(it->worldFragment->singleIntersection + Vector3(0.5,0,0));
						targetFocalDistance = (mCamera->getPosition() - it->worldFragment->singleIntersection).length();
						break;
					} else
					{
						/*
						// this wont work since we would need to go down to the polygon level :(
						if(debugNode) debugNode->setPosition(focusRay.getPoint(it->distance));
						targetFocalDistance = it->distance;
						break;
						*/
					}

				}
			}

			// Slowly adjust the focal distance (emulate auto focus motor)
			if (currentFocalDistance < targetFocalDistance)
			{
				mLens->setFocalDistance(
					std::min<Real>(currentFocalDistance + mAutoSpeed * evt.timeSinceLastFrame, targetFocalDistance));
			}
			else if (currentFocalDistance > targetFocalDistance)
			{
				mLens->setFocalDistance(
					std::max<Real>(currentFocalDistance - mAutoSpeed * evt.timeSinceLastFrame, targetFocalDistance));
			}

			break;
		}

		case Manual:
			//we set the values elsewhere
			break;
	}

	// Update Depth of Field effect
	if (mFocusMode != Pinhole)
	{
		mDepthOfFieldEffect->setEnabled(true);

		// Calculate and set depth of field using lens
		float nearDepth, focalDepth, farDepth;
		mLens->recalculateDepthOfField(nearDepth, focalDepth, farDepth);
		mDepthOfFieldEffect->setFocalDepths(nearDepth, focalDepth, farDepth);
	}
	else
	{
		mDepthOfFieldEffect->setEnabled(false);
	}

	return true;
}
