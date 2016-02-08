//--------------------------------------------------------------------------------------
// BTH - Stefan Petersson 2014.
//--------------------------------------------------------------------------------------
#define GLM_FORCE_RADIANS
#include "Player.h"
#include "Object.h"
#include "datatypes.h"
#include "Simple OpenGL Image Library\src\SOIL.h"


#include <windows.h>
#include <chrono>

#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdio.h>

#include <streambuf>
#include "glm\glm.hpp" 
#include "glm\gtx\transform.hpp"
#include "glm\gtc\matrix_transform.hpp"

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
GLuint texture = 0;


mat4 M;
mat4 Cam;
mat4 Persp;

material standardMat = {vec3(1,0,0),vec3(0,1,0),vec3(0,0,1), "", 0};

uint nrOfFaces = 0;

Player player;

#define BUFFER_OFFSET(i) ((char *)nullptr + (i))

// functions implemented in Helper.cpp
// This function ALLOCATES MEMORY!
int loadshader(char* filename, char** ShaderSource, unsigned long& len);
// This function RELEASES THE MEMORY ALLOCATED
void unloadshader(GLubyte** ShaderSource);

int objectOffset = 0;

vector<vtx> vertices;
vector<nvtx> normalVertices;
vector<tex> textureCoords;
//vector<face> faces;
vector<material> allMaterials;
vector<Object*> objects;

map<string, GLuint> texmapid;

void CreateTriangleData();

GLuint CreateTexture(string fileName)
{
	// Load textures
	GLuint texIndex;
	glGenTextures(1, &texIndex);

	int width, height;
	unsigned char* image;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texIndex);
	image = SOIL_load_image(fileName.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glUniform1i(glGetUniformLocation(gShaderProgram, fileName.c_str()), texIndex);
	return texIndex;	
}

void loadMaterials(string materials)
{
	ifstream matFile(materials);
	istringstream inputString;
	string line, special, input, name;
	material mat;
	bool first = true;
	bool newMaterial = true;
	while (getline(matFile, input))
	{
		inputString.clear();
		inputString.str(input);
		line = input.substr(0, 2);

		if (line == "ne")
		{
			inputString >> special >> name;
			newMaterial = true;
			for (int i = 0; i < allMaterials.size(); i++) {
				if (name == allMaterials[i].materialName) {
					newMaterial = false;
				}
			}
			if (newMaterial) {
				if (!first) {
					allMaterials.push_back(mat);
				}
				first = false;

				mat.materialName = "";

				mat.materialName = name;
				mat.texid = 0;
			}
		}
		if (newMaterial) {
			if (line == "Ka")
			{
				inputString >> special >> mat.Ka.x >> mat.Ka.y >> mat.Ka.z;
			}
			else if (line == "Kd")
			{
				inputString >> special >> mat.Kd.x >> mat.Kd.y >> mat.Kd.z;
			}
			else if (line == "Ks")
			{
				inputString >> special >> mat.Ks.x >> mat.Ks.y >> mat.Ks.z;
			}
			else if (line == "ma")
			{
				inputString >> special >> name;
				if (!texmapid[name]) {
					texmapid[name] = CreateTexture(name);
				}
				mat.texid = texmapid[name];
			}
		}
	}
	
	if (newMaterial) {
		allMaterials.push_back(mat);
	}
}

