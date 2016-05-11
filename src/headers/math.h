﻿//------------------------------------------------------------------------------
#ifndef MATH_H
#define MATH_H

#define GLM_FORCE_RADIANS

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>
#include <cmath>
#include "types.h"

//------------------------------------------------------------------------------
namespace NMath
{
  static const float PI = 3.1415926535897932384626433832795f;
  static const float PI_2 = 6.283185307179586476925286766559f;
  static const float DIV_PI = 1.5707963267948966192313216916398f;
  static const float DIV_2_PI = 0.63661977236758134307553505349006f;
  static const float PI_DEG = 180.0f;
  static const float PI_2_DEG = 360.0f;
  static const float DIV_PI_DEG = 90.0f;
  static const float DIV_2_PI_DEG = 45.0f;
  static const float RAD_2_DEG = PI_DEG / PI;
  static const float DEG_2_RAD = PI / PI_DEG;
  static const uint8 VEC2 = 2;
  static const uint8 VEC3 = 3;
  static const uint8 VEC4 = 4;
  static const uint8 MAT3 = 9;
  static const uint8 MAT4 = 16;
}
//------------------------------------------------------------------------------
/*struct SQuaternion
{
  static inline glm::quat angle2quat(const glm::vec3 &v)
  {
    return glm::quat::fromAxisAndAngle(glm::vec3(1.0, 0.0, 0.0), v.x()) * glm::quat::fromAxisAndAngle(glm::vec3(0.0, 1.0, 0.0), v.y()) * glm::quat::fromAxisAndAngle(glm::vec3(0.0, 0.0, 1.0), v.z());
  }
  //------------------------------------------------------------------------------
  static inline glm::vec3 quat2angle(const glm::quat &q)
  {
    float x = q.x();
    float y = q.y();
    float z = q.z();
    float w = q.scalar();
    glm::vec3 vec(0.0, 0.0, 0.0);

    float qx2 = x * x;
    float qy2 = y * y;
    float qz2 = z * z;
    float test = x * y + z * w;

    if(test > 0.49999f)
    {
      vec.setX(0.0);
      vec.setY(NMath::RAD_2_DEG * 2.0f * atan2f(x, w));
      vec.setZ(90.0);
    }
    else if(test < -0.49999f)
    {
      vec.setX(0.0);
      vec.setY(-NMath::RAD_2_DEG * 2.0f * atan2f(x, w));
      vec.setZ(-90.0);
    }
    else
    {
      float h = atan2f(2.0f * y * w - 2.0f * x * z, 1.0f - 2.0f * qy2 - 2.0f * qz2);
      float a = asinf(2.0f * x * y + 2.0f * z * w);
      float b = atan2f(2.0f * x * w - 2.0f * y * z, 1.0f - 2.0f * qx2 - 2.0f * qz2);
      vec.setX(b * NMath::RAD_2_DEG);
      vec.setY(h * NMath::RAD_2_DEG);
      vec.setZ(a * NMath::RAD_2_DEG);
    }

    return vec;
  }
};*/
//------------------------------------------------------------------------------
struct SMatrix
{
  /*static inline std::vector<float> toFloat(const glm::mat4 &mat)
  {
    std::vector<float> out(NMath::MAT4);

#if QT_VERSION >= 0x050000 || defined(Q_OS_SYMBIAN)
    memcpy(&out[0], mat.constData(), sizeof(float) * NMath::MAT4);
#else
    const double *inDouble = mat.constData();
    for(uint8 i = 0; i < NMath::MAT4; i++)
      out[i] = static_cast<float>(inDouble[i]);
#endif

    return out;
  }
  //------------------------------------------------------------------------------
  static inline std::vector<float> toFloat(const glm::mat3 &mat)
  {
    std::vector<float> out(NMath::MAT3);

#if QT_VERSION >= 0x050000 || defined(Q_OS_SYMBIAN)
    memcpy(&out[0], mat.constData(), sizeof(float) * NMath::MAT3);
#else
    const double *inDouble = mat.constData();
    for(uint8 i = 0; i < NMath::MAT3; i++)
      out[i] = static_cast<float>(inDouble[i]);
#endif

    return out;
  }*/
  //------------------------------------------------------------------------------
  static inline glm::mat4 composeTransformation(const glm::vec3 &position, const glm::quat &rotation, const glm::vec3 &scale)
  {
    return glm::mat4(glm::scale(glm::translate(glm::mat4(1.0), position) * glm::mat4_cast(rotation), scale));
  }
};
//------------------------------------------------------------------------------
struct SSpline
{
  //------------------------------------------------------------------------------
  static inline glm::vec3 catmullRomSpline(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, float f)
  {
    const float f2 = f * f;
    const float f3 = f2 * f;

    return glm::vec3(
      0.5f * ((2.0f * p1.x) + (-p0.x + p2.x) * f + (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * f2 + (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * f3),
      0.5f * ((2.0f * p1.y) + (-p0.y + p2.y) * f + (2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * f2 + (-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * f3),
      0.5f * ((2.0f * p1.z) + (-p0.z + p2.z) * f + (2.0f * p0.z - 5.0f * p1.z + 4.0f * p2.z - p3.z) * f2 + (-p0.z + 3.0f * p1.z - 3.0f * p2.z + p3.z) * f3));
  }
  //------------------------------------------------------------------------------
};
//------------------------------------------------------------------------------
#endif //MATH_H
