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
  window = new CWindow(&context
#ifdef ENV_QT
    , this
#endif
  );
  context.setContext(this, window, &scenes, &models, &renderer, &shaders, &culling, &pickColor, &framebuffers, &maps, &camera, &openGL, &filesystem, &exceptions);
  context.setEngineCallbacks(&staticShowMessage, &staticIncDrawCalls, &staticClearDrawCalls, &staticGetClassName, &staticGetEngine);

  engine.consoleVisible = true;
  //engine.fullscreen = true;
  engine.maxTextureSize = 256;

#if defined(ENV_QT)
  connect(window, SIGNAL(onInitializeGL()), this, SLOT(initialize()));
  connect(window, SIGNAL(onInitializeFinishGL()), this, SLOT(initializeFinish()));
  connect(window, SIGNAL(onMousePress(NEngine::EMouseButton buttons)), this, SLOT(mousePress(NEngine::EMouseButton buttons)));
  connect(window, SIGNAL(onMouseRelease(NEngine::EMouseButton buttons)), this, SLOT(mouseRelease(NEngine::EMouseButton buttons)));
  connect(window, SIGNAL(onMouseMove(int32 x, int32 y)), this, SLOT(mouseMove(int32 x, int32 y)));
  connect(window, SIGNAL(onKeyPress(NEngine::EKey key)), this, SLOT(keyPress(NEngine::EKey key)));
  connect(window, SIGNAL(onKeyRelease(NEngine::EKey key)), this, SLOT(keyRelease(NEngine::EKey key)));

  // fill keys move map
#elif defined(ENV_SDL)
  engine.keysMap[SDLK_ESCAPE] = NEngine::KEY_QUIT;

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
#endif
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

  camera.setSize(1024, 600);
}
//------------------------------------------------------------------------------
void CEngine::initializeFinish()
{
  camera.setRange(0.1f, 200.0f);

  scenes.addScene(SScene("scene"));
  if(CScene *s = scenes.setActiveScene("scene"))
  {
    s->addSceneObjectLight(SSceneObject("light_amb"), SSceneLight(NScene::OBJECT_LIGHT_TYPE_AMBIENT, glm::vec3(0.1f, 0.2f, 0.3f)));
    s->addSceneObjectLight(SSceneObject("light_fog"), SSceneLight(NScene::OBJECT_LIGHT_TYPE_FOG, glm::vec3(0.5f, 0.6f, 0.7f), glm::vec2(0.0f, 1.0f)));
    s->addSceneObjectLight(SSceneObject("light_sun", glm::vec3(200000.0f, 1000000.0f, -500000.0f)), SSceneLight(NScene::OBJECT_LIGHT_TYPE_POINT, glm::vec3(2.0f, 1.7f, 1.4f), glm::vec2(9999999.0f, 10000000.0f), glm::vec4(100.0f, 100.0f, 100.0f, 512.0f)));

    s->addSceneObjectModel(
      SSceneObject("scene.4ds"),
      SSceneModel(models.addModel(SModel(std::string(NFile::STR_DATA_MODELS)+"sponza.4ds"))))
      ->update();
  }

  camera.setSpeed(10.0f);

#if defined(ENV_QT)
  QTimer::singleShot(NEngine::REDRAW_TIMER_MS, this, SLOT(onTimeout()));
#elif defined(ENV_SDL)
  engine.timers.push_back(SDL_AddTimer(NEngine::REDRAW_TIMER_MS, staticOnTimeout, &context));
#endif
}
//------------------------------------------------------------------------------
#ifdef ENV_SDL
int32 CEngine::event()
{
  initialize();
  window->initializeGL();
  initializeFinish();

  SDL_Event event;

  while(SDL_WaitEvent(&event))
  {
    if(event.type == SDL_WINDOWEVENT)
    {
      uint32 type = event.window.event;

      if(type == SDL_WINDOWEVENT_EXPOSED)
        simulationStep();
      else if(type == SDL_WINDOWEVENT_RESIZED)
        window->resizeGL(event.window.data1, event.window.data2);
    }
    else if(event.type == SDL_QUIT)
      break;
    else if(event.type == SDL_MOUSEBUTTONDOWN)
      mousePress(getMouseButton(event.button.button));
    else if(event.type == SDL_MOUSEBUTTONUP)
      mouseRelease(getMouseButton(event.button.button));
    else if(event.type == SDL_MOUSEMOTION)
      mouseMove(SPoint(event.motion.x, event.motion.y), getMouseButton(event.button.button));
    else if(event.type == SDL_KEYDOWN)
      keyPress(getKey(event.key.keysym.sym));
    else if(event.type == SDL_KEYUP)
      keyRelease(getKey(event.key.keysym.sym));
  }

  return 0;
}
#endif
//------------------------------------------------------------------------------
void CEngine::simulationStep()
{
  context.getCamera()->setMove();
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
    SDL_Event event;
    event.type = SDL_WINDOWEVENT;
    event.window.event = SDL_WINDOWEVENT_EXPOSED;
    SDL_PushEvent(&event);
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
  QMessageBox *msg = new QMessageBox(this);
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
     (engine.keys & NEngine::KEY_DOWN))
    return true;
  
  return false;
}
//------------------------------------------------------------------------------
NEngine::EMouseButton CEngine::getMouseButton(int32 button) const
{
  return static_cast<NEngine::EMouseButton>(
    (((button == SDL_BUTTON_LEFT) || (button == SDL_BUTTON_X2)) ? NEngine::MOUSE_BTN_LEFT : NEngine::MOUSE_BTN_NO) |
    (((button == SDL_BUTTON_RIGHT) || (button == SDL_BUTTON_X1) || (button == SDL_BUTTON_X2)) ? NEngine::MOUSE_BTN_RIGHT : NEngine::MOUSE_BTN_NO));
}
//------------------------------------------------------------------------------
NEngine::EKey CEngine::getKey(int32 key) const
{
  auto it = engine.keysMap.find(key);

  if(it == engine.keysMap.cend())
    return NEngine::KEY;

  return it->second;
}
//------------------------------------------------------------------------------
void CEngine::updateTicks()
{
  engine.tickNew =
#if defined(ENV_QT)
    static_cast<uint32>(timer.elapsed());
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
  UNUSED(object);
  return "abc";
}
//------------------------------------------------------------------------------
