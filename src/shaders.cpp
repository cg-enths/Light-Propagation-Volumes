﻿//------------------------------------------------------------------------------
#include "shaders.h"

//------------------------------------------------------------------------------
CShader::CShader() : CEngineBase()
{
}
//------------------------------------------------------------------------------
CShader::CShader(CContext *context, const SShader &shader) : CEngineBase(context), shader(shader)
{
}
//------------------------------------------------------------------------------
CShader::~CShader()
{
}
//------------------------------------------------------------------------------
void CShader::compile()
{
  if(shader.file == NShader::STR_SHADER_UNUSED)
    return;

  COpenGL *gl = context->getOpenGL();
  const GLenum shaderType = NShader::TYPE_SHADERS[shader.type];
  const NEngine::EGPUPlatform platform = context->engineGetEngine()->gpuPlatform;
  GLint status = 0;
  GLint infoLength = 0;
  std::string log;

  if(((shader.type == NShader::TYPE_GEOMETRY) && (platform < NEngine::GPU_PLATFORM_GL0302)) ||
     ((shader.type == NShader::TYPE_TESSELATION_CONTROL) && (platform < NEngine::GPU_PLATFORM_GL0400)) ||
     ((shader.type == NShader::TYPE_TESSELATION_EVALUATION) && (platform < NEngine::GPU_PLATFORM_GL0400)) ||
     ((shader.type == NShader::TYPE_COMPUTE) && (platform < NEngine::GPU_PLATFORM_GL0403)))
  {
    context->log(CStr(NShader::STR_ERROR_COMPILE_SKIP, shader.file.c_str()));
    return;
  }

  CFile *f = context->getFilesystem()->open(SFile(std::string(NFile::STR_DATA_SHADERS)+shader.file));

  if(!f)
    return;

  std::string data(f->size(), ' ');
  f->read(&data[0], data.length());

  if(context->engineGetEngine()->gpuPlatform == NEngine::GPU_PLATFORM_GL0200_ES)
    data = setES2compatible(data);

  const char *d = data.c_str();

  //qDebug(d);
  shader.shader = gl->createShader(shaderType);
  gl->shaderSource(shader.shader, 1, &d, NULL);
  gl->compileShader(shader.shader);
  gl->getShaderiv(shader.shader, NOpenGL::COMPILE_STATUS, &status);
  gl->getShaderiv(shader.shader, NOpenGL::INFO_LOG_LENGTH, &infoLength);

  log.resize(infoLength);
  if(infoLength)
    gl->getShaderInfoLog(shader.shader, infoLength, &infoLength, &log[0]);
  if((&log[0]) && (status == NOpenGL::TRUE) && (log.find("warning") != std::string::npos))
  {
    context->log("aaa");
    context->log(log);
  }

  if(status == NOpenGL::FALSE)
    context->engineShowMessage(CStr(NShader::STR_ERROR_COMPILE, shader.file.c_str()), &log[0], false);

  f->close();
}
//------------------------------------------------------------------------------
std::string CShader::setES2compatible(const std::string &data)
{
  std::string d = data;
#ifndef Q_OS_ANDROID
  std::string precision("lowp");
#else
  std::string precision("highp");
#endif

  // replace any version and precision by #version 100 - ES 2.0
  auto index = d.find("in ");
  if(index == std::string::npos)
    index = d.find("uniform ");
  if(index == std::string::npos)
    index = d.find("out ");
  d.erase(0, index);
  d.insert(0, "#version 100\r\n\r\n");

  if(shader.type == NShader::TYPE_VERTEX)
  {
    d = CStr::replaceAll(d, "in ", "attribute ");
    d = CStr::replaceAll(d, "out ", "varying ");
  }
  else if(shader.type == NShader::TYPE_FRAGMENT)
  {
    d = CStr::replaceAll(d, "in ", "varying ");
    d = CStr::replaceAll(d, "out vec4 glFragColor;", "");
    d = CStr::replaceAll(d, "glFragColor", "NOpenGL::FragColor");
    d = CStr::replaceAll(d, "texture(", "texture2D(");
  }

  d = CStr::replaceAll(d, "float ", precision+" float ");
  d = CStr::replaceAll(d, "vec2 ", precision+" vec2 ");
  d = CStr::replaceAll(d, "vec3 ", precision+" vec3 ");
  d = CStr::replaceAll(d, "vec4 ", precision+" vec4 ");
  d = CStr::replaceAll(d, "mat2 ", precision+" mat2 ");
  d = CStr::replaceAll(d, "mat3 ", precision+" mat3 ");
  d = CStr::replaceAll(d, "mat4 ", precision+" mat4 ");
  d = CStr::replaceAll(d, "sampler2D ", precision+" sampler2D ");

  return d;
}
//------------------------------------------------------------------------------
CShaderProgram::CShaderProgram() : CEngineBase()
{
}
//------------------------------------------------------------------------------
CShaderProgram::CShaderProgram(CContext *context, const SShaderProgram &shaderProgram) : CEngineBase(context), program(shaderProgram)
{
}
//------------------------------------------------------------------------------
CShaderProgram::~CShaderProgram()
{
}
//------------------------------------------------------------------------------
void CShaderProgram::link()
{
  COpenGL *gl = context->getOpenGL();
  GLint status = 0;
  GLint infoLength = 0;
  std::string log;
  const NEngine::EGPUPlatform platform = context->engineGetEngine()->gpuPlatform;

  program.program = gl->createProgram();

  if(((program.geometryShader->getShader()->type != NShader::TYPE_UNDEFINED) && (platform < NEngine::GPU_PLATFORM_GL0302)) ||
     ((program.tesselationControlShader->getShader()->type != NShader::TYPE_UNDEFINED) && (platform < NEngine::GPU_PLATFORM_GL0400)) ||
     ((program.tesselationEvaluationShader->getShader()->type != NShader::TYPE_UNDEFINED) && (platform < NEngine::GPU_PLATFORM_GL0400)) ||
     ((program.computeShader->getShader()->type != NShader::TYPE_UNDEFINED) && (platform < NEngine::GPU_PLATFORM_GL0403)))
  {
    context->log(CStr(NShader::STR_ERROR_LINK_SKIP, NShader::STR_PROGRAM_LIST[program.name]));
    return;
  }

  if(program.vertexShader->getShader()->type != NShader::TYPE_UNDEFINED)
    gl->attachShader(program.program, program.vertexShader->getShader()->shader);
  if(program.geometryShader->getShader()->type != NShader::TYPE_UNDEFINED)
    gl->attachShader(program.program, program.geometryShader->getShader()->shader);
  if(program.tesselationControlShader->getShader()->type != NShader::TYPE_UNDEFINED)
    gl->attachShader(program.program, program.tesselationControlShader->getShader()->shader);
  if(program.tesselationEvaluationShader->getShader()->type != NShader::TYPE_UNDEFINED)
    gl->attachShader(program.program, program.tesselationEvaluationShader->getShader()->shader);
  if(program.fragmentShader->getShader()->type != NShader::TYPE_UNDEFINED)
    gl->attachShader(program.program, program.fragmentShader->getShader()->shader);
  if(program.computeShader->getShader()->type != NShader::TYPE_UNDEFINED)
    gl->attachShader(program.program, program.computeShader->getShader()->shader);

  gl->linkProgram(program.program);

  gl->getProgramiv(program.program, NOpenGL::LINK_STATUS, &status);
  gl->getProgramiv(program.program, NOpenGL::INFO_LOG_LENGTH, &infoLength);
  log.resize(infoLength);
  if(infoLength)
    gl->getProgramInfoLog(program.program, infoLength, &infoLength, &log[0]);

#ifndef Q_OS_SYMBIAN
  if((&log[0]) && (status == NOpenGL::TRUE) && (log.find("warning") != std::string::npos))
  {
    context->log("bbb");
    context->log(log);
  }
#endif

  if(status == NOpenGL::FALSE)
    context->engineShowMessage(CStr(NShader::STR_ERROR_LINK, NShader::STR_PROGRAM_LIST[program.name]), &log[0], false);
}
//------------------------------------------------------------------------------
void CShaderProgram::initUniforms()
{
  COpenGL *gl = context->getOpenGL();

  program.uniforms.vertexPosition = gl->getAttribLocation(program.program, NShader::STR_ATTRIB_VERTEX_POSITION);
  program.uniforms.vertexNormal = gl->getAttribLocation(program.program, NShader::STR_ATTRIB_VERTEX_NORMAL);
  program.uniforms.vertexNormalTangent = gl->getAttribLocation(program.program, NShader::STR_ATTRIB_VERTEX_NORMAL_TANGENT);
  //program.uniforms.vertexNormalBitangent = gl->getAttribLocation(program.program, NShader::STR_ATTRIB_VERTEX_NORMAL_BITANGENT);
  program.uniforms.vertexTexCoord = gl->getAttribLocation(program.program, NShader::STR_ATTRIB_VERTEX_TEX_COORD);
  program.uniforms.vertexColor = gl->getAttribLocation(program.program, NShader::STR_ATTRIB_VERTEX_COLOR);

  program.uniforms.mw = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_MW);
  program.uniforms.mwnit = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_MWNIT);
  program.uniforms.mvp = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_MVP);
  program.uniforms.mvpdb = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_MVPDB);
  program.uniforms.cam = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_CAM);

  program.uniforms.difTex = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_DIF_TEX);
  program.uniforms.alpTex = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_ALP_TEX);
  program.uniforms.speTex = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_SPE_TEX);
  program.uniforms.norTex = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_NOR_TEX);
  program.uniforms.envTex = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_ENV_TEX);
  program.uniforms.depthTex = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_DEPTH_TEX);
  program.uniforms.lpvTexR0 = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_LPV_TEX_R0);
  program.uniforms.lpvTexG0 = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_LPV_TEX_G0);
  program.uniforms.lpvTexB0 = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_LPV_TEX_B0);

  program.uniforms.type = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_TYPE);
  program.uniforms.opacity = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_OPACITY);
  program.uniforms.depthTexelSize = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_DEPTH_TEXEL_SIZE);
  program.uniforms.lightAmb = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_LIGHT_AMB);
  program.uniforms.lightPos = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_LIGHT_POS);
  program.uniforms.lightRange = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_LIGHT_RANGE);
  program.uniforms.lightColor = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_LIGHT_COLOR);
  program.uniforms.lightSpeColor = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_LIGHT_SPEC_COLOR);
  program.uniforms.fogRange = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_FOG_RANGE);
  program.uniforms.fogColor = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_FOG_COLOR);

  program.uniforms.fragColorImg = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_FRAG_COLOR_IMG);
  program.uniforms.fragPosImg = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_FRAG_POS_IMG);
  program.uniforms.fragNormalImg = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_FRAG_NORMAL_IMG);
  program.uniforms.fragDepthImg = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_FRAG_DEPTH_IMG);
  program.uniforms.lpvImgR0 = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_LPV_IMG_R0);
  program.uniforms.lpvImgG0 = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_LPV_IMG_G0);
  program.uniforms.lpvImgB0 = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_LPV_IMG_B0);
  program.uniforms.gvImgA0 = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_GV_IMG_A0);
  program.uniforms.fragSize = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_FRAG_SIZE);
  program.uniforms.lpvPos = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_LPV_POS);
  program.uniforms.lpvSize = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_LPV_SIZE);
  program.uniforms.lpvCellSize = gl->getUniformLocation(program.program, NShader::STR_UNIFORM_LPV_CELL_SIZE);
}
//------------------------------------------------------------------------------
void CShaderProgram::bind() const
{
  context->getOpenGL()->useProgram(program.program);
}
//------------------------------------------------------------------------------
void CShaderProgram::begin(const SShaderTechnique *technique, NRenderer::EMode mode) const
{
  COpenGL *gl = context->getOpenGL();
  if(program.name < NShader::PROGRAM_LPV_CLEAR)
  {
    uint32 stride = sizeof(float) * (((mode == NRenderer::MODE_PICK) || (mode == NRenderer::MODE_DEPTH)) ? NModel::VERTEX_P_SIZE : NModel::VERTEX_PNTTC_SIZE);

    if(program.name >= NShader::PROGRAM_COLOR)
    {
      gl->enableVertexAttribArray(program.uniforms.vertexPosition);
      gl->vertexAttribPointer(program.uniforms.vertexPosition, 3, NOpenGL::FLOAT, NOpenGL::FALSE, stride, reinterpret_cast<float *>(sizeof(float) * NModel::VBO_OFFSET_POSITION));
    }
    if(program.name >= NShader::PROGRAM_BASIC)
    {
      gl->enableVertexAttribArray(program.uniforms.vertexTexCoord);
      gl->enableVertexAttribArray(program.uniforms.vertexColor);
      gl->vertexAttribPointer(program.uniforms.vertexTexCoord, 2, NOpenGL::FLOAT, NOpenGL::FALSE, stride, reinterpret_cast<float *>(sizeof(float) * NModel::VBO_OFFSET_TEX_COORD));
      gl->vertexAttribPointer(program.uniforms.vertexColor, 4, NOpenGL::FLOAT, NOpenGL::FALSE, stride, reinterpret_cast<float *>(sizeof(float) * NModel::VBO_OFFSET_COLOR));
    }
    if(program.name >= NShader::PROGRAM_PER_FRAGMENT)
    {
      gl->enableVertexAttribArray(program.uniforms.vertexNormal);
      gl->vertexAttribPointer(program.uniforms.vertexNormal, 3, NOpenGL::FLOAT, NOpenGL::FALSE, stride, reinterpret_cast<float *>(sizeof(float) * NModel::VBO_OFFSET_NORMAL));
    }
    if(program.name >= NShader::PROGRAM_PER_FRAGMENT_NORMAL)
    {
      gl->enableVertexAttribArray(program.uniforms.vertexNormalTangent);
      //gl->enableVertexAttribArray(program.uniforms.vertexNormalBitangent);
      gl->vertexAttribPointer(program.uniforms.vertexNormalTangent, 3, NOpenGL::FLOAT, NOpenGL::FALSE, stride, reinterpret_cast<float *>(sizeof(float) * NModel::VBO_OFFSET_NORMAL_TANGENT));
      //gl->vertexAttribPointer(program.uniforms.vertexNormalBitangent, 3, NOpenGL::FLOAT, NOpenGL::FALSE, sizeof(float) * NModel::VERTEX_PNTBTC_SIZE, reinterpret_cast<float *>(sizeof(float) * NModel::VBO_OFFSET_NORMAL_BITANGENT));
    }
  }

  const SEngine *e = context->engineGetEngine();
  const CMap *depthMap = context->getMaps()->getMap(NWindow::STR_ORTHO_DEPTH_FBO_MAP);
  const CMap *lpvMapR0 = context->getMaps()->getMap(NWindow::STR_LPV_MAP_R0);
  const CMap *lpvMapG0 = context->getMaps()->getMap(NWindow::STR_LPV_MAP_G0);
  const CMap *lpvMapB0 = context->getMaps()->getMap(NWindow::STR_LPV_MAP_B0);

  if(technique)
  {
    gl->uniformMatrix4fv(program.uniforms.mw, 1, NOpenGL::FALSE, glm::value_ptr(technique->mw));
    gl->uniformMatrix3fv(program.uniforms.mwnit, 1, NOpenGL::FALSE, glm::value_ptr(technique->mwnit));
    gl->uniformMatrix4fv(program.uniforms.mvp, 1, NOpenGL::FALSE, glm::value_ptr(technique->mvp));
    gl->uniformMatrix4fv(program.uniforms.mvpdb, 1, NOpenGL::FALSE, glm::value_ptr(technique->mvpdb));
    gl->uniform3f(program.uniforms.cam, technique->cam.x, technique->cam.y, technique->cam.z);

    if(technique->material)
    {
      const SMaterial *m = technique->material;

      if((program.name == NShader::PROGRAM_BASIC_ALPHA) ||
         (program.name == NShader::PROGRAM_PER_FRAGMENT_ALPHA) ||
         (program.name == NShader::PROGRAM_PER_FRAGMENT_NORMAL_ALPHA))
        glEnable(NOpenGL::BLEND);

      // použít
      /*const uint8 matFlags = ((m->type & NModel::MAT_MIPMAP) ? NMap::FORMAT_MIPMAP : NMap::FORMAT) |
                             ((m->type & NModel::MAT_LINEAR) ? NMap::FORMAT_LINEAR : NMap::FORMAT) |
                             ((m->type & NModel::MAT_EDGE) ? NMap::FORMAT_EDGE : NMap::FORMAT);*/

      if(program.name == NShader::PROGRAM_BASIC)
      {
        setSampler(m->diffuseMap, program.uniforms.difTex, NShader::SAMPLER_BASIC_DIF, ((m->type & NModel::MATERIAL_MIP_MAPPING) ? NMap::FORMAT_MIPMAP : NMap::FORMAT_LINEAR) | (mode == NRenderer::MODE_BACKDROP ? NMap::FORMAT_EDGE : NMap::FORMAT));
      }
      else if((program.name == NShader::PROGRAM_BASIC_ALPHA))
      {
        setSampler(m->diffuseMap, program.uniforms.difTex, NShader::SAMPLER_BASIC_APLHA_DIF, m->type & NModel::MATERIAL_MIP_MAPPING ? NMap::FORMAT_MIPMAP : NMap::FORMAT_LINEAR);
        setSampler(m->alphaMap, program.uniforms.alpTex, NShader::SAMPLER_BASIC_APLHA_ALP, m->type & NModel::MATERIAL_MIP_MAPPING ? NMap::FORMAT_MIPMAP : NMap::FORMAT_LINEAR);
      }
      else if((program.name >= NShader::PROGRAM_PER_FRAGMENT) && (program.name <= NShader::PROGRAM_PER_FRAGMENT_SHADOW_JITTER))
      {
        setSampler(m->diffuseMap, program.uniforms.difTex, NShader::SAMPLER_PER_FRAGMENT_DIF, m->type & NModel::MATERIAL_MIP_MAPPING ? NMap::FORMAT_MIPMAP : NMap::FORMAT_LINEAR);
        setSampler(m->specularMap, program.uniforms.speTex, NShader::SAMPLER_PER_FRAGMENT_SPE, m->type & NModel::MATERIAL_MIP_MAPPING ? NMap::FORMAT_MIPMAP : NMap::FORMAT_LINEAR);
        if(program.name != NShader::PROGRAM_PER_FRAGMENT)
        {
          setSampler(depthMap, program.uniforms.depthTex, NShader::SAMPLER_PER_FRAGMENT_DEPTH, NMap::FORMAT_LINEAR | NMap::FORMAT_DEPTH | NMap::FORMAT_EDGE);
          setSampler(lpvMapR0, program.uniforms.lpvTexR0, NShader::SAMPLER_PER_FRAGMENT_LPV_R0, NMap::FORMAT_LINEAR | NMap::FORMAT_EDGE);
          setSampler(lpvMapG0, program.uniforms.lpvTexG0, NShader::SAMPLER_PER_FRAGMENT_LPV_G0, NMap::FORMAT_LINEAR | NMap::FORMAT_EDGE);
          setSampler(lpvMapB0, program.uniforms.lpvTexB0, NShader::SAMPLER_PER_FRAGMENT_LPV_B0, NMap::FORMAT_LINEAR | NMap::FORMAT_EDGE);
        }
      }
      else if((program.name >= NShader::PROGRAM_PER_FRAGMENT_ALPHA) && (program.name <= NShader::PROGRAM_PER_FRAGMENT_ALPHA_SHADOW_JITTER))
      {
        setSampler(m->diffuseMap, program.uniforms.difTex, NShader::SAMPLER_PER_FRAGMENT_ALPHA_DIF, m->type & NModel::MATERIAL_MIP_MAPPING ? NMap::FORMAT_MIPMAP : NMap::FORMAT_LINEAR);
        setSampler(m->alphaMap, program.uniforms.alpTex, NShader::SAMPLER_PER_FRAGMENT_ALPHA_ALP, m->type & NModel::MATERIAL_MIP_MAPPING ? NMap::FORMAT_MIPMAP : NMap::FORMAT_LINEAR);
        setSampler(m->specularMap, program.uniforms.speTex, NShader::SAMPLER_PER_FRAGMENT_ALPHA_SPE, m->type & NModel::MATERIAL_MIP_MAPPING ? NMap::FORMAT_MIPMAP : NMap::FORMAT_LINEAR);
        if(program.name != NShader::PROGRAM_PER_FRAGMENT_ALPHA)
        {
          setSampler(depthMap, program.uniforms.depthTex, NShader::SAMPLER_PER_FRAGMENT_ALPHA_DEPTH, NMap::FORMAT_LINEAR | NMap::FORMAT_DEPTH | NMap::FORMAT_EDGE);
          setSampler(lpvMapR0, program.uniforms.lpvTexR0, NShader::SAMPLER_PER_FRAGMENT_ALPHA_LPV_R0, NMap::FORMAT_LINEAR | NMap::FORMAT_EDGE);
          setSampler(lpvMapG0, program.uniforms.lpvTexG0, NShader::SAMPLER_PER_FRAGMENT_ALPHA_LPV_G0, NMap::FORMAT_LINEAR | NMap::FORMAT_EDGE);
          setSampler(lpvMapB0, program.uniforms.lpvTexB0, NShader::SAMPLER_PER_FRAGMENT_ALPHA_LPV_B0, NMap::FORMAT_LINEAR | NMap::FORMAT_EDGE);
        }
      }
      else if((program.name >= NShader::PROGRAM_PER_FRAGMENT_NORMAL) && (program.name <= NShader::PROGRAM_PER_FRAGMENT_NORMAL_SHADOW_JITTER))
      {
        setSampler(m->diffuseMap, program.uniforms.difTex, NShader::SAMPLER_PER_FRAGMENT_NORMAL_DIF, m->type & NModel::MATERIAL_MIP_MAPPING ? NMap::FORMAT_MIPMAP : NMap::FORMAT_LINEAR);
        setSampler(m->specularMap, program.uniforms.speTex, NShader::SAMPLER_PER_FRAGMENT_NORMAL_SPE, m->type & NModel::MATERIAL_MIP_MAPPING ? NMap::FORMAT_MIPMAP : NMap::FORMAT_LINEAR);
        setSampler(m->normalMap, program.uniforms.norTex, NShader::SAMPLER_PER_FRAGMENT_NORMAL_NOR, m->type & NModel::MATERIAL_MIP_MAPPING ? NMap::FORMAT_MIPMAP : NMap::FORMAT_LINEAR);
        if(program.name != NShader::PROGRAM_PER_FRAGMENT_NORMAL)
        {
          setSampler(depthMap, program.uniforms.depthTex, NShader::SAMPLER_PER_FRAGMENT_NORMAL_DEPTH, NMap::FORMAT_LINEAR | NMap::FORMAT_DEPTH | NMap::FORMAT_EDGE);
          setSampler(lpvMapR0, program.uniforms.lpvTexR0, NShader::SAMPLER_PER_FRAGMENT_NORMAL_LPV_R0, NMap::FORMAT_LINEAR | NMap::FORMAT_EDGE);
          setSampler(lpvMapG0, program.uniforms.lpvTexG0, NShader::SAMPLER_PER_FRAGMENT_NORMAL_LPV_G0, NMap::FORMAT_LINEAR | NMap::FORMAT_EDGE);
          setSampler(lpvMapB0, program.uniforms.lpvTexB0, NShader::SAMPLER_PER_FRAGMENT_NORMAL_LPV_B0, NMap::FORMAT_LINEAR | NMap::FORMAT_EDGE);
        }
      }
      else if((program.name >= NShader::PROGRAM_PER_FRAGMENT_NORMAL_ALPHA) && (program.name <= NShader::PROGRAM_PER_FRAGMENT_NORMAL_ALPHA_SHADOW_JITTER))
      {
        setSampler(m->diffuseMap, program.uniforms.difTex, NShader::SAMPLER_PER_FRAGMENT_NORMAL_ALPHA_DIF, m->type & NModel::MATERIAL_MIP_MAPPING ? NMap::FORMAT_MIPMAP : NMap::FORMAT_LINEAR);
        setSampler(m->alphaMap, program.uniforms.alpTex, NShader::SAMPLER_PER_FRAGMENT_NORMAL_ALPHA_ALP, m->type & NModel::MATERIAL_MIP_MAPPING ? NMap::FORMAT_MIPMAP : NMap::FORMAT_LINEAR);
        setSampler(m->specularMap, program.uniforms.speTex, NShader::SAMPLER_PER_FRAGMENT_NORMAL_ALPHA_SPE, m->type & NModel::MATERIAL_MIP_MAPPING ? NMap::FORMAT_MIPMAP : NMap::FORMAT_LINEAR);
        setSampler(m->normalMap, program.uniforms.norTex, NShader::SAMPLER_PER_FRAGMENT_NORMAL_ALPHA_NOR, m->type & NModel::MATERIAL_MIP_MAPPING ? NMap::FORMAT_MIPMAP : NMap::FORMAT_LINEAR);
        if(program.name != NShader::PROGRAM_PER_FRAGMENT_NORMAL_ALPHA)
        {
          setSampler(depthMap, program.uniforms.depthTex, NShader::SAMPLER_PER_FRAGMENT_NORMAL_ALPHA_DEPTH, NMap::FORMAT_LINEAR | NMap::FORMAT_DEPTH | NMap::FORMAT_EDGE);
          setSampler(lpvMapR0, program.uniforms.lpvTexR0, NShader::SAMPLER_PER_FRAGMENT_NORMAL_ALPHA_LPV_R0, NMap::FORMAT_LINEAR | NMap::FORMAT_EDGE);
          setSampler(lpvMapG0, program.uniforms.lpvTexG0, NShader::SAMPLER_PER_FRAGMENT_NORMAL_ALPHA_LPV_G0, NMap::FORMAT_LINEAR | NMap::FORMAT_EDGE);
          setSampler(lpvMapB0, program.uniforms.lpvTexB0, NShader::SAMPLER_PER_FRAGMENT_NORMAL_ALPHA_LPV_B0, NMap::FORMAT_LINEAR | NMap::FORMAT_EDGE);
        }
      }
      else if(program.name == NShader::PROGRAM_GEOMETRY)
      {
        setSampler(m->diffuseMap, program.uniforms.difTex, NShader::SAMPLER_GEOMETRY_DIF, m->type & NModel::MATERIAL_MIP_MAPPING ? NMap::FORMAT_MIPMAP : NMap::FORMAT_LINEAR);
        setSampler(m->normalMap, program.uniforms.norTex, NShader::SAMPLER_GEOMETRY_NOR, m->type & NModel::MATERIAL_MIP_MAPPING ? NMap::FORMAT_MIPMAP : NMap::FORMAT_LINEAR);
      }

      gl->uniform1i(program.uniforms.type, m->type);
      gl->uniform1f(program.uniforms.opacity, m->opacity);

      context->getMaps()->finishBind();
    }

    if(program.name == NShader::PROGRAM_COLOR)
      gl->uniform3f(program.uniforms.lightAmb, technique->pickColor.x, technique->pickColor.y, technique->pickColor.z);
    else
      gl->uniform3f(program.uniforms.lightAmb, technique->lightAmb.x, technique->lightAmb.y, technique->lightAmb.z);

    if((program.name >= NShader::PROGRAM_PER_FRAGMENT) && (program.name <= NShader::PROGRAM_PER_FRAGMENT_NORMAL_ALPHA_SHADOW_JITTER))
    {
      const SCamera *cam = context->getCamera()->getCamera();

      gl->uniform3f(program.uniforms.depthTexelSize, 0.5f / static_cast<float>(depthMap->getMap()->width), 0.5f / static_cast<float>(depthMap->getMap()->height), context->engineGetEngine()->shadowJittering);

      gl->uniform3f(program.uniforms.lightPos, technique->lightPos.x, technique->lightPos.y, technique->lightPos.z);
      gl->uniform2f(program.uniforms.lightRange, technique->lightRange.x, technique->lightRange.y);
      gl->uniform3f(program.uniforms.lightColor, technique->lightColor.x, technique->lightColor.y, technique->lightColor.z);
      gl->uniform4f(program.uniforms.lightSpeColor, technique->lightSpeColor.x, technique->lightSpeColor.y, technique->lightSpeColor.z, technique->lightSpeColor.w);
      gl->uniform2f(program.uniforms.fogRange, technique->fogRange.x * cam->clipFar, technique->fogRange.y * cam->clipFar);
      gl->uniform3f(program.uniforms.fogColor, technique->fogColor.x, technique->fogColor.y, technique->fogColor.z);
    }
  }

  if(program.name >= NShader::PROGRAM_PER_FRAGMENT)
  {
    const glm::vec3 lpvCellSize(1.0f / e->lpvCellSize / e->lpvTextureSize);
    const glm::vec3 lpvPos(e->lpvCellSize * e->lpvTextureSize * 0.5f - e->lpvPos);
    gl->uniform2f(program.uniforms.fragSize, e->geometryTextureSize, e->geometryTextureSize);
    gl->uniform4f(program.uniforms.lpvPos, lpvPos.x, lpvPos.y, lpvPos.z, e->lpvIntensity * e->showLPV);
    gl->uniform3f(program.uniforms.lpvSize, e->lpvTextureSize.x, e->lpvTextureSize.y, e->lpvTextureSize.z);
    gl->uniform3f(program.uniforms.lpvCellSize, lpvCellSize.x, lpvCellSize.y, lpvCellSize.z);
  }

  if(program.name == NShader::PROGRAM_LPV_CLEAR)
  {
    const CMap *gvMap = context->getMaps()->getMap(NWindow::STR_GV_MAP_A0);

    setSampler(lpvMapR0, program.uniforms.lpvImgR0, NShader::SAMPLER_LPV_CLEAR_LPV_IMG_R0, NMap::FORMAT_IMAGE_W);
    setSampler(lpvMapG0, program.uniforms.lpvImgG0, NShader::SAMPLER_LPV_CLEAR_LPV_IMG_G0, NMap::FORMAT_IMAGE_W);
    setSampler(lpvMapB0, program.uniforms.lpvImgB0, NShader::SAMPLER_LPV_CLEAR_LPV_IMG_B0, NMap::FORMAT_IMAGE_W);
    setSampler(gvMap, program.uniforms.gvImgA0, NShader::SAMPLER_LPV_CLEAR_GV_IMG_A0, NMap::FORMAT_IMAGE_W);
  }
  else if(program.name == NShader::PROGRAM_LPV_INJECTION)
  {
    const CMap *fragColor = context->getMaps()->getMap(NWindow::STR_ORTHO_GEOMETRY_FBO_AMB_MAP);
    const CMap *fragPos = context->getMaps()->getMap(NWindow::STR_ORTHO_GEOMETRY_FBO_POS_MAP);
    const CMap *fragNormal = context->getMaps()->getMap(NWindow::STR_ORTHO_GEOMETRY_FBO_NORMAL_MAP);
    //const CMap *fragDepth = context->getMaps()->getMap(NWindow::STR_ORTHO_GEOMETRY_FBO_DEPTH_MAP);
    const CMap *gvMap = context->getMaps()->getMap(NWindow::STR_GV_MAP_A0);

    const glm::vec4 &fboGeoCam = context->getFramebuffers()->getFramebuffer(NWindow::STR_ORTHO_GEOMETRY_FBO)->getFrameBuffer()->camera.position;
    gl->uniform3f(program.uniforms.lightPos, fboGeoCam.x, fboGeoCam.y, fboGeoCam.z);

    setSampler(fragColor, program.uniforms.fragColorImg, NShader::SAMPLER_LPV_INJECTION_FRAG_COLOR_IMG, NMap::FORMAT_IMAGE_R);
    setSampler(fragPos, program.uniforms.fragPosImg, NShader::SAMPLER_LPV_INJECTION_FRAG_POS_IMG, NMap::FORMAT_IMAGE_R);
    setSampler(fragNormal, program.uniforms.fragNormalImg, NShader::SAMPLER_LPV_INJECTION_FRAG_NORMAL_IMG, NMap::FORMAT_IMAGE_R);
    //setSampler(fragDepth, program.uniforms.fragDepthImg, NShader::SAMPLER_LPV_INJECTION_FRAG_DEPTH_IMG, NMap::FORMAT_IMAGE_R);
    setSampler(lpvMapR0, program.uniforms.lpvImgR0, NShader::SAMPLER_LPV_INJECTION_LPV_IMG_R0, NMap::FORMAT_IMAGE_RW);
    setSampler(lpvMapG0, program.uniforms.lpvImgG0, NShader::SAMPLER_LPV_INJECTION_LPV_IMG_G0, NMap::FORMAT_IMAGE_RW);
    setSampler(lpvMapB0, program.uniforms.lpvImgB0, NShader::SAMPLER_LPV_INJECTION_LPV_IMG_B0, NMap::FORMAT_IMAGE_RW);
    setSampler(gvMap, program.uniforms.gvImgA0, NShader::SAMPLER_LPV_INJECTION_GV_IMG_A0, NMap::FORMAT_IMAGE_RW);
  }
  else if(program.name == NShader::PROGRAM_LPV_PROPAGATION)
  {
    const CMap *gvMap = context->getMaps()->getMap(NWindow::STR_GV_MAP_A0);

    setSampler(lpvMapR0, program.uniforms.lpvImgR0, NShader::SAMPLER_LPV_PROPAGATION_LPV_IMG_R0, NMap::FORMAT_IMAGE_RW);
    setSampler(lpvMapG0, program.uniforms.lpvImgG0, NShader::SAMPLER_LPV_PROPAGATION_LPV_IMG_G0, NMap::FORMAT_IMAGE_RW);
    setSampler(lpvMapB0, program.uniforms.lpvImgB0, NShader::SAMPLER_LPV_PROPAGATION_LPV_IMG_B0, NMap::FORMAT_IMAGE_RW);
    setSampler(gvMap, program.uniforms.gvImgA0, NShader::SAMPLER_LPV_PROPAGATION_GV_IMG_A0, NMap::FORMAT_IMAGE_RW);
  }
}
//------------------------------------------------------------------------------
void CShaderProgram::end(const SShaderTechnique *technique) const
{
  COpenGL *gl = context->getOpenGL();

  if((technique) && (technique->material))
  {
    if((program.name == NShader::PROGRAM_BASIC_ALPHA) ||
       (program.name == NShader::PROGRAM_PER_FRAGMENT_ALPHA) ||
       (program.name == NShader::PROGRAM_PER_FRAGMENT_NORMAL_ALPHA))
      glDisable(NOpenGL::BLEND);
  }

  if(program.name < NShader::PROGRAM_LPV_CLEAR)
  {
    if(program.name >= NShader::PROGRAM_COLOR)
      gl->disableVertexAttribArray(program.uniforms.vertexPosition);
    if(program.name >= NShader::PROGRAM_BASIC)
    {
      gl->disableVertexAttribArray(program.uniforms.vertexTexCoord);
      gl->disableVertexAttribArray(program.uniforms.vertexColor);
    }
    if(program.name >= NShader::PROGRAM_PER_FRAGMENT)
      gl->disableVertexAttribArray(program.uniforms.vertexNormal);
    if(program.name >= NShader::PROGRAM_PER_FRAGMENT_NORMAL)
    {
      gl->disableVertexAttribArray(program.uniforms.vertexNormalTangent);
      //gl->disableVertexAttribArray(program.uniforms.vertexNormalBitangent);
    }
  }

  if(program.name == NShader::PROGRAM_LPV_CLEAR)
  {
    setSampler(NULL, program.uniforms.lpvImgR0, NShader::SAMPLER_LPV_CLEAR_LPV_IMG_R0, NMap::FORMAT_IMAGE_W);
    setSampler(NULL, program.uniforms.lpvImgG0, NShader::SAMPLER_LPV_CLEAR_LPV_IMG_G0, NMap::FORMAT_IMAGE_W);
    setSampler(NULL, program.uniforms.lpvImgB0, NShader::SAMPLER_LPV_CLEAR_LPV_IMG_B0, NMap::FORMAT_IMAGE_W);
    setSampler(NULL, program.uniforms.gvImgA0, NShader::SAMPLER_LPV_CLEAR_GV_IMG_A0, NMap::FORMAT_IMAGE_W);
  }
  else if(program.name == NShader::PROGRAM_LPV_INJECTION)
  {
    setSampler(NULL, program.uniforms.fragColorImg, NShader::SAMPLER_LPV_INJECTION_FRAG_COLOR_IMG, NMap::FORMAT_IMAGE_R);
    setSampler(NULL, program.uniforms.fragPosImg, NShader::SAMPLER_LPV_INJECTION_FRAG_POS_IMG, NMap::FORMAT_IMAGE_R);
    setSampler(NULL, program.uniforms.fragNormalImg, NShader::SAMPLER_LPV_INJECTION_FRAG_NORMAL_IMG, NMap::FORMAT_IMAGE_R);
    setSampler(NULL, program.uniforms.fragDepthImg, NShader::SAMPLER_LPV_INJECTION_FRAG_DEPTH_IMG, NMap::FORMAT_IMAGE_R);
    setSampler(NULL, program.uniforms.lpvImgR0, NShader::SAMPLER_LPV_INJECTION_LPV_IMG_R0, NMap::FORMAT_IMAGE_RW);
    setSampler(NULL, program.uniforms.lpvImgG0, NShader::SAMPLER_LPV_INJECTION_LPV_IMG_G0, NMap::FORMAT_IMAGE_RW);
    setSampler(NULL, program.uniforms.lpvImgB0, NShader::SAMPLER_LPV_INJECTION_LPV_IMG_B0, NMap::FORMAT_IMAGE_RW);
    setSampler(NULL, program.uniforms.gvImgA0, NShader::SAMPLER_LPV_INJECTION_GV_IMG_A0, NMap::FORMAT_IMAGE_RW);
  }
  else if(program.name == NShader::PROGRAM_LPV_PROPAGATION)
  {
    setSampler(NULL, program.uniforms.lpvImgR0, NShader::SAMPLER_LPV_PROPAGATION_LPV_IMG_R0, NMap::FORMAT_IMAGE_RW);
    setSampler(NULL, program.uniforms.lpvImgG0, NShader::SAMPLER_LPV_PROPAGATION_LPV_IMG_G0, NMap::FORMAT_IMAGE_RW);
    setSampler(NULL, program.uniforms.lpvImgB0, NShader::SAMPLER_LPV_PROPAGATION_LPV_IMG_B0, NMap::FORMAT_IMAGE_RW);
    setSampler(NULL, program.uniforms.gvImgA0, NShader::SAMPLER_LPV_PROPAGATION_GV_IMG_A0, NMap::FORMAT_IMAGE_RW);
  }
}
//------------------------------------------------------------------------------
void CShaderProgram::unbind() const
{
  context->getOpenGL()->useProgram(0);
}
//------------------------------------------------------------------------------
void CShaderProgram::dispatch(uint32 x, uint32 y, uint32 z, NRenderer::EMode mode, GLbitfield preSync, GLbitfield postSync) const
{
  COpenGL *gl = context->getOpenGL();

  if(preSync)
    gl->memoryBarrier(NOpenGL::ALL_BARRIER_BITS);

  bind();
  begin(NULL, mode);
  gl->dispatchCompute(x, y, z);
  end(NULL);
  unbind();

  if(postSync)
    gl->memoryBarrier(NOpenGL::ALL_BARRIER_BITS);
}
//------------------------------------------------------------------------------
CShaders::CShaders() : CEngineBase()
{
}
//------------------------------------------------------------------------------
CShaders::CShaders(CContext *context) : CEngineBase(context)
{
}
//------------------------------------------------------------------------------
CShaders::~CShaders()
{
}
//------------------------------------------------------------------------------
