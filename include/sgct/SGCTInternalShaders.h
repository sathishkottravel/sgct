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

#ifndef _SGCT_INTERNAL_SHADERS_H_
#define _SGCT_INTERNAL_SHADERS_H_

#include <string>

namespace core_sgct
{
	/*
		All shaders are in GLSL 1.2 for compability with Mac OS X
	*/
	namespace shaders
	{
		/*

		#version 120
		uniform sampler2D LeftTex;
		uniform sampler2D RightTex;

		void main()
		{
			gl_TexCoord[0] = gl_MultiTexCoord0;
			gl_TexCoord[1] = gl_MultiTexCoord1;

			gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
		}

		*/
		const std::string Anaglyph_Vert_Shader = "\n#version 120\nuniform sampler2D LeftTex;\nuniform sampler2D RightTex;\nvoid main()\n{\ngl_TexCoord[0] = gl_MultiTexCoord0;\ngl_TexCoord[1] = gl_MultiTexCoord1;\ngl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n}\n";		
		
		/*

		#version 120

		uniform sampler2D LeftTex;
		uniform sampler2D RightTex;

		void main()
		{
			vec4 leftVals = texture2D( LeftTex, gl_TexCoord[0].st);
			float leftLum = 0.3 * leftVals.r + 0.59 * leftVals.g + 0.11 * leftVals.b;
	
			vec4 rightVals = texture2D( RightTex, gl_TexCoord[1].st);
			float rightLum = 0.3 * rightVals.r + 0.59 * rightVals.g + 0.11 * rightVals.b;
	
			gl_FragColor.r = leftLum;
			gl_FragColor.g = rightLum;
			gl_FragColor.b = rightLum;
			gl_FragColor.a = 1.0;
		}

		*/
		const std::string Anaglyph_Red_Cyan_Frag_Shader = "\n#version 120\nuniform sampler2D LeftTex;\nuniform sampler2D RightTex;\nvoid main()\n{\nvec4 leftVals = texture2D( LeftTex, gl_TexCoord[0].st);\nfloat leftLum = 0.3 * leftVals.r + 0.59 * leftVals.g + 0.11 * leftVals.b;\nvec4 rightVals = texture2D( RightTex, gl_TexCoord[1].st);\nfloat rightLum = 0.3 * rightVals.r + 0.59 * rightVals.g + 0.11 * rightVals.b;\ngl_FragColor.r = leftLum;\ngl_FragColor.g = rightLum;\ngl_FragColor.b = rightLum;\ngl_FragColor.a = 1.0;\n}\n";		
	
		/*

		#version 120

		uniform sampler2D LeftTex;
		uniform sampler2D RightTex;

		void main()
		{
			vec4 leftVals = texture2D( LeftTex, gl_TexCoord[0].st);
			vec4 rightVals = texture2D( RightTex, gl_TexCoord[1].st);

			vec3 coef = vec3(0.15, 0.15, 0.70);
	
			float rightMix = dot(rightVals.rbg, coef);
			gl_FragColor.r = leftVals.r;
			gl_FragColor.g = leftVals.g;
			gl_FragColor.b = rightMix;
			gl_FragColor.a = 1.0;
		}

		*/
		const std::string Anaglyph_Amber_Blue_Frag_Shader = "\n#version 120\nuniform sampler2D LeftTex;\nuniform sampler2D RightTex;\nvoid main()\n{\nvec4 leftVals = texture2D( LeftTex, gl_TexCoord[0].st);\nvec4 rightVals = texture2D( RightTex, gl_TexCoord[1].st);\nvec3 coef = vec3(0.15, 0.15, 0.70);\nfloat rightMix = dot(rightVals.rbg, coef);\ngl_FragColor.r = leftVals.r;\ngl_FragColor.g = leftVals.g;\ngl_FragColor.b = rightMix;\ngl_FragColor.a = 1.0;\n}\n";
	}
}
#endif