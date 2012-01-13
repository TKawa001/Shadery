#version 330

out vec4 vFragColor;
smooth in vec2 varyingTexCoord;

uniform vec4 ambientColor;
uniform vec4 diffuseColor;
uniform vec4 specularColor;
uniform float alpha;

smooth in vec3 varyingNormal;
smooth in vec3 varyingLightDirection;
smooth in vec3 varyingPosition;

uniform sampler2D texture0;


void main() {
	float diff=max(0.0, dot(normalize(varyingNormal),normalize(varyingLightDirection)));
	vFragColor =  diff*diffuseColor;
	vFragColor += ambientColor;
	vec3 reflection=normalize(reflect(-normalize(varyingLightDirection),normalize(varyingNormal)));
	float spec= max(0.0,dot(-normalize(varyingPosition),reflection));
	if(diff!=0.0) {
		float fSpec=pow(spec,100.0);
		vFragColor.rgb+=vec3(specularColor)*vec3(fSpec,fSpec,fSpec);
	}
	vFragColor *= vec4(vec3(texture(texture0, varyingTexCoord)),alpha);
}