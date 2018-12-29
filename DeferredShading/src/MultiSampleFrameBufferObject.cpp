#include "MultiSampleFrameBufferObject.h"


std::unique_ptr<MultisampleFrameBufferObject> MultisampleFrameBufferObject::create(int _w, int _h, size_t _numAttatchments) noexcept
{
  // note can't use make_unique here as private ctor being invoked.
  return std::unique_ptr<MultisampleFrameBufferObject>(new MultisampleFrameBufferObject(_w,_h,_numAttatchments));
}



MultisampleFrameBufferObject::MultisampleFrameBufferObject(int _w, int _h, size_t _numAttatchments) noexcept
  : FrameBufferObject (_w,_h,_numAttatchments)
{

}


bool MultisampleFrameBufferObject::addColourAttachment(const std::string &_name, GLAttatchment _attachment,
                              GLTextureFormat _format, GLTextureInternalFormat _iformat,
                              GLTextureDataType _type, GLTextureMinFilter _min, GLTextureMagFilter _mag,
                              GLTextureWrap _swrap, GLTextureWrap _twrap, bool _immutable) noexcept
{
  if(m_bound!=true)
  {
    ngl::msg->addError("Trying to add attachment to unboud Framebuffer");
    return false;
  }

  GLuint id;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, id);
  // set params
  glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, toGLType(_mag));
  glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, toGLType(_min));
  glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, toGLType(_swrap));
  glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, toGLType(_twrap));
  if(_immutable ==true)
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, toGLType(_iformat), m_width, m_height,true);
  else
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4,static_cast<int>(toGLType(_iformat)),m_width, m_height, true);
  glFramebufferTexture2D(GL_FRAMEBUFFER,toGLType(_attachment), GL_TEXTURE_2D_MULTISAMPLE, id, 0);
  ngl::msg->addMessage(fmt::format("Adding Texture {0} {1}",_name,id));
  TextureAttachment t;
  t.id=id;
  t.name=_name;
  // convert enum to size_t for vector index.
  size_t index=toGLType(_attachment)-GL_COLOR_ATTACHMENT0;
  m_attachments[index]=t;
  return true;


}

bool MultisampleFrameBufferObject::addDepthBuffer(GLTextureDepthFormats _format, GLTextureMinFilter _min,
                          GLTextureMagFilter _mag, GLTextureWrap _swrap, GLTextureWrap _twrap, bool _immutable) noexcept
{
  if(m_bound!=true)
  {
    ngl::msg->addError("Trying to add depthbuffer to unboud Framebuffer");
    return false;
  }
  glGenTextures(1, &m_depthBufferID);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_depthBufferID);
  // set params
  glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, toGLType(_mag));
  glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, toGLType(_min));
  glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, toGLType(_swrap));
  glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, toGLType(_twrap));
  if(_immutable == true)
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, toGLType(_format), m_width, m_height,true);
  else
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4,static_cast<int>(toGLType(_format)),m_width, m_height, true);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, m_depthBufferID, 0);
  return true;
}




MultisampleFrameBufferObject::~MultisampleFrameBufferObject() noexcept
{

}
