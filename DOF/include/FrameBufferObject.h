#ifndef FRAMEBUFFEROBJECT_H_
#define FRAMEBUFFEROBJECT_H_
#include <ngl/Types.h>
#include <string>
#include <unordered_map>
#include <memory>
#include "TextureTypes.h"

/* The FrameBufferObject class encapsulates an OpenGL FrameBufferObject and the associated textures
 * It is also possible to attach textures created elsewhere to the FBO and either a Depth or
 * Depth Stencil. This class is designed to be as Flexible as Possible but can not deal with all
 * possible circumstances so it may still be necessary to create your own Framebuffers
 */
class FrameBufferObject
{
  public :
    enum class Target : GLenum {FRAMEBUFFER=GL_FRAMEBUFFER,DRAW=GL_DRAW_FRAMEBUFFER,READ=GL_READ_FRAMEBUFFER};
    // we can only create on the heap as a smart pointer
    // @param _w the width of the Framebuffer
    // @param _h the height of the Framebuffer
    // @param number of Colour attchments 8 is the min number
    static std::unique_ptr<FrameBufferObject> create(int _w, int _h, size_t _numAttatchments=8);
    // make non-copyable
    FrameBufferObject(const FrameBufferObject &)=delete;
    FrameBufferObject & operator=(const FrameBufferObject &)=delete;
    FrameBufferObject(FrameBufferObject &&)=delete;
    FrameBufferObject & operator=(FrameBufferObject &&)=delete;
    ~FrameBufferObject();
    bool addDepthBuffer(GLTextureDepthFormats _format, GLTextureDataType _type, GLTextureMinFilter _min,
                        GLTextureMagFilter _mag,GLTextureWrap _swap, GLTextureWrap _twrap, bool _immutable=false);
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
    FrameBufferObject(int _w, int _h, size_t _numAttatchments);
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
