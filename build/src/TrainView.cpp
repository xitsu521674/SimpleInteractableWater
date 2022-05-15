/************************************************************************
     File:        TrainView.cpp

     Author:     
                  Michael Gleicher, gleicher@cs.wisc.edu

     Modifier
                  Yu-Chi Lai, yu-chi@cs.wisc.edu
     
     Comment:     
						The TrainView is the window that actually shows the 
						train. Its a
						GL display canvas (Fl_Gl_Window).  It is held within 
						a TrainWindow
						that is the outer window with all the widgets. 
						The TrainView needs 
						to be aware of the window - since it might need to 
						check the widgets to see how to draw

	  Note:        we need to have pointers to this, but maybe not know 
						about it (beware circular references)

     Platform:    Visio Studio.Net 2003/2005

*************************************************************************/

#include <iostream>
#include <Fl/fl.h>

// we will need OpenGL, and OpenGL needs windows.h
#include <windows.h>
//#include "GL/gl.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <GL/glu.h>

#include "TrainView.H"
#include "TrainWindow.H"
#include "Utilities/3DUtils.H"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#ifdef EXAMPLE_SOLUTION
#	include "TrainExample/TrainExample.H"
#endif


//************************************************************************
//
// * Constructor to set up the GL window
//========================================================================
TrainView::
TrainView(int x, int y, int w, int h, const char* l) 
	: Fl_Gl_Window(x,y,w,h,l)
//========================================================================
{
	mode( FL_RGB|FL_ALPHA|FL_DOUBLE | FL_STENCIL );

	resetArcball();
}

//************************************************************************
//
// * Reset the camera to look at the world
//========================================================================
void TrainView::
resetArcball()
//========================================================================
{
	// Set up the camera to look at the world
	// these parameters might seem magical, and they kindof are
	// a little trial and error goes a long way
	arcball.setup(this, 40, 250, .2f, .4f, 0);
}