void loadObj(string fileName) {
	string input;
	ifstream objFile(fileName);
	istringstream inputString;
	string line, special, mat;
	char* specialChar = (char*)malloc(5);
	vtx v;
	nvtx n;
	tex t;
	face f;
	Object* o = new Object();
	o->mat = &standardMat;

	while (getline(objFile, input))
	{
		inputString.clear();
		inputString.str(input);
		line = input.substr(0, 2);
		
		if (line == "v ") {
			inputString >> special >> v.x >> v.y >> v.z;
			vertices.push_back(v);
		}
		else if (line == "vn") {
			inputString >> special >> n.x >> n.y >> n.z;
			normalVertices.push_back(n);
		}
		else if (line == "vt") {
			inputString >> special >> t.u >> t.v;
			textureCoords.push_back(t);
		}
		else if (line == "f ") {
			sscanf(input.c_str(), "%s %i/%i/%i %i/%i/%i %i/%i/%i", specialChar, &f.v[0], &f.t[0], &f.n[0], &f.v[1], &f.t[1], &f.n[1], &f.v[2], &f.t[2], &f.n[2]);
			o->addFace(f);
			nrOfFaces++;
		}
		else if (line == "us"){
			inputString >> special >> mat;
			if (!o->isEmpty()) {
				objects.push_back(o);
				o = new Object();
				o->mat = &standardMat;
			}
			for (int i = 0; i < allMaterials.size(); i++) {
				if (mat == allMaterials[i].materialName) {
					o->mat = &allMaterials[i];
					break;
				}
			}

		}
		else if (line == "mt") {
			inputString >> special >> mat;
			loadMaterials(mat);
		}	
	}
	if (!o->isEmpty()) {
		objects.push_back(o);
	}
	free(specialChar);
	objFile.close();
	CreateTriangleData();
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
	TriangleVertex *triangleVertices;

	int vIndex;
	int tIndex;
	int nIndex;
	for (uint o = objectOffset; o < objects.size(); o++) {
		triangleVertices = new TriangleVertex[objects[o]->faces.size()*3];
		for (uint i = 0; i < objects[o]->faces.size(); i++) {
			for (int j = 0; j < 3; j++) {
				vIndex = objects[o]->faces[i].v[j] - 1;
				tIndex = objects[o]->faces[i].t[j] - 1;
				nIndex = objects[o]->faces[i].n[j] - 1;
				triangleVertices[(3 * i) + j] = { vertices[vIndex].x, vertices[vIndex].y, vertices[vIndex].z, textureCoords[tIndex].u, textureCoords[tIndex].v, 
					normalVertices[nIndex].x, normalVertices[nIndex].y ,normalVertices[nIndex].z };
			}
		}
		glGenVertexArrays(1, &gVertexAttribute);
		objects[o]->VAOid = gVertexAttribute;
		glBindVertexArray(objects[o]->VAOid);
		glGenBuffers(1, &gVertexBuffer);
		objects[o]->BUFFid = gVertexBuffer;
		glBindBuffer(GL_ARRAY_BUFFER, objects[o]->BUFFid);
		glBufferData(GL_ARRAY_BUFFER, objects[o]->faces.size() * 3 * sizeof(TriangleVertex), triangleVertices, GL_STATIC_DRAW);
		
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		// query where which slot corresponds to the input vertex_position in the Vertex Shader 
		GLuint vertexPos = glGetAttribLocation(gShaderProgram, "vertex_position");
		// specify that: the vertex attribute "vertexPos", of 3 elements of type FLOAT, not normalized, with STRIDE != 0,
		//               starts at offset 0 of the gVertexBuffer (it is implicitly bound!)
		glVertexAttribPointer(vertexPos, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleVertex), BUFFER_OFFSET(0));

		// query where which slot corresponds to the input vertex_color in the Vertex Shader 
		GLuint vertexColor = glGetAttribLocation(gShaderProgram, "vertex_tex");
		// specify that: the vertex attribute "vertex_color", of 3 elements of type FLOAT, not normalized, with STRIDE != 0,
		//               starts at offset (12 bytes) of the gVertexBuffer 
		glVertexAttribPointer(vertexColor, 2, GL_FLOAT, GL_FALSE, sizeof(TriangleVertex), BUFFER_OFFSET(sizeof(float) * 3));

		GLuint vertexNormal = glGetAttribLocation(gShaderProgram, "vertex_normal");
		glVertexAttribPointer(vertexNormal, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleVertex), BUFFER_OFFSET(sizeof(float) * 5));


		delete[] triangleVertices;
	}
	objectOffset = objects.size();
	vertices.clear();
	normalVertices.clear();
	textureCoords.clear();
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
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	GLuint ambientLight = glGetUniformLocation(gShaderProgram, "ambientLight");
	GLuint diffuseLight = glGetUniformLocation(gShaderProgram, "diffuseLight");
	GLuint specularLight = glGetUniformLocation(gShaderProgram, "specularLight");
	GLuint cameraPos = glGetUniformLocation(gShaderProgram, "cameraPos");

	GLuint camMatrix = glGetUniformLocation(gShaderProgram, "Camera");
	GLuint perspMatrix = glGetUniformLocation(gShaderProgram, "Perspective");

	glUniform3fv(cameraPos, 1, &pos[0]);

	glUniformMatrix4fv(camMatrix, 1, GL_FALSE, &Cam[0][0]);
	glUniformMatrix4fv(perspMatrix, 1, GL_FALSE, &Persp[0][0]);

	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | dGL_DEPTH_BUFFER_BIT);	
	glUseProgram(gShaderProgram);
	for (uint i = 0; i < objects.size(); i++)
	{
		ambient = objects[i]->mat->Ka;
		diffuse = objects[i]->mat->Kd;
		specular = objects[i]->mat->Ks;

		glUniform3fv(ambientLight, 1, &ambient[0]);
		glUniform3fv(diffuseLight, 1, &diffuse[0]);
		glUniform3fv(specularLight, 1, &specular[0]);

		glBindVertexArray(objects[i]->VAOid);	
		GLuint tempTexid = 5;
		if (objects[i]->mat->materialName != "") {
			tempTexid = objects[i]->mat->texid;
		}
		if (tempTexid == 0)
		{
			tempTexid = 5;
		}
		glBindTexture(GL_TEXTURE_2D, tempTexid);
		
		glDrawArrays(GL_TRIANGLES, 0, objects[i]->faces.size() * 3);	
		glBindVertexArray(0);
	}
	
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
	
		loadObj("mesh.obj");
		loadObj("obj.obj");
		loadObj("file.obj");
		
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

