#version 150
precision lowp float;

#define LPV_SUN_SKY_SPEC_CASCADES_COUNT

in vec3 _vertexPosition;
in vec3 _vertexNormal;
in vec3 _vertexNormalTangent;
in vec3 _vertexNormalBitangent;
in vec2 _vertexTexCoord;
in vec4 _vertexColor;

uniform mat4 mw;
uniform mat3 mwnit;

#ifndef CASCADE
uniform mat4 mvp;
#else
uniform mat4 mvp[LPV_SUN_SKY_SPEC_CASCADES_COUNT];

uniform vec4 tiles;
uniform int tileInstances[LPV_SUN_SKY_SPEC_CASCADES_COUNT];
#endif

out vec3 positionWorld;
out vec3 normal;
out vec2 texCoord;
out vec4 color;
out mat3 mtbnti;

mat3 inv(mat3 m)
{
  float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2];
  float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2];
  float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2];

  float b01 = a22 * a11 - a12 * a21;
  float b11 = -a22 * a10 + a12 * a20;
  float b21 = a21 * a10 - a11 * a20;

  float det = a00 * b01 + a01 * b11 + a02 * b21;

  return mat3(b01, (-a22 * a01 + a02 * a21), (a12 * a01 - a02 * a11),
              b11, (a22 * a00 - a02 * a20), (-a12 * a00 + a02 * a10),
              b21, (-a21 * a00 + a01 * a20), (a11 * a00 - a01 * a10)) / det;
}

void main()
{
  positionWorld = vec4(mw * vec4(_vertexPosition, 1.0)).xyz;
  normal = mwnit * _vertexNormal;
  texCoord = _vertexTexCoord;
  color = _vertexColor;
  mtbnti = inv(transpose(mat3(normalize(mwnit * _vertexNormalTangent), normalize(mwnit * _vertexNormalBitangent), normalize(mwnit * _vertexNormal))));

#ifndef CASCADE
  gl_Position = mvp * vec4(_vertexPosition, 1.0);
#else
  int ti = tileInstances[gl_InstanceID];
  vec2 tileMin = vec2(float(ti % int(tiles.x)), float(ti / int(tiles.x))) * tiles.zw * 2.0 - 1.0;
  vec2 tileMax = tileMin + tiles.zw * 2.0;
  vec4 p = mvp[ti] * vec4(_vertexPosition, 1.0);

  gl_ClipDistance[0] = dot(vec4(1.0, 0.0, 0.0, -tileMin.x), p);
  gl_ClipDistance[1] = dot(vec4(-1.0, 0.0, 0.0, tileMax.x), p);
  gl_ClipDistance[2] = dot(vec4(0.0, -1.0, 0.0, -tileMin.y), p);
  gl_ClipDistance[3] = dot(vec4(0.0, 1.0, 0.0, tileMax.y), p);

  gl_Position = p;
#endif
}
