#version 120
uniform   vec4 vDiffuseColor;
uniform   vec4 vAmbientColor;
uniform   vec4 vSpecularColor;
uniform	  float fShiness;
uniform   sampler2D textureUnit0;
uniform	  vec4 vLightPos; 
varying		vec3 vOutNormal;
varying     vec3 vOutPos;
varying		vec2 vOutTexCoords;

void main(void) 
{ 
   
	// ambient light
	vec4 vOutColor = vAmbientColor;
    vec3 vLightDir = normalize(vLightPos.xyz - vOutPos);
 
	// diffuse light
	float intens = max(0.0, dot(vOutNormal, vLightDir));
	vOutColor.rgb += vDiffuseColor.rgb * intens;
	vOutColor *= texture2D(textureUnit0, vOutTexCoords);

    // TODO: why use -vOutPos??????
    intens = max(0.0, dot(normalize(-vOutPos), normalize(reflect(-vLightDir, vOutNormal))));
	float shine = pow(intens, fShiness);
	vOutColor.rgb += vSpecularColor.rgb * shine;
	gl_FragColor = vOutColor;
}
