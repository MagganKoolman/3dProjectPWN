//--------------------------------------------------------------------------------------
// BTH - Stefan Petersson 2014.
//--------------------------------------------------------------------------------------
#define GLM_FORCE_RADIANS
#include "Player.h"
#include "Object.h"
#include "datatypes.h"

#include <windows.h>
#include <chrono>

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdio.h>

#include <streambuf>
#include "glm\glm.hpp" 
#include "glm\gtx\transform.hpp"
#include "glm\gtc\matrix_transform.hpp"

#include <gl/glew.h>
#include <gl/GL.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")

using namespace std;
using namespace glm;

HWND InitWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HGLRC CreateOpenGLContext(HWND wndHandle);

GLuint gVertexBuffer = 0;
GLuint gVertexAttribute = 0;
GLuint gShaderProgram = 0;
GLuint bth_tex = 0;

mat4 M;
mat4 Cam;
mat4 Persp;

uint nrOfFaces = 0;

Player player;

#define BUFFER_OFFSET(i) ((char *)nullptr + (i))

// functions implemented in Helper.cpp
// This function ALLOCATES MEMORY!
int loadshader(char* filename, char** ShaderSource, unsigned long& len);
// This function RELEASES THE MEMORY ALLOCATED
void unloadshader(GLubyte** ShaderSource);


vector<vtx> vertices;
vector<nvtx> normalVertices;
vector<tex> textureCoords;
//vector<face> faces;
vector<Object*> objects;

void loadObj() {
	string input;
	ifstream objFile("mesh.obj");
	istringstream inputString;
	string line, special;
	char* specialChar = new char[5];
	vtx v;
	nvtx n;
	tex t;
	face f;
	Object* o = new Object();

	while (getline(objFile, input)) {
		inputString.clear();
		inputString.str(input);
		line = input.substr(0, 2);
		if (line == "v ") {
			inputString >> special >> v.x >> v.y >> v.z;
			vertices.push_back(v);
		}
		/*else if (line == "vn") {
			inputString >> special >> n.x >> n.y >> n.z;
			normalVertices.push_back(n);
		}*/
		else if (line == "vt") {
			inputString >> special >> t.u >> t.v;
			textureCoords.push_back(t);
		}
		else if (line == "f ") {
			sscanf(input.c_str(), "%s %i/%i/%i %i/%i/%i %i/%i/%i", &specialChar, &f.v[0], &f.t[0], &f.n[0], &f.v[1], &f.t[1], &f.n[1], &f.v[2], &f.t[2], &f.n[2]);
			o->addFace(f);
			nrOfFaces++;
		}
		else if (line == "us") {
			if (!o->isEmpty()) {
				objects.push_back(o);
				o = new Object();
			}
			//fix texture

		}
	}
	if (!o->isEmpty()) {
		objects.push_back(o);
	}
	objFile.close();
}
void CreateShaders()
{
	//create vertex shader
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	// open glsl file and put it in a string
	ifstream shaderFile("VertexShader.glsl");
	std::string shaderText((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());
	shaderFile.close();
	// make a double pointer (only valid here)
	const char* shaderTextPtr = shaderText.c_str();
	// ask GL to load this
	glShaderSource(vs, 1, &shaderTextPtr, nullptr);
	// ask GL to compile it
	glCompileShader(vs);

	//create fragment shader | same process.
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	shaderFile.open("Fragment.glsl");
	shaderText.assign((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());
	shaderFile.close();
	shaderTextPtr = shaderText.c_str();
	glShaderSource(fs, 1, &shaderTextPtr, nullptr);
	glCompileShader(fs);

	GLint success;
	glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLint infoLength;
		glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &infoLength);
		std::string infoLog;
		infoLog.resize(infoLength);
		glGetShaderInfoLog(fs, infoLength, nullptr, &infoLog[0]);
		throw std::runtime_error(infoLog);
	}

	//link shader program (connect vs and ps)
	gShaderProgram = glCreateProgram();
	glAttachShader(gShaderProgram, fs);
	//glAttachShader(gShaderProgram, gs);
	glAttachShader(gShaderProgram, vs);
	glLinkProgram(gShaderProgram);
	
	glGetProgramiv(gShaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		GLint infoLength;
		glGetProgramiv(gShaderProgram, GL_INFO_LOG_LENGTH, &infoLength);
		std::string infoLog;
		infoLog.resize(infoLength);
		glGetProgramInfoLog(gShaderProgram, infoLength, nullptr, &infoLog[0]);
		throw std::runtime_error(infoLog);
	}
}

