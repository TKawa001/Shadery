// Triangle.cpp
// Our first OpenGL program that will just draw a triangle on the screen.

#include <GLTools.h>            // OpenGL toolkit
#include <GLFrame.h>
#include <GLFrustum.h>
#include <StopWatch.h>

#ifdef __APPLE__
#include <glut/glut.h>          // OS X version of GLUT
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>            // Windows FreeGlut equivalent
#endif

static const float M_PI = 3.14159265358979323846;

GLuint shader;
GLuint MVPMatrixLocation;

GLFrame cameraFrame;

GLFrustum frustum;
M3DMatrix44f matrixVP;
M3DMatrix44f matrixCamera;

M3DVector3f upVector;
M3DVector3f posVector;
M3DVector3f forVector;
CStopWatch timer;

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

void drawPyramid() {
	//Podstawa piramidy
	glBegin(GL_QUADS);
		glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0, 0, 0);
		glVertex3f(-0.5f, -0.5f, 0.0f);
		glVertex3f(0.5f, -0.5f, 0.0f);
		glVertex3f(0.5f, 0.5f, 0.0f);
		glVertex3f(-0.5f, 0.5f, 0.0f);
	glEnd();

	//œciany piramidy
	glBegin(GL_TRIANGLES);
		glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 1.0f);
		glVertex3f(-0.5f, -0.5f, 0.0f);
		glVertex3f(0.5f, -0.5f, 0.0f);

		glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0f, 1.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 1.0f);
		glVertex3f(0.5f, -0.5f, 0.0f);
		glVertex3f(0.5f, 0.5f, 0.0f);

		glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.0f, 1.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 1.0f);
		glVertex3f(0.5f, 0.5f, 0.0f);
		glVertex3f(-0.5f, 0.5f, 0.0f);

		glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 0.0f, 1.0f);
		glVertex3f(-0.5f, 0.5f, 0.0f);
		glVertex3f(-0.5f, -0.5f, 0.0f);
	glEnd();
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

    shader = gltLoadShaderPairWithAttributes("pass_thru_shader.vp", "pass_thru_shader.fp",
            2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_COLOR, "vColor");
    fprintf(stdout, "GLT_ATTRIBUTE_VERTEX : %d\nGLT_ATTRIBUTE_COLOR : %d \n",
            GLT_ATTRIBUTE_VERTEX, GLT_ATTRIBUTE_COLOR);
	
	MVPMatrixLocation=glGetUniformLocation(shader,"MVPMatrix");
	if(MVPMatrixLocation==-1)
	{
		fprintf(stderr,"uniform MVPMatrix could not be found\n");
	}

	glEnable(GL_DEPTH_TEST);

	upVector[0] = 0.0f;
	upVector[1] = 0.0f;
	upVector[2] = 1.0f;

	forVector[0] = 0.0f;
	forVector[1] = 0.0f;
	forVector[2] = 0.0f;

	posVector[0] = 1.0f;
	posVector[1] = 1.0f;
	posVector[2] = 1.0f;

}

///////////////////////////////////////////////////////////////////////////////
// Called to draw scene

void RenderScene(void) {
    // Clear the window with current clearing color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	float angle = timer.GetElapsedSeconds() * M_PI / 3;

	posVector[0] = 6.4f * cos(angle);
	posVector[1] = 6.0f * sin(angle);
	posVector[2] = 4.5f;

	LookAt(cameraFrame, posVector, forVector, upVector);
	cameraFrame.GetCameraMatrix(matrixCamera);
	m3dMatrixMultiply44(matrixVP, frustum.GetProjectionMatrix(), matrixCamera);

	glUniformMatrix4fv(MVPMatrixLocation,1,GL_FALSE,matrixVP);

    glUseProgram(shader);
	
	//siatka 20x20
	glBegin(GL_LINES);
		glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0, 0, 0);
		for(int i = 0; i <= 20; ++i) {
			glVertex3f(-10.f, -10.f + (float)i, 0.f);
			glVertex3f(10.f, -10.f + (float)i, 0.f);	
			glVertex3f(-10.f + (float)i, -10.f, 0.f);
			glVertex3f(-10.f + (float)i, 10.f, 0.f);
		}
	glEnd();
	
	GLShaderManager

	//przesuniêcie priamidy
	M3DMatrix44f matrixM;
	m3dTranslationMatrix44(matrixM, 1.f, 1.0f, 0.f);
	
	M3DMatrix44f matrixMVP;
	m3dMatrixMultiply44(matrixMVP, matrixVP, matrixM);
	glUniformMatrix4fv(MVPMatrixLocation,1,GL_FALSE,matrixMVP);
    
	//Rysowanie Piramidy
	drawPyramid();
	
	//kolejne przesuniêcie i obrót
	M3DMatrix44f matrixT;
	m3dTranslationMatrix44(matrixT, 1.0f, -2.0f, 0.f);
	M3DMatrix44f matrixR;
	m3dRotationMatrix44(matrixR, -10*angle, 0.f, 0.f, 1.f);
	m3dMatrixMultiply44(matrixM, matrixT, matrixR);
	m3dMatrixMultiply44(matrixMVP, matrixVP, matrixM);
	glUniformMatrix4fv(MVPMatrixLocation,1,GL_FALSE,matrixMVP);

	//rysowanie drugiej piramidy
	drawPyramid();

    // Perform the buffer swap to display back buffer
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
