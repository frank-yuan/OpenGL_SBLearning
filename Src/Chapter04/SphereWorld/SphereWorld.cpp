// SphereWorld.cpp
// OpenGL SuperBible
// New and improved (performance) sphere world
// Program by Richard S. Wright Jr.

#include <GLTools.h>
#include <GLShaderManager.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

#include <math.h>
#include <stdio.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
//#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif





GLShaderManager		shaderManager;			// Shader Manager
GLMatrixStack		modelViewMatrix;		// Modelview Matrix
GLMatrixStack		projectionMatrix;		// Projection Matrix
GLFrustum			viewFrustum;			// View Frustum
GLGeometryTransform	transformPipeline;		// Geometry Transform Pipeline

GLTriangleBatch		torusBatch;
GLBatch				floorBatch;
GLTriangleBatch		sphereBatch;

GLFrame				cameraFrame;

//################ custom shader defnination #####################
GLuint				lightShader;
GLuint              textureShader;
//################################################################

//################ custom shader defnination #####################
#define             TEXTURE_COUNT 1
GLuint				textures[TEXTURE_COUNT];
const char          *textureFileName[TEXTURE_COUNT] = {"data/texture/brick.tga"};
//################################################################


//////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering
// context. 
void SetupRC()
    {
	// Initialze Shader Manager
	shaderManager.InitializeStockShaders();
	
	glEnable(GL_DEPTH_TEST);
//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	// This makes a torus
	gltMakeTorus(torusBatch, 0.4f, 0.15f, 30, 30);
	gltMakeSphere(sphereBatch, 0.65f, 26, 13);	
    	
	floorBatch.Begin(GL_TRIANGLE_STRIP, 8, 1);
        GLfloat step = 1.0f;
        GLfloat z = -2.0f;
    for(GLfloat x = -1.0; x < 2.0f; x+= step * 2) {
        floorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
        floorBatch.Color4f(1, 0, 0, 0);
        floorBatch.Vertex3f(x - step, -0.5f, z -step);
        
        floorBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
        floorBatch.Color4f(0, 0, 1, 0);
        floorBatch.Vertex3f(x - step, -0.5f, z + step);
        
        floorBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
        floorBatch.Color4f(1, 0, 1, 0);
        floorBatch.Vertex3f(x + step, -0.5f, z-step);
        
        floorBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
        floorBatch.Color4f(1, 1, 0, 0);
        floorBatch.Vertex3f(x + step, -0.5f, z + step);
    }
    floorBatch.End();    

	//################ custom shader initialize #####################
	lightShader = gltLoadShaderPairWithAttributes("shaders/ADGLight.vp", "shaders/ADGLight.fp", 2, 
					GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_NORMAL, "vNormal");
        textureShader = gltLoadShaderPairWithAttributes("shaders/texture.vp", "shaders/texture.fp", 2,
                                                        GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
	//################################################################

    //################ texture #####################
        glGenTextures(TEXTURE_COUNT, textures);
        for (int i = 0; i < TEXTURE_COUNT; ++i)
        {
            GLint iWidth, iHeight, iComponents;
            GLenum eFormat;
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            GLbyte* pBytes = gltReadTGABits(textureFileName[i], &iWidth ,&iHeight, &iComponents, &eFormat);
            if (pBytes)
            {
                glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
                free(pBytes);
            }
        }
    //###############################################
    }

///////////////////////////////////////////////////
// Screen changes size or is initialized
void ChangeSize(int nWidth, int nHeight)
    {
	glViewport(0, 0, nWidth, nHeight);
	
    // Create the projection matrix, and load it on the projection matrix stack
	viewFrustum.SetPerspective(35.0f, float(nWidth)/float(nHeight), 1.0f, 100.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    // Set the transformation pipeline to use the two matrix stacks 
	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
    }

        
// Called to draw scene
void RenderScene(void)
	{
    // Color values
    static GLfloat vFloorColor[] = { 0.0f, 1.0f, 0.0f, 1.0f};
    static GLfloat vTorusColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    static GLfloat vAmbientColor[] = { 0.0f, 0.0f, 0.3f, 1.0f };
    static GLfloat vSpecularColor[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    static GLfloat vSphereColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    static GLfloat vLightPos[] = { .0f, 100.0f, 100.0f, 1.0f};

    // Time Based animation
	static CStopWatch	rotTimer;
	float yRot = rotTimer.GetElapsedSeconds() * 60.0f;
	
	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
    //vLightPos = modelViewMatrix
    // Save the current modelview matrix (the identity matrix)
        modelViewMatrix.LoadIdentity();
	modelViewMatrix.PushMatrix();	

	//cameraFrame.RotateLocalY(yRot / 30.0f);
	//cameraFrame.MoveForward(-3.0f);
	M3DMatrix44f cameraMatrix;
	cameraFrame.GetCameraMatrix(cameraMatrix);
	modelViewMatrix.LoadMatrix(cameraMatrix);
		// Draw the ground
		/*shaderManager.UseStockShader(GLT_SHADER_SHADED,
									 transformPipeline.GetModelViewProjectionMatrix(),
									 vFloorColor);	*/
        if (textureShader)
        {
//            shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipeline.GetModelViewProjectionMatrix(), 0);

            GLint iMvpMatrix, iTextureUnit0;
            iMvpMatrix = glGetUniformLocation(textureShader, "mvpMatrix");
            iTextureUnit0 = glGetUniformLocation(textureShader, "textureUnit0");
            glUniformMatrix4fv(iMvpMatrix, 1, false, transformPipeline.GetModelViewProjectionMatrix());
			glUniform1i(iTextureUnit0, 0);
            glBindTexture(GL_TEXTURE_2D, textures[0]);
            floorBatch.Draw();
        }

    	// Draw the spinning Torus
    	modelViewMatrix.Translate(0.0f, 0.0f, -2.5f);
    	modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);
    	//shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(),
    	//                            vTorusColor);
    	//torusBatch.Draw();

		//################ custom shader rendering #####################
		if (lightShader)
		{
			modelViewMatrix.PushMatrix();
			{
				//modelViewMatrix.Translate(0.4f, 0, 0);
				//modelViewMatrix.Rotate(yRot / 2, 0.0f, 1.0f, 0.0f);
				//modelViewMatrix.Translate(0.3f, 0, 0);
				glUseProgram(lightShader);

				GLint iDiffuseColor, iAmbientColor, iMvpMatrix, iMvMatrix, iNormalMatrix, iLightPos, iSpecularColor, iShiness;
				iDiffuseColor = glGetUniformLocation(lightShader, "vDiffuseColor");
				iAmbientColor = glGetUniformLocation(lightShader, "vAmbientColor");
				iMvpMatrix = glGetUniformLocation(lightShader, "mvpMatrix");
				iMvMatrix = glGetUniformLocation(lightShader, "mvMatrix");
				iNormalMatrix = glGetUniformLocation(lightShader, "normalMatrix");
				iLightPos = glGetUniformLocation(lightShader, "vLightPos");
				iSpecularColor = glGetUniformLocation(lightShader, "vSpecularColor");
				iShiness = glGetUniformLocation(lightShader, "fShiness");
				M3DVector4f lightPos;
				m3dTransformVector4(lightPos, vLightPos, cameraMatrix);
				glUniform4fv(iLightPos, 1, lightPos);
				glUniform4fv(iDiffuseColor, 1, vSphereColor);
				glUniform4fv(iAmbientColor, 1, vAmbientColor);
				glUniform4fv(iSpecularColor, 1, vSpecularColor);
				glUniformMatrix4fv(iMvpMatrix, 1, false, transformPipeline.GetModelViewProjectionMatrix());
				glUniformMatrix4fv(iMvMatrix, 1, false, transformPipeline.GetModelViewMatrix());
				glUniformMatrix3fv(iNormalMatrix, 1, false, transformPipeline.GetNormalMatrix());
				glUniform1f(iShiness, 8.0f);
				sphereBatch.Draw();
			}
			modelViewMatrix.PopMatrix();
		}
		else
		{
			modelViewMatrix.PushMatrix();
			{
				M3DVector4f lightPos;
				m3dTransformVector4(lightPos, vLightPos, cameraMatrix);

				shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(), 
					transformPipeline.GetProjectionMatrix(), lightPos, vFloorColor);
				sphereBatch.Draw();
			}
			modelViewMatrix.PopMatrix();
		}
		//################################################################
	// Restore the previous modleview matrix (the idenity matrix)
	modelViewMatrix.PopMatrix();
        
    // Do the buffer Swap
    glutSwapBuffers();
        
    // Tell GLUT to do it again
    glutPostRedisplay();
    }


GLfloat gStepSize = 0.05f;
void KeyboardHandler(unsigned char key, int x, int y)
{
	printf("input key %c, %d, (%d, %d)\n", key, key, x, y);
	if (key == 'w')
		cameraFrame.MoveForward(gStepSize);

	if(key == 's')
		cameraFrame.MoveForward(-gStepSize);

	if (key == 'a')
		cameraFrame.MoveRight(gStepSize);
	if(key == 'd')
		cameraFrame.MoveRight(-gStepSize);

}
void SpecialKeys(int key, int x, int y)
{


	if(key == GLUT_KEY_UP)
		cameraFrame.RotateLocalX(-gStepSize);

	if(key == GLUT_KEY_DOWN)
		cameraFrame.RotateLocalX(gStepSize);

	if(key == GLUT_KEY_LEFT)
		cameraFrame.RotateLocalY(gStepSize);

	if(key == GLUT_KEY_RIGHT)
		cameraFrame.RotateLocalY(-gStepSize);
	//  //this is not Pitch
	//	if (key == GLUT_KEY_PAGE_UP)
	//		cameraFrame.RotateLocalX(gStepSize);
	//
	//	if (key == GLUT_KEY_PAGE_DOWN)
	//		cameraFrame.RotateLocalX(-gStepSize);

	if (key == GLUT_KEY_PAGE_UP)
		cameraFrame.MoveUp(gStepSize);

	if (key == GLUT_KEY_PAGE_DOWN)
		cameraFrame.MoveUp(-gStepSize);

}


int main(int argc, char* argv[])
    {

	gltSetWorkingDirectory(argv[0]);
		
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800,600);
  
    glutCreateWindow("OpenGL SphereWorld");
 
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(SpecialKeys);
	glutKeyboardFunc(KeyboardHandler);
    
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
        }
        

    SetupRC();
    glutMainLoop();    
    return 0;
    }