void CreateTriangleData()
{
	loadObj();

	struct TriangleVertex
	{
		float x, y, z, r, g, b;
	};
	
	TriangleVertex *triangleVertices = new TriangleVertex[nrOfFaces*3];

	int vIndex;
	int tIndex;
	int verticeIndex = 0;
	for (uint o = 0; o < objects.size(); o++) {
		for (uint i = 0; i < objects[o]->faces.size(); i++) {
			for (int j = 0; j < 3; j++) {
				vIndex = objects[o]->faces[i].v[j] - 1;
				tIndex = objects[o]->faces[i].t[j] - 1;
				triangleVertices[verticeIndex++] = { vertices[vIndex].x, vertices[vIndex].y, vertices[vIndex].z, /*temporärt --->>*/ textureCoords[tIndex].u, textureCoords[tIndex].v, 0.3f };
			}
		}
	}
	// Vertex Array Object (VAO)
	glGenVertexArrays(1, &gVertexAttribute);
	
	// bind == enable
	glBindVertexArray(gVertexAttribute);
	// this activates the first and second attributes of this VAO
	glEnableVertexAttribArray(0); 
	glEnableVertexAttribArray(1);

	// create a vertex buffer object (VBO) id
	glGenBuffers(1, &gVertexBuffer);
	// Bind the buffer ID as an ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
	// This "could" imply copying to the GPU, depending on what the driver wants to do...
	
	glBufferData(GL_ARRAY_BUFFER, nrOfFaces*3*sizeof(TriangleVertex), triangleVertices, GL_STATIC_DRAW);
	
	// query where which slot corresponds to the input vertex_position in the Vertex Shader 
	GLuint vertexPos = glGetAttribLocation(gShaderProgram, "vertex_position");
	// specify that: the vertex attribute "vertexPos", of 3 elements of type FLOAT, not normalized, with STRIDE != 0,
	//               starts at offset 0 of the gVertexBuffer (it is implicitly bound!)
	glVertexAttribPointer(vertexPos, 3,    GL_FLOAT, GL_FALSE,     sizeof(TriangleVertex), BUFFER_OFFSET(0));

	// query where which slot corresponds to the input vertex_color in the Vertex Shader 
	GLuint vertexColor = glGetAttribLocation(gShaderProgram, "vertex_tex");
	// specify that: the vertex attribute "vertex_color", of 3 elements of type FLOAT, not normalized, with STRIDE != 0,
	//               starts at offset (12 bytes) of the gVertexBuffer 
	glVertexAttribPointer(vertexColor, 3,    GL_FLOAT, GL_FALSE,     sizeof(TriangleVertex), BUFFER_OFFSET(sizeof(float)*3));
	delete[] triangleVertices;
}

void SetViewport()
{
	glViewport(0, 0, 1080, 720);
}

