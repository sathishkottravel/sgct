/*************************************************************************
Copyright (c) 2012-2014 Miroslav Andel
All rights reserved.

For conditions of distribution and use, see copyright notice in sgct.h 
*************************************************************************/

#ifndef _CORRECTION_MESH_H_
#define _CORRECTION_MESH_H_

namespace sgct_core
{

struct CorrectionMeshVertex
{
	float x, y;	//Vertex 8
	float s, t;	//Texcoord0 8
	float r, g, b, a; //color 16

	//ATI performs better using sizes of power of two
};

class CorrectionMeshGeometry
{
public:
	CorrectionMeshGeometry();
	~CorrectionMeshGeometry();

	GLenum mGeometryType;
	unsigned int mNumberOfVertices;
	unsigned int mNumberOfIndices;
	unsigned int mMeshData[3];
};

class Viewport;

/*!
Helper class for reading and rendering a correction mesh.
A correction mesh is used for warping and edge-blending.
*/
class CorrectionMesh
{
public:
	CorrectionMesh();
	~CorrectionMesh();
	void setViewportCoords(float vpXSize, float vpYSize, float vpXPos, float vpYPos);
	bool readAndGenerateMesh(const char * meshPath, Viewport * parent);
	void render(bool mask);

private:
	bool readAndGenerateScalableMesh(const char * meshPath, Viewport * parent);
	bool readAndGenerateScissMesh(const char * meshPath, Viewport * parent);
	void setupSimpleMesh();
	void setupMaskMesh();
	void createMesh(CorrectionMeshGeometry * geomPtr);
	void cleanUp();

	enum buffer { Vertex = 0, Index, Array };
	enum textureCoordMode { NoMask = 0, Mask};

	CorrectionMeshVertex * mTempVertices;
	unsigned int * mTempIndices;

	CorrectionMeshGeometry mGeometries[2];

	float mXSize;
	float mYSize;
	float mXOffset;
	float mYOffset;
};

} //sgct_core

#endif
