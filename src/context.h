﻿//------------------------------------------------------------------------------
#ifndef CONTEXT_H
#define CONTEXT_H

#include <iostream>
#include <vector>
#include <list>
#include <map>

#include "matrix.h"
#include "headers/engineTypes.h"

//------------------------------------------------------------------------------
class CEngineBase;
class CEngine;
class CWindow;
class CScenes;
class CModels;
class CRenderer;
class CShaders;
class CCulling;
class CPickColor;
class CFramebuffers;
class CMaps;
class CCamera;
class COpenGL;
class CFilesystem;
class CExceptions;

//------------------------------------------------------------------------------
class CContext
{
  private:
    CEngine *engine;
    CWindow *window;
    CScenes *scenes;
    CModels *models;
    CRenderer *renderer;
    CShaders *shaders;
    CCulling *culling;
    CPickColor *pickColor;
    CFramebuffers *framebuffers;
    CMaps *maps;
    CCamera *camera;
    COpenGL *openGL;
    CFilesystem *filesystem;
    CExceptions *exceptions;

    // callbacks engine static
    void (*fncEngineShowMessage)(const CContext *context, const std::string &title, const std::string &text, bool modal);
    void (*fncEngineIncDrawCalls)(CContext *context);
    void (*fncEngineClearDrawCalls)(CContext *context);
    std::string (*fncEngineGetClassName)(CContext *context, const CEngineBase *object);
    const SEngine *(*fncEngineGetEngine)(const CContext *context);

  public:
    inline CContext() : engine(NULL), scenes(NULL), models(NULL), renderer(NULL), shaders(NULL), culling(NULL), pickColor(NULL), framebuffers(NULL), maps(NULL), camera(NULL), openGL(NULL), filesystem(NULL), exceptions(NULL), fncEngineGetClassName(NULL) {}
    inline CContext(CEngine *engine, CWindow *window, CScenes *scenes, CModels *models, CRenderer *renderer, CShaders *shaders, CCulling *culling, CPickColor *pickColor, CFramebuffers *framebuffers, CMaps *maps, CCamera *camera, COpenGL *openGL, CFilesystem *filesystem, CExceptions *exceptions) : engine(engine), window(window), scenes(scenes), models(models), renderer(renderer), shaders(shaders), culling(culling), pickColor(pickColor), framebuffers(framebuffers), maps(maps), camera(camera), openGL(openGL), filesystem(filesystem), exceptions(exceptions), fncEngineGetClassName(NULL) {}
    inline ~CContext() {}

    void setContext(CEngine *engine, CWindow *window, CScenes *scenes, CModels *models, CRenderer *renderer, CShaders *shaders, CCulling *culling, CPickColor *pickColor, CFramebuffers *framebuffers, CMaps *maps, CCamera *camera, COpenGL *openGL, CFilesystem *filesystem, CExceptions *exceptions);
    void setEngineCallbacks(void (*fncEngineShowMessage)(const CContext *context, const std::string &title, const std::string &text, bool modal), void (*fncEngineIncDrawCalls)(CContext *context), void (*fncEngineClearDrawCalls)(CContext *context), std::string (*fncEngineGetClassName)(CContext *context, const CEngineBase *object), const SEngine *(*fncEngineGetEngine)(const CContext *context));

    // callbacks engine
    inline void engineShowMessage(const std::string &title, const std::string &text, bool modal = true) const { fncEngineShowMessage(this, title, text, modal); }
    inline void engineIncDrawCalls() { fncEngineIncDrawCalls(this); }
    inline void engineClearDrawCalls() { fncEngineClearDrawCalls(this); }
    inline std::string engineGetClassName(const CEngineBase *object) { return fncEngineGetClassName(this, object); }
    inline const SEngine *engineGetEngine() const { return fncEngineGetEngine(this); }

    // console
    inline void log(const std::string &msg) const { std::cout << msg << "\n"; }
    inline void warning(const std::string &msg) const { std::cout << "WARNING: " << msg << "!\n"; }
    inline void error(const std::string &msg) const { std::cout << "ERROR: " << msg << "!\n"; }

