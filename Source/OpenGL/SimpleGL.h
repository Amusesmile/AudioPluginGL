//
//  SimpleGL.hpp
//  
//
//  Created by Joshua Dickinson on 8/22/16.
//
//

#ifndef SimpleGL_hpp
#define SimpleGL_hpp

#include "../JuceLibraryCode/JuceHeader.h"
#include "Shape.h"
#include <cmath>

const String vert = "attribute vec4 position;\n"
"attribute vec4 sourceColour;\n"
"\n"
"uniform mat4 projectionMatrix;\n"
"uniform mat4 viewMatrix;\n"
"uniform float grainAlpha;\n"
"\n"
"varying vec4 destinationColour;\n"
"varying vec2 textureCoordOut;\n"
"\n"
"void main()\n"
"{\n"
"    destinationColour = sourceColour;\n"
"    gl_Position = projectionMatrix * viewMatrix * position;\n"
"}\n";

const String frag =
#if JUCE_OPENGL_ES
"varying lowp vec4 destinationColour;\n"
"varying lowp vec2 textureCoordOut;\n"

#else
"#version 120\n"
"uniform float grainAlpha;\n"
"varying vec4 destinationColour;\n"
"varying vec2 textureCoordOut;\n"
#endif
"\n"
"void main()\n"
"{\n"
"    gl_FragColor = destinationColour;\n"
"	 gl_FragColor.a = grainAlpha-pow(pow(gl_PointCoord.x-0.5, 2.0) + pow(gl_PointCoord.y-0.5, 2.0), 0.1);\n"
"	 if(gl_FragColor.a < 0.0){ gl_FragColor.a = 0.0;}\n"
"}\n";


