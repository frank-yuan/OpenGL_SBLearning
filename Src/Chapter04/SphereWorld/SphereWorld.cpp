// SphereWorld.cpp
// OpenGL SuperBible
// New and improved (performance) sphere world
// Program by Richard S. Wright Jr.
#include <assert.h>
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
GLTriangleBatch		moonBatch;
GLTriangleBatch     reflectBatch;
GLBatch				skyBatch;

GLFrame				cameraFrame;

//################ custom shader defnination #####################
GLuint				lightShader;
GLuint              textureShader;
GLuint              cubeMapShader;
GLuint              cubeMapReflectShader;
//################################################################

//################ custom shader defnination #####################
#define             TEXTURE2D_COUNT 2
GLuint				textures2D[TEXTURE2D_COUNT];
const char          *texture2DFileName[TEXTURE2D_COUNT] = {	"data/texture/earth_2k.tga", 
														"data/texture/moon.tga",
														};
#define             TEXTURECUBE_COUNT 1
GLuint				texturesCube[TEXTURECUBE_COUNT];
const char          *textureCubeFileName[TEXTURE2D_COUNT] = {	
														"data/texture/galaxy_cube.tga"
														};
//################################################################

const GLuint CUBE_COLUMN = 4;
const GLuint CUBE_ROW = 3;
GLenum cube[CUBE_ROW][CUBE_COLUMN] = { 
	{0, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, 0}, 
	// why this is right hand axis?
	{GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z}, 
	{0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, 0}
};
GLuint TileSize = 256;
void LoadCubeMap()
{
	glGenTextures(TEXTURECUBE_COUNT, texturesCube);
	for (int i = 0; i < TEXTURECUBE_COUNT; ++i)
	{
		GLint iWidth, iHeight, iComponents;
		GLenum eFormat;
		glBindTexture(GL_TEXTURE_CUBE_MAP, texturesCube[i]);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		GLbyte* pBytes = gltReadTGABits(textureCubeFileName[i], &iWidth ,&iHeight, &iComponents, &eFormat);
		if (pBytes)
		{
			assert(iWidth / TileSize == CUBE_COLUMN);
			assert(iHeight / TileSize == CUBE_ROW);

			GLuint pixelBytes = 3;

			GLbyte* pSliced = new GLbyte[TileSize * TileSize * pixelBytes];
			for (int row = 0; row < CUBE_ROW; ++row)
			{
				for (int col = 0; col < CUBE_COLUMN; ++col)
				{
					GLuint target = cube[row][col];
					if (target != 0)
					{
						GLuint StartX = col * TileSize;
						GLuint StartY = row * TileSize;
						// copy data by rows
						for (int j = 0; j < TileSize; ++j)
						{
							memcpy(pSliced + j * TileSize * pixelBytes, pBytes + ((StartY + j) * iWidth + StartX) * pixelBytes , TileSize * pixelBytes);
						}
						glTexImage2D(target, 0, iComponents, TileSize, TileSize, 0, eFormat, GL_UNSIGNED_BYTE, pSliced);
					}
				}
			}
			free(pSliced);
			free(pBytes);
		}
	}
}

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
	gltMakeSphere(moonBatch, 0.15f, 26, 13);
	gltMakeSphere(reflectBatch, 0.8f, 26, 13);
	gltMakeCube(skyBatch, 40.0f);


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
	lightShader = gltLoadShaderPairWithAttributes("shaders/ADGLight.vp", "shaders/ADGLight.fp", 3, 
		GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_NORMAL, "vNormal", GLT_ATTRIBUTE_TEXTURE0, "vTexture");
	textureShader = gltLoadShaderPairWithAttributes("shaders/texture.vp", "shaders/texture.fp", 2,
		GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
	cubeMapShader = gltLoadShaderPairWithAttributes("shaders/CubeMap.vp", "shaders/CubeMap.fp", 1,
		GLT_ATTRIBUTE_VERTEX, "vVertex");
    cubeMapReflectShader = gltLoadShaderPairWithAttributes("shaders/CubeMapReflect.vp", "shaders/CubeMapReflect.fp", 1,
                                                    GLT_ATTRIBUTE_VERTEX, "vVertex");
    
	//################################################################

	//################ texture #####################
	glGenTextures(TEXTURE2D_COUNT, textures2D);
	for (int i = 0; i < TEXTURE2D_COUNT; ++i)
	{
		GLint iWidth, iHeight, iComponents;
		GLenum eFormat;
		glBindTexture(GL_TEXTURE_2D, textures2D[i]);
		// The process of calculating color fragments 
		// from a stretched or shrunken texture map is
		// called texture filtering.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		// If texture coordinates fall outside this 
		// range, OpenGL handles them according
		// to the current texture wrapping mode.
		// GL_REPEAT, GL_CLAMP, GL_CLAMP_TO_EDGE, or GL_CLAMP_TO_BORDER
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		GLbyte* pBytes = gltReadTGABits(texture2DFileName[i], &iWidth ,&iHeight, &iComponents, &eFormat);
		if (pBytes)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
			free(pBytes);
		}
	}
	//###############################################
	LoadCubeMap();
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
    static GLfloat vAmbientColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    static GLfloat vSpecularColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    static GLfloat vSphereColor[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    static GLfloat vLightPos[] = { 10.0f, 00.0f, 10.0f, 1.0f};
    
    // Time Based animation
	static CStopWatch	rotTimer;
	float yRot = rotTimer.GetElapsedSeconds() * 50.0f;
	
	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
    //vLightPos = modelViewMatrix
    // Save the current modelview matrix (the identity matrix)
    modelViewMatrix.LoadIdentity();
	
	//cameraFrame.RotateLocalY(yRot / 30.0f);
	//cameraFrame.MoveForward(-3.0f);
	M3DMatrix44f cameraMatrix;
	cameraFrame.GetCameraMatrix(cameraMatrix);
	modelViewMatrix.LoadMatrix(cameraMatrix);
    if (cubeMapShader)
    {
        glUseProgram(cubeMapShader);
        GLint iMvpMatrix = glGetUniformLocation(cubeMapShader, "mvpMatrix");
        GLint iTextureUnit = glGetUniformLocation(cubeMapShader, "textureUnit0");
        glUniformMatrix4fv(iMvpMatrix, 1, false, transformPipeline.GetModelViewProjectionMatrix());
        glUniform1i(iTextureUnit, 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texturesCube[0]);
        skyBatch.Draw();
    }
    modelViewMatrix.PushMatrix();
    {
		// Draw the ground

        //if (textureShader)
        //{
		//	glUseProgram(textureShader);
        //    GLint iMvpMatrix, iTextureUnit0;
        //    iMvpMatrix = glGetUniformLocation(textureShader, "mvpMatrix");
        //    iTextureUnit0 = glGetUniformLocation(textureShader, "textureUnit0");
        //    glUniformMatrix4fv(iMvpMatrix, 1, false, transformPipeline.GetModelViewProjectionMatrix());
		//	glUniform1i(iTextureUnit0, 0);
        //    glBindTexture(GL_TEXTURE_2D, textures2D[1]);
        //    floorBatch.Draw();
        //}
        
        
    	// Draw the spinning Torus
    	modelViewMatrix.Translate(0.0f, 0.0f, -12.5f);
    	modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
    	modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);
        
		//################ custom shader rendering #####################
        
		if (lightShader)
		{
			modelViewMatrix.PushMatrix();
			{
				//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				//modelViewMatrix.Translate(0.4f, 0, 0);
				//modelViewMatrix.Rotate(yRot / 2, 0.0f, 1.0f, 0.0f);
				//modelViewMatrix.Translate(0.3f, 0, 0);
				glUseProgram(lightShader);
                
				GLint iDiffuseColor = glGetUniformLocation(lightShader, "vDiffuseColor");
				GLint iAmbientColor = glGetUniformLocation(lightShader, "vAmbientColor");
				GLint iMvpMatrix = glGetUniformLocation(lightShader, "mvpMatrix");
				GLint iMvMatrix = glGetUniformLocation(lightShader, "mvMatrix");
				GLint iNormalMatrix = glGetUniformLocation(lightShader, "normalMatrix");
				GLint iLightPos = glGetUniformLocation(lightShader, "vLightPos");
				GLint iSpecularColor = glGetUniformLocation(lightShader, "vSpecularColor");
				GLint iShiness = glGetUniformLocation(lightShader, "fShiness");
				GLint iTextureUnit0 = glGetUniformLocation(textureShader, "textureUnit0");
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
				glUniform1i(iTextureUnit0, 0);
				glBindTexture(GL_TEXTURE_2D, textures2D[0]);
				sphereBatch.Draw();
                
                
				// Draw Moon
				modelViewMatrix.PushMatrix();
				{
					modelViewMatrix.Translate(1.0f, 0, 0);
					modelViewMatrix.Rotate(yRot,0, 0, 1);
					glUniformMatrix4fv(iMvpMatrix, 1, false, transformPipeline.GetModelViewProjectionMatrix());
					glUniformMatrix4fv(iMvMatrix, 1, false, transformPipeline.GetModelViewMatrix());
					glUniformMatrix3fv(iNormalMatrix, 1, false, transformPipeline.GetNormalMatrix());
					glBindTexture(GL_TEXTURE_2D, textures2D[1]);
					moonBatch.Draw();
				}
                modelViewMatrix.PopMatrix();
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
        
    }
    modelViewMatrix.PopMatrix();
    
    if (cubeMapReflectShader)
    {
        modelViewMatrix.PushMatrix();
        {
            modelViewMatrix.Translate(0, 0, -4);
            glUseProgram(cubeMapReflectShader);
            GLint iMvpMatrix = glGetUniformLocation(cubeMapReflectShader, "mvpMatrix");
            GLint iTextureUnit = glGetUniformLocation(cubeMapReflectShader, "textureUnit0");
            glUniformMatrix4fv(iMvpMatrix, 1, false, transformPipeline.GetModelViewProjectionMatrix());
            glUniform1i(iTextureUnit, 0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, texturesCube[0]);
            reflectBatch.Draw();
            
        }
        modelViewMatrix.PopMatrix();
    }
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
