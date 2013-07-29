#version 130
uniform     samplerCube textureUnit0;
varying     vec3        vOutTexCoords;

void main(void) 
{ 	
	gl_FragColor = texture(textureUnit0, vOutTexCoords);
}
