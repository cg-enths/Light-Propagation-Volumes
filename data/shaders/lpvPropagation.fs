#version 150
precision lowp float;

#ifdef LPV_GATHERING
in float texPos;
#else
#endif

uniform sampler3D lpvTexR;
uniform sampler3D lpvTexG;
uniform sampler3D lpvTexB;
uniform sampler3D lpvAccumTexR;
uniform sampler3D lpvAccumTexG;
uniform sampler3D lpvAccumTexB;
uniform sampler3D gvTex;

out vec4 glFragColor0LpvR;
out vec4 glFragColor1LpvG;
out vec4 glFragColor2LpvB;
out vec4 glFragColor3LpvAccumR;
out vec4 glFragColor4LpvAccumG;
out vec4 glFragColor5LpvAccumB;
out vec4 glFragColor6Gv;

void main()
{
#ifdef LPV_GATHERING
  float a = texPos;

  glFragColor0LpvR = vec4(1.0);
  glFragColor1LpvG = vec4(1.0);
  glFragColor2LpvB = vec4(1.0);
  glFragColor3LpvAccumR = vec4(1.0);
  glFragColor4LpvAccumG = vec4(1.0);
  glFragColor5LpvAccumB = vec4(1.0);
  glFragColor6Gv = vec4(0.0);
#else
  glFragColor0LpvR = vec4(0.0);
  glFragColor1LpvG = vec4(0.0);
  glFragColor2LpvB = vec4(0.0);
  glFragColor3LpvAccumR = vec4(0.0);
  glFragColor4LpvAccumG = vec4(0.0);
  glFragColor5LpvAccumB = vec4(0.0);
  glFragColor6Gv = vec4(0.0);
#endif
}
