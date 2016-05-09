﻿//------------------------------------------------------------------------------
#ifndef SHADERS_H
#define SHADERS_H

#if defined(ENV_QT)
#include <QMessageBox>
#endif

#include "culling.h"
#include "headers/rendererTypes.h"
#include "headers/sceneTypes.h"

//------------------------------------------------------------------------------
class CShader : public CEngineBase
{
  private:
    SShader shader;

  public:
    CShader();
    CShader(CContext *context, const SShader &shader);
    ~CShader();

    bool compile();

    std::string setES2compatible(const std::string &data);
    std::string setDefines(const std::string &data);

    inline const SShader *getShader() const { return &shader; }
};
//------------------------------------------------------------------------------
class CShaderProgram : public CEngineBase
{
  private:
    SShaderProgram program;

    void setSampler(const CMap *texture, GLuint uniform, uint8 sampler, uint32 format = NMap::FORMAT_MIPMAP) const;
    void beginLPV(NShader::ESamplerTex startDepthSampler) const;

  public:
    CShaderProgram();
    CShaderProgram(CContext *context, const SShaderProgram &shaderProgram);
    ~CShaderProgram();

    bool link();
    void initUniforms();

    void bind() const;
    void begin(const SShaderState *technique, NRenderer::EMode mode = NRenderer::MODE_STANDARD) const; // mode for backdrop, etc...
    void end(const SShaderState *technique) const;
    void unbind() const;
    void dispatch(uint32 x, uint32 y, uint32 z, NRenderer::EMode mode, GLbitfield preSync = GL_NONE, GLbitfield postSync = GL_NONE) const;

    inline const SShaderProgram *getProgram() const { return &program; }
};
//------------------------------------------------------------------------------
class CShaders : public CEngineBase
{
  private:
    std::map<std::string, CShader> shaders;
    std::map<NShader::EProgram, CShaderProgram> programs;

  public:
    CShaders();
    CShaders(CContext *context);
    ~CShaders();

    CShader *addShader(const SShader &shader);
    CShaderProgram *addShaderProgram(const SShaderProgram &shaderProgram);

    inline CShader *getShader(const std::string &file) { auto it = shaders.find(file); if(it == shaders.end()) return NULL; return &it->second; }
    inline CShaderProgram *getProgram(NShader::EProgram name) { auto it = programs.find(name); if(it == programs.end()) return NULL; return &it->second; }
};
//------------------------------------------------------------------------------
inline CShader *CShaders::addShader(const SShader &shader)
{
  CShader *vs = getShader(shader.name);

  if(vs)
    return vs;

  shaders[shader.name] = CShader(context, shader);
  vs = getShader(shader.name);
  vs->compile();

  return vs;
}
//------------------------------------------------------------------------------
inline CShaderProgram *CShaders::addShaderProgram(const SShaderProgram &shaderProgram)
{
  CShaderProgram *sp = getProgram(shaderProgram.name);

  if(sp)
    return sp;

  programs[shaderProgram.name] = CShaderProgram(context, shaderProgram);
  sp = getProgram(shaderProgram.name);
  if(sp->link())
    sp->initUniforms();

  return sp;
}
//------------------------------------------------------------------------------
inline void CShaderProgram::setSampler(const CMap *texture, GLuint uniform, uint8 sampler, uint32 format) const
{
  if(texture)
    texture->bind(uniform, sampler, format);
  else
    context->getMaps()->unbind(uniform, sampler, format);
}
//------------------------------------------------------------------------------
inline void CShaderProgram::beginLPV(NShader::ESamplerTex startDepthSampler) const
{
  const SShaderUniforms *u = &program.uniforms;
  const CMap *const lpvMaps[] = {
    u->lpvAccumGs0MapR, u->lpvAccumGs0MapG, u->lpvAccumGs0MapB, u->sslpvAccumGs0MapR, u->sslpvAccumGs0MapG, u->sslpvAccumGs0MapB,
    u->lpvAccumGs1MapR, u->lpvAccumGs1MapG, u->lpvAccumGs1MapB, u->sslpvAccumGs1MapR, u->sslpvAccumGs1MapG, u->sslpvAccumGs1MapB };

  const uint32 i = (!context->engineGetEngine()->lpvPropagationSwap) ? 0 : 6;
  uint32 filter = NMap::FORMAT_LINEAR;

  setSampler(u->shadowMap, u->shadTex, startDepthSampler, filter | NMap::FORMAT_DEPTH | NMap::FORMAT_BORDER);

  //setSampler(u->gvGsMap, u->lpvAccumTexR, startDepthSampler + 1, filter | NMap::FORMAT_BORDER);
  setSampler(lpvMaps[i + 0], u->lpvAccumTexR, startDepthSampler + 1, filter | NMap::FORMAT_BORDER);
  setSampler(lpvMaps[i + 1], u->lpvAccumTexG, startDepthSampler + 2, filter | NMap::FORMAT_BORDER);
  setSampler(lpvMaps[i + 2], u->lpvAccumTexB, startDepthSampler + 3, filter | NMap::FORMAT_BORDER);
  setSampler(lpvMaps[i + 3], u->sslpvAccumTexR, startDepthSampler + 4, filter | NMap::FORMAT_BORDER);
  setSampler(lpvMaps[i + 4], u->sslpvAccumTexG, startDepthSampler + 5, filter | NMap::FORMAT_BORDER);
  setSampler(lpvMaps[i + 5], u->sslpvAccumTexB, startDepthSampler + 6, filter | NMap::FORMAT_BORDER);

  if(program.fUniforms & NShader::UNIFORM_SHAD_TEX_SIZE)
    setSampler(u->shadowDepthMap, u->shadDepthTex, startDepthSampler + 7, NMap::FORMAT_BORDER);
}
//------------------------------------------------------------------------------
#endif // SHADERS_H