//************************************************************************
//
// * FlTk Event handler for the window
//########################################################################
// TODO: 
//       if you want to make the train respond to other events 
//       (like key presses), you might want to hack this.
//########################################################################
//========================================================================
int TrainView::handle(int event)
{
	// see if the ArcBall will handle the event - if it does, 
	// then we're done
	// note: the arcball only gets the event if we're in world view
	if (tw->worldCam->value())
		if (arcball.handle(event)) 
			return 1;

	// remember what button was used
	static int last_push;

	switch(event) {
		// Mouse button being pushed event
		case FL_PUSH:
			last_push = Fl::event_button();
			// if the left button be pushed is left mouse button
			if (last_push == FL_LEFT_MOUSE  ) {
				doPick();
				damage(1);
				return 1;
			};
			break;

	   // Mouse button release event
		case FL_RELEASE: // button release
			damage(1);
			last_push = 0;
			return 1;

		// Mouse button drag event
		case FL_DRAG:

			// Compute the new control point position
			if ((last_push == FL_LEFT_MOUSE) && (selectedCube >= 0)) {
				ControlPoint* cp = &m_pTrack->points[selectedCube];

				double r1x, r1y, r1z, r2x, r2y, r2z;
				getMouseLine(r1x, r1y, r1z, r2x, r2y, r2z);

				double rx, ry, rz;
				mousePoleGo(r1x, r1y, r1z, r2x, r2y, r2z, 
								static_cast<double>(cp->pos.x), 
								static_cast<double>(cp->pos.y),
								static_cast<double>(cp->pos.z),
								rx, ry, rz,
								(Fl::event_state() & FL_CTRL) != 0);

				cp->pos.x = (float) rx;
				cp->pos.y = (float) ry;
				cp->pos.z = (float) rz;
				damage(1);
			}
			break;

		// in order to get keyboard events, we need to accept focus
		case FL_FOCUS:
			return 1;

		// every time the mouse enters this window, aggressively take focus
		case FL_ENTER:	
			focus(this);
			break;

		case FL_KEYBOARD:
		 		int k = Fl::event_key();
				int ks = Fl::event_state();
				if (k == 'p') {
					// Print out the selected control point information
					if (selectedCube >= 0) 
						printf("Selected(%d) (%g %g %g) (%g %g %g)\n",
								 selectedCube,
								 m_pTrack->points[selectedCube].pos.x,
								 m_pTrack->points[selectedCube].pos.y,
								 m_pTrack->points[selectedCube].pos.z,
								 m_pTrack->points[selectedCube].orient.x,
								 m_pTrack->points[selectedCube].orient.y,
								 m_pTrack->points[selectedCube].orient.z);
					else
						printf("Nothing Selected\n");

					return 1;
				};
				break;
	}

	return Fl_Gl_Window::handle(event);
}
unsigned int generateFloorCubemap() {
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width = 1000;
	int height = 1000;
	for (unsigned int i = 0; i < 6; i++)
	{
		float* data = new float[width * height * 3];
		//注意 unsigned char表示顏色是0~255 但是float是 [0,1]
		//unsigned char* data = new unsigned char[width * height * 3];
		if (i != 3) {
			for (int h = 0; h < height; ++h) {
				for (int w = 0; w < width; ++w) {
					data[(h * width + w) * 3] = 0;
					data[(h * width + w) * 3 + 1] = 0;
					data[(h * width + w) * 3 + 2] = 1.0f;
					//std::cout << data[(h * width + w) * 3 + 0] << " " << data[(h * width + w) * 3 + 1] << " " << data[(h * width + w) * 3 + 2] << "\n";

				}
			}
		}
		else {
			for (int h = 0; h < height; ++h) {
				for (int w = 0; w < width; ++w) {
					data[(h * width + w) * 3] = (float)w/1000;
					data[(h * width + w) * 3 + 1] = (float)h/1000;
					data[(h * width + w) * 3 + 2] = 0;
				}
			}

		}
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, data);
		delete[] data;
		
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
unsigned int loadTileCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 5);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	return textureID;
}
unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
unsigned int loadHeightmap(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
void TrainView::updateClickmap() {
	unsigned char tmp[40000];
	for (int i = 0; i < 200; ++i) {
		for (int j = 0; j < 200; ++j) {
			float sum = 0;
			sum += clickmap[(i) * 200 + j];
			if (j - 1 >= 0)
				sum += clickmap[(i) * 200 + j - 1];
			if (j + 1 < 200)
				sum += clickmap[(i) * 200 + j + 1];
			if (i - 1 >= 0) {
				sum += clickmap[(i - 1) * 200 + j];
				if (j - 1 >= 0)
					sum += clickmap[(i - 1) * 200 + j - 1];
				if (j + 1 < 200)
					sum += clickmap[(i - 1) * 200 + j + 1];
			}
			if (i + 1 < 200) {
				sum += clickmap[(i + 1) * 200 + j];
				if (j - 1 >= 0)
					sum += clickmap[(i + 1) * 200 + j - 1];
				if (j + 1 < 200)
					sum += clickmap[(i + 1) * 200 + j + 1];
			}
			tmp[(i) * 200 + j] = sum / 9;
		}
	}


	memcpy(clickmap, tmp, 40000);
	
	int width = 200, height = 200;
	GLenum format = GL_RED;
	glBindTexture(GL_TEXTURE_2D, clickTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, clickmap);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

}
void TrainView::updateSinwavemap() {
	

	memset(sinwavemap, 0, 40000);
	float radius = 10;
	float strength = 128;
	for (int c = 0; c < 220; ++c) {
		if (sinwavePos[c] == 0)
			continue;
		for (int x = 10; x < 210; ++x) {
			if ((x <= c - radius) || (x >= c + radius))
				continue;
			float deltaY = sin((radius - abs(c - x)) / radius * 1.57079633)* strength;
			for (int y = 1; y < 199; ++y) {
				sinwavemap[y * 200 + x-10] += deltaY;
			}
		}
	}

	for (int i = 219; i > 0; --i) {
		sinwavePos[i] = sinwavePos[i - 1];
	}
	sinwavePos[0] = 0;
	/*for (int i = 0; i < 200; ++i) {
		sinwavemap[i * 200] = 100;
	}*/
	
	int width = 200, height = 200;
	GLenum format = GL_RED;
	glBindTexture(GL_TEXTURE_2D, sinwaveTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, sinwavemap);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

}
//************************************************************************
//
// * this is the code that actually draws the window
//   it puts a lot of the work into other routines to simplify things
//========================================================================
void TrainView::draw()
{

	//*********************************************************************
	//
	// * Set up basic opengl informaiton
	//
	//**********************************************************************
	//initialized glad
	if (gladLoadGL())
	{
		//initiailize VAO, VBO, Shader...
		
		if (!this->shader)
			this->shader = new
			Shader(
				PROJECT_DIR "/src/shaders/simple.vert",
				nullptr, nullptr, nullptr, 
				PROJECT_DIR "/src/shaders/simple.frag");

		if (!this->commom_matrices)
			this->commom_matrices = new UBO();
			this->commom_matrices->size = 2 * sizeof(glm::mat4);
			glGenBuffers(1, &this->commom_matrices->ubo);
			glBindBuffer(GL_UNIFORM_BUFFER, this->commom_matrices->ubo);
			glBufferData(GL_UNIFORM_BUFFER, this->commom_matrices->size, NULL, GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

		if (!this->plane) {
			GLfloat  vertices[] = {
				-0.5f ,0.0f , -0.5f,
				-0.5f ,0.0f , 0.5f ,
				0.5f ,0.0f ,0.5f ,
				0.5f ,0.0f ,-0.5f };
			GLfloat  normal[] = {
				0.0f, 1.0f, 0.0f,
				0.0f, 1.0f, 0.0f,
				0.0f, 1.0f, 0.0f,
				0.0f, 1.0f, 0.0f };
			GLfloat  texture_coordinate[] = {
				0.0f, 0.0f,
				1.0f, 0.0f,
				1.0f, 1.0f,
				0.0f, 1.0f };
			GLuint element[] = {
				0, 1, 2,
				0, 2, 3, };

			this->plane = new VAO;
			this->plane->element_amount = sizeof(element) / sizeof(GLuint);
			glGenVertexArrays(1, &this->plane->vao);
			glGenBuffers(3, this->plane->vbo);
			glGenBuffers(1, &this->plane->ebo);

			glBindVertexArray(this->plane->vao);

			// Position attribute
			glBindBuffer(GL_ARRAY_BUFFER, this->plane->vbo[0]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);

			// Normal attribute
			glBindBuffer(GL_ARRAY_BUFFER, this->plane->vbo[1]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(normal), normal, GL_STATIC_DRAW);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(1);

			// Texture Coordinate attribute
			glBindBuffer(GL_ARRAY_BUFFER, this->plane->vbo[2]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(texture_coordinate), texture_coordinate, GL_STATIC_DRAW);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(2);

			//Element attribute
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->plane->ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(element), element, GL_STATIC_DRAW);

			// Unbind VAO
			glBindVertexArray(0);
		}

		if (!this->texture)
			this->texture = new Texture2D(PROJECT_DIR "/Images/church.png");

		if (!this->device){
			//Tutorial: https://ffainelli.github.io/openal-example/
			this->device = alcOpenDevice(NULL);
			if (!this->device)
				puts("ERROR::NO_AUDIO_DEVICE");

			ALboolean enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
			if (enumeration == AL_FALSE)
				puts("Enumeration not supported");
			else
				puts("Enumeration supported");

			this->context = alcCreateContext(this->device, NULL);
			if (!alcMakeContextCurrent(context))
				puts("Failed to make context current");

			this->source_pos = glm::vec3(0.0f, 5.0f, 0.0f);

			ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
			alListener3f(AL_POSITION, source_pos.x, source_pos.y, source_pos.z);
			alListener3f(AL_VELOCITY, 0, 0, 0);
			alListenerfv(AL_ORIENTATION, listenerOri);

			alGenSources((ALuint)1, &this->source);
			alSourcef(this->source, AL_PITCH, 1);
			alSourcef(this->source, AL_GAIN, 1.0f);
			alSource3f(this->source, AL_POSITION, source_pos.x, source_pos.y, source_pos.z);
			alSource3f(this->source, AL_VELOCITY, 0, 0, 0);
			alSourcei(this->source, AL_LOOPING, AL_TRUE);

			alGenBuffers((ALuint)1, &this->buffer);

			ALsizei size, freq;
			ALenum format;
			ALvoid* data;
			ALboolean loop = AL_TRUE;

			//Material from: ThinMatrix
			alutLoadWAVFile((ALbyte*)PROJECT_DIR "/Audios/bounce.wav", &format, &data, &size, &freq, &loop);
			alBufferData(this->buffer, format, data, size, freq);
			alSourcei(this->source, AL_BUFFER, this->buffer);

			if (format == AL_FORMAT_STEREO16 || format == AL_FORMAT_STEREO8)
				puts("TYPE::STEREO");
			else if (format == AL_FORMAT_MONO16 || format == AL_FORMAT_MONO8)
				puts("TYPE::MONO");

			//alSourcePlay(this->source);

			// cleanup context
			//alDeleteSources(1, &source);
			//alDeleteBuffers(1, &buffer);
			//device = alcGetContextsDevice(context);
			//alcMakeContextCurrent(NULL);
			//alcDestroyContext(context);
			//alcCloseDevice(device);
		}
		if (!this->skyboxShader) {
			float skyboxVertices[] = {
				// positions          
				 -1.0f,  1.0f, -1.0f,
				-1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,

				-1.0f, -1.0f,  1.0f,
				-1.0f, -1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f,  1.0f,
				-1.0f, -1.0f,  1.0f,

				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,

				-1.0f, -1.0f,  1.0f,
				-1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f, -1.0f,  1.0f,
				-1.0f, -1.0f,  1.0f,

				-1.0f,  1.0f, -1.0f,
				 1.0f,  1.0f, -1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				-1.0f,  1.0f,  1.0f,
				-1.0f,  1.0f, -1.0f,

				-1.0f, -1.0f, -1.0f,
				-1.0f, -1.0f,  1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				-1.0f, -1.0f,  1.0f,
				 1.0f, -1.0f,  1.0f
			};
			this->skyboxShader = new Shader(PROJECT_DIR "/src/shaders/skybox.vert",
				nullptr, nullptr, nullptr,
				PROJECT_DIR "/src/shaders/skybox.frag");
			glGenVertexArrays(1, &skyboxVAO);
			glGenBuffers(1, &skyboxVBO);
			glBindVertexArray(skyboxVAO);
			glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


			vector<std::string> faces
			{
				PROJECT_DIR "/Images/right.jpg",
				PROJECT_DIR "/Images/left.jpg",
				PROJECT_DIR "/Images/top.jpg",
				PROJECT_DIR "/Images/bottom.jpg",
				PROJECT_DIR "/Images/front.jpg",
				PROJECT_DIR "/Images/back.jpg"
			};

			cubemapTexture = loadCubemap(faces);
			

		}
		if (!this->tileShader) {
			//為了讓cull正常 這組vertices是調過的 cull不正常的原因未知
			float tileVertices[] = {
				// positions          // texture Coords
				-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
				 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
				 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,

				 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
				-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
				-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,

				-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
				 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
				 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
				 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
				-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
				-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

				-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
				-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
				-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
				-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
				-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
				-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

				 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
				 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
				 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,

				 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
				 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
				 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

				-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
				 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
				 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
				 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
				-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
				-0.5f, -0.5f, -0.5f,  0.0f, 1.0f

				/*-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
				 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
				 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
				 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
				-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
				-0.5f,  0.5f, -0.5f,  0.0f, 1.0f*/
			};
			this->tileShader = new Shader(PROJECT_DIR "/src/shaders/tiles.vert",
				nullptr, nullptr, nullptr,
				PROJECT_DIR "/src/shaders/tiles.frag");
			glGenVertexArrays(1, &tileVAO);
			glGenBuffers(1, &tileVBO);
			glBindVertexArray(tileVAO);
			glBindBuffer(GL_ARRAY_BUFFER, tileVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(tileVertices), &tileVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

			tileTexture = loadTexture(PROJECT_DIR "/Images/tiles.jpg");

			tileShader->Use();
			glUniform1i(glGetUniformLocation(tileShader->Program, "texture1"), 0);
		}
		if (!this->fillShader) {
			float fillVertices[] = {
				-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, 0.0f,    0.0f,  0.0f,
				 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, 0.0f,	0.0f,  0.0f,
				 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, 0.0f,	0.0f,  0.0f,
				 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, 0.0f,	0.0f,  0.0f,
				-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, 0.0f,	0.0f,  0.0f,
				-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, 0.0f,	0.0f,  0.0f,

				-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 0.0f,	0.0f,  0.0f,
				 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 0.0f,	0.0f,  0.0f,
				 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 0.0f,	0.0f,  0.0f,
				 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 0.0f,	0.0f,  0.0f,
				-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 0.0f,	0.0f,  0.0f,
				-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 0.0f,	0.0f,  0.0f,

				-0.5f,  0.5f,  0.5f, 0.0f,  0.0f,  0.0f,	0.0f,  0.0f,
				-0.5f,  0.5f, -0.5f, 0.0f,  0.0f,  0.0f,	0.0f,  0.0f,
				-0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  0.0f,	0.0f,  0.0f,
				-0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  0.0f,	0.0f,  0.0f,
				-0.5f, -0.5f,  0.5f, 0.0f,  0.0f,  0.0f,	0.0f,  0.0f,
				-0.5f,  0.5f,  0.5f, 0.0f,  0.0f,  0.0f,	0.0f,  0.0f,

				 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  0.0f,	0.0f,  0.0f,
				 0.5f,  0.5f, -0.5f,  0.0f,  0.0f,  0.0f,	0.0f,  0.0f,
				 0.5f, -0.5f, -0.5f,  0.0f,  0.0f,  0.0f,	0.0f,  0.0f,
				 0.5f, -0.5f, -0.5f,  0.0f,  0.0f,  0.0f,	0.0f,  0.0f,
				 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  0.0f,	0.0f,  0.0f,
				 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  0.0f,	0.0f,  0.0f,

				-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f,	0.0f,  0.0f,
				 0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f,	0.0f,  0.0f,
				 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f,	0.0f,  0.0f,
				 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f,	0.0f,  0.0f,
				-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f,	0.0f,  0.0f,
				-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f,	0.0f,  0.0f,

				-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,	0.0f,  0.0f,
				 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,	0.0f,  0.0f,
				 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,	0.0f,  0.0f,
				 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,	0.0f,  0.0f,
				-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,	0.0f,  0.0f,
				-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,	0.0f,  0.0f
			};
			fillShader = new Shader(PROJECT_DIR "/src/shaders/fill.vert", NULL, NULL, NULL, PROJECT_DIR "/src/shaders/fill.frag");

			glGenVertexArrays(1, &fillVAO);
			glGenBuffers(1, &fillVBO);
			glBindVertexArray(fillVAO);
			glBindBuffer(GL_ARRAY_BUFFER, fillVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(fillVertices), &fillVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		}
		if (!this->waterShader) {
			//float waterVertices[] = {
			//	// positions          // normals
			//	-0.5f,  0.0f, -0.5f,  0.0f, 1.0f, 0.0f,
			//	 0.5f,  0.0f, -0.5f,  0.0f, 1.0f, 0.0f,
			//	 0.5f,  0.0f,  0.5f,  0.0f, 1.0f, 0.0f,

			//	-0.5f,  0.0f, -0.5f,  0.0f, 1.0f, 0.0f,
			//	 0.5f,  0.0f,  0.5f,  0.0f, 1.0f, 0.0f,
			//	-0.5f,  0.0f,  0.5f,  0.0f, 1.0f, 0.0f,
			//};

			float waterVertices[] = {
				// positions          // normals			//texcoord

				-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,	0.0f,  0.0f,
				 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,	1.0f,  0.0f,
				 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,	1.0f,  1.0f,
				 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,	1.0f,  1.0f,
				-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,	0.0f,  1.0f,
				-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,	0.0f,  0.0f,
			};
			
			waterShader = new Shader(PROJECT_DIR "/src/shaders/water.vert", NULL, NULL, NULL, PROJECT_DIR "/src/shaders/water.frag");
			std::string path = PROJECT_DIR "/Images/waves/";
			for (int i = 0; i < 200; ++i) {
				std::string str = std::to_string(i);
				while (str.length() < 3)
					str = "0" + str;
				str += ".png";
				unsigned int val = loadHeightmap((path + str).c_str());
				heightmaps.push_back(val);
			}

			glGenVertexArrays(1, &waterVAO);
			glGenBuffers(1, &waterVBO);
			glBindVertexArray(waterVAO);
			glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(waterVertices), &waterVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));


			vector<std::string> fracts
			{
				PROJECT_DIR "/Images/tilesforRefraction.jpg",
				PROJECT_DIR "/Images/tilesforRefraction.jpg",
				PROJECT_DIR "/Images/tilesforRefraction.jpg",
				PROJECT_DIR "/Images/tilesforRefraction.jpg",
				PROJECT_DIR "/Images/tilesforRefraction.jpg",
				PROJECT_DIR "/Images/tilesforRefraction.jpg"
			};

			refractTexture = loadTileCubemap(fracts);

		}
		if (!this->watersinShader) {
			//float waterVertices[] = {
			//	// positions          // normals
			//	-0.5f,  0.0f, -0.5f,  0.0f, 1.0f, 0.0f,
			//	 0.5f,  0.0f, -0.5f,  0.0f, 1.0f, 0.0f,
			//	 0.5f,  0.0f,  0.5f,  0.0f, 1.0f, 0.0f,

			//	-0.5f,  0.0f, -0.5f,  0.0f, 1.0f, 0.0f,
			//	 0.5f,  0.0f,  0.5f,  0.0f, 1.0f, 0.0f,
			//	-0.5f,  0.0f,  0.5f,  0.0f, 1.0f, 0.0f,
			//};

			float waterVertices[] = {
				// positions          // normals			//texcoord

				-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,	0.0f,  0.0f,
				 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,	1.0f,  0.0f,
				 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,	1.0f,  1.0f,
				 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,	1.0f,  1.0f,
				-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,	0.0f,  1.0f,
				-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,	0.0f,  0.0f,
			};

			watersinShader = new Shader(PROJECT_DIR "/src/shaders/waterSin.vert", NULL, NULL, NULL, PROJECT_DIR "/src/shaders/waterSin.frag");
			

			glGenVertexArrays(1, &watersinVAO);
			glGenBuffers(1, &watersinVBO);
			glBindVertexArray(watersinVAO);
			glBindBuffer(GL_ARRAY_BUFFER, watersinVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(waterVertices), &waterVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));


		}
		if (!this->frameShader) {
			float screenVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
				// positions   // texCoords
				-1.0f,  1.0f,  0.0f, 1.0f,
				-1.0f, -1.0f,  0.0f, 0.0f,
				 1.0f, -1.0f,  1.0f, 0.0f,

				-1.0f,  1.0f,  0.0f, 1.0f,
				 1.0f, -1.0f,  1.0f, 0.0f,
				 1.0f,  1.0f,  1.0f, 1.0f
			};
			frameShader = new Shader(PROJECT_DIR "/src/shaders/frame.vert", NULL, NULL, NULL, PROJECT_DIR "/src/shaders/frame.frag");
			glGenVertexArrays(1, &screenVAO);
			glGenBuffers(1, &screenVBO);
			glBindVertexArray(screenVAO);
			glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), &screenVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));


			glGenFramebuffers(1, &fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);

			glGenTextures(1, &cbo);
			glBindTexture(GL_TEXTURE_2D, cbo);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w(), h(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cbo, 0);

			glGenRenderbuffers(1, &rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w(), h()); // use a single renderbuffer object for both a depth AND stencil buffer.
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
			
																										  // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

		}
		if (!this->pickShader) {
			float pickVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
				// positions          // color
				-0.5f,  0.5f, -0.5f,  0.0f,  0.0f,  0.0f,
				 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
				 0.5f,  0.5f,  0.5f,  1.0f,  1.0f,  0.0f,

				 0.5f,  0.5f,  0.5f,  1.0f,  1.0f,  0.0f,
				-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
				-0.5f,  0.5f, -0.5f,  0.0f,  0.0f,  0.0f,
			};
			pickShader = new Shader(PROJECT_DIR "/src/shaders/pick.vert", NULL, NULL, NULL, PROJECT_DIR "/src/shaders/pick.frag");
			glGenVertexArrays(1, &pickVAO);
			glGenBuffers(1, &pickVBO);
			glBindVertexArray(pickVAO);
			glBindBuffer(GL_ARRAY_BUFFER, pickVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(pickVertices), &pickVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

			glGenTextures(1, &clickTexture);
			//memset(clickmap, 255, 40000);

			glGenTextures(1, &sinwaveTexture);
		}
		if (!this->computeShader) {
			computeShader = new Shader(PROJECT_DIR "/src/shaders/compute.comp");
			glGenBuffers(1, &out_tex);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, out_tex);
			// ???
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 200, 200, 0, GL_RED, GL_FLOAT, NULL);
			glBindImageTexture(0, out_tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

			floorCubeTexture = generateFloorCubemap();
		}
		if (!this->pointShader) {
			float pointVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
				// positions          // color
				-0.5f,  0.5f, -0.5f,
				 0.5f,  0.5f, -0.5f,
				 0.5f,  0.5f,  0.5f,

				 0.5f,  0.5f,  0.5f,
				-0.5f,  0.5f,  0.5f,
				-0.5f,  0.5f, -0.5f,
			};
			pointShader = new Shader(PROJECT_DIR "/src/shaders/point.vert", NULL, NULL, NULL, PROJECT_DIR "/src/shaders/point.frag");
			glGenVertexArrays(1, &pointVAO);
			glGenBuffers(1, &pointVBO);
			glBindVertexArray(pointVAO);
			glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(pointVertices), &pointVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			
		}
}
	else
		throw std::runtime_error("Could not initialize GLAD!");

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClearColor(0, 0, 1.0, 0);		// background should be blue
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	pickShader->Use();
	glBindVertexArray(pickVAO);
	glm::mat4 pickmodel = glm::mat4(1.0f);
	pickmodel = glm::translate(pickmodel, glm::vec3(0, 35, 0));
	pickmodel = glm::scale(pickmodel, glm::vec3(200.0f, 1.0f, 200.0f));
	glUniformMatrix4fv(
		glGetUniformLocation(this->pickShader->Program, "model"), 1, GL_FALSE, &pickmodel[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Set up the view port
	glViewport(0,0,w(),h());

	// clear the window, be sure to clear the Z-Buffer too
	glClearColor(0,0,.3f,0);		// background should be blue

	// we need to clear out the stencil buffer since we'll use
	// it for shadows
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH);

	// Blayne prefers GL_DIFFUSE
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	// prepare for projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	setProjection();		// put the code to set up matrices here

	//######################################################################
	// TODO: 
	// you might want to set the lighting up differently. if you do, 
	// we need to set up the lights AFTER setting up the projection
	//######################################################################
	// enable the lighting
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// top view only needs one light
	if (tw->topCam->value()) {
		glDisable(GL_LIGHT1);
		glDisable(GL_LIGHT2);
	} else {
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
	}

	//*********************************************************************
	//
	// * set the light parameters
	//
	//**********************************************************************
	GLfloat lightPosition1[]	= {0,1,1,0}; // {50, 200.0, 50, 1.0};
	GLfloat lightPosition2[]	= {1, 0, 0, 0};
	GLfloat lightPosition3[]	= {0, -1, 0, 0};
	GLfloat yellowLight[]		= {0.5f, 0.5f, .1f, 1.0};
	GLfloat whiteLight[]			= {1.0f, 1.0f, 1.0f, 1.0};
	GLfloat blueLight[]			= {.1f,.1f,.3f,1.0};
	GLfloat grayLight[]			= {.3f, .3f, .3f, 1.0};

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteLight);
	glLightfv(GL_LIGHT0, GL_AMBIENT, grayLight);

	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition2);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, yellowLight);

	glLightfv(GL_LIGHT2, GL_POSITION, lightPosition3);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, blueLight);

	// set linstener position 
	if(selectedCube >= 0)
		alListener3f(AL_POSITION, 
			m_pTrack->points[selectedCube].pos.x,
			m_pTrack->points[selectedCube].pos.y,
			m_pTrack->points[selectedCube].pos.z);
	else
		alListener3f(AL_POSITION, 
			this->source_pos.x, 
			this->source_pos.y,
			this->source_pos.z);







	//*********************************************************************
	// now draw the ground plane
	//*********************************************************************
	// set to opengl fixed pipeline(use opengl 1.x draw function)
	glUseProgram(0);

	setupFloor();
	glDisable(GL_LIGHTING);
	//drawFloor(200,10);


	//*********************************************************************
	// now draw the object and we need to do it twice
	// once for real, and then once for shadows
	//*********************************************************************
	glEnable(GL_LIGHTING);
	setupObjects();

	drawStuff();

	// this time drawing is for shadows (except for top view)
	if (!tw->topCam->value()) {
		setupShadows();
		//drawStuff(true);
		unsetupShadows();
	}

	setUBO();
	glBindBufferRange(
		GL_UNIFORM_BUFFER, /*binding point*/0, this->commom_matrices->ubo, 0, this->commom_matrices->size);

	//bind shader
	this->shader->Use();

	glm::mat4 model_matrix = glm::mat4();
	model_matrix = glm::translate(model_matrix, this->source_pos);
	model_matrix = glm::scale(model_matrix, glm::vec3(10.0f, 10.0f, 10.0f));
	glUniformMatrix4fv(
		glGetUniformLocation(this->shader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);
	glUniform3fv(
		glGetUniformLocation(this->shader->Program, "u_color"), 
		1, 
		&glm::vec3(0.0f, 1.0f, 0.0f)[0]);
	this->texture->bind(0);
	glUniform1i(glGetUniformLocation(this->shader->Program, "u_texture"), 0);
	
	//bind VAO
	glBindVertexArray(this->plane->vao);

	//glDrawElements(GL_TRIANGLES, this->plane->element_amount, GL_UNSIGNED_INT, 0);



	//畫磁磚cull不畫前面
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);


	tileShader->Use();
	glm::mat4 tilemodel = glm::mat4(1.0f);
	tilemodel = glm::scale(tilemodel, glm::vec3(200.0f, 100.0f, 200.0f));
	glUniformMatrix4fv(
		glGetUniformLocation(this->tileShader->Program, "model"), 1, GL_FALSE, &tilemodel[0][0]);
	
	// tiles


	glBindVertexArray(tileVAO);
	glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, clickTexture);
	glBindTexture(GL_TEXTURE_2D, tileTexture);
	glDrawArrays(GL_TRIANGLES, 0, 30);
	glBindVertexArray(0);



	glDisable(GL_CULL_FACE);


	

	

	GLfloat view_matrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, view_matrix);
	glm::mat4 a_modelView(1.0f);
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			a_modelView[i][j] = view_matrix[i * 4 + j];
		}
	}

	glm::mat3 rotMat(a_modelView);
	glm::vec3 d(a_modelView[3]);

	glm::vec3 camPos = -d * rotMat;

	//std::cout << camPos.x << " " << camPos.y << " " << camPos.z << "\n";

	if (tw->surfaceBrowser->value() == 1) {
		waterShader->Use();
		
		glUniform1f(glGetUniformLocation(this->waterShader->Program, "lightmode"), 0);
		if (tw->lightButton->value() == 1) {
			glUniform1f(glGetUniformLocation(this->waterShader->Program, "lightmode"), 1);
			glUniform3f(glGetUniformLocation(this->waterShader->Program, "light.ambient"), 0.2, 0.2, 0.2);
			glUniform3f(glGetUniformLocation(this->waterShader->Program, "light.diffuse"), 0.5, 0.5, 0.5);
			glUniform3f(glGetUniformLocation(this->waterShader->Program, "light.specular"), 1.0, 1.0, 1.0);
			glUniform3f(glGetUniformLocation(this->waterShader->Program, "light.direction"), 0.75, 1.0, 0.0);
		}
		else if(tw->lightButton->value() == 0){
			glUniform1f(glGetUniformLocation(this->waterShader->Program, "lightmode"), 0);
		}
		
		glUniform3fv(glGetUniformLocation(this->waterShader->Program, "cameraPos"), 1, &camPos[0]);

		
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glUniform1i(glGetUniformLocation(this->waterShader->Program, "skybox"), 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, refractTexture);
		glUniform1i(glGetUniformLocation(this->waterShader->Program, "tile"), 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, heightmaps[(int)time % 200]);
		glUniform1i(glGetUniformLocation(this->waterShader->Program, "heighmap"), 2);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, clickTexture);
		glUniform1i(glGetUniformLocation(this->waterShader->Program, "clickmap"), 3);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, sinwaveTexture);
		glUniform1i(glGetUniformLocation(this->waterShader->Program, "sinwavemap"), 4);




		// tiles
		glBindVertexArray(waterVAO);

		/*glm::mat4 watermodel = glm::mat4(1.0f);
		watermodel = glm::translate(watermodel, glm::vec3(0.0f, -10.0f, 0.0f));
		watermodel = glm::scale(watermodel, glm::vec3(200.0f, 75.0f, 200.0f));
		glUniformMatrix4fv(
			glGetUniformLocation(this->waterShader->Program, "model"), 1, GL_FALSE, &watermodel[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, 6);*/
		
		
		for (int i = 0; i < 100; ++i) {
			for (int j = 0; j < 100; ++j) {
				glm::mat4 watermodel = glm::mat4(1.0f);
				watermodel = glm::translate(watermodel, glm::vec3(-100.0f, 15.0f, -100.0f));
				watermodel = glm::translate(watermodel, glm::vec3(i * 2.0f, 0.0f, j * 2.0f));
				watermodel = glm::scale(watermodel, glm::vec3(2.0f, 1.0f, 2.0f));
				glUniformMatrix4fv(
					glGetUniformLocation(this->waterShader->Program, "model"), 1, GL_FALSE, &watermodel[0][0]);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
		}
		glBindVertexArray(0);


		computeShader->Use();
		
		glUniform1f(glGetUniformLocation(this->computeShader->Program, "sinMode"), 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, floorCubeTexture);
		glUniform1i(glGetUniformLocation(this->computeShader->Program, "floorCube"), 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, heightmaps[(int)time % 200]);
		glUniform1i(glGetUniformLocation(this->computeShader->Program, "heighmap"), 2);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, clickTexture);
		glUniform1i(glGetUniformLocation(this->computeShader->Program, "clickmap"), 3);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, sinwaveTexture);
		glUniform1i(glGetUniformLocation(this->computeShader->Program, "sinwavemap"), 4);
		
		
		
		glUniform3fv(glGetUniformLocation(this->computeShader->Program, "cameraPos"), 1, &camPos[0]);
		glUniform3f(glGetUniformLocation(this->computeShader->Program, "lightPos"), 50,100,0);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, out_tex);

		float* data = new float[40000];
		//float data[40000];
		memset(data, 0, 40000 * 4);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 200, 200, 0, GL_RED, GL_FLOAT, data);


		glDispatchCompute(800, 800, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		
		unsigned int collection_size = 40000;
		std::vector<float> compute_data(collection_size);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, compute_data.data());

		/*for (float ff : compute_data) {
				std::cout << ff << " ";
		}*/
		pointShader->Use();
		glBindVertexArray(pointVAO);

		for (int i = 0; i < 200; ++i) {
			for (int j = 0; j < 200; ++j) {
				if (i == 0 && j == 0)
					continue;
				if (compute_data[i * 200 + j] > 10) {
					float strength = compute_data[i * 200 + j]*1.5;
					glm::mat4 pointmodel = glm::mat4(1.0f);
					pointmodel = glm::translate(pointmodel, glm::vec3(-100.0f, -45.0f, -100.0f));
					pointmodel = glm::translate(pointmodel, glm::vec3(j * 1.0f, 0.0f, i * 1.0f));
					pointmodel = glm::scale(pointmodel, glm::vec3(0.05f* strength, 0.1f, 0.05f* strength));
					glUniformMatrix4fv(
						glGetUniformLocation(this->pointShader->Program, "model"), 1, GL_FALSE, &pointmodel[0][0]);
					glDrawArrays(GL_TRIANGLES, 0, 6);
				}
			}
		}
		glBindVertexArray(0);
		delete[] data;
	}
	else if (tw->surfaceBrowser->value() == 2) {
		watersinShader->Use();

		glUniform1f(glGetUniformLocation(this->watersinShader->Program, "lightmode"), 0);
		if (tw->lightButton->value() == 1) {
			glUniform1f(glGetUniformLocation(this->watersinShader->Program, "lightmode"), 1);
			glUniform3f(glGetUniformLocation(this->watersinShader->Program, "light.ambient"), 0.2, 0.2, 0.2);
			glUniform3f(glGetUniformLocation(this->watersinShader->Program, "light.diffuse"), 0.5, 0.5, 0.5);
			glUniform3f(glGetUniformLocation(this->watersinShader->Program, "light.specular"), 1.0, 1.0, 1.0);
			glUniform3f(glGetUniformLocation(this->watersinShader->Program, "light.direction"), 0.75, 1.0, 0.0);
		}
		else if (tw->lightButton->value() == 0) {
			glUniform1f(glGetUniformLocation(this->watersinShader->Program, "lightmode"), 0);
		}



		glUniform3fv(glGetUniformLocation(this->watersinShader->Program, "cameraPos"), 1, &camPos[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glUniform1i(glGetUniformLocation(this->watersinShader->Program, "skybox"), 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, refractTexture);
		glUniform1i(glGetUniformLocation(this->watersinShader->Program, "tile"), 1);

		
		glUniform1f(glGetUniformLocation(this->watersinShader->Program, "time"), time*0.002);

		glBindVertexArray(waterVAO);
		for (int i = 0; i < 200; ++i) {
			for (int j = 0; j < 200; ++j) {
				glm::mat4 watermodel = glm::mat4(1.0f);
				watermodel = glm::translate(watermodel, glm::vec3(-100.0f, 40.0f, -100.0f));
				watermodel = glm::translate(watermodel, glm::vec3(i * 1.0f, 0.0f, j * 1.0f));
				watermodel = glm::scale(watermodel, glm::vec3(1.0f, 1.0f, 1.0f));
				glUniformMatrix4fv(
					glGetUniformLocation(this->watersinShader->Program, "model"), 1, GL_FALSE, &watermodel[0][0]);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
		}
		glBindVertexArray(0);





		//computeShader->Use();

		//glUniform1f(glGetUniformLocation(this->computeShader->Program, "sinMode"), 1);

		//glUniform1f(glGetUniformLocation(this->computeShader->Program, "amplitude"), tw->sinwaveAmp->value() * 2);
		//glUniform1f(glGetUniformLocation(this->computeShader->Program, "wavelength"), tw->sinwaveLength->value() * 10);
		//glUniform1f(glGetUniformLocation(this->computeShader->Program, "speed"), 1.0f);
		//glUniform1f(glGetUniformLocation(this->computeShader->Program, "time"), time * 0.05);

		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, floorCubeTexture);
		//glUniform1i(glGetUniformLocation(this->computeShader->Program, "floorCube"), 1);
		//



		//glUniform3fv(glGetUniformLocation(this->computeShader->Program, "cameraPos"), 1, &camPos[0]);
		//glUniform3f(glGetUniformLocation(this->computeShader->Program, "lightPos"), 50, 100, 0);


		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, out_tex);

		//float* data = new float[40000];
		////float data[40000];
		//memset(data, 0, 40000 * 4);

		//glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 200, 200, 0, GL_RED, GL_FLOAT, data);


		//glDispatchCompute(800, 800, 1);
		//glMemoryBarrier(GL_ALL_BARRIER_BITS);

		//unsigned int collection_size = 40000;
		//std::vector<float> compute_data(collection_size);
		//glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, compute_data.data());

		///*for (float ff : compute_data) {
		//		std::cout << ff << " ";
		//}*/
		//pointShader->Use();
		//glBindVertexArray(pointVAO);

		//for (int i = 0; i < 200; ++i) {
		//	for (int j = 0; j < 200; ++j) {
		//		if (i == 0 && j == 0)
		//			continue;
		//		if (compute_data[i * 200 + j] > 30) {
		//			float strength = compute_data[i * 200 + j] * 1.5;
		//			glm::mat4 pointmodel = glm::mat4(1.0f);
		//			pointmodel = glm::translate(pointmodel, glm::vec3(-100.0f, -45.0f, -100.0f));
		//			pointmodel = glm::translate(pointmodel, glm::vec3(j * 1.0f, 0.0f, i * 1.0f));
		//			pointmodel = glm::scale(pointmodel, glm::vec3(0.05f * strength, 0.1f, 0.05f * strength));
		//			glUniformMatrix4fv(
		//				glGetUniformLocation(this->pointShader->Program, "model"), 1, GL_FALSE, &pointmodel[0][0]);
		//			glDrawArrays(GL_TRIANGLES, 0, 6);
		//		}
		//	}
		//}
		//glBindVertexArray(0);
		//delete[] data;
	}
	
	if (tw->fillButton->value() == 1) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		fillShader->Use();

		if (tw->surfaceBrowser->value() == 2) {
			glUniform1i(glGetUniformLocation(this->fillShader->Program, "sinwavemode"), 1);

			glUniform1f(glGetUniformLocation(this->fillShader->Program, "time"), time * 0.05);
		}
		else {
			glUniform1i(glGetUniformLocation(this->fillShader->Program, "sinwavemode"), 0);
		}
		glUniform3fv(glGetUniformLocation(this->fillShader->Program, "cameraPos"), 1, &camPos[0]);

		glUniform1f(glGetUniformLocation(this->watersinShader->Program, "lightmode"), 0);
		if (tw->lightButton->value() == 1) {
			glUniform1f(glGetUniformLocation(this->watersinShader->Program, "lightmode"), 1);
			glUniform3f(glGetUniformLocation(this->watersinShader->Program, "light.ambient"), 0.2, 0.2, 0.2);
			glUniform3f(glGetUniformLocation(this->watersinShader->Program, "light.diffuse"), 0.5, 0.5, 0.5);
			glUniform3f(glGetUniformLocation(this->watersinShader->Program, "light.specular"), 1.0, 1.0, 1.0);
			glUniform3f(glGetUniformLocation(this->watersinShader->Program, "light.direction"), 0.75, 1.0, 0.0);
		}
		else if (tw->lightButton->value() == 0) {
			glUniform1f(glGetUniformLocation(this->watersinShader->Program, "lightmode"), 0);
		}




		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glUniform1i(glGetUniformLocation(this->fillShader->Program, "skybox"), 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, refractTexture);
		glUniform1i(glGetUniformLocation(this->fillShader->Program, "tile"), 1);


		glBindVertexArray(fillVAO);

		glm::mat4 fillmodel = glm::mat4(1.0f);
		fillmodel = glm::translate(fillmodel, glm::vec3(0.0f, -5.0f, 0.0f));
		fillmodel = glm::scale(fillmodel, glm::vec3(200.0f, 80.0f, 200.0f));
		glUniformMatrix4fv(
			glGetUniformLocation(this->fillShader->Program, "model"), 1, GL_FALSE, &fillmodel[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		glDisable(GL_BLEND);
	}

	





	glm::mat4 model(1.0f);
	model = glm::scale(model, glm::vec3(100000.0f, 100000.0f, 100000.0f));



	glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
	skyboxShader->Use();
	
	glUniformMatrix4fv(
		glGetUniformLocation(this->skyboxShader->Program, "model"), 1, GL_FALSE, &model[0][0]);
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, floorCubeTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS); // set depth function back to default




	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
	//// clear all relevant buffers
	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
	//glClear(GL_COLOR_BUFFER_BIT);

	//frameShader->Use();
	//glActiveTexture(GL_TEXTURE0);
	//glBindVertexArray(screenVAO);
	//glUniform1i(glGetUniformLocation(frameShader->Program, "screenTexture"), 0);
	//glBindTexture(GL_TEXTURE_2D, cbo);	// use the color attachment texture as the texture of the quad plane
	//glDrawArrays(GL_TRIANGLES, 0, 6);











	//unbind VAO
	glBindVertexArray(0);

	//unbind shader(switch to fixed pipeline)
	glUseProgram(0);
}

