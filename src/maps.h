﻿//------------------------------------------------------------------------------
#ifndef MAPS_H
#define MAPS_H

#if defined(ENV_QT)
#include <QImage>
#elif defined(ENV_SDL)
#include <SDL2/SDL_image.h>
#endif

#include "camera.h"
#include "headers/mapTypes.h"

//------------------------------------------------------------------------------
class CMap : public CEngineBase
{
  private:
    SMap map;

  public:
    CMap();
    CMap(CContext *context, const SMap &map);
    ~CMap();

    void load();
    static void setColorKeyIntoAlpha(uint8 *data, uint32 width, uint32 height); // rgba 32-bit
    static void clampBitmap(uint8 *data, uint32 width, uint32 height, uint32 &newWidth, uint32 &newHeight, uint32 maxSize);

    void bind(GLuint uniform = 0, uint8 sampler = 0, uint8 format = NMap::FORMAT_MIPMAP) const;

    inline const SMap *getMap() const { return &map; }
};
//------------------------------------------------------------------------------
class CMaps : public CEngineBase
{
  private:
    std::map<std::string, CMap> maps;

  public:
    CMaps();
    CMaps(CContext *context);
    ~CMaps();

    void loadDefaultMaps();

    CMap *addMap(const SMap &map);
    uint32 removeMap(const std::string &file);

    inline void finishBind() const { /*context->getOpenGL()->*/glActiveTexture(GL_TEXTURE0); }
    void unbind(GLuint uniform = 0, uint8 sampler = 0) const;

    inline CMap *getMap(const std::string &file) { auto it = maps.find(file); if(it == maps.end()) return NULL; return &it->second; }
};
//------------------------------------------------------------------------------
inline void CMap::bind(GLuint uniform, uint8 sampler, uint8 format) const
{
  //COpenGL *gl = context->getOpenGL();

  GLenum texFormat = (map.format & NMap::FORMAT_3D) ? GL_TEXTURE_3D : ((map.format & NMap::FORMAT_CUBE) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE0 + sampler);
  glBindTexture(texFormat, map.texture);
  glTexParameteri(texFormat, GL_TEXTURE_MAG_FILTER, (format & NMap::FORMAT_LINEAR) ? GL_LINEAR : ((format & NMap::FORMAT_MIPMAP) ? GL_LINEAR : GL_NEAREST));
  glTexParameteri(texFormat, GL_TEXTURE_MIN_FILTER, (format & NMap::FORMAT_LINEAR) ? GL_LINEAR : ((format & NMap::FORMAT_MIPMAP) ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST));
  glTexParameteri(texFormat, GL_TEXTURE_WRAP_S, (format & NMap::FORMAT_EDGE) ? GL_CLAMP_TO_EDGE : GL_REPEAT);
  glTexParameteri(texFormat, GL_TEXTURE_WRAP_T, (format & NMap::FORMAT_EDGE) ? GL_CLAMP_TO_EDGE : GL_REPEAT);
  /*if(format & NMap::FORMAT_DEPTH)
  {
    glTexParameteri(texFormat, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    glTexParameteri(texFormat, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  }*/
  glUniform1i(uniform, sampler);
}
//------------------------------------------------------------------------------
inline CMap *CMaps::addMap(const SMap &map)
{
  CMap *m = getMap(map.file);

  if(m)
    return m;

  maps[map.file] = CMap(context, map);
  m = getMap(map.file);
  m->load();

  return m;
}
//------------------------------------------------------------------------------
inline uint32 CMaps::removeMap(const std::string &file)
{
  //COpenGL *gl = context->getOpenGL();

  if(CMap *m = getMap(file))
  {
    GLuint texture = m->getMap()->texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &texture);
    return maps.erase(file);
  }

  return 0;
}
//------------------------------------------------------------------------------
inline void CMaps::unbind(GLuint uniform, uint8 sampler) const
{
  //COpenGL *gl = context->getOpenGL();

  glActiveTexture(GL_TEXTURE0 + sampler);
  glBindTexture(GL_TEXTURE_2D, 0);
  glUniform1i(uniform, sampler);
}
//------------------------------------------------------------------------------
#endif // MAPS_H
