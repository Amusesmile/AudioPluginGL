//
//  Shape.hpp
//  
//
//  Created by Joshua Dickinson on 8/22/16.
//
//

#ifndef Shape_hpp
#define Shape_hpp

#include "VertexBuffer.h"
#include "Attributes.h"

struct Shape
{
	Shape (OpenGLContext& openGLContext, std::vector<std::vector<double>> data, bool _dynamic = false)
	{
		vertexBuffers.add (new VertexBuffer (openGLContext, data, _dynamic));
	}
	
	void draw (OpenGLContext& openGLContext, Attributes& attributes)
	{
		for (int i = 0; i < vertexBuffers.size(); ++i)
		{
			VertexBuffer& vertexBuffer = *vertexBuffers.getUnchecked (i);
			vertexBuffer.bind();
			attributes.enable (openGLContext);
			
			glPointSize(20.0);
			
			//prevent depth sorting
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glEnable(0x8861);
			
			glDrawElements (GL_POINTS, vertexBuffer.numIndices, GL_UNSIGNED_INT, 0);
			
			attributes.disable (openGLContext);
			
			//cleanup
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);
			glDisable(0x8861);
		}
	}
	
public:
	OwnedArray<VertexBuffer> vertexBuffers;

};


#endif /* Shape_hpp */