/*int GLGraphics::RenderInstanced(ICamera* _camera)
{
t+=0.001f;

ModelRenderInfo* MRI;

m_standardShaderProgram.UseProgram();
glEnable(GL_DEPTH_TEST);

m_standardShaderProgram.SetUniformV("EyePosition", vec4(_camera->GetPosition(), 1.0f));

for(int i=0; i< m_models.size();i++)
{
MRI = m_models[i];
int instances = m_models[i]->instances.size();

glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, m_models[i]->texHandle);

//Update matrix buffer//
glBindBuffer(GL_ARRAY_BUFFER,m_models[i]->buffers[4]);
glm::mat4* matrices = (glm::mat4*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
int j = 0;

for (std::map < int, ModelInstance*>::const_iterator insIt = m_models[i]->instances.begin(); insIt != m_models[i]->instances.end(); ++insIt)
{
matrices[j] = *insIt->second->world;
//PRINTMATRIX(&matrices[j]);
j++;
}
glUnmapBuffer(GL_ARRAY_BUFFER);
glBindBuffer(GL_ARRAY_BUFFER,0);
//Update matrix buffer//

//Update explosion buffer//

glBindBuffer(GL_ARRAY_BUFFER,m_models[i]->buffers[2]);
float* explosion = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

j = 0;

for (std::map < int, ModelInstance*>::const_iterator insIt = m_models[i]->instances.begin(); insIt != m_models[i]->instances.end(); ++insIt)
{
if(insIt->second->explosion)
explosion[j] = *insIt->second->explosion;
else
explosion[j] = 0.0f;

j++;
}
glUnmapBuffer(GL_ARRAY_BUFFER);
glBindBuffer(GL_ARRAY_BUFFER,0);
//Update explosion buffer<//

glBindVertexArray(MRI->bufferVAOID);

glDrawArraysInstanced(GL_TRIANGLES,0,MRI->vertices,instances);

glBindVertexArray(0);

}

return 1;
}
void GLGraphics::AddObject(int id, std::string model, MATRIX4 _world, MATRIX4 worldInverseTranspose,float* explosion)
{
for(int i=0; i < m_models.size();i++)
{
if (m_models[0]->instances.count(_id) !=0)
return;
}

int newModelID = -1;

for(int i=0; i < m_models.size();i++)
{
if(m_models[i]->name == _model)
{
newModelID = i;
int size = 0;

//Resize instance buffers only
glBindBuffer(GL_ARRAY_BUFFER, m_models[i]->buffers[2]);
glGetBufferParameteriv(GL_ARRAY_BUFFER,GL_BUFFER_SIZE,&size);
glBufferData(GL_ARRAY_BUFFER, size + sizeof(float), NULL, GL_DYNAMIC_DRAW);

glBindBuffer(GL_ARRAY_BUFFER, m_models[i]->buffers[3]);
glGetBufferParameteriv(GL_ARRAY_BUFFER,GL_BUFFER_SIZE,&size);
glBufferData(GL_ARRAY_BUFFER, size + 4 * sizeof(float), NULL, GL_STATIC_READ);

glBindBuffer(GL_ARRAY_BUFFER, m_models[i]->buffers[4]);
glGetBufferParameteriv(GL_ARRAY_BUFFER,GL_BUFFER_SIZE,&size);
glBufferData(GL_ARRAY_BUFFER, size + 4  4  sizeof(float), NULL, GL_DYNAMIC_DRAW);

glBindBuffer(GL_ARRAY_BUFFER,0);
break;
}
}

if(newModelID == -1)
{
newModelID = m_models.size();
printf("Model not found, using LoadModel\n");
LoadModel(_model);
}

ModelInstance *mi = new ModelInstance();

mi->world = _world;
mi->explosion = _explosion;
mi->worldInverseTranspose = _worldInverseTranspose;

m_models[newModelID]->instances.insert(pair<int, ModelInstance*>(_id, mi));
}






struct ModelInstance
{
private:

public:

float   *explosion;
glm::mat4 *world;
glm::mat4 *worldInverseTranspose;
};

struct ModelRenderInfo
{
public:

int vertices;
GLuint bufferVAOID;
GLuint texHandle;
std::string name;
std::map<int, ModelInstance*> instances;

GLuint buffers[6];

ModelRenderInfo(){}

};
*/

/*
int GLGraphics::RenderStandard()
{
	for (int i = 0;i<m_models.size();i++)
	{
		glBindVertexArray(m_models[i]->bufferVAOID);

		glDrawArrays(GL_TRIANGLES, 0, m_models[i]->vertices);

		glBindVertexArray(0);
	}
}
*/