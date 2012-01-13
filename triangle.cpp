// Triangle.cpp
// Our first OpenGL program that will just draw a triangle on the screen.

#include <GLTools.h>            // OpenGL toolkit
#include <GLFrame.h>
#include <GLFrustum.h>
#include <StopWatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <vector>
#ifdef __APPLE__
#include <glut/glut.h>          // OS X version of GLUT
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>            // Windows FreeGlut equivalent
#endif

static const float M_PI = 3.14159265358979323846;

GLuint shader;
GLuint gouraudShader;
GLuint textureShader;

//pass_thru_shader
GLuint MVPMatrixLocation;

//textureShader
GLint textureMVPLocation;
GLint textureMVLocation;
GLint textureNormalMatrixLocation;
GLint textureLightPostion;
GLint textureAmbientLocation;
GLint textureDiffuseLocation;
GLint textureSpecularLocation;
GLint textureAlphaLocation;

GLFrustum frustum;


GLFrame cameraFrame;

M3DMatrix44f matrixCamera;

M3DVector3f upVector;
M3DVector3f posVector;
M3DVector3f forVector;
CStopWatch timer;

//gouard shader
GLint shaderPositionLocation;
GLint shaderColorLocation;
GLint shaderAngleLocation;
GLint shaderAttenuation0Location;
GLint shaderAttenuation1Location;
GLint shaderAttenuation2Location;

GLint gouraudMVPLocation;
GLint gouraudMVLocation;
GLint gouraudNormalMatrixLocation;

GLint gouraudAmbientLocation;
GLint gouraudDiffuseLocation;
GLint gouraudSpecularLocation;
GLint gouraudExponentLocation;

M3DVector3f color = { 1.0f, 0.8f, 0.8f }; //kolor œwiat³a
M3DVector3f position = { 4.0f, 4.0f, 3.0f }; //pozycja œwiat³a

float attenuation0 = 1.0f;
float attenuation1 = 0.045f;
float attenuation2 = 0.0075f;

M3DVector3f ambientLightColor = { 1.0f, 1.0f, 1.0f };
//M3DVector3f ambientLightColor = { 0.1f, 0.1f, 0.1f };
M3DVector3f specularColor = { 1.0f, 1.0f, 1.0f };

GLuint vertex_buffer; //adres bufora wierzcho³ków
GLuint faces_buffer; //adres bufroa wierzcho³ków
int n_faces = 0; //ile œcian posiada obiekt

//texture
GLuint textureID;
GLbyte *pBits;
int nWidth, nHeight, nComponents;
GLenum eFormat;
GLint textureLocation;

void CheckForErrors() {
	GLenum err;
	while((err = glGetError()) != GL_NO_ERROR) {
		switch (err) {
			case GL_INVALID_ENUM:
				fprintf(stderr,"The enum argument is out of range.\n");
				break;
			case GL_INVALID_VALUE:
				fprintf(stderr,"The numeric argument is out of range.\n");
				break;
			case GL_INVALID_OPERATION:
				fprintf(stderr,"The operation is illegal in its current state.\n");
				break;
			case GL_OUT_OF_MEMORY:
				fprintf(stderr,"Not enough memory is left to execute the commandn");
				break;
		}			
	}
}

void LoadAndSetTexture() {
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1,&textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	pBits = gltReadTGABits("flash.tga", &nWidth, &nHeight, &nComponents, &eFormat);
	if(pBits == NULL) {
		fprintf(stderr,"Error reading texture\n");
	}
	glTexImage2D(GL_TEXTURE_2D, 0, nComponents, nWidth, nHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBits);
	textureLocation = glGetUniformLocation(textureShader, "texture0");
	if(textureLocation == -1) {
		fprintf(stderr,"uniform textureLocation could not be found in textureShader \n");
	}
	glUniform1i(textureLocation, 0);
}

