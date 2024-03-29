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
#include "dashboard.h"

#include "Console.h"
#include "ResourceBuffer.h"
#include "TruckHUD.h"

using namespace Ogre;

class DashboardListener;

Dashboard::Dashboard(SceneManager *mSceneMgr) :
	  mScene(mSceneMgr)
	, mDashCam(0)
	, mDashboardListener(0)
	, rttTex(0)
{
	TexturePtr rttTexPtr = TextureManager::getSingleton().createManual("dashtexture", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, 1024, 512, 0, PF_R8G8B8, TU_RENDERTARGET, new ResourceBuffer());
	rttTex = rttTexPtr->getBuffer()->getRenderTarget();

	mDashCam = mSceneMgr->createCamera("DashCam");
	mDashCam->setNearClipDistance(1.0);
	mDashCam->setFarClipDistance(10.0);
	mDashCam->setPosition(Vector3(0.0, -10000.0, 0.0));

	mDashCam->setAspectRatio(2.0);

	Viewport *v = rttTex->addViewport(mDashCam);
	v->setClearEveryFrame(true);
	v->setBackgroundColour(ColourValue::Black);
	//v->setOverlaysEnabled(false);

	MaterialPtr mat = MaterialManager::getSingleton().getByName("renderdash");
	mat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName("dashtexture");

	mDashboardListener = new DashboardListener(mSceneMgr);

	rttTex->addListener(mDashboardListener);

	mDashboardListener->dashOverlay     = OverlayManager::getSingleton().getByName("tracks/3D_DashboardOverlay");
	mDashboardListener->needlesOverlay  = OverlayManager::getSingleton().getByName("tracks/3D_NeedlesOverlay");
	mDashboardListener->blendOverlay    = OverlayManager::getSingleton().getByName("tracks/3D_BlendOverlay");
	mDashboardListener->truckHUDOverlay = OverlayManager::getSingleton().getByName("tracks/TruckInfoBox");
//	mDashboardListener->dbdebugOverlay  = OverlayManager::getSingleton().getByName("Core/DebugOverlay");
//	mDashboardListener->dbeditorOverlay = OverlayManager::getSingleton().getByName("tracks/EditorOverlay");
}

void Dashboard::setEnable(bool en)
{
	rttTex->setActive(en);
}

void Dashboard::prepareShutdown()
{
	if (rttTex)
	{
		rttTex->removeListener(mDashboardListener);
	}
}

DashboardListener::DashboardListener(Ogre::SceneManager *mSceneMgr) : 
	  consolevisible(false)
	, fpsDisplayed(false)
	, fpsOverlay(0)
	, mScene(mSceneMgr)
	, truckHUD(false)
	, truckhHUDvisible(false)
{
}

void DashboardListener::preRenderTargetUpdate(const RenderTargetEvent& evt)
{
	// hide everything
	mScene->setFindVisibleObjects(false);

	// hide fps stats
	if (fpsOverlay)
	{
		fpsDisplayed = fpsOverlay->isVisible();
		if (fpsDisplayed)
		{
			fpsOverlay->hide();
		}
	} else
	{
		// this must be here and not in the constructor, as upon construction time the overlaymanager is not working, somehow
		fpsOverlay = OverlayManager::getSingleton().getByName("Core/DebugOverlay");
	}

	// hide truck HUD
	truckhHUDvisible = truckHUDOverlay->isVisible();
	if (truckhHUDvisible)
	{
		truckHUDOverlay->hide();
	}

	//show overlay
	dashOverlay->show();
	needlesOverlay->show();
	blendOverlay->show();
}

void DashboardListener::postRenderTargetUpdate(const RenderTargetEvent& evt)
{
	// show everything
	mScene->setFindVisibleObjects(true);

	// show everything again, if it was displayed before hiding it...
	if (fpsOverlay && fpsDisplayed)
	{
		fpsOverlay->show();
	}

	if (truckHUDOverlay && truckhHUDvisible)
	{
		truckHUDOverlay->show();
	}

	// hide overlay
	dashOverlay->hide();
	needlesOverlay->hide();
	blendOverlay->hide();
}
