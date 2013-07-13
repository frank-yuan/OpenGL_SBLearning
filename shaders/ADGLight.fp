#version 120
uniform   vec4 vDiffuseColor;
uniform   vec4 vAmbientColor;
uniform   vec4 vSpecularColor;
uniform	  float fShiness;
varying		vec3 vOutNormal;
varying     vec3 vLightDir;

void main(void) 
{ 
   
	// ambient light
	vec4 vOutColor = vAmbientColor;
 
	// diffuse light
	float intens = max(0.0, dot(vOutNormal, vLightDir));
	vOutColor.rgb += vDiffuseColor.rgb * intens;

    intens = max(0.0, dot(vOutNormal, reflect(-vLightDir, vOutNormal)));
	float shine = pow(intens, fShiness);
	vOutColor.rgb += vSpecularColor.rgb * shine;
	gl_FragColor = vOutColor;
}