void GLcheck() {
	fprintf(stdout,"Vendor: ");
	fprintf(stdout,(const char *)glGetString(GL_VENDOR));
	fprintf(stdout,"\nRenderer: ");
	fprintf(stdout,(const char *)glGetString(GL_RENDERER));
	fprintf(stdout,"\nOpenGL Version: ");
	fprintf(stdout,(const char *)glGetString(GL_VERSION));
	fprintf(stdout,"\nShading Language Version: ");
	fprintf(stdout,(const char *)glGetString(GL_SHADING_LANGUAGE_VERSION));
	fprintf(stdout,"\n\n");
}
void LoadVertices() {
	FILE * fvertices=fopen("geode_vertices.dat","r");
	
	if(fvertices==NULL) {
		fprintf(stderr,"cannot open vertices file for reading\n");
		exit(-1);
	}
	char line[120];
   
	std::vector<float > vertices;
	float x,y,z;
	double norm;
	int n_vertices = 0;
	while(fgets(line,120,fvertices)!=NULL) {
		sscanf(line,"%f %f %f",&x,&y,&z);
		  
		norm=x*x+y*y+z*z;
		norm=sqrt(norm);
		n_vertices++;
		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(z);
		vertices.push_back(1.0f);
		vertices.push_back(x/norm);
		vertices.push_back(y/norm);
		vertices.push_back(z/norm);
	}

	fprintf(stderr,"nv = %u %u\n",n_vertices,vertices.size());

	glGenBuffers(1,&vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	glBufferData(GL_ARRAY_BUFFER,n_vertices*sizeof(float)*7,&vertices[0],GL_STATIC_DRAW);
	if(glGetError()!=GL_NO_ERROR) {
		fprintf(stderr,"error copying vertices\n");
	}
	
	glVertexAttribPointer(GLT_ATTRIBUTE_VERTEX,4,GL_FLOAT,GL_FALSE,sizeof(float)*7,(const GLvoid *)0);

	glVertexAttribPointer(GLT_ATTRIBUTE_NORMAL,3,GL_FLOAT,GL_FALSE, sizeof(float)*7,(const GLvoid *)(4*sizeof(float)) );

	glEnableVertexAttribArray(GLT_ATTRIBUTE_VERTEX);
	glEnableVertexAttribArray(GLT_ATTRIBUTE_NORMAL);
}

void LoadFaces() {
	FILE *ffaces=fopen("geode_faces.dat","r");
	if(ffaces==NULL) {
		fprintf(stderr,"cannot open faces file for reading\n");
		exit(-1);
	}

	char line[120];

	std::vector<GLuint> faces;
	GLuint  i,j,k;
	while(fgets(line,120,ffaces)!=NULL) {
		   
		if(3!=sscanf(line,"%u %u %u",&i,&j,&k)){
			fprintf(stderr,"error reading faces\n"); 
			exit(-1);
		}
		//fprintf(stderr,"%u %u %u\n",i-1,j-1,k-1);
		n_faces++;
		faces.push_back(i-1);
		faces.push_back(j-1);
		faces.push_back(k-1);
	}
 	fprintf(stderr,"nf = %u\n",n_faces);

	glGenBuffers(1,&faces_buffer);
	if(glGetError()!=GL_NO_ERROR) {
		fprintf(stderr,"faces_buffer invalid\n");
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,faces_buffer);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER,n_faces*sizeof(GLuint)*3,&faces[0],GL_STATIC_DRAW);
	if(glGetError()!=GL_NO_ERROR) {
		fprintf(stderr,"error copying faces\n");
	}
}


void SetUpFrame(GLFrame &frame,const M3DVector3f origin, const M3DVector3f forward, const M3DVector3f up) {
	frame.SetOrigin(origin);
	frame.SetForwardVector(forward);
	M3DVector3f side,oUp;
	m3dCrossProduct3(side,forward,up);
	m3dCrossProduct3(oUp,side,forward);
	frame.SetUpVector(oUp);
	frame.Normalize();
};

void LookAt(GLFrame &frame, const M3DVector3f eye, const M3DVector3f at, const M3DVector3f up) {
	M3DVector3f forward;
	m3dSubtractVectors3(forward, at, eye);
	SetUpFrame(frame, eye, forward, up);
}



///////////////////////////////////////////////////////////////////////////////
// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.