    // gets
    inline CEngine *getEngine() { return engine; }
    inline CScenes *getScenes() { return scenes; }
    inline CModels *getModels() { return models; }
    inline CRenderer *getRenderer() { return renderer; }
    inline CShaders *getShaders() { return shaders; }
    inline CCulling *getCulling() { return culling; }
    inline CPickColor *getPickColor() { return pickColor; }
    inline CFramebuffers *getFramebuffers() { return framebuffers; }
    inline CMaps *getMaps() { return maps; }
    inline CCamera *getCamera() { return camera; }
    inline COpenGL *getOpenGL() { return openGL; }
    inline CFilesystem *getFilesystem() { return filesystem; }
    inline CExceptions *getExceptions() { return exceptions; }

    inline const CEngine *getEngine() const { return engine; }
    inline const CScenes *getScenes() const { return scenes; }
    inline const CModels *getModels() const { return models; }
    inline const CRenderer *getRenderer() const { return renderer; }
    inline const CShaders *getShaders() const { return shaders; }
    inline const CCulling *getCulling() const { return culling; }
    inline const CPickColor *getPickColor() const { return pickColor; }
    inline const CFramebuffers *getFramebuffers() const { return framebuffers; }
    inline const CMaps *getMaps() const { return maps; }
    inline const CCamera *getCamera() const { return camera; }
    inline const COpenGL *getOpenGL() const { return openGL; }
    inline const CFilesystem *getFilesystem() const { return filesystem; }
    inline const CExceptions *getExceptions() const { return exceptions; }
};
//------------------------------------------------------------------------------
class CEngineBase
{
  protected:
    CContext *context;

  public:
    inline CEngineBase() : context(NULL) {}
    inline CEngineBase(CContext *context) : context(context) {}
    inline ~CEngineBase() {}

    inline CContext *getContext() { return context; }
};
//------------------------------------------------------------------------------
inline void CContext::setContext(CEngine *engine, CWindow *window, CScenes *scenes, CModels *models, CRenderer *renderer, CShaders *shaders, CCulling *culling, CPickColor *pickColor, CFramebuffers *framebuffers, CMaps *maps, CCamera *camera, COpenGL *openGL, CFilesystem *filesystem, CExceptions *exceptions)
{
  if(engine)       this->engine = engine;
  if(window)       this->window = window;
  if(scenes)       this->scenes = scenes;
  if(models)       this->models = models;
  if(renderer)     this->renderer = renderer;
  if(shaders)      this->shaders = shaders;
  if(culling)      this->culling = culling;
  if(pickColor)    this->pickColor = pickColor;
  if(framebuffers) this->framebuffers = framebuffers;
  if(maps)         this->maps = maps;
  if(camera)       this->camera = camera;
  if(openGL)       this->openGL = openGL;
  if(filesystem)   this->filesystem = filesystem;
  if(exceptions)   this->exceptions = exceptions;
}
//------------------------------------------------------------------------------
inline void CContext::setEngineCallbacks(void (*fncEngineShowMessage)(const CContext *context, const std::string &title, const std::string &text, bool modal), void (*fncEngineIncDrawCalls)(CContext *context), void (*fncEngineClearDrawCalls)(CContext *context), std::string (*fncEngineGetClassName)(CContext *context, const CEngineBase *object), const SEngine *(*fncEngineGetEngine)(const CContext *context))
{
  if(fncEngineShowMessage)  this->fncEngineShowMessage = fncEngineShowMessage;
  if(fncEngineIncDrawCalls) this->fncEngineIncDrawCalls = fncEngineIncDrawCalls;
  if(fncEngineClearDrawCalls) this->fncEngineClearDrawCalls = fncEngineClearDrawCalls;
  if(fncEngineGetClassName) this->fncEngineGetClassName = fncEngineGetClassName;
  if(fncEngineGetEngine)    this->fncEngineGetEngine = fncEngineGetEngine;
}
//------------------------------------------------------------------------------
#endif // CONTEXT_H