//************************************************************************
//
// * This sets up both the Projection and the ModelView matrices
//   HOWEVER: it doesn't clear the projection first (the caller handles
//   that) - its important for picking
//========================================================================
void TrainView::
setProjection()
//========================================================================
{
	// Compute the aspect ratio (we'll need it)
	float aspect = static_cast<float>(w()) / static_cast<float>(h());

	// Check whether we use the world camp
	if (tw->worldCam->value())
		arcball.setProjection(false);
	// Or we use the top cam
	else if (tw->topCam->value()) {
		float wi, he;
		if (aspect >= 1) {
			wi = 110;
			he = wi / aspect;
		} 
		else {
			he = 110;
			wi = he * aspect;
		}

		// Set up the top camera drop mode to be orthogonal and set
		// up proper projection matrix
		glMatrixMode(GL_PROJECTION);
		glOrtho(-wi, wi, -he, he, 200, -200);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(-90,1,0,0);
	} 
	// Or do the train view or other view here
	//####################################################################
	// TODO: 
	// put code for train view projection here!	
	//####################################################################
	else {
#ifdef EXAMPLE_SOLUTION
		trainCamView(this,aspect);
#endif
	}
}

//************************************************************************
//
// * this draws all of the stuff in the world
//
//	NOTE: if you're drawing shadows, DO NOT set colors (otherwise, you get 
//       colored shadows). this gets called twice per draw 
//       -- once for the objects, once for the shadows
//########################################################################
// TODO: 
// if you have other objects in the world, make sure to draw them
//########################################################################
//========================================================================
void TrainView::drawStuff(bool doingShadows)
{
	// Draw the control points
	// don't draw the control points if you're driving 
	// (otherwise you get sea-sick as you drive through them)
	if (!tw->trainCam->value()) {
		for(size_t i=0; i<m_pTrack->points.size(); ++i) {
			if (!doingShadows) {
				if ( ((int) i) != selectedCube)
					glColor3ub(240, 60, 60);
				else
					glColor3ub(240, 240, 30);
			}
			m_pTrack->points[i].draw();
		}
	}
	// draw the track
	//####################################################################
	// TODO: 
	// call your own track drawing code
	//####################################################################

#ifdef EXAMPLE_SOLUTION
	drawTrack(this, doingShadows);
#endif

	// draw the train
	//####################################################################
	// TODO: 
	//	call your own train drawing code
	//####################################################################
#ifdef EXAMPLE_SOLUTION
	// don't draw the train if you're looking out the front window
	if (!tw->trainCam->value())
		drawTrain(this, doingShadows);
#endif
}

