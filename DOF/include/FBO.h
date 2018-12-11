#ifndef FBO_H_
#define FBO_H_
#include <ngl/Types.h>
#include <string>
#include <unordered_map>
#include <memory>
#include "TextureTypes.h"


class FBO
{
  public :
    enum class Target : GLenum {FRAMEBUFFER=GL_FRAMEBUFFER,DRAW=GL_DRAW_FRAMEBUFFER,READ=GL_READ_FRAMEBUFFER};
    // we can only create on the heap as a smart pointer
    // @param _w the width of the Framebuffer
    // @param _h the height of the Framebuffer
    // @param number of Colour attchments 8 is the min number
    static std::unique_ptr<FBO> create(int _w, int _h, size_t _numAttatchments=8);
    // make non-copyable
    FBO(const FBO &)=delete;
    FBO & operator=(const FBO &)=delete;
    FBO(FBO &&)=delete;
    FBO & operator=(FBO &&)=delete;
    ~FBO();
    bool addDepthBuffer(GLTextureDepthFormats _format, GLTextureDataType _type, GLTextureMinFilter _min,
                        GLTextureMagFilter _mag,GLTextureWrap _swap, GLTextureWrap _twrap);
    bool addDepthStencilBuffer();
    bool addColourAttachment(const std::string &_name, GLenum _attachment,
                             GLTextureFormat _format, GLTextureInternalFormat _iformat, GLTextureDataType _type,
                             GLTextureMinFilter _min, GLTextureMagFilter _mag,
                             GLTextureWrap _swap, GLTextureWrap _twrap);
    void bind(Target _target=Target::FRAMEBUFFER);
    void unbind();
    GLuint getTextureID(const std::string &_name);
    bool bindToSampler(const std::string &_name,GLuint _location);
    void print() const;
    bool isComplete(Target _target=Target::FRAMEBUFFER);
    GLuint getID();
    GLuint getDepthTextureID(){return  m_depthBufferID;}
    void setViewport() const;

  private :
    GLuint m_id;
    FBO(int _w, int _h, size_t _numAttatchments);
    struct TextureAttachment
    {
        GLuint id=0;
        std::string name;
        GLuint samplerLocation;
    };

    int m_width; /// width of buffer (and usually textures)
    int m_height; /// height of buffer (and usually textures)
    std::vector<TextureAttachment>m_attachments;
    GLuint m_depthBufferID;
    bool m_bound=false;
    size_t m_attachmentSize=8;
};

#endif