void ChangeSize(int w, int h) {
    glViewport(0, 0, w, h);

	frustum.SetPerspective(45.0f, (float)w / (float)h, 0.1f, 1000.0f);
}


///////////////////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering context.
// This is the first opportunity to do any OpenGL related tasks.

void SetupRC() {
    // Blue background
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	GLcheck();

	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);
	CheckForErrors();

	shader = gltLoadShaderPairWithAttributes("pass_thru_shader.vp", "pass_thru_shader.fp", 2, 
			GLT_ATTRIBUTE_VERTEX, "vVertex", 
			GLT_ATTRIBUTE_COLOR, "vColor");
	
	fprintf(stdout, "GLT_ATTRIBUTE_VERTEX : %d\nGLT_ATTRIBUTE_COLOR : %d \n", GLT_ATTRIBUTE_VERTEX, GLT_ATTRIBUTE_COLOR);

    gouraudShader = gltLoadShaderPairWithAttributes("gouraud.vp", "gouraud.fp", 2, 
			GLT_ATTRIBUTE_VERTEX, "vVertex",  
			GLT_ATTRIBUTE_NORMAL, "vNormal");

    fprintf(stdout, "GLT_ATTRIBUTE_VERTEX : %d\nGLT_ATTRIBUTE_COLOR : %d \n", GLT_ATTRIBUTE_VERTEX, GLT_ATTRIBUTE_COLOR);

	textureShader = gltLoadShaderPairWithAttributes("shader.vp", "shader.fp", 4,
            GLT_ATTRIBUTE_VERTEX, "vVertex",
            GLT_ATTRIBUTE_COLOR, "vColor",
            GLT_ATTRIBUTE_TEXTURE0, "texCoord0",
            GLT_ATTRIBUTE_NORMAL, "vNormal");
	
	//shader unifroms
	MVPMatrixLocation = glGetUniformLocation(shader, "MVPMatrix");

	if(MVPMatrixLocation == -1) {
		fprintf(stderr,"uniform MVPMatrix could not be found in shader\n");
	}
	
	//textureShader Uniforms
	textureMVPLocation = glGetUniformLocation(textureShader, "MVPMatrix");
	if(textureMVPLocation == -1) {
		fprintf(stderr,"uniform MVPMatrix could not be found in textureShader \n");
	}

	textureMVLocation = glGetUniformLocation(textureShader, "MVMatrix");
	if(textureMVLocation == -1) {
		fprintf(stderr,"uniform MVMatrix could not be found in textureShader \n");
	}

	textureNormalMatrixLocation = glGetUniformLocation(textureShader, "normalMatrix");
	if(textureMVLocation == -1) {
		fprintf(stderr,"uniform normalMatrix could not be found in textureShader \n");
	}

	textureLightPostion = glGetUniformLocation(textureShader, "lightPosition");
	if(textureMVLocation == -1) {
		fprintf(stderr,"uniform lightPosition could not be found in textureShader \n");
	}

	textureAmbientLocation = glGetUniformLocation(textureShader, "ambientColor");
	if(textureMVLocation == -1) {
		fprintf(stderr,"uniform ambientColor could not be found in textureShader \n");
	}
	
	textureDiffuseLocation = glGetUniformLocation(textureShader, "diffuseColor");
	if(textureMVLocation == -1) {
		fprintf(stderr,"uniform diffuseColor could not be found in textureShader \n");
	}
	
	textureSpecularLocation = glGetUniformLocation(textureShader, "specularColor");
	if(textureMVLocation == -1) {
		fprintf(stderr,"uniform specularColor could not be found in textureShader \n");
	}
	
	textureAlphaLocation = glGetUniformLocation(textureShader, "alpha");
	if(textureMVLocation == -1) {
		fprintf(stderr,"uniform alpha could not be found in textureShader \n");
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	upVector[0] = 0.0f;
	upVector[1] = 0.0f;
	upVector[2] = 1.0f;

	forVector[0] = 0.0f;
	forVector[1] = 0.0f;
	forVector[2] = 0.0f;

	posVector[0] = 1.0f;
	posVector[1] = 1.0f;
	posVector[2] = 1.0f;

	shaderPositionLocation = glGetUniformLocation(gouraudShader, "light1.position");

	if(shaderPositionLocation == -1) {
		fprintf(stderr,"light1.position could not be found\n");
	}

	shaderColorLocation = glGetUniformLocation(gouraudShader, "light1.color");

	if(shaderColorLocation == -1) {
		fprintf(stderr,"light1.color could not be found\n");
	}

	shaderAttenuation0Location = glGetUniformLocation(gouraudShader, "light1.attenuation0");
	
	if(shaderAttenuation0Location == -1) {
		fprintf(stderr,"light1.attenuation0 could not be found\n");
	}

	shaderAttenuation1Location = glGetUniformLocation(gouraudShader, "light1.attenuation1");
	
	if(shaderAttenuation1Location == -1) {
		fprintf(stderr,"light1.attenuation1 could not be found\n");
	}

	shaderAttenuation2Location = glGetUniformLocation(gouraudShader, "light1.attenuation2");
	
	if(shaderAttenuation2Location == -1) {
		fprintf(stderr,"light1.attenuation2 could not be found\n");
	}

	gouraudAmbientLocation = glGetUniformLocation(gouraudShader, "material.ambientColor");

	if(gouraudAmbientLocation == -1) {
		fprintf(stderr,"material.ambientColor could not be found\n");
	}

	gouraudDiffuseLocation = glGetUniformLocation(gouraudShader, "material.diffuseColor");

	if(gouraudDiffuseLocation == -1) {
		fprintf(stderr,"material.diffuseColor could not be found\n");
	}

	gouraudSpecularLocation = glGetUniformLocation(gouraudShader, "material.specularColor");

	if(gouraudSpecularLocation == -1) {
		fprintf(stderr,"material.specularColor could not be found\n");
	}

	gouraudExponentLocation = glGetUniformLocation(gouraudShader, "material.specularExponent");

	if(gouraudExponentLocation == -1) {
		fprintf(stderr,"material.specularExponent could not be found\n");
	}

	gouraudMVPLocation = glGetUniformLocation(gouraudShader, "MVP");
	
	if(gouraudMVPLocation == -1) {
		fprintf(stderr,"MV could not be found\n");
	}

	gouraudMVLocation = glGetUniformLocation(gouraudShader, "MV");
	
	if(gouraudMVLocation == -1) {
		fprintf(stderr,"MV could not be found\n");
	}
	gouraudNormalMatrixLocation = glGetUniformLocation(gouraudShader, "normalMatrix");
	
	if(gouraudNormalMatrixLocation == -1) {
		fprintf(stderr,"normal matrix could not be found\n");
	}

	LoadVertices();
	LoadFaces();
	LoadAndSetTexture();

}

