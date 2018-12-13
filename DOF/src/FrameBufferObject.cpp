#include "FrameBufferObject.h"
#include <ngl/NGLInit.h>
#include <iostream>
#include <algorithm>

std::unique_ptr<FrameBufferObject> FrameBufferObject::create(int _w, int _h, size_t _numAttatchments)
{
  // note can't use make_unique here as private ctor being invoked.
  return std::unique_ptr<FrameBufferObject>(new FrameBufferObject(_w,_h,_numAttatchments));
}

void FrameBufferObject::setViewport() const
{
  glViewport(0, 0, m_width, m_height);

}
FrameBufferObject::FrameBufferObject(int _w, int _h, size_t _numAttatchments) : m_width(_w),m_height(_h)
{
  glGenFramebuffers(1, &m_id);
  m_attachments.resize(_numAttatchments);
}
FrameBufferObject::~FrameBufferObject()
{
  glDeleteFramebuffers(1,&m_id);
  for(auto t : m_attachments)
  {
    if(t.id !=0)
      glDeleteTextures(1,&t.id);
  }
}

bool FrameBufferObject::addDepthBuffer(GLTextureDepthFormats _format, GLTextureMinFilter _min,
                          GLTextureMagFilter _mag, GLTextureWrap _swrap, GLTextureWrap _twrap, bool _immutable)
{
  if(m_bound!=true)
  {
    ngl::msg->addError("Trying to add depthbuffer to unboud Framebuffer");
    return false;
  }
  glGenTextures(1, &m_depthBufferID);
  glBindTexture(GL_TEXTURE_2D, m_depthBufferID);
  // set params
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, toGLType(_mag));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, toGLType(_min));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, toGLType(_swrap));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, toGLType(_twrap));
  if(_immutable == true)
    glTexStorage2D(GL_TEXTURE_2D, 1, toGLType(_format), m_width, m_height);
  else
    glTexImage2D(GL_TEXTURE_2D, 0, toGLType(_format), m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthBufferID, 0);
  return true;
}
bool FrameBufferObject::addColourAttachment(const std::string &_name, GLenum _attachment,
                              GLTextureFormat _format, GLTextureInternalFormat _iformat,
                              GLTextureDataType _type, GLTextureMinFilter _min, GLTextureMagFilter _mag,
                              GLTextureWrap _swap, GLTextureWrap _twrap, bool _immutable)
{
  if(m_bound!=true)
  {
    ngl::msg->addError("Trying to add attachment to unboud Framebuffer");
    return false;
  }

  GLuint id;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);
//  // set params
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, toGLType(_mag));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, toGLType(_min));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, toGLType(_swap));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, toGLType(_twrap));
  if(_immutable ==true)
    glTexStorage2D(GL_TEXTURE_2D, 1, toGLType(_iformat), m_width, m_height);
  else
    glTexImage2D(GL_TEXTURE_2D, 0, toGLType(_iformat),m_width, m_height, 0, toGLType(_format), toGLType(_type), nullptr);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+_attachment, GL_TEXTURE_2D, id, 0);
  ngl::msg->addMessage(fmt::format("Adding Texture {0} {1}",_name,id));
  TextureAttachment t;
  t.id=id;
  t.name=_name;
  m_attachments[_attachment]=t;
  return true;


}
void FrameBufferObject::bind(Target _target)
{
  glBindFramebuffer(toGLType(_target), m_id);
  m_bound=true;
}
void FrameBufferObject::unbind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  m_bound=false;
}
GLuint FrameBufferObject::getTextureID(const std::string &_name)
{
  GLuint id=0;
  auto it=std::find_if(std::begin(m_attachments),std::end(m_attachments),[_name](TextureAttachment _t)
        {
          return _t.name==_name;
        }
      );
  if(it !=m_attachments.end())
  {
    id= it->id;
  }
  return id;
}
bool FrameBufferObject::bindToSampler(const std::string &_name,GLuint _location)
{

}
void FrameBufferObject::print() const
{
  if(m_bound !=true)
  {
    ngl::msg->addError("Trying to print unbound FrameBufferObject\n");
    return;
  }
  int res;
  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &res);
  ngl::msg->addMessage(fmt::format("Max Color Attachments: {0}\n",res),Colours::YELLOW,TimeFormat::NONE);

  glBindFramebuffer(GL_FRAMEBUFFER,m_id);

  GLint isBuffer;
  GLenum i = 0;
  do
  {
    glGetIntegerv(GL_DRAW_BUFFER0+i, &isBuffer);

    if (isBuffer != GL_NONE)
    {
      ngl::msg->addMessage(fmt::format("{0} Shader Output Location {1} - color attachment {2}",m_attachments[i].name,i,isBuffer),Colours::YELLOW,TimeFormat::NONE);
      glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, static_cast<GLenum>(isBuffer), GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &res);
      ngl::msg->addMessage(fmt::format("\tAttachment Type : {0} ",(res==GL_TEXTURE? "Texture":"Render Buffer")),Colours::YELLOW,TimeFormat::NONE);

      glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, static_cast<GLenum>(isBuffer), GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &res);
      ngl::msg->addMessage(fmt::format("\tAttachment object name :  {0} ",res),Colours::YELLOW,TimeFormat::NONE);
    }
    ++i;

  } while (isBuffer != GL_NONE);

}

bool FrameBufferObject::isComplete(Target _target)
{
  auto result=glCheckFramebufferStatus(toGLType(_target));
  return result==GL_FRAMEBUFFER_COMPLETE ? true : false;
}
GLuint FrameBufferObject::getID()
{
  return m_id;
}
