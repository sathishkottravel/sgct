#include <stdlib.h>
#include <stdio.h>
#include "sgct.h"

sgct::Engine * gEngine;

void myDrawFun();
void myPreSyncFun();
void myPostSyncPreDrawFun();
void myInitOGLFun();
void myCleanUpFun();
void myEncodeFun();
void myDecodeFun();

void drawScene();
void clearBuffers();
void createFBOs();
void resizeFBOs();
void createTextures();

//FBO-stuff
struct fbData
{
	unsigned int texture;
	unsigned int fbo;
	unsigned int renderBuffer;
	unsigned int depthBuffer;
	int width;
	int height;
};

std::vector<fbData> buffers;
unsigned int draw_counter = 0;

size_t myTextureHandle; //the box's texture
sgct_utils::SGCTBox * myBox = NULL;

//variables to share across cluster
sgct::SharedDouble curr_time(0.0);

int main( int argc, char* argv[] )
{
	gEngine = new sgct::Engine( argc, argv );

	gEngine->setInitOGLFunction( myInitOGLFun );
	gEngine->setDrawFunction( myDrawFun );
	gEngine->setPreSyncFunction( myPreSyncFun );
	gEngine->setPostSyncPreDrawFunction( myPostSyncPreDrawFun );
	gEngine->setCleanUpFunction( myCleanUpFun );

	if( !gEngine->init() )
	{
		delete gEngine;
		return EXIT_FAILURE;
	}

	sgct::SharedData::Instance()->setEncodeFunction(myEncodeFun);
	sgct::SharedData::Instance()->setDecodeFunction(myDecodeFun);

	// Main loop
	gEngine->render();

	// Clean up
	delete gEngine;

	// Exit program
	exit( EXIT_SUCCESS );
}

void myDrawFun()
{
	unsigned int index = draw_counter % buffers.size();
	
	gEngine->getFBOPtr()->unBind();

	glViewport( 0, 0, buffers[index].width, buffers[index].height );

	//bind fbo
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, buffers[index].fbo );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffers[index].texture, 0);

	glMatrixMode(GL_PROJECTION);

	glLoadMatrixf( glm::value_ptr(gEngine->getActiveViewProjectionMatrix()) );

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf( glm::value_ptr( gEngine->getModelMatrix() ) );

	//clear
	glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//draw scene to texture target
	drawScene();

	//un-bind fbo
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	gEngine->getFBOPtr()->bind();

	//render a quad in ortho/2D mode with target texture
	//--------------------------------------------------
	//set viewport
	const int * curr_vp = gEngine->getActiveViewport();
	glViewport(curr_vp[0], curr_vp[1], curr_vp[2], curr_vp[3]);
	
	//enter ortho mode (2D projection)
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glPushMatrix();
	gluOrtho2D(0.0, 1.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);

	sgct::ShaderManager::Instance()->bindShader( "InvertColor" );
	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_LIGHTING_BIT );
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	glLoadIdentity();

	glBindTexture(GL_TEXTURE_2D, buffers[index].texture);

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
	glEnd();

	//restore
	glPopAttrib();
	sgct::ShaderManager::Instance()->unBindShader();

	//exit ortho mode
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	draw_counter++;
}

void drawScene()
{	
	glPushAttrib( GL_ENABLE_BIT );
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

	double speed = 25.0;
	
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -3.0f);
	glRotated(curr_time.getVal() * speed, 0.0, -1.0, 0.0);
	glRotated(curr_time.getVal() * (speed/2.0), 1.0, 0.0, 0.0);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBindTexture( GL_TEXTURE_2D, sgct::TextureManager::Instance()->getTextureByHandle(myTextureHandle) );
	//draw the box
	myBox->draw();
	glPopMatrix();

	glPopAttrib();
}

void myPreSyncFun()
{
	if( gEngine->isMaster() )
	{
		curr_time.setVal(sgct::Engine::getTime());
	}
}

void myPostSyncPreDrawFun()
{
	//Fisheye cubemaps are constant size
	if( gEngine->getWindowPtr(0)->isWindowResized() && !gEngine->isFisheye() )
		resizeFBOs();

	//reset draw counter before draw
	draw_counter = 0;
}

