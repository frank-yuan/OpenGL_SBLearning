#version 120
uniform     samplerCube textureUnit0;
varying     vec3        vOutTexCoords;

void main(void) 
{ 	
	gl_FragColor = textureCube(textureUnit0, vOutTexCoords);
}