void Render()
{
	vec3 pos = player.getPosition();
	Cam = lookAt(pos, pos+player.getForward(), vec3(0,1,0));

	Persp = perspective(45.0f, 1080.f/720.0f, 0.5f, 150.0f);
	GLuint camMatrix = glGetUniformLocation(gShaderProgram, "Camera");
	GLuint perspMatrix = glGetUniformLocation(gShaderProgram, "Perspective");

	glUniformMatrix4fv(camMatrix, 1, GL_FALSE, &Cam[0][0]);
	glUniformMatrix4fv(perspMatrix, 1, GL_FALSE, &Persp[0][0]);

	glClearColor(0.1f, 0, 0.1f, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glUseProgram(gShaderProgram);
	glBindVertexArray(gVertexAttribute);
	glDrawArrays(GL_TRIANGLES, 0, nrOfFaces*3);
}

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	MSG msg = { 0 };
	HWND wndHandle = InitWindow(hInstance); //1. Skapa fönster

	if (wndHandle)
	{
		player = Player();

		HDC hDC = GetDC(wndHandle);
		HGLRC hRC = CreateOpenGLContext(wndHandle); //2. Skapa och koppla OpenGL context

		glewInit(); //3. Initiera The OpenGL Extension Wrangler Library (GLEW)
		
		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_CULL_FACE);
		
		SetViewport(); //4. Sätt viewport

		CreateShaders(); //5. Skapa vertex- och fragment-shaders

		CreateTriangleData(); //6. Definiera triangelvertiser, 7. Skapa vertex buffer object (VBO), 8.Skapa vertex array object (VAO)
		
		ShowWindow(wndHandle, nCmdShow);
		ShowCursor(false);
		bool running = true;

		chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
		long long dt = ms.count();
		while (WM_QUIT != msg.message && running)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else if(GetActiveWindow() == wndHandle)
			{
				chrono::milliseconds temp = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
				dt = temp.count() - ms.count();
				running = !GetAsyncKeyState(VK_ESCAPE);
				player.update(((float)dt)/100, wndHandle);
				Render(); //9. Rendera
				SwapBuffers(hDC); //10. Växla front- och back-buffer
				ms = temp;
			}
		}
		for (uint i = 0; i < objects.size(); i++) {
			delete objects[i];
		}
		wglMakeCurrent(NULL, NULL);
		ReleaseDC(wndHandle, hDC);
		wglDeleteContext(hRC);
		DestroyWindow(wndHandle);
	}
	return (int) msg.wParam;
}

HWND InitWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;
	wcex.hInstance      = hInstance;
	wcex.lpszClassName = L"BTH_GL_DEMO";
	if( !RegisterClassEx(&wcex) )
		return false;

	RECT rc = { 0, 0, 1080, 720 };
	AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	
	HWND handle = CreateWindow(
		L"BTH_GL_DEMO",
		L"BTH OpenGL Demo",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	return handle;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch (message) 
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;		
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

HGLRC CreateOpenGLContext(HWND wndHandle)
{
	//get handle to a device context (DC) for the client area
	//of a specified window or for the entire screen
	HDC hDC = GetDC(wndHandle);

	//details: http://msdn.microsoft.com/en-us/library/windows/desktop/dd318286(v=vs.85).aspx
	static  PIXELFORMATDESCRIPTOR pixelFormatDesc =
	{
		sizeof(PIXELFORMATDESCRIPTOR),    // size of this pfd  
		1,                                // version number  
		PFD_DRAW_TO_WINDOW |              // support window  
		PFD_SUPPORT_OPENGL |              // support OpenGL  
		PFD_DOUBLEBUFFER |                // double buffered
		0,               // disable depth buffer <-- added by Stefan		//PFD_DEPTH_DONTCARE
		PFD_TYPE_RGBA,                    // RGBA type  
		32,                               // 32-bit color depth  
		0, 0, 0, 0, 0, 0,                 // color bits ignored  
		0,                                // no alpha buffer  
		0,                                // shift bit ignored  
		0,                                // no accumulation buffer  
		0, 0, 0, 0,                       // accum bits ignored  
		0,                                // 0-bits for depth buffer <-- modified by Stefan      
		0,                                // no stencil buffer  
		0,                                // no auxiliary buffer  
		PFD_MAIN_PLANE,                   // main layer  
		0,                                // reserved  
		0, 0, 0                          // layer masks ignored  
	};

	//attempt to match an appropriate pixel format supported by a
	//device context to a given pixel format specification.
	int pixelFormat = ChoosePixelFormat(hDC, &pixelFormatDesc);

	//set the pixel format of the specified device context
	//to the format specified by the iPixelFormat index.
	SetPixelFormat(hDC, pixelFormat, &pixelFormatDesc);

	//create a new OpenGL rendering context, which is suitable for drawing
	//on the device referenced by hdc. The rendering context has the same
	//pixel format as the device context.
	HGLRC hRC = wglCreateContext(hDC);
	
	//makes a specified OpenGL rendering context the calling thread's current
	//rendering context. All subsequent OpenGL calls made by the thread are
	//drawn on the device identified by hdc. 
	wglMakeCurrent(hDC, hRC);

	return hRC;
}