#version 130
precision lowp float;

in vec3 positionWorld;
in vec3 normal;
in vec2 texCoord;
in vec4 color;

uniform mat3 mwnit;
uniform vec3 cam;

uniform sampler2D difTex;
uniform sampler2D alpTex;
uniform sampler2D speTex;

uniform int type;
uniform float opacity;

uniform vec3 lightAmb;
uniform vec3 lightPos;
uniform vec2 lightRange;
uniform vec3 lightColor;
uniform vec4 lightSpeColor;
uniform vec2 fogRange;
uniform vec3 fogColor;

out vec4 glFragColor;

void main()
{
  vec4 fragDif = texture(difTex, texCoord);

  if(((type & 0x20000000) != 0) && (fragDif.a < 0.8))
    discard;

  vec3 fragAlp = texture(alpTex, texCoord).rgb;
  vec3 fragSpe = texture(speTex, texCoord).rgb;

  float fragDist = distance(cam, positionWorld);
  vec3 viewDir = normalize(cam - positionWorld);
  vec3 normalDir = normalize(mwnit * normalize(normal));
  vec3 lightDir = lightPos - positionWorld;

  float lightDist = clamp((length(lightDir) - lightRange.x) / (lightRange.y - lightRange.x) * -1.0 + 1.0, 0.0, 1.0);
  lightDir = normalize(lightDir);
  float lightDot = max(0.0, dot(normalDir, lightDir));

  vec3 colorDif = lightColor * lightDot * lightDist + lightAmb;
  vec3 colorSpe = lightSpeColor.rgb * pow(max(0.0, dot(viewDir, reflect(-lightDir, normalDir))), lightSpeColor.a) * lightDot;

  float fogDist = clamp((fragDist - fogRange.x) / (fogRange.y - fogRange.x), 0.0, 1.0);
  /*float fogDot = pow(max(0.0, dot(normalize(cam - lightPos), viewDir)), 16.0);
  float fresPow = clamp(pow(1.0 - dot(viewDir, normalDir) * 0.5, 8.0), 0.0, 1.0) * 1.0;*/

  float alpha = (fragAlp.r + fragAlp.g + fragAlp.b) * 0.3333333334 * color.a * opacity + (colorSpe.r + colorSpe.g + colorSpe.b) * 0.3333333334;
  glFragColor = vec4(mix(fragDif.rgb * color.rgb * colorDif + fragSpe * colorSpe/* + fresPow * fogColor*/, fogColor/* + fogDot * lightColor*/, fogDist), alpha);
}
