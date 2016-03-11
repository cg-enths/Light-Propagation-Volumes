﻿//------------------------------------------------------------------------------
#include "engine.h"

//------------------------------------------------------------------------------
CEngine::CEngine(
#ifdef ENV_QT
  QObject *parent
#endif
  ) :
#ifdef ENV_QT
  QObject(parent),
#endif
  window(NULL)
{
#ifdef ENV_SDL
  engine.flags = NEngine::EFLAG_SHOW_CONSOLE;
#endif
  //engine.flags = NEngine::EFLAG_FULLSCREEN;
  engine.gpuPlatform = NEngine::GPU_PLATFORM_GL0302;
  engine.multisampling = 1;
  engine.maxTextureSize = 256;
  engine.depthTextureSize = 1024;
  engine.geometryTextureSize = 256;
  engine.lpvTextureSize = glm::vec3(32.0f);
  engine.lpvCellSize = glm::vec3(1.0f);
  engine.lpvPropagationSteps = 0;
  //engine.lpvIntensity = 1.0f;
  engine.defaultScreenWidth = 1024;
  engine.defaultScreenHeight = 600;
  //engine.orthoDepthSize = 64.0f;
  //engine.orthoDepthDepth = 200.0f;
  engine.shadowJittering = 0.0f;

  context.setContext(this, window, &scenes, &models, &renderer, &shaders, &culling, &pickColor, &framebuffers, &maps, &camera, &openGL, &filesystem, &exceptions);
  context.setEngineCallbacks(&staticInitialize, &staticInitializeFinish, &staticShowMessage, &staticSetPlatform, &staticIncDrawCalls, &staticClearDrawCalls, &staticGetClassName, &staticGetEngine);

  window = new CWindow(&context
#ifdef ENV_QT
    , this
#endif
  );

#if defined(ENV_QT)
  engine.timer.start();

  //connect(window, SIGNAL(onInitializeGL()), this, SLOT(initialize()), Qt::DirectConnection);
  //connect(window, SIGNAL(onInitializeFinishGL()), this, SLOT(initializeFinish()), Qt::DirectConnection);
  connect(window, SIGNAL(onMousePress(NEngine::EMouseButton)), this, SLOT(mousePress(NEngine::EMouseButton)));
  connect(window, SIGNAL(onMouseRelease(NEngine::EMouseButton)), this, SLOT(mouseRelease(NEngine::EMouseButton)));
  connect(window, SIGNAL(onMouseMove(const SPoint &, NEngine::EMouseButton)), this, SLOT(mouseMove(const SPoint &, NEngine::EMouseButton)));
  connect(window, SIGNAL(onKeyPress(NEngine::EKey)), this, SLOT(keyPress(NEngine::EKey)));
  connect(window, SIGNAL(onKeyRelease(NEngine::EKey)), this, SLOT(keyRelease(NEngine::EKey)));
#endif

  // fill keys mapping
  engine.keysMap[SDLK_w] = NEngine::KEY_FRONT;
  engine.keysMap[SDLK_UP] = NEngine::KEY_FRONT;
  engine.keysMap[SDLK_s] = NEngine::KEY_BACK;
  engine.keysMap[SDLK_DOWN] = NEngine::KEY_BACK;
  engine.keysMap[SDLK_a] = NEngine::KEY_LEFT;
  engine.keysMap[SDLK_LEFT] = NEngine::KEY_LEFT;
  engine.keysMap[SDLK_d] = NEngine::KEY_RIGHT;
  engine.keysMap[SDLK_RIGHT] = NEngine::KEY_RIGHT;
  engine.keysMap[SDLK_q] = NEngine::KEY_DOWN;
  engine.keysMap[SDLK_e] = NEngine::KEY_UP;

  engine.keysMap[SDLK_i] = NEngine::KEY_SPECIAL_FRONT;
  engine.keysMap[SDLK_k] = NEngine::KEY_SPECIAL_BACK;
  engine.keysMap[SDLK_j] = NEngine::KEY_SPECIAL_LEFT;
  engine.keysMap[SDLK_l] = NEngine::KEY_SPECIAL_RIGHT;
  engine.keysMap[SDLK_u] = NEngine::KEY_SPECIAL_DOWN;
  engine.keysMap[SDLK_o] = NEngine::KEY_SPECIAL_UP;

  engine.keysMap[SDLK_3] = NEngine::KEY_LPV_MODE; // with Qt (cz): shift pressed is needed
  engine.keysMap[SDLK_4] = NEngine::KEY_LPV_TECHNIQUE;
  engine.keysMap[SDLK_5] = NEngine::KEY_LPV_PROPAGATION_DOWN;
  engine.keysMap[SDLK_6] = NEngine::KEY_LPV_PROPAGATION_UP;
  engine.keysMap[SDLK_7] = NEngine::KEY_LPV_INTENSITY_DOWN;
  engine.keysMap[SDLK_8] = NEngine::KEY_LPV_INTENSITY_UP;
  engine.keysMap[SDLK_9] = NEngine::KEY_SHADOW_JITTERING_DOWN;
  engine.keysMap[SDLK_0] = NEngine::KEY_SHADOW_JITTERING_UP;

  engine.keysMap[SDLK_f] = NEngine::KEY_FRUSTUM_UPDATE;
  engine.keysMap[SDLK_g] = NEngine::KEY_SHOW_GEOMETRY_BUFFER;

  engine.keysMap[SDLK_ESCAPE] = NEngine::KEY_QUIT;
}
//------------------------------------------------------------------------------
CEngine::~CEngine()
{
#ifndef ENV_QT
  delete window;
#endif
}
//------------------------------------------------------------------------------
void CEngine::initialize()
{
  scenes = CScenes(&context);
  models = CModels(&context);
  renderer = CRenderer(&context);
  shaders = CShaders(&context);
  culling = CCulling(&context);
  pickColor = CPickColor(&context);
  framebuffers = CFramebuffers(&context);
  maps = CMaps(&context);
  camera = CCamera(&context);
  openGL = COpenGL(&context);
  filesystem = CFilesystem(&context);
  exceptions = CExceptions(&context);

  for(uint32 i = 0; i < NFile::SEARCH_PATHES_COUNT; i++)
    filesystem.addSearchPath(NFile::STR_SEARCH_PATHES[i]);
  //filesystem.addSearchPath("C:/Hry/Mafia/");

  camera.setSize(engine.defaultScreenWidth, engine.defaultScreenHeight);
}
//------------------------------------------------------------------------------
void CEngine::initializeFinish()
{
#if defined(ENV_QT)
  QTimer::singleShot(NEngine::INIT_LOAD_TIMER_MS, this, SLOT(onTimeoutInit()));
#elif defined(ENV_SDL)
  engine.initSceneEvent = SDL_RegisterEvents(1);
  engine.initSceneTimer = SDL_AddTimer(NEngine::INIT_LOAD_TIMER_MS, staticOnTimeoutInit, &context);
#endif
}
//------------------------------------------------------------------------------
void CEngine::onTimeoutInit()
{
  context.log("Loading Scene...");

#ifdef ENV_SDL
  SDL_RemoveTimer(engine.initSceneTimer);
#endif

  camera.setRange(0.01f, 200.0f);
  camera.setSpeed(5.0f);

  scenes.addScene(SScene("scene"));
  if(CScene *s = scenes.setActiveScene("scene"))
  {
    const glm::quat sunRot(0.0f, 0.0f, 0.91f, 1.87f);
    const glm::vec3 sunPos = glm::vec3(sinf(sunRot.z) * cosf(sunRot.y), sunRot.y, cosf(sunRot.z) * cosf(sunRot.y)) * NScene::SUN_DIR_MUL;
    s->addSceneObjectLight(SSceneObject(NScene::STR_OBJECT_LIGHT_AMB), SSceneLight(NScene::OBJECT_LIGHT_TYPE_AMBIENT, glm::vec3(0.1f, 0.2f, 0.3f)));
    s->addSceneObjectLight(SSceneObject(NScene::STR_OBJECT_LIGHT_FOG), SSceneLight(NScene::OBJECT_LIGHT_TYPE_FOG, glm::vec3(0.819f, 0.839f, 0.729f), glm::vec2(0.0f, 1.0f)));
    s->addSceneObjectLight(SSceneObject(NScene::STR_OBJECT_LIGHT_SUN, sunPos, sunRot), SSceneLight(NScene::OBJECT_LIGHT_TYPE_POINT, glm::vec3(1.6f, 1.35f, 1.2f), glm::vec2(9999999.0f, 10000000.0f), glm::vec4(3.0f, 3.0f, 3.0f, 32.0f)));

    s->addSceneObjectModel(
      SSceneObject("sky", glm::vec3(0.0f), glm::quat(glm::vec3(0.0f, -90.0f, 0.0f))),
      SSceneModel(models.addModel(SModel(std::string(NFile::STR_DATA_MODELS)+"denjasno2.4ds")), true));
    s->addSceneObjectModel(
      SSceneObject("scene"),
      SSceneModel(models.addModel(SModel(std::string(NFile::STR_DATA_MODELS)+"sponza.4ds"))));
  }

  window->repaint();

#if defined(ENV_QT)
  QTimer::singleShot(NEngine::REDRAW_TIMER_MS, this, SLOT(onTimeout()));
#elif defined(ENV_SDL)
  engine.timers.push_back(SDL_AddTimer(NEngine::REDRAW_TIMER_MS, staticOnTimeout, &context));
#endif

  context.log("Done.");
}
//------------------------------------------------------------------------------
#ifdef ENV_SDL
int32 CEngine::event()
{
  window->initializeGL();

  SDL_Event event;

  while(SDL_WaitEvent(&event))
  {
    if(event.type == SDL_WINDOWEVENT)
    {
      uint32 type = event.window.event;

      if((type == SDL_WINDOWEVENT_EXPOSED) && (!engine.waitForFlushTimers))
        simulationStep();
      else if(type == SDL_WINDOWEVENT_RESIZED)
        window->resizeGL(event.window.data1, event.window.data2);
    }
    else if(event.type == SDL_QUIT)
      break;
    else if(event.type == SDL_MOUSEBUTTONDOWN)
      mousePress(window->getMouseButton(event.button.button));
    else if(event.type == SDL_MOUSEBUTTONUP)
      mouseRelease(window->getMouseButton(event.button.button));
    else if(event.type == SDL_MOUSEMOTION)
      mouseMove(SPoint(event.motion.x, event.motion.y), window->getMouseButton(event.button.button));
    else if(event.type == SDL_KEYDOWN)
      keyPress(window->getKey(event.key.keysym.sym));
    else if(event.type == SDL_KEYUP)
      keyRelease(window->getKey(event.key.keysym.sym));
    else if(event.type == engine.initSceneEvent)
      onTimeoutInit();
  }

  return 0;
}
#endif
//------------------------------------------------------------------------------
void CEngine::simulationStep()
{
  context.getCamera()->setMove();

  if((engine.keys & NEngine::KEY_SPECIAL_FRONT) ||
     (engine.keys & NEngine::KEY_SPECIAL_BACK) ||
     (engine.keys & NEngine::KEY_SPECIAL_LEFT) ||
     (engine.keys & NEngine::KEY_SPECIAL_RIGHT))
  {
    if(CScene *s = scenes.getActiveScene())
    {
      if(CSceneObject *sun = s->getSceneObject(NScene::STR_OBJECT_LIGHT_SUN))
      {
        const SSceneObject *o = sun->getObject();
        glm::quat r = o->rotation;
        const float d = NMath::PI / 72.0f; // 5 deg

        if(engine.keys & NEngine::KEY_SPECIAL_FRONT)
        {
          r.y += d;
          if(r.y > NMath::DIV_PI)
            r.y = NMath::DIV_PI;
        }
        else if(engine.keys & NEngine::KEY_SPECIAL_BACK)
        {
          r.y -= d;
          if(r.y < -NMath::DIV_PI)
            r.y = -NMath::DIV_PI;
        }
        else if(engine.keys & NEngine::KEY_SPECIAL_LEFT)
          r.z -= d;
        else if(engine.keys & NEngine::KEY_SPECIAL_RIGHT)
          r.z += d;

        sun->setPosition(glm::vec3(sinf(r.z) * cosf(r.y), sinf(r.y), cosf(r.z) * cosf(r.y)) * NScene::SUN_DIR_MUL);
        sun->setRotation(r);
        if(CFramebuffer *f = framebuffers.getFramebuffer(NWindow::STR_ORTHO_DEPTH_FBO))
          f->setChanged();
        //std::cout << "sun " << o->position.x << " " << o->position.y << " " << o->position.z << ", " << (r.y * NMath::RAD_2_DEG) << " " << (r.z * NMath::RAD_2_DEG) << "\n";
      }
    }
  }

  window->repaint();
}
//------------------------------------------------------------------------------
void CEngine::onTimeout()
{
#ifdef ENV_SDL
  for(uint32 i = 0; i < engine.timers.size(); i++)
    SDL_RemoveTimer(engine.timers[i]);
  engine.timers.clear();
#endif

  updateTicks();

  if((isKeyForDelayedRendering()) || (engine.activeRendering))
  {
#if defined(ENV_QT)
    simulationStep();
#elif defined(ENV_SDL)
    SDL_Event upEvent;
    engine.waitForFlushTimers = SDL_PeepEvents(&upEvent, 1, SDL_PEEKEVENT, SDL_KEYUP, SDL_KEYUP);

    if(!engine.waitForFlushTimers)
    {
      SDL_Event event;
      event.type = SDL_WINDOWEVENT;
      event.window.event = SDL_WINDOWEVENT_EXPOSED;
      SDL_PushEvent(&event);
    }
#endif
  }

#if defined(ENV_QT)
  QTimer::singleShot(NEngine::REDRAW_TIMER_MS, this, SLOT(onTimeout()));
#elif defined(ENV_SDL)
  engine.timers.push_back(SDL_AddTimer(NEngine::REDRAW_TIMER_MS, staticOnTimeout, &context));
#endif
}
//------------------------------------------------------------------------------
void CEngine::mousePress(NEngine::EMouseButton buttons)
{
  UNUSED(buttons);
}
//------------------------------------------------------------------------------
void CEngine::mouseRelease(NEngine::EMouseButton buttons)
{
  UNUSED(buttons);
}
//------------------------------------------------------------------------------
void CEngine::mouseMove(const SPoint &point, NEngine::EMouseButton buttons)
{
  engine.cursor = glm::vec2(static_cast<float>(point.y), static_cast<float>(point.x));

  if(buttons & NEngine::MOUSE_BTN_RIGHT)
  {
    context.getCamera()->setRotate();
    window->repaint();
  }

  engine.cursorOld = engine.cursor;
}
//------------------------------------------------------------------------------
void CEngine::keyPress(NEngine::EKey key)
{
  if(key & NEngine::KEY_QUIT)
    quit();
  else if(key & NEngine::KEY_LPV_MODE)
  {
    engine.lpvMode = static_cast<NEngine::ELPVMode>((static_cast<uint32>(engine.lpvMode) + 1) % NEngine::LPV_MODES_COUNT);
    context.log(CStr("LPV Mode: %s", NEngine::STR_LPV_MODES[engine.lpvMode]));
    if(CFramebuffer *f = framebuffers.getFramebuffer(NWindow::STR_ORTHO_DEPTH_FBO))
      f->setChanged();
    window->repaint();
  }
  else if(key & NEngine::KEY_LPV_TECHNIQUE)
  {
    engine.lpvTechnique = static_cast<NEngine::ELPVTechnique>((static_cast<uint32>(engine.lpvTechnique) + 1) % NEngine::LPV_TECHNIQUES_COUNT);
    context.log(CStr("LPV Technique: %s", NEngine::STR_LPV_MODES[engine.lpvMode]));
    if(CFramebuffer *f = framebuffers.getFramebuffer(NWindow::STR_ORTHO_DEPTH_FBO))
      f->setChanged();
    window->repaint();
  }
  else if(key & NEngine::KEY_LPV_PROPAGATION_DOWN)
  {
    if(engine.lpvPropagationSteps > 0)
      engine.lpvPropagationSteps--;
  }
  else if(key & NEngine::KEY_LPV_PROPAGATION_UP)
    engine.lpvPropagationSteps++;
  else if(key & NEngine::KEY_LPV_INTENSITY_DOWN)
  {
    engine.lpvIntensity *= 0.5f;
    if(engine.lpvIntensity < 0.0f)
      engine.lpvIntensity = 0.0;
  }
  else if(key & NEngine::KEY_LPV_INTENSITY_UP)
    engine.lpvIntensity *= 2.0f;
  else if(key & NEngine::KEY_SHADOW_JITTERING_DOWN)
  {
    engine.shadowJittering -= 1.0f;
    if(engine.shadowJittering < 0.0f)
      engine.shadowJittering = 0.0;
  }
  else if(key & NEngine::KEY_SHADOW_JITTERING_UP)
    engine.shadowJittering += 1.0f;
  else if(key & NEngine::KEY_FRUSTUM_UPDATE)
  {
    engine.updateFrustum = !engine.updateFrustum;
    if(CFramebuffer *f = framebuffers.getFramebuffer(NWindow::STR_ORTHO_DEPTH_FBO))
      f->setChanged();
    window->repaint();
  }
  else if(key & NEngine::KEY_SHOW_GEOMETRY_BUFFER)
  {
    engine.showGeometryBuffer = !engine.showGeometryBuffer;
    window->repaint();
  }

  if(key & (NEngine::KEY_LPV_PROPAGATION_DOWN | NEngine::KEY_LPV_PROPAGATION_UP))
  {
    context.log(CStr("LPV Propagation Steps: %ud", engine.lpvPropagationSteps));
    if(CFramebuffer *f = framebuffers.getFramebuffer(NWindow::STR_ORTHO_DEPTH_FBO))
      f->setChanged();
    window->repaint();
  }
  else if(key & (NEngine::KEY_LPV_INTENSITY_DOWN | NEngine::KEY_LPV_INTENSITY_UP))
  {
    context.log(CStr("LPV Intensity: %f", static_cast<double>(engine.lpvIntensity)));
    window->repaint();
  }
  else if(key & (NEngine::KEY_SHADOW_JITTERING_DOWN | NEngine::KEY_SHADOW_JITTERING_UP))
  {
    context.log(CStr("Shadow Jittering: %f", static_cast<double>(engine.shadowJittering)));
    if(CFramebuffer *f = framebuffers.getFramebuffer(NWindow::STR_ORTHO_DEPTH_FBO))
      f->setChanged();
    window->repaint();
  }

  engine.keys = static_cast<NEngine::EKey>(static_cast<uint32>(engine.keys) | static_cast<uint32>(key));
}
//------------------------------------------------------------------------------
void CEngine::keyRelease(NEngine::EKey key)
{
  engine.keys = static_cast<NEngine::EKey>(static_cast<uint32>(engine.keys) & (~static_cast<uint32>(key)));
}
//------------------------------------------------------------------------------
void CEngine::showMessage(const std::string &title, const std::string &text, bool modal) const
{
  context.log(CStr("%s: %s", title.c_str(), text.c_str()));

#if defined(ENV_QT)
  QMessageBox *msg = new QMessageBox(window);
  msg->setWindowTitle(title.c_str());
  msg->setText(text.c_str());
  msg->setStandardButtons(QMessageBox::Ok);
  msg->setDefaultButton(QMessageBox::Ok);
  msg->setModal(modal);
  msg->show();
#elif defined(ENV_SDL)
  UNUSED(modal);
  SDL_ShowSimpleMessageBox(0, title.c_str(), text.c_str(), NULL);
#endif
}
//------------------------------------------------------------------------------
bool CEngine::isKeyForDelayedRendering() const
{
  if((engine.keys & NEngine::KEY_FRONT) ||
     (engine.keys & NEngine::KEY_BACK) ||
     (engine.keys & NEngine::KEY_LEFT) ||
     (engine.keys & NEngine::KEY_RIGHT) ||
     (engine.keys & NEngine::KEY_UP) ||
     (engine.keys & NEngine::KEY_DOWN) ||
     (engine.keys & NEngine::KEY_SPECIAL_FRONT) ||
     (engine.keys & NEngine::KEY_SPECIAL_BACK) ||
     (engine.keys & NEngine::KEY_SPECIAL_LEFT) ||
     (engine.keys & NEngine::KEY_SPECIAL_RIGHT) ||
     (engine.keys & NEngine::KEY_SPECIAL_UP) ||
     (engine.keys & NEngine::KEY_SPECIAL_DOWN))
    return true;
  
  return false;
}
//------------------------------------------------------------------------------
void CEngine::updateTicks()
{
  engine.tickNew =
#if defined(ENV_QT)
    static_cast<uint32>(engine.timer.elapsed());
#elif defined(ENV_SDL)
    SDL_GetTicks();
#endif

  if(!engine.tickOld)
    engine.tickOld = engine.tickNew;

  if((engine.tickNew != engine.tickOld) && (!engine.fpsCounter))
  {
    engine.simulationStep = static_cast<float>(engine.tickNew - engine.tickOld) * NEngine::FPS_MS / NEngine::FPS_COUNTER_MAX;
    engine.fps = 1.0f / engine.simulationStep;
    engine.tickOld = engine.tickNew;
  }
  engine.fpsCounter++;
  if(engine.fpsCounter == NEngine::FPS_COUNTER_MAX)
    engine.fpsCounter = 0;
}
//------------------------------------------------------------------------------
std::string CEngine::getClassName(const CEngineBase *object)
{
  UNUSED(object); // todo doplnit !!! třídy
  return "abc";
}
//------------------------------------------------------------------------------
