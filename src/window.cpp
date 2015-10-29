﻿//------------------------------------------------------------------------------
#include "window.h"

//------------------------------------------------------------------------------
CWindow::CWindow(CContext *context
#ifdef ENV_QT
  , QObject *parent
#endif
  ) :
#ifdef ENV_QT
#if QT_VERSION < 0x050400
  QGLWidget
#else
  QOpenGLWidget
#endif
  (qobject_cast<QWidget *>(parent)),
#endif
  CEngineBase(context)
#ifdef ENV_SDL
  , SDLwindow(NULL)
#endif
{
}
//------------------------------------------------------------------------------
CWindow::~CWindow()
{
#ifdef ENV_SDL
  IMG_Quit();
#endif
}
//------------------------------------------------------------------------------
void CWindow::initializeGL()
{
  const SCamera *c = CWindow::context->getCamera()->getCamera();

  if(CWindow::context->engineGetEngine()->consoleVisible)
  {
#if defined(_WIN32)
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
#elif defined(__linux__)
    //
#endif
  }

#if defined(ENV_SDL)
  int32 ret;

  if((ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) < -1)
    CWindow::context->getExceptions()->throwException(SException(this, NWindow::STR_ERROR_INIT_SDL));

  /*SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);*/

  if(!(SDLwindow = SDL_CreateWindow(
    NEngine::STR_APP_NAME, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, c->width, c->height,
    SDL_WINDOW_OPENGL | ((context->engineGetEngine()->fullscreen) ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE)
  )))
    CWindow::context->getExceptions()->throwException(SException(this, NWindow::STR_ERROR_INIT_WINDOW));

  SDLcontext = SDL_GL_CreateContext(SDLwindow);

  uint32 imgInited = IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
  if(!(imgInited & IMG_INIT_JPG))
     CWindow::context->getExceptions()->throwException(SException(this, NWindow::STR_ERROR_INIT_IMG_JPG));
  if(!(imgInited & IMG_INIT_PNG))
    CWindow::context->getExceptions()->throwException(SException(this, NWindow::STR_ERROR_INIT_IMG_PNG));

  /*if(SDL_GL_SetSwapInterval(-1) == -1)
    SDL_GL_SetSwapInterval(1);*/

  if(glewInit() != GLEW_OK)
    CWindow::context->getExceptions()->throwException(SException(this, NWindow::STR_ERROR_INIT_GLEW));
#elif defined(ENV_QT)
  emit onInitializeGL();
#endif

  //COpenGL *gl = CEngineBase::context->getOpenGL();
  CShaders *s = CEngineBase::context->getShaders();
  //gl->makeCurrent();

  context->log(std::string("Vendor: ")+reinterpret_cast<const char *>(glGetString(GL_VENDOR)));
  context->log(std::string("Renderer: ")+reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
  context->log(std::string("Version: ")+reinterpret_cast<const char *>(glGetString(GL_VERSION)));
  context->log(std::string("GLSL Version: ")+reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

  glEnable(GL_DEPTH_TEST);

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CW);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_BLEND);

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  context->getMaps()->loadDefaultMaps();

  std::vector<uint8> fboAttachments;
  const uint32 maxDepthTextureSize = CEngineBase::context->engineGetEngine()->maxDepthTextureSize;
  fboAttachments.push_back(NMap::FORMAT_2D | NMap::FORMAT_DEPTH | NMap::FORMAT_EDGE);
  CEngineBase::context->getFramebuffers()->addFbo(SFramebuffer(NWindow::STR_ORTHO_DEPTH_FBO, fboAttachments, NMap::RBO, maxDepthTextureSize, maxDepthTextureSize));

  for(uint32 i = 0; i < NShader::VERTEX_SHADERS_COUNT; i++)
    s->addShader(SShader(NShader::TYPE_VERTEX, NShader::STR_VERTEX_SHADER_LIST[i]));
  for(uint32 i = 0; i < NShader::FRAGMENT_SHADERS_COUNT; i++)
    s->addShader(SShader(NShader::TYPE_FRAGMENT, NShader::STR_FRAGMENT_SHADER_LIST[i]));

  for(uint32 i = 0; i < NShader::PROGRAMS_COUNT; i++)
    s->addShaderProgram(SShaderProgram(
      static_cast<NShader::EProgram>(i),
      s->getVertexShader(NShader::STR_PROGRAM_VERTEX_SHADER_LIST[i]),
      s->getFragmentShader(NShader::STR_PROGRAM_FRAGMENT_SHADER_LIST[i])));

#ifdef ENV_QT
  emit onInitializeFinishGL();
#endif
}
//------------------------------------------------------------------------------
void CWindow::paintGL()
{
  //COpenGL *gl = CEngineBase::context->getOpenGL();
  CRenderer *ren = CEngineBase::context->getRenderer();
  CFramebuffers *fbo = CEngineBase::context->getFramebuffers();
  CCamera *cam = CEngineBase::context->getCamera();
  const SCamera *c = cam->getCamera();
  //gl->makeCurrent();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  context->engineClearDrawCalls();

  if(CScene *s = context->getScenes()->getActiveScene())
  {
    const glm::vec3 pos = glm::vec3(c->position);
    const glm::vec3 rot = glm::vec3(c->rotation);
    const float clipNear = c->clipNear;
    const float clipFar = c->clipFar;

    // sbybox
    cam->setPosition(glm::vec3());
    cam->setRange(NCamera::CLIP_BACKDROP_NEAR, NCamera::CLIP_BACKDROP_FAR);

    ren->setMode(NRenderer::MODE_BACKDROP);
    s->render();
    ren->dispatch();
    ren->clearGroups();

    // depth map
    CFramebuffer *f = fbo->getFramebuffer(NWindow::STR_ORTHO_DEPTH_FBO);
    CSceneObject *sun = s->getSceneObject(NScene::STR_OBJECT_LIGHT_SUN);
    if((f) && (sun))
    {
      cam->setRange(-50.0f, 50.0f, -30.0f, 30.0f, 30.0f, -30.0f);

      const glm::quat r = sun->getObject()->rotation;
      const float step = (c->clipRight - c->clipLeft) / f->getFrameBuffer()->width;
      const glm::vec3 gridPos0(pos.x * cosf(-r.z) - pos.z * sinf(-r.z), pos.y * cosf(r.y), pos.x * sinf(-r.z) + pos.z * cosf(-r.z));
      const glm::vec3 gridPos1(static_cast<float>(static_cast<int32>(gridPos0.x / step)) * step, static_cast<float>(static_cast<int32>(gridPos0.y / step)) * step, static_cast<float>(static_cast<int32>(gridPos0.z / step)) * step);
      const glm::vec3 gridPos2(gridPos1.x * cosf(r.z) - gridPos1.z * sinf(r.z), gridPos1.y * cosf(-r.y), gridPos1.x * sinf(r.z) + gridPos1.z * cosf(r.z));

      //std::cout << "pos " << pos.x << " " << pos.y << " " << pos.z << ", pos2 " << gridPos2.x << " " << gridPos2.y << " " << gridPos2.z << "\n";
      cam->setPosition(gridPos2);
      cam->setRotation(glm::vec3(r.y * NMath::RAD_2_DEG, -r.z * NMath::RAD_2_DEG, 0.0f));
      cam->setScale(glm::vec3(NCamera::SCALE_X, NCamera::SCALE_Y, -NCamera::SCALE_Z));

      f->setCamera(*c);
      f->bind();
      glClear(GL_DEPTH_BUFFER_BIT);

      ren->setMode(NRenderer::MODE_DEPTH);
      s->render();
      ren->dispatch();
      ren->clearGroups();

      fbo->unbind();
    }
    
    // standard
    cam->setPosition(pos);
    cam->setRotation(rot);
    cam->resetScale();
    cam->setRange(clipNear, clipFar);

    ren->setMode(NRenderer::MODE_STANDARD);
    s->render();
    ren->dispatch();
    ren->clearGroups();
  }

  std::string title = CStr(NWindow::STR_APP_TITLE, context->engineGetEngine()->drawCalls);
#if defined(ENV_QT)
  setWindowTitle(title.c_str());
#elif defined(ENV_SDL)
  SDL_SetWindowTitle(SDLwindow, title.c_str());
  SDL_GL_SwapWindow(SDLwindow);
#endif
}
//------------------------------------------------------------------------------
void CWindow::resizeGL(int width, int height)
{
  //COpenGL *gl = CEngineBase::context->getOpenGL();
  //gl->makeCurrent();

  glViewport(0, 0, width, height);
  CEngineBase::context->getCamera()->setSize(width, height);

#ifdef ENV_SDL
  repaint();
#endif
}
//------------------------------------------------------------------------------
#ifdef ENV_QT
bool CWindow::event(QEvent *event)
{
  // qt mouse and key events

  return
#if QT_VERSION < 0x050400
    QGLWidget
#else
    QOpenGLWidget
#endif
    ::event(event);
}
#endif
//------------------------------------------------------------------------------
