//
//  Attributes.hpp
//  
//
//  Created by Joshua Dickinson on 8/22/16.
//
//

#ifndef Attributes_hpp
#define Attributes_hpp

#include "../JuceLibraryCode/JuceHeader.h"
#include "VertexBuffer.h"

//==============================================================================
// This class just manages the uniform values that the demo shaders use.
struct Uniforms
{
	Uniforms (OpenGLContext& openGLContext, OpenGLShaderProgram& shader)
	{
		projectionMatrix = createUniform (openGLContext, shader, "projectionMatrix");
		viewMatrix       = createUniform (openGLContext, shader, "viewMatrix");
		texture          = createUniform (openGLContext, shader, "demoTexture");
		lightPosition    = createUniform (openGLContext, shader, "lightPosition");
		bouncingNumber   = createUniform (openGLContext, shader, "bouncingNumber");
		grainAlpha		 = createUniform (openGLContext, shader, "grainAlpha");
	}
	
	ScopedPointer<OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix, texture, lightPosition, bouncingNumber, grainAlpha;
	
private:
	static OpenGLShaderProgram::Uniform* createUniform (OpenGLContext& openGLContext,
														OpenGLShaderProgram& shader,
														const char* uniformName)
	{
		if (openGLContext.extensions.glGetUniformLocation (shader.getProgramID(), uniformName) < 0)
			return nullptr;
		
		return new OpenGLShaderProgram::Uniform (shader, uniformName);
	}
};

// This class just manages the attributes that the demo shaders use.
struct Attributes
{
	Attributes (OpenGLContext& openGLContext, OpenGLShaderProgram& shader)
	{
		position      = createAttribute (openGLContext, shader, "position");
		sourceColour  = createAttribute (openGLContext, shader, "sourceColour");
	}
	
	void enable (OpenGLContext& openGLContext)
	{
		if (position != nullptr)
		{
			openGLContext.extensions.glVertexAttribPointer (position->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), 0);
			openGLContext.extensions.glEnableVertexAttribArray (position->attributeID);
		}
		
		if (sourceColour != nullptr)
		{
			openGLContext.extensions.glVertexAttribPointer (sourceColour->attributeID, 4, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 3));
			openGLContext.extensions.glEnableVertexAttribArray (sourceColour->attributeID);
		}
	}
	
	void disable (OpenGLContext& openGLContext)
	{
		if (position != nullptr)       openGLContext.extensions.glDisableVertexAttribArray (position->attributeID);
		if (sourceColour != nullptr)   openGLContext.extensions.glDisableVertexAttribArray (sourceColour->attributeID);
	}
	
	ScopedPointer<OpenGLShaderProgram::Attribute> position, normal, sourceColour, texureCoordIn;
	
private:
	static OpenGLShaderProgram::Attribute* createAttribute (OpenGLContext& openGLContext,
															OpenGLShaderProgram& shader,
															const char* attributeName)
	{
		if (openGLContext.extensions.glGetAttribLocation (shader.getProgramID(), attributeName) < 0)
			return nullptr;
		
		return new OpenGLShaderProgram::Attribute (shader, attributeName);
	}
};

#endif /* Attributes_hpp */