// 
//************************************************************************
//
// * this tries to see which control point is under the mouse
//	  (for when the mouse is clicked)
//		it uses OpenGL picking - which is always a trick
//########################################################################
// TODO: 
//		if you want to pick things other than control points, or you
//		changed how control points are drawn, you might need to change this
//########################################################################
//========================================================================
void TrainView::
doPick()
//========================================================================
{
	// since we'll need to do some GL stuff so we make this window as 
	// active window
	make_current();		

	// where is the mouse?
	int mx = Fl::event_x(); 
	int my = Fl::event_y();
	//std::cout << mx << " " << my << "\n";
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glm::vec3 uv;
	glReadPixels(mx, h() - my - 1, 1, 1, GL_RGB, GL_FLOAT, &uv[0]);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	//std::cout << uv.x*200 << " " << uv.y * 200 << " " << uv.z * 200 << "\n";
	if (uv.z == 0) {
		float centerX = uv.x * 200;
		float centerY = uv.y * 200;
		float radius = 10;
		float strength = 50;
		for (int i = 0; i < 2*radius; ++i) {
			for (int j = 0; j < 2*radius; ++j) {
				int curY = centerY - radius + i;
				int curX = centerX - radius + j;
				if (curX >= 0 && curX < 200 && curY >= 0 && curY < 200) {
					float distance = sqrt((curX - centerX) * (curX - centerX) + (curY - centerY) * (curY - centerY));
					float tmpHeight = clickmap[curY * 200 + curX];
					tmpHeight += strength * (radius*2 - distance) / radius*2;
					if (tmpHeight > 255)
						tmpHeight = 255;
					clickmap[curY * 200 + curX] = tmpHeight;
				}
			}
		}
	}
	

	// get the viewport - most reliable way to turn mouse coords into GL coords
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Set up the pick matrix on the stack - remember, FlTk is
	// upside down!
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity ();
	gluPickMatrix((double)mx, (double)(viewport[3]-my), 
						5, 5, viewport);

	// now set up the projection
	setProjection();

	// now draw the objects - but really only see what we hit
	GLuint buf[100];
	glSelectBuffer(100,buf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);

	// draw the cubes, loading the names as we go
	for(size_t i=0; i<m_pTrack->points.size(); ++i) {
		glLoadName((GLuint) (i+1));
		m_pTrack->points[i].draw();
	}

	// go back to drawing mode, and see how picking did
	int hits = glRenderMode(GL_RENDER);
	if (hits) {
		// warning; this just grabs the first object hit - if there
		// are multiple objects, you really want to pick the closest
		// one - see the OpenGL manual 
		// remember: we load names that are one more than the index
		selectedCube = buf[3]-1;
	} else // nothing hit, nothing selected
		selectedCube = -1;

	printf("Selected Cube %d\n",selectedCube);
}

void TrainView::setUBO()
{
	float wdt = this->pixel_w();
	float hgt = this->pixel_h();

	glm::mat4 view_matrix;
	glGetFloatv(GL_MODELVIEW_MATRIX, &view_matrix[0][0]);
	//HMatrix view_matrix; 
	//this->arcball.getMatrix(view_matrix);

	glm::mat4 projection_matrix;
	glGetFloatv(GL_PROJECTION_MATRIX, &projection_matrix[0][0]);
	//projection_matrix = glm::perspective(glm::radians(this->arcball.getFoV()), (GLfloat)wdt / (GLfloat)hgt, 0.01f, 1000.0f);

	glBindBuffer(GL_UNIFORM_BUFFER, this->commom_matrices->ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &projection_matrix[0][0]);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &view_matrix[0][0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}