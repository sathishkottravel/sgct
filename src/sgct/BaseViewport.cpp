/*************************************************************************
Copyright (c) 2012-2015 Miroslav Andel
All rights reserved.

For conditions of distribution and use, see copyright notice in sgct.h 
*************************************************************************/

#include "../include/sgct/BaseViewport.h"
#include "../include/sgct/ClusterManager.h"
#include "../include/sgct/MessageHandler.h"

bool sgct_core::BaseViewport::isEnabled()
{
	return mEnabled;
}

/*!
Name this viewport
*/
void sgct_core::BaseViewport::setName(const std::string & name)
{
	mName = name;
}

void sgct_core::BaseViewport::setPos(float x, float y)
{
	mX = x;
	mY = y;
}

void sgct_core::BaseViewport::setSize(float x, float y)
{
	mXSize = x;
	mYSize = y;
}

void sgct_core::BaseViewport::setEnabled(bool state)
{
	mEnabled = state;
}

void sgct_core::BaseViewport::setEye(sgct_core::Frustum::FrustumMode eye)
{
	mEye = eye;
}

std::string sgct_core::BaseViewport::getName()
{
	return mName;
}

float sgct_core::BaseViewport::getX()
{
	return mX;
}

float sgct_core::BaseViewport::getY()
{
	return mY;
}

float sgct_core::BaseViewport::getXSize()
{
	return mXSize;
}

float sgct_core::BaseViewport::getYSize()
{
	return mYSize;
}

void sgct_core::BaseViewport::setUser(sgct_core::SGCTUser * user)
{
	mUser = user;
}

void sgct_core::BaseViewport::setUserName(std::string userName)
{
	mUserName.assign(userName);
	linkUserName();
}

void sgct_core::BaseViewport::linkUserName()
{
	SGCTUser * user = ClusterManager::instance()->getUserPtr(mUserName);
	if (user != NULL)
	{
		mUser = user;
	}
}

void sgct_core::BaseViewport::calculateFrustum(const sgct_core::Frustum::FrustumMode &frustumMode, float near_clipping_plane, float far_clipping_plane)
{
	glm::vec3 eyePos = mUser->getPos(frustumMode);
	mProjections[frustumMode].calculateProjection(eyePos, &mProjectionPlane, near_clipping_plane, far_clipping_plane);
}

void sgct_core::BaseViewport::calculateFisheyeFrustum(const sgct_core::Frustum::FrustumMode &frustumMode, float near_clipping_plane, float far_clipping_plane)
{
	glm::vec3 eyePos = mUser->getPos();
	glm::vec3 offset = mUser->getPos(frustumMode) - eyePos;
	mProjections[frustumMode].calculateProjection(eyePos, &mProjectionPlane, near_clipping_plane, far_clipping_plane, offset);
}

void sgct_core::BaseViewport::setViewPlaneCoordsUsingFOVs(float up, float down, float left, float right, glm::quat rot, float dist)
{
	glm::vec3 unTransformedViewPlaneCoords[3];

	unTransformedViewPlaneCoords[SGCTProjectionPlane::LowerLeft].x = dist * tanf(glm::radians<float>(left));
	unTransformedViewPlaneCoords[SGCTProjectionPlane::LowerLeft].y = dist * tanf(glm::radians<float>(down));
	unTransformedViewPlaneCoords[SGCTProjectionPlane::LowerLeft].z = -dist;

	unTransformedViewPlaneCoords[SGCTProjectionPlane::UpperLeft].x = dist * tanf(glm::radians<float>(left));
	unTransformedViewPlaneCoords[SGCTProjectionPlane::UpperLeft].y = dist * tanf(glm::radians<float>(up));
	unTransformedViewPlaneCoords[SGCTProjectionPlane::UpperLeft].z = -dist;

	unTransformedViewPlaneCoords[SGCTProjectionPlane::UpperRight].x = dist * tanf(glm::radians<float>(right));
	unTransformedViewPlaneCoords[SGCTProjectionPlane::UpperRight].y = dist * tanf(glm::radians<float>(up));
	unTransformedViewPlaneCoords[SGCTProjectionPlane::UpperRight].z = -dist;

	//mProjectionPlane.setCoordinate(SGCTProjectionPlane::LowerLeft, )

	for (std::size_t i = 0; i < 3; i++)
		mProjectionPlane.setCoordinate(i, (rot * unTransformedViewPlaneCoords[i]) - mUser->getPos());

	sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_DEBUG,
		"Viewplane lower left coords: %f %f %f\n",
		mProjectionPlane.getCoordinatePtr(SGCTProjectionPlane::LowerLeft)->x,
		mProjectionPlane.getCoordinatePtr(SGCTProjectionPlane::LowerLeft)->y,
		mProjectionPlane.getCoordinatePtr(SGCTProjectionPlane::LowerLeft)->z);

	sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_DEBUG,
		"Viewplane upper left coords: %f %f %f\n",
		mProjectionPlane.getCoordinatePtr(SGCTProjectionPlane::UpperLeft)->x,
		mProjectionPlane.getCoordinatePtr(SGCTProjectionPlane::UpperLeft)->y,
		mProjectionPlane.getCoordinatePtr(SGCTProjectionPlane::UpperLeft)->z);

	sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_DEBUG,
		"Viewplane upper right coords: %f %f %f\n\n",
		mProjectionPlane.getCoordinatePtr(SGCTProjectionPlane::UpperRight)->x,
		mProjectionPlane.getCoordinatePtr(SGCTProjectionPlane::UpperRight)->y,
		mProjectionPlane.getCoordinatePtr(SGCTProjectionPlane::UpperRight)->z);
}