void myInitOGLFun()
{
	sgct::TextureManager::Instance()->setAnisotropicFilterSize(8.0f);
	sgct::TextureManager::Instance()->setCompression(sgct::TextureManager::S3TC_DXT);
	sgct::TextureManager::Instance()->loadTexure(myTextureHandle, "box", "box.png", true);

	myBox = new sgct_utils::SGCTBox(1.0f, sgct_utils::SGCTBox::Regular);
	//myBox = new sgct_utils::SGCTBox(1.0f, sgct_utils::SGCTBox::CubeMap);
	//myBox = new sgct_utils::SGCTBox(1.0f, sgct_utils::SGCTBox::SkyBox);

	//set up shader
	sgct::ShaderManager::Instance()->addShader( "InvertColor", "simple.vert", "simple.frag" );
	sgct::ShaderManager::Instance()->bindShader( "InvertColor" );
	sgct::ShaderManager::Instance()->unBindShader();
	
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_COLOR_MATERIAL );
	glDisable( GL_LIGHTING );

	//Set up backface culling
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW); //our polygon winding is counter clockwise

	if( gEngine->isFisheye() )
	{
		fbData tmpBuffer;
		buffers.push_back( tmpBuffer );
	}
	else
	{
		sgct_core::SGCTNode * thisNode = sgct_core::ClusterManager::Instance()->getThisNodePtr();
		for(unsigned int i=0; i < thisNode->getNumberOfViewports(); i++)
		{	
			if( thisNode->getViewport(i)->isEnabled() )
			{
				fbData tmpBuffer;
				buffers.push_back( tmpBuffer );
			}
		}
	}

	sgct::MessageHandler::Instance()->print("Number of targets: %d\n", buffers.size());

	createFBOs();
}

void myCleanUpFun()
{
	if(myBox != NULL) delete myBox;
	
	clearBuffers();
	buffers.clear();
}

void myEncodeFun()
{
	sgct::SharedData::Instance()->writeDouble( &curr_time );
}

void myDecodeFun()
{
	sgct::SharedData::Instance()->readDouble(  &curr_time  );
}

void createFBOs()
{
	int fb_width;
	int fb_height;
	
	if(gEngine->isFisheye())
	{
		fb_width = sgct_core::SGCTSettings::Instance()->getCubeMapResolution();
		fb_height = sgct_core::SGCTSettings::Instance()->getCubeMapResolution();
	}
	else
	{
		fb_width = gEngine->getWindowPtr(0)->getXFramebufferResolution();
		fb_height = gEngine->getWindowPtr(0)->getYFramebufferResolution();
	}

	unsigned int index = 0; //index to buffers
	sgct_core::SGCTWindow * mainWin = gEngine->getWindowPtr(0);
	for(std::size_t i=0; i < mainWin->getNumberOfViewports(); i++)
	{	
		if( mainWin->getViewport(i)->isEnabled() && index < buffers.size())
		{
			buffers[index].fbo = 0;
			buffers[index].renderBuffer = 0;
			buffers[index].depthBuffer = 0;
			buffers[index].texture = 0;
			buffers[index].width = static_cast<int>(mainWin->getViewport(i)->getXSize() * static_cast<double>(fb_width));
			buffers[index].height = static_cast<int>(mainWin->getViewport(i)->getYSize() * static_cast<double>(fb_height));
			index++;
		}
	}
	
	//create targets
	createTextures();

	for(unsigned int i=0; i < buffers.size(); i++)
	{
		//sgct::MessageHandler::Instance()->print("Creating a %dx%d fbo...\n", buffers[i].width, buffers[i].height);

		glGenFramebuffers(1, &(buffers[i].fbo));
		glGenRenderbuffers(1, &(buffers[i].renderBuffer));
		glGenRenderbuffers(1, &(buffers[i].depthBuffer));

		//setup color buffer
		glBindFramebuffer(GL_FRAMEBUFFER, buffers[i].fbo);
		glBindRenderbuffer(GL_RENDERBUFFER, buffers[i].renderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, buffers[i].width, buffers[i].height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, buffers[i].renderBuffer);	

		//setup depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, buffers[i].fbo);
		glBindRenderbuffer(GL_RENDERBUFFER, buffers[i].depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, buffers[i].width, buffers[i].height );
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffers[i].depthBuffer);

		//Does the GPU support current FBO configuration?
		if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
			sgct::MessageHandler::Instance()->print("Something went wrong creating FBO!\n");

		//unbind
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void resizeFBOs()
{
	sgct::MessageHandler::Instance()->print("Re-sizing buffers\n");
	
	clearBuffers();

	//create FBO
	createFBOs();
}

void clearBuffers()
{
	for(unsigned int i=0; i < buffers.size(); i++)
	{
		//delete buffers
		glDeleteFramebuffers(1,		&(buffers[i].fbo));
		glDeleteRenderbuffers(1,	&(buffers[i].renderBuffer));
		glGenRenderbuffers(1,		&(buffers[i].depthBuffer));

		//delete textures
		glDeleteTextures(1,	&(buffers[i].texture));
		
		buffers[i].fbo = 0;
		buffers[i].renderBuffer = 0;
		buffers[i].depthBuffer = 0;
		buffers[i].texture = 0;
		buffers[i].width = 1;
		buffers[i].height = 1;
	}
}

void createTextures()
{
	glEnable(GL_TEXTURE_2D);

	for( unsigned int i=0; i<buffers.size(); i++ )
	{
		glGenTextures(1, &(buffers[i].texture));
		glBindTexture(GL_TEXTURE_2D, buffers[i].texture);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ); //must be linear if warping, blending or fix resolution is used
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, buffers[i].width, buffers[i].height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
	}

	gEngine->checkForOGLErrors();
	//sgct::MessageHandler::Instance()->print("%d target textures created.\n", numberOfTargets);

	glDisable(GL_TEXTURE_2D);
}
