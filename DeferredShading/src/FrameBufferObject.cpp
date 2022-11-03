#include "FrameBufferObject.h"
#include <ngl/NGLInit.h>
#include <iostream>
#include <algorithm>

GLuint FrameBufferObject::s_copyFBO = 0;
GLuint FrameBufferObject::s_defaultFBO = 0;
std::unique_ptr<FrameBufferObject> FrameBufferObject::create(int _w, int _h, size_t _numAttatchments) noexcept
{
  // note can't use make_unique here as private ctor being invoked.
  return std::unique_ptr<FrameBufferObject>(new FrameBufferObject(_w, _h, _numAttatchments));
}

void FrameBufferObject::setViewport() const noexcept
{
  glViewport(0, 0, m_width, m_height);
}
FrameBufferObject::FrameBufferObject(int _w, int _h, size_t _numAttatchments) noexcept : m_width(_w), m_height(_h)
{
  glGenFramebuffers(1, &m_id);
  m_attachments.resize(_numAttatchments);
}
FrameBufferObject::~FrameBufferObject() noexcept
{
  glDeleteFramebuffers(1, &m_id);
  for (auto t : m_attachments)
  {
    if (t.id != 0)
      glDeleteTextures(1, &t.id);
  }
}

bool FrameBufferObject::addDepthBuffer(GLTextureDepthFormats _format, GLTextureMinFilter _min,
                                       GLTextureMagFilter _mag, GLTextureWrap _swrap, GLTextureWrap _twrap, bool _immutable) noexcept
{
  if (m_bound != true)
  {
    ngl::NGLMessage::addError("Trying to add depthbuffer to unboud Framebuffer");
    return false;
  }
  glGenTextures(1, &m_depthBufferID);
  glBindTexture(GL_TEXTURE_2D, m_depthBufferID);
  // set params
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, toGLType(_mag));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, toGLType(_min));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, toGLType(_swrap));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, toGLType(_twrap));
  if (_immutable == true)
    glTexStorage2D(GL_TEXTURE_2D, 1, toGLType(_format), m_width, m_height);
  else
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<int>(toGLType(_format)), m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthBufferID, 0);
  return true;
}
bool FrameBufferObject::addColourAttachment(const std::string &_name, GLAttatchment _attachment,
                                            GLTextureFormat _format, GLTextureInternalFormat _iformat,
                                            GLTextureDataType _type, GLTextureMinFilter _min, GLTextureMagFilter _mag,
                                            GLTextureWrap _swrap, GLTextureWrap _twrap, bool _immutable) noexcept
{
  if (m_bound != true)
  {
    ngl::NGLMessage::addError("Trying to add attachment to unboud Framebuffer");
    return false;
  }

  GLuint id;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);
  // set params
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, toGLType(_mag));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, toGLType(_min));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, toGLType(_swrap));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, toGLType(_twrap));
  if (_immutable == true)
    glTexStorage2D(GL_TEXTURE_2D, 1, toGLType(_iformat), m_width, m_height);
  else
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<int>(toGLType(_iformat)), m_width, m_height, 0, toGLType(_format), toGLType(_type), nullptr);
  glFramebufferTexture2D(GL_FRAMEBUFFER, toGLType(_attachment), GL_TEXTURE_2D, id, 0);
  ngl::NGLMessage::addMessage(fmt::format("Adding Texture {0} {1}", _name, id));
  TextureAttachment t;
  t.id = id;
  t.name = _name;
  // convert enum to size_t for vector index.
  size_t index = toGLType(_attachment) - GL_COLOR_ATTACHMENT0;
  m_attachments[index] = t;
  return true;
}
void FrameBufferObject::bind(Target _target) noexcept
{
  glBindFramebuffer(toGLType(_target), m_id);
  m_bound = true;
}
void FrameBufferObject::unbind() noexcept
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  m_bound = false;
}
GLuint FrameBufferObject::getTextureID(const std::string &_name) noexcept
{
  GLuint id = 0;
  auto it = std::find_if(std::begin(m_attachments), std::end(m_attachments), [_name](TextureAttachment _t)
                         { return _t.name == _name; });
  if (it != m_attachments.end())
  {
    id = it->id;
  }
  return id;
}
bool FrameBufferObject::bindToSampler(const std::string &_name, GLuint _location) noexcept
{
  return false;
}
void FrameBufferObject::print() const noexcept
{
  if (m_bound != true)
  {
    ngl::NGLMessage::addError("Trying to print unbound FrameBufferObject\n");
    return;
  }
  int res;
  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &res);
  ngl::NGLMessage::addMessage(fmt::format("Max Color Attachments: {0}\n", res), ngl::Colours::YELLOW, ngl::TimeFormat::NONE);

  glBindFramebuffer(GL_FRAMEBUFFER, m_id);

  GLint isBuffer;
  GLenum i = 0;
  do
  {
    glGetIntegerv(GL_DRAW_BUFFER0 + i, &isBuffer);

    if (isBuffer != GL_NONE)
    {
      ngl::NGLMessage::addMessage(fmt::format("{0} Shader Output Location {1} - color attachment {2}", m_attachments[i].name, i, isBuffer), ngl::Colours::YELLOW, ngl::TimeFormat::NONE);
      glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, static_cast<GLenum>(isBuffer), GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &res);
      ngl::NGLMessage::addMessage(fmt::format("\tAttachment Type : {0} ", (res == GL_TEXTURE ? "Texture" : "Render Buffer")), ngl::Colours::YELLOW, ngl::TimeFormat::NONE);

      glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, static_cast<GLenum>(isBuffer), GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &res);
      ngl::NGLMessage::addMessage(fmt::format("\tAttachment object name :  {0} ", res), ngl::Colours::YELLOW, ngl::TimeFormat::NONE);
    }
    ++i;

  } while (isBuffer != GL_NONE);
}

void FrameBufferObject::copyFrameBufferTexture(GLuint _srcID, GLuint _dstID, GLuint _width, GLuint _height, GLenum _mode) noexcept
{
  // glCopyImageSubData(_srcID,GL_TEXTURE_BUFFER,1,0,0,0, _dstID,GL_TEXTURE_BUFFER,1,0,0,0, _width,height,0);
  if (s_copyFBO == 0)
    glGenFramebuffers(1, &s_copyFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, s_copyFBO);
  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, _srcID, 0);
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
                         GL_TEXTURE_2D, _dstID, 0);
  glDrawBuffer(GL_COLOR_ATTACHMENT1);
  glBlitFramebuffer(0, 0, _width, _height, 0, 0, _width, _height,
                    _mode, GL_NEAREST);

  glBindFramebuffer(GL_FRAMEBUFFER, s_defaultFBO);
}

bool FrameBufferObject::isComplete(Target _target) noexcept
{
  auto result = glCheckFramebufferStatus(toGLType(_target));
  return result == GL_FRAMEBUFFER_COMPLETE ? true : false;
}
GLuint FrameBufferObject::getID() noexcept
{
  return m_id;
}