void triangleFace(const M3DVector3f a, const M3DVector3f b, const M3DVector3f c) {
	M3DVector3f normal, bMa, cMa;
	m3dSubtractVectors3(bMa, b, a);
	m3dSubtractVectors3(cMa, c, a);
	m3dCrossProduct3(normal, bMa, cMa);
	m3dNormalizeVector3(normal);

	glVertexAttrib3fv(GLT_ATTRIBUTE_NORMAL, normal);

	glVertexAttrib2f(GLT_ATTRIBUTE_TEXTURE0, 0.5f, 1.f);
	glVertex3fv(a);
	glVertexAttrib2f(GLT_ATTRIBUTE_TEXTURE0, 0.0f, 0.0f);
	glVertex3fv(b);
	glVertexAttrib2f(GLT_ATTRIBUTE_TEXTURE0, 1.f, 0.f);
	glVertex3fv(c);
}

void drawTexturedPyramid() {
	static const float specularExponent = 100.0f;
	static const M3DVector3f diffuseColor = { 0.6f, 0.6f, 0.6f };
	static const M3DVector3f ambientColor = { 0.6f, 0.6f, 0.6f };

	static const M3DVector3f a = { 0.0f, 0.0f , 2.0f };
	static const M3DVector3f b = { -1.0f, -1.0f , 0.0f };
	static const M3DVector3f c = { 1.0f, -1.0f , 0.0f };
	static const M3DVector3f d = { 1.0f, 1.0f , 0.0f };
	static const M3DVector3f e = { -1.0f, 1.0f , 0.0f };

	/*glUniform3fv(shaderColorLocation, 1, color);
	glUniform1f(shaderAttenuation0Location, attenuation0);
	glUniform1f(shaderAttenuation1Location, attenuation1);
	glUniform1f(shaderAttenuation2Location, attenuation2);*/

	glUniform4fv(textureAmbientLocation, 1, ambientColor);
	glUniform4fv(textureDiffuseLocation, 1, diffuseColor);
	glUniform4fv(textureSpecularLocation, 1, specularColor);
	glUniform1f(textureAlphaLocation, 0.5f);

	glBegin(GL_TRIANGLES);		
		triangleFace(a, b, c);
		triangleFace(a, c, d);
		triangleFace(a, d, e);
		triangleFace(a, e, b);
	glEnd();

	glBegin(GL_TRIANGLES);
		glVertexAttrib2f(GLT_ATTRIBUTE_TEXTURE0, 0.f, 0.f);
		glVertex3fv(b);
		glVertexAttrib2f(GLT_ATTRIBUTE_TEXTURE0, 1.f, 0.0f);
		glVertex3fv(c);
		glVertexAttrib2f(GLT_ATTRIBUTE_TEXTURE0, 1.f, 1.f);
		glVertex3fv(d);
		//triangleFace(b, c, d);
		glVertexAttrib2f(GLT_ATTRIBUTE_TEXTURE0, 1.f, 1.f);
		glVertex3fv(d);
		glVertexAttrib2f(GLT_ATTRIBUTE_TEXTURE0, 0.f, 1.f);
		glVertex3fv(e);
		glVertexAttrib2f(GLT_ATTRIBUTE_TEXTURE0, 0.f, 0.f);
		glVertex3fv(b);
		//triangleFace(d, e, b);
	glEnd();
}
void drawPyramid() {
	static const float specularExponent = 100.0f;
	static const M3DVector3f diffuseColor = { 0.0f, 0.9f, 0.3f };
	static const M3DVector3f ambientColor = { 0.1f, 0.3f, 0.1f };

	static const M3DVector3f a = { 0.0f, 0.0f , 2.0f };
	static const M3DVector3f b = { -1.0f, -1.0f , 0.0f };
	static const M3DVector3f c = { 1.0f, -1.0f , 0.0f };
	static const M3DVector3f d = { 1.0f, 1.0f , 0.0f };
	static const M3DVector3f e = { -1.0f, 1.0f , 0.0f };

	glUniform3fv(shaderColorLocation, 1, color);
	glUniform1f(shaderAttenuation0Location, attenuation0);
	glUniform1f(shaderAttenuation1Location, attenuation1);
	glUniform1f(shaderAttenuation2Location, attenuation2);

	glUniform3fv(gouraudAmbientLocation, 1, ambientColor);
	glUniform3fv(gouraudDiffuseLocation, 1, diffuseColor);
	glUniform3fv(gouraudSpecularLocation, 1, specularColor);
	glUniform1f(gouraudExponentLocation, specularExponent);

	glBegin(GL_TRIANGLES);
		triangleFace(a, b, c);
		triangleFace(a, c, d);
		triangleFace(a, d, e);
		triangleFace(a, e, b);
	glEnd();

	glBegin(GL_TRIANGLES);
		triangleFace(b, c, d);
		triangleFace(d, e, b);
	glEnd();
}