class SimpleGL
: public juce::Component
, public OpenGLRenderer
{
public:
	SimpleGL(Component* topLevelComponent, OpenGLContext* _openGLContext)
	: scale (0.5f)
	, rotationSpeed (0.01f)
	, rotation (0.0f)
	, openGLContext(_openGLContext)
	{
		setOpaque (true);
		openGLContext->setRenderer (this);
		openGLContext->attachTo (*this);
		openGLContext->setContinuousRepainting (true);
	}
	
	~SimpleGL()
	{
		openGLContext->detach();
	}
	
	void setData(std::vector<std::vector<double>> _data)
	{
		data = _data;
		updateShader(true);
	}
	
	void newOpenGLContextCreated() override
	{
		// nothing to do in this case - we'll initialise our shaders + textures
		// on demand, during the render callback.
		freeAllContextObjects();
	}
	
	void openGLContextClosing() override
	{
		// When the context is about to close, you must use this callback to delete
		// any GPU resources while the context is still current.
		freeAllContextObjects();
	}
	
	void freeAllContextObjects()
	{
		shape = nullptr;
		shader = nullptr;
		attributes = nullptr;
		uniforms = nullptr;
	}
	
	// This is a virtual method in OpenGLRenderer, and is called when it's time
	// to do your GL rendering.
	void renderOpenGL() override
	{
		jassert (OpenGLHelpers::isContextActive());
		
		
		const float desktopScale = (float) openGLContext->getRenderingScale();
		
		OpenGLHelpers::clear (Colours::black);

		updateShader();   // Check whether we need to compile a new shader
		
		if (shader == nullptr)
			return;
		
		// Having used the juce 2D renderer, it will have messed-up a whole load of GL state, so
		// we need to initialise some important settings before doing our normal GL 3D drawing..
		glEnable (GL_DEPTH_TEST);
		glDepthFunc (GL_LESS);
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		openGLContext->extensions.glActiveTexture (GL_TEXTURE0);
		glEnable (GL_TEXTURE_2D);
		
		glViewport (0, 0, roundToInt (desktopScale * getWidth()), roundToInt (desktopScale * getHeight()));
		
		shader->use();
		
		if (uniforms->projectionMatrix != nullptr)
			uniforms->projectionMatrix->setMatrix4 (getProjectionMatrix().mat, 1, false);
		
		if (uniforms->viewMatrix != nullptr)
			uniforms->viewMatrix->setMatrix4 (getViewMatrix().mat, 1, false);
		
		if (uniforms->texture != nullptr)
			uniforms->texture->set ((GLint) 0);
		
		if (uniforms->lightPosition != nullptr)
			uniforms->lightPosition->set (-15.0f, 10.0f, 15.0f, 0.0f);
		
		if (uniforms->grainAlpha != nullptr)
			uniforms->grainAlpha->set(0.9f);
		
		shape->draw (*openGLContext, *attributes);
		
		if (uniforms->grainAlpha != nullptr)
			uniforms->grainAlpha->set(0.8f);
		
		// Reset the element buffers so child Components draw correctly
		openGLContext->extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
		openGLContext->extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
		
		rotation += (float) rotationSpeed;
	}
	
	Matrix3D<float> getProjectionMatrix() const
	{
		float w = 1.0f / (scale + 0.1f);
		float h = w * getLocalBounds().toFloat().getAspectRatio (false);
		return Matrix3D<float>::fromFrustum (-w, w, -h, h, 4.0f, 30.0f);
	}
	
	Matrix3D<float> getViewMatrix() const
	{
		Matrix3D<float> viewMatrix = draggableOrientation.getRotationMatrix()
		* Vector3D<float> (0.0f, 1.0f, -10.0f);
		
		Matrix3D<float> rotationMatrix = viewMatrix.rotated (Vector3D<float> (rotation, rotation, -0.3f));
		
		return rotationMatrix * viewMatrix;
	}
	
	void setShaderProgram (const String& vertexShader, const String& fragmentShader)
	{
		newVertexShader = vertexShader;
		newFragmentShader = fragmentShader;
	}
	
	void paint (Graphics&) override {}
	
	void resized() override
	{
		draggableOrientation.setViewport (getLocalBounds());
	}
	
private:
	void updateShader(bool force = false)
	{
		if(force)
		{
			newVertexShader = vert;
			newFragmentShader = frag;
		}
		
		if (newVertexShader.isNotEmpty() || newFragmentShader.isNotEmpty())
		{
			ScopedPointer<OpenGLShaderProgram> newShader (new OpenGLShaderProgram (*openGLContext));
			String statusText;
			
			if (newShader->addVertexShader (OpenGLHelpers::translateVertexShaderToV3 (newVertexShader))
				&& newShader->addFragmentShader (OpenGLHelpers::translateFragmentShaderToV3 (newFragmentShader))
				&& newShader->link())
			{
				shape = nullptr;
				attributes = nullptr;
				uniforms = nullptr;
				
				shader = newShader;
				shader->use();
				
				shape      = new Shape (*openGLContext, data);
				attributes = new Attributes (*openGLContext, *shader);
				uniforms   = new Uniforms (*openGLContext, *shader);
				
				statusText = "GLSL: v" + String (OpenGLShaderProgram::getLanguageVersion(), 2) + " " + String(random());
			}
			else
			{
				statusText = newShader->getLastError();
			}
			
			newVertexShader = String::empty;
			newFragmentShader = String::empty;
		}
	}
	
	
	
private:
	
	OpenGLContext *openGLContext;
	float rotation;
	ScopedPointer<OpenGLShaderProgram> shader;
	ScopedPointer<Shape> shape;
	ScopedPointer<Attributes> attributes;
	ScopedPointer<Uniforms> uniforms;
	
	std::vector<std::vector<double>> data;
	std::vector<std::vector<double>> currentGrainsData;
	
	String newVertexShader, newFragmentShader;
	
	Draggable3DOrientation draggableOrientation;
	float scale;
	float rotationSpeed;
	
};


#endif /* SimpleGL_hpp */
