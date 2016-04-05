#version 150
precision lowp float;

#define LPV_CASCADES_COUNT
#define LPV_SUN_SKY_DIRS_COUNT

in vec2 texPos;

uniform sampler2D geomColorTex;
uniform sampler2D geomPosTex;
uniform sampler2D geomNormalTex;
//uniform sampler2D geomDepthTex;

uniform vec3 lightAmb[LPV_SUN_SKY_DIRS_COUNT];
uniform vec3 lightPos[LPV_SUN_SKY_DIRS_COUNT];

uniform vec4 tiles;
uniform vec4 geomTexSize;
//uniform vec3 lpvTexSize;
//uniform vec3 lpvCellSize[LPV_CASCADES_COUNT];

out vec4 glFragColor0LpvR;
out vec4 glFragColor1LpvG;
out vec4 glFragColor2LpvB;
out vec4 glFragColor3Gv;
out vec4 glFragColor4SkyR;
out vec4 glFragColor5SkyG;
out vec4 glFragColor6SkyB;

void main()
{
  int x = int(texPos.x);
  int y = int(texPos.y);
  int sunSkyLight = x / int(geomTexSize.x * tiles.z);
  vec2 tex = vec2(float(x), float(y)) * geomTexSize.zw;

  vec3 pos = texture(geomPosTex, tex).xyz;
  vec3 light = texture(geomColorTex, tex).rgb * lightAmb[sunSkyLight];
  vec3 normal = normalize(texture(geomNormalTex, tex).xyz);
  light *= max(0.0, dot(normal, normalize(lightPos[sunSkyLight] - pos)));
  //light = vec3(max(0.0, dot(normal, normalize(lightPos[sunSkyLight] - pos))));

  vec4 cosineLobe = vec4(0.8862, -1.0233, 1.0233, -1.0233);
  vec4 shNormal = cosineLobe * vec4(1.0, normal.y, normal.z, normal.x);

  glFragColor0LpvR = shNormal * light.r;
  glFragColor1LpvG = shNormal * light.g;
  glFragColor2LpvB = shNormal * light.b;
  glFragColor3Gv = vec4(0.0, 0.0, 0.0, 1.0);
  glFragColor4SkyR = vec4(0.0, 0.0, 0.0, 0.0);
  glFragColor5SkyG = vec4(0.0, 0.0, 0.0, 0.0);
  glFragColor6SkyB = vec4(0.0, 0.0, 0.0, 0.0);

  if(sunSkyLight > 0)
  {
    glFragColor4SkyR = shNormal * light.r;
    glFragColor5SkyG = shNormal * light.g;
    glFragColor6SkyB = shNormal * light.b;
  }
}