void drawFloor() {
	static const float specularExponent = 100.0f;
	static const M3DVector3f diffuseColor = { 0.9f, 0.9f, 0.3f };
	static const M3DVector3f ambientColor = { 0.2f, 0.1f, 0.1f };

	static const M3DVector3f a = { -10.0f, -10.0f , 0.0f };
	static const M3DVector3f b = { 10.0f, -10.0f , 0.0f };
	static const M3DVector3f c = { 10.0f, 10.0f , 0.0f };
	static const M3DVector3f d = { -10.0f, 10.0f , 0.0f };

	glUniform3fv(shaderColorLocation, 1, color);
	glUniform1f(shaderAttenuation0Location, attenuation0);
	glUniform1f(shaderAttenuation1Location, attenuation1);
	glUniform1f(shaderAttenuation2Location, attenuation2);

	glUniform3fv(gouraudAmbientLocation, 1, ambientColor);
	glUniform3fv(gouraudDiffuseLocation, 1, diffuseColor);
	glUniform3fv(gouraudSpecularLocation, 1, specularColor);
	glUniform1f(gouraudExponentLocation, specularExponent);

	glBegin(GL_TRIANGLES);
		triangleFace(a, b, c);
		triangleFace(c, d, a);
	glEnd();
}

///////////////////////////////////////////////////////////////////////////////
// Called to draw scene

