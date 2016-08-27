//
//  VertexBuffer.hpp
//  
//
//  Created by Joshua Dickinson on 8/22/16.
//
//

#ifndef VertexBuffer_hpp
#define VertexBuffer_hpp

#include "../JuceLibraryCode/JuceHeader.h"

struct Vertex
{
	float position[3];
	float colour[4];
};

struct VertexBuffer
{
	VertexBuffer (OpenGLContext& context, std::vector<std::vector<double>> data, bool _dynamic = false)
	: openGLContext (context)
	, dynamic(_dynamic)
	, needsUpdating(false)
	{
		openGLContext.extensions.glGenBuffers (1, &vertexBuffer);
		openGLContext.extensions.glGenBuffers (1, &indexBuffer);
		
		updateBuffer(data);
	}
	
	~VertexBuffer()
	{
		openGLContext.extensions.glDeleteBuffers (1, &vertexBuffer);
		openGLContext.extensions.glDeleteBuffers (1, &indexBuffer);
	}
	
	void updateBufferOnNextDraw(std::vector<std::vector<double>> &data)
	{
		nextData = data;
		needsUpdating = true;
	}
	
	void updateBuffer()
	{
		if(needsUpdating)
		{
			updateBuffer(nextData);
			needsUpdating = false;
		}
	}
	
	void updateBuffer(std::vector<std::vector<double>> &data)
	{
		numIndices = data.size();//shape.mesh.indices.size();
		
		const float scale = 5.0f;
		vertices.clear();
		indices.clear();
		for (int i = 0; i < data.size(); ++i)
		{
			float exponent = 1.0;
			float offset = 0.5;//0.5;
			
			
			float x = powf((float)data[i][0]-offset, exponent)*scale;
			float y = powf((float)data[i][1]-offset, exponent)*scale;
			float z = powf((float)data[i][2]-offset, exponent)*scale;
			
			//http://www.nyu.edu/classes/tuckerman/adv.chem/lectures/lecture_8/lecture_8.pdf
			float distance =	y;
			float azimuth =		3.1415926*2*x;
			float elevation =	3.1415926*2*z;
			
			x = distance * sin(elevation) * cos(azimuth);
			y = distance * sin(elevation) * sin(azimuth);
			z = distance * cos(elevation);
			
			float r = data[i][0];//powf((float)data[i][0], exponent)*scale;
			float g = data[i][1];//powf((float)data[i][1], exponent)*scale;
			float b = data[i][2];//powf((float)data[i][2], exponent)*scale;
			
			Vertex vert =
			{
				{ x, y, z},
				{ r, g, b, 0.7 }
			};
			
			indices.add(i);
			vertices.add (vert);
		}
		
		int vertexSize = vertices.size();
		int numVert = (int) sizeof (Vertex);
		
		openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
		openGLContext.extensions.glBufferData (GL_ARRAY_BUFFER, vertices.size() * (int) sizeof (Vertex),
											   vertices.getRawDataPointer(), dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
		
		openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		openGLContext.extensions.glBufferData (GL_ELEMENT_ARRAY_BUFFER, numIndices * (int) sizeof (juce::uint32),
											   indices.getRawDataPointer(), dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
	}
	
	void bind()
	{
		openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
		openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	}
	
	GLuint vertexBuffer, indexBuffer;
	int numIndices;
	OpenGLContext& openGLContext;
	bool dynamic;
	bool needsUpdating;
	std::vector<std::vector<double>> nextData;
	
	juce::Array<Vertex> vertices;
	juce::Array<juce::uint32> indices;
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VertexBuffer)
};

#endif /* VertexBuffer_hpp */
