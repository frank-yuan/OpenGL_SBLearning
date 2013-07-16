#version 120
uniform     sampler2D textureUnit0;
varying     vec2        vOutTexCoords;

void main(void) 
{ 	
	gl_FragColor = texture2D(textureUnit0, vOutTexCoords);
}
