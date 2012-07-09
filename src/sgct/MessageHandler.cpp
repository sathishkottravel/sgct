/*************************************************************************
Copyright (c) 2012 Miroslav Andel, Link�ping University.
All rights reserved.

Original Authors:
Miroslav Andel, Alexander Fridlund

For any questions or information about the SGCT project please contact: miroslav.andel@liu.se

This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/ or send a letter to
Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.
*************************************************************************/

#include "../include/sgct/MessageHandler.h"
#include "../include/sgct/NetworkManager.h"
#include "../include/sgct/Engine.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <stdarg.h>
#include <string.h>

#define MESSAGE_HANDLER_MAX_SIZE 8192

sgct::MessageHandler * sgct::MessageHandler::mInstance = NULL;

sgct::MessageHandler::MessageHandler(void)
{
    mParseBuffer	= NULL;
	mParseBuffer	= reinterpret_cast<char*>( malloc(MESSAGE_HANDLER_MAX_SIZE) );

	headerSpace		= NULL;
	headerSpace		= reinterpret_cast<unsigned char*>( malloc(core_sgct::SGCTNetwork::mHeaderSize) );

	mRecBuffer.reserve(MESSAGE_HANDLER_MAX_SIZE);
	mBuffer.reserve(MESSAGE_HANDLER_MAX_SIZE);

	for(unsigned int i=0; i<core_sgct::SGCTNetwork::mHeaderSize; i++)
		headerSpace[i] = core_sgct::SGCTNetwork::SyncByte;
	mBuffer.insert(mBuffer.begin(), headerSpace, headerSpace+core_sgct::SGCTNetwork::mHeaderSize);

    mLocal = true;
}

sgct::MessageHandler::~MessageHandler(void)
{
    free(mParseBuffer);
    mParseBuffer = NULL;

	free(headerSpace);
    headerSpace = NULL;

	mBuffer.clear();
	mRecBuffer.clear();
}

void sgct::MessageHandler::decode(const char * receivedData, int receivedlength, int clientIndex)
{
	Engine::lockMutex(core_sgct::NetworkManager::gMutex);
		mRecBuffer.clear();
		mRecBuffer.insert(mRecBuffer.end(), receivedData, receivedData + receivedlength);
		mRecBuffer.push_back('\0');
		fprintf(stderr, "\n[client %d]: %s [end]\n", clientIndex, &mRecBuffer[0]);
    Engine::unlockMutex(core_sgct::NetworkManager::gMutex);
}


void sgct::MessageHandler::printv(const char *fmt, va_list ap)
{
#if (_MSC_VER >= 1400) //visual studio 2005 or later
    vsprintf_s(mParseBuffer, MESSAGE_HANDLER_MAX_SIZE, fmt, ap);	// And Converts Symbols To Actual Numbers
#else
    vsprintf(mParseBuffer, fmt, ap);
#endif
    va_end(ap);		// Results Are Stored In Text

    //print local
    std::cerr << mParseBuffer;

    //if client send to server
    if(!mLocal && core_sgct::NetworkManager::gMutex != NULL)
    {
        Engine::lockMutex(core_sgct::NetworkManager::gMutex);
        if(mBuffer.empty())
            mBuffer.insert(mBuffer.begin(), headerSpace, headerSpace+core_sgct::SGCTNetwork::mHeaderSize);
        mBuffer.insert(mBuffer.end(), mParseBuffer, mParseBuffer+strlen(mParseBuffer));
        Engine::unlockMutex(core_sgct::NetworkManager::gMutex);
    }
}

void sgct::MessageHandler::print(const char *fmt, ...)
{
    va_list		ap;			// Pointer To List Of Arguments

	if (fmt == NULL)		// If There's No Text
	{
		*mParseBuffer=0;	// Do Nothing
		return;
	}
    
    va_start(ap, fmt);	// Parses The String For Variables
    printv(fmt, ap);
}

void sgct::MessageHandler::clearBuffer()
{
	Engine::lockMutex(core_sgct::NetworkManager::gMutex);
	mBuffer.clear();
	Engine::unlockMutex(core_sgct::NetworkManager::gMutex);
}

char * sgct::MessageHandler::getMessage()
{
	return &mBuffer[0];
}

void sgct::MessageHandler::printDebug(const char *fmt, ...)
{
#ifdef __SGCT_DEBUG__
    va_list ap;
    if (fmt == NULL)
    {
        *mParseBuffer = 0;
        return;
    }
    va_start(ap, fmt);	// Parses The String For Variables
    printv(fmt, ap);
#endif
}

void sgct::MessageHandler::printIndent(unsigned int indentation, const char* fmt, ...)
{
    va_list ap;
    if (fmt == NULL)
    {
        *mParseBuffer = 0;
        return;
    }
    
    if (indentation > 0) {
        const std::string padding(indentation, ' ');
        const std::string fmtString = std::string(fmt);
        const std::string fmtComplete = padding + fmtString;

        const char *fmtIndented = fmtComplete.c_str();
        va_start(ap, fmt);	// Parses The String For Variables
        printv(fmtIndented, ap);
    }
    else {
        va_start(ap, fmt);	// Parses The String For Variables
        printv(fmt, ap);
    }
}