void RenderScene(void) {
    // Clear the window with current clearing color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	float angle = timer.GetElapsedSeconds() * M_PI / 6;
	static float distanceFromCenter = 1.5f; 
	posVector[0] = distanceFromCenter * 15.4f * cos(angle/2);
	posVector[1] = distanceFromCenter * 15.0f * sin(angle/2);
	posVector[2] = distanceFromCenter * 11.5f;
	
	GLMatrixStack modelViewMatrix; // Modelview Matrix
	GLMatrixStack projectionMatrix; // Projection Matrix
	GLGeometryTransform geometryPipeline; // Geometry Transform Pipeline

	projectionMatrix.LoadMatrix(frustum.GetProjectionMatrix());
	// Set the transformation pipeline to use the two matrix stacks
	geometryPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);

	LookAt(cameraFrame, posVector, forVector, upVector);
	
	glUseProgram(shader);

	//dodanie kamery do macierzy widoku
	modelViewMatrix.PushMatrix();
	cameraFrame.GetCameraMatrix(matrixCamera);
	modelViewMatrix.MultMatrix(matrixCamera);
	glUniformMatrix4fv(MVPMatrixLocation,1,GL_FALSE,geometryPipeline.GetModelViewProjectionMatrix());
	
	//siatka 20x20
	glBegin(GL_LINES);
		glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1, 1, 1);
		for(int i = 0; i <= 20; ++i) {
			glVertex3f(-10.f, -10.f + (float)i, 0.f);
			glVertex3f(10.f, -10.f + (float)i, 0.f);	
			glVertex3f(-10.f + (float)i, -10.f, 0.f);
			glVertex3f(-10.f + (float)i, 10.f, 0.f);
		}
	glEnd();

	glUseProgram(gouraudShader);

	glPolygonOffset(1.0f, 1.0f);
	glEnable(GL_POLYGON_OFFSET_FILL);

	glUseProgram(gouraudShader);
	glUniformMatrix4fv(gouraudMVPLocation, 1, GL_FALSE, geometryPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix4fv(gouraudMVLocation, 1, GL_FALSE, geometryPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(gouraudNormalMatrixLocation, 1, GL_FALSE, geometryPipeline.GetNormalMatrix());
	M3DVector3f lightPos;
	m3dTransformVector3(lightPos, position, geometryPipeline.GetModelViewMatrix());
	glUniform3fv(shaderPositionLocation, 1, lightPos);
	drawFloor();
	glDisable(GL_POLYGON_OFFSET_FILL);
	
	//rysowanie kuli z bufora
	modelViewMatrix.PushMatrix();
	modelViewMatrix.Translate(0.f, 0.f, 4.f);
	modelViewMatrix.Scale(1.5f, 1.5f, 1.5f);
	glUniformMatrix4fv(gouraudMVPLocation, 1, GL_FALSE, geometryPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix4fv(gouraudMVLocation, 1, GL_FALSE, geometryPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(gouraudNormalMatrixLocation, 1, GL_FALSE, geometryPipeline.GetNormalMatrix());
	glDrawElements(GL_TRIANGLES,3*n_faces,GL_UNSIGNED_INT,0);
	modelViewMatrix.PopMatrix();
	
	//-----
	glUseProgram(textureShader);

	modelViewMatrix.PushMatrix();
	modelViewMatrix.Translate(5.0f,0.0f, 0.0f);
	glUniformMatrix4fv(textureMVPLocation, 1, GL_FALSE, geometryPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix4fv(textureMVLocation, 1, GL_FALSE, geometryPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(textureNormalMatrixLocation, 1, GL_FALSE, geometryPipeline.GetNormalMatrix());
	m3dTransformVector3(lightPos, position, geometryPipeline.GetModelViewMatrix());
	glUniform3fv(textureLightPostion, 1, lightPos);
	//Rysowanie Nieruchomej Piramidy na œrodku
	drawTexturedPyramid();
	modelViewMatrix.PopMatrix();
	

	glUseProgram(gouraudShader);

	//-----	
	glUniformMatrix4fv(gouraudMVPLocation, 1, GL_FALSE, geometryPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix4fv(gouraudMVLocation, 1, GL_FALSE, geometryPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(gouraudNormalMatrixLocation, 1, GL_FALSE, geometryPipeline.GetNormalMatrix());
	m3dTransformVector3(lightPos, position, geometryPipeline.GetModelViewMatrix());
	glUniform3fv(shaderPositionLocation, 1, lightPos);
	//Rysowanie Nieruchomej Piramidy na œrodku
	drawPyramid();

	modelViewMatrix.PushMatrix();
	modelViewMatrix.Translate(2.5f, 2.5f, 0.f);
	modelViewMatrix.Scale(1.5f, 1.5f, 1.5f);
	glUniformMatrix4fv(gouraudMVPLocation,1,GL_FALSE,geometryPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix4fv(gouraudMVLocation, 1, GL_FALSE, geometryPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(gouraudNormalMatrixLocation, 1, GL_FALSE, geometryPipeline.GetNormalMatrix());
	//Ryswoanie 2x powiêkszonej piramidy
	drawPyramid();

	modelViewMatrix.Translate(2.5f, 2.5f, 0.f);
	modelViewMatrix.Scale(1.5f, 1.5f, 1.5f);
	glUniformMatrix4fv(gouraudMVPLocation,1,GL_FALSE,geometryPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix4fv(gouraudMVLocation, 1, GL_FALSE, geometryPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(gouraudNormalMatrixLocation, 1, GL_FALSE, geometryPipeline.GetNormalMatrix());
	//Rysowanie 4x powiêkszonej piramidy
	drawPyramid();
	
	//wrócenie do stanu w którym jest przemno¿ona kamera 
	modelViewMatrix.PopMatrix();
						
	//-----
	glUseProgram(textureShader);
	modelViewMatrix.PushMatrix();
	modelViewMatrix.Translate(-4.0f, -1.0f, 0.f);
	modelViewMatrix.PushMatrix();
	modelViewMatrix.Rotate(-10*(angle*180/M_PI), 0.f, 0.f, 1.f);
	glUniformMatrix4fv(textureMVPLocation, 1, GL_FALSE, geometryPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix4fv(textureMVLocation, 1, GL_FALSE, geometryPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(textureNormalMatrixLocation, 1, GL_FALSE, geometryPipeline.GetNormalMatrix());
	m3dTransformVector3(lightPos, position, geometryPipeline.GetModelViewMatrix());
	glUniform3fv(textureLightPostion, 1, lightPos);
	//rysowanie dolnej krêc¹cej siê piramidy
	drawTexturedPyramid();
						
	modelViewMatrix.PopMatrix();
	modelViewMatrix.Translate(0.f, 0.f, 4.0f);
	modelViewMatrix.Rotate(180.f, 1.f, 0.f, 0.f);
	modelViewMatrix.Rotate(-10*(angle*180/M_PI), 0.f, 0.f, 1.f);
	glUniformMatrix4fv(textureMVPLocation, 1, GL_FALSE, geometryPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix4fv(textureMVLocation, 1, GL_FALSE, geometryPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(textureNormalMatrixLocation, 1, GL_FALSE, geometryPipeline.GetNormalMatrix());
	m3dTransformVector3(lightPos, position, geometryPipeline.GetModelViewMatrix());
	glUniform3fv(textureLightPostion, 1, lightPos);
	//rysowanie górnej krêc¹cej siê piramidy
	drawTexturedPyramid();

	//wrócenie do stanu w którym jest przemno¿ona kamera i przeskalowany œwiat
	modelViewMatrix.PopMatrix();

	//-----
	modelViewMatrix.PushMatrix();
	modelViewMatrix.Translate(-5.f, 5.f, 0.f);
	modelViewMatrix.Translate(0.f, 0.f, 5.f);
	static float angle2 = 0.0f;
	static float d = 0.01f;
	modelViewMatrix.Rotate(angle2, 1.f, 0.f, 0.f);
	if(angle2 > 45) {
		d = d * (-1);
	} else if (angle2 < - 45) {
		d = d * (-1);
	}
	angle2 = angle2 + d;
	modelViewMatrix.Translate(0.f, 0.f, -5.f);
	modelViewMatrix.Scale(1.f, 1.f, 4.f);
	glUniformMatrix4fv(textureMVPLocation, 1, GL_FALSE, geometryPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix4fv(textureMVLocation, 1, GL_FALSE, geometryPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(textureNormalMatrixLocation, 1, GL_FALSE, geometryPipeline.GetNormalMatrix());
	m3dTransformVector3(lightPos, position, geometryPipeline.GetModelViewMatrix());
	glUniform3fv(textureLightPostion, 1, lightPos);
	//rysowanie piramidy przeskalowanej w górê)
	drawTexturedPyramid();
	
	//wrócenie do stanu w którym jest przemno¿ona kamera i przeskalowany œwiat
	modelViewMatrix.PopMatrix();


	modelViewMatrix.PushMatrix();
	modelViewMatrix.Translate(-3.f, -6.f, 0.f);
	static float x = 0.0f;
	static float d2 = 0.001f;
	modelViewMatrix.Translate(x, 0.f ,0.f);
	if(x > 2) {
		d2 = d2 * (-1);
	} else if (x < -2) {
		d2 = d2 * (-1);
	}
	x = x + d2;
	glUniformMatrix4fv(textureMVPLocation, 1, GL_FALSE, geometryPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix4fv(textureMVLocation, 1, GL_FALSE, geometryPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(textureNormalMatrixLocation, 1, GL_FALSE, geometryPipeline.GetNormalMatrix());
	m3dTransformVector3(lightPos, position, geometryPipeline.GetModelViewMatrix());
	glUniform3fv(textureLightPostion, 1, lightPos);
	//rysowanie poruszaj¹cej siê piramidy
	drawTexturedPyramid();

	//wrócenie do stanu w którym jest przemno¿ona kamera i przeskalowany œwiat
	modelViewMatrix.PopMatrix();

	//-----
	
	modelViewMatrix.PushMatrix();
	modelViewMatrix.Translate(5.f,-5.f,1.f);
	modelViewMatrix.Rotate(90.f, 1.f, 0.f, 0.f);
	modelViewMatrix.Rotate(-5*(angle*180/M_PI), 0.f, 1.f, 0.f);
	modelViewMatrix.Translate(0.f, 0.f, -3.f);
	glUniformMatrix4fv(textureMVPLocation, 1, GL_FALSE, geometryPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix4fv(textureMVLocation, 1, GL_FALSE, geometryPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(textureNormalMatrixLocation, 1, GL_FALSE, geometryPipeline.GetNormalMatrix());
	m3dTransformVector3(lightPos, position, geometryPipeline.GetModelViewMatrix());
	glUniform3fv(textureLightPostion, 1, lightPos);
	//rysowanie krêc¹cej siê obróconej piramidy 
	drawTexturedPyramid();

	//wyczszczenie stosu
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PopMatrix();
	//Perform the buffer swap to display back buffer
    glutSwapBuffers();

	glutPostRedisplay();
}


///////////////////////////////////////////////////////////////////////////////
// Main entry point for GLUT based programs

int main(int argc, char* argv[]) {
  

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Triangles");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }

    SetupRC();

    glutMainLoop();
    return 0;
}
