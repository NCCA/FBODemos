#ifndef FRAMEBUFFEROBJECT_H_
#define FRAMEBUFFEROBJECT_H_
#include <ngl/Types.h>
#include <ngl/Vec2.h>
#include <string>
#include <unordered_map>
#include <memory>
#include "TextureTypes.h"
//----------------------------------------------------------------------------------------------------------------------
// The FrameBufferObject class encapsulates an OpenGL FrameBufferObject and the associated textures
// It is also possible to attach textures created elsewhere to the FBO and either a Depth or
// Depth Stencil. This class is designed to be as Flexible as Possible but can not deal with all
// possible circumstances so it may still be necessary to create your own Framebuffers
//
//----------------------------------------------------------------------------------------------------------------------

class FrameBufferObject
{
  public :

    //----------------------------------------------------------------------------------------------------------------------
    // decide what the FBO Target is we will default where possible to GL_FRAMEBUFFER
    //----------------------------------------------------------------------------------------------------------------------
    enum class Target : GLenum {FRAMEBUFFER=GL_FRAMEBUFFER,DRAW=GL_DRAW_FRAMEBUFFER,READ=GL_READ_FRAMEBUFFER};
    //----------------------------------------------------------------------------------------------------------------------
    // we can only create on the heap as a smart pointer
    // @param _w the width of the Framebuffer
    // @param _h the height of the Framebuffer
    // @param number of Colour attchments 8 is the min number (spec mandates) so will use this as
    // default.
    //----------------------------------------------------------------------------------------------------------------------
    static std::unique_ptr<FrameBufferObject> create(int _w, int _h, size_t _numAttatchments=8) noexcept;
    //----------------------------------------------------------------------------------------------------------------------
    // make non-copyable using rule of 5
    //----------------------------------------------------------------------------------------------------------------------
    FrameBufferObject(const FrameBufferObject &)=delete;
    FrameBufferObject & operator=(const FrameBufferObject &)=delete;
    FrameBufferObject(FrameBufferObject &&)=delete;
    FrameBufferObject & operator=(FrameBufferObject &&)=delete;
    //----------------------------------------------------------------------------------------------------------------------
    // Dtor will release all textures associated with the FBO so be careful if sharing them.
    //----------------------------------------------------------------------------------------------------------------------
    ~FrameBufferObject() noexcept;
    //----------------------------------------------------------------------------------------------------------------------
    // this will add a depth buffer using a texture attachment this will always use an internal float buffer
    /// @param _format the format for the depth component (see  GLTextureDepthFormats for enumerations to GL types)
    /// @param _min the minificaction filter mode of the texture attached
    /// @param _mag the magnification mode of the texture attached
    /// @param _swrap texture wrap mode in s
    /// @param _twrap texture wrap mode in t
    /// @param _immutable Choose to use either faster immutable storage (glTexStorage) or slower mutable
    /// storage, glTexImage2d
    //----------------------------------------------------------------------------------------------------------------------
    bool addDepthBuffer(GLTextureDepthFormats _format,  GLTextureMinFilter _min,
                        GLTextureMagFilter _mag,GLTextureWrap _swrap,
                        GLTextureWrap _twrap, bool _immutable=false) noexcept;
    //----------------------------------------------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------------------------------
    bool addDepthStencilBuffer() noexcept;
    //----------------------------------------------------------------------------------------------------------------------
    /// add a colour attatchment to the FBO,
    /// @param _name name of the attachment so we can refer to it by name in code
    /// @param _attachment the point where the texture is to be attached
    /// @param _format the format of the texture
    /// @param _iformat the internal format of the texture
    /// @param _min minification filter
    /// @param _mag the magnification filter
    /// @param _swrap texture wrap mode in the s
    /// @param _twrap texture wrap mode in the t
    /// @param _immutable Choose to use either faster immutable storage (glTexStorage) or slower mutable
    /// storage, glTexImage2d
    //----------------------------------------------------------------------------------------------------------------------
    bool addColourAttachment(const std::string &_name, GLAttatchment _attachment,
                             GLTextureFormat _format, GLTextureInternalFormat _iformat, GLTextureDataType _type,
                             GLTextureMinFilter _min, GLTextureMagFilter _mag,
                             GLTextureWrap _swrap, GLTextureWrap _twrap,
                             bool _immutable=false) noexcept;
    //----------------------------------------------------------------------------------------------------------------------
    /// bind this FBO as the active one
    /// @param _target default to FRAMEBUFFER but can be bound for reading or writing
    //----------------------------------------------------------------------------------------------------------------------
    void bind(Target _target=Target::FRAMEBUFFER) noexcept;
    //----------------------------------------------------------------------------------------------------------------------
    /// unbind the FBO (defaults to FBO 0 but may need to use defaultFramebufferObject in Qt
    //----------------------------------------------------------------------------------------------------------------------
    void unbind() noexcept;
    //----------------------------------------------------------------------------------------------------------------------
    /// get the texture ID of bound attachment by name
    /// @param _name the name of the texture attachment
    /// @returns The texture ID of texture if found else 0 (default GL texture object)
    //----------------------------------------------------------------------------------------------------------------------
    GLuint getTextureID(const std::string &_name) noexcept;

    static void copyFrameBufferTexture(GLuint _srcID, GLuint _dstID, GLuint _width, GLuint _height, GLenum _mode=GL_COLOR_BUFFER_BIT) noexcept;
    //----------------------------------------------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------------------------------
    bool bindToSampler(const std::string &_name,GLuint _location) noexcept;
    //----------------------------------------------------------------------------------------------------------------------
    /// print debug info
    //----------------------------------------------------------------------------------------------------------------------
    void print() const noexcept;
    //----------------------------------------------------------------------------------------------------------------------
    /// check is framebuffer is complete
    /// @param _target which framebuffer mode we use
    /// @returns true if complete else false
    //----------------------------------------------------------------------------------------------------------------------
    bool isComplete(Target _target=Target::FRAMEBUFFER) noexcept;
    //----------------------------------------------------------------------------------------------------------------------
    /// get the underlying ID of the FBO so we can bind and use our own GL commands etc.
    //----------------------------------------------------------------------------------------------------------------------
    GLuint getID() noexcept;
    //----------------------------------------------------------------------------------------------------------------------
    /// as we can only have one depth attachment we store this as a single value
    /// @returns the texture ID of the depth buffer
    //----------------------------------------------------------------------------------------------------------------------
    GLuint getDepthTextureID()const noexcept{return  m_depthBufferID;}
    //----------------------------------------------------------------------------------------------------------------------
    /// calls glViewport using the FBO dimensions, useful when drawing
    //----------------------------------------------------------------------------------------------------------------------
    void setViewport() const noexcept;
    //----------------------------------------------------------------------------------------------------------------------
    ngl::Vec2 getSize() const noexcept {return ngl::Vec2(m_width,m_height);}
    float width()  const  noexcept {return m_width;}
    float height() const noexcept {return m_height;}
    //----------------------------------------------------------------------------------------------------------------------
    static void setDefaultFBO(GLuint _id){s_defaultFBO=_id;}
  private :
    //----------------------------------------------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------------------------------
    GLuint m_id;
    //----------------------------------------------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------------------------------
    FrameBufferObject(int _w, int _h, size_t _numAttatchments) noexcept;
    //----------------------------------------------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------------------------------
    struct TextureAttachment
    {
        GLuint id=0;
        std::string name;
        GLuint samplerLocation;
    };

    //----------------------------------------------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------------------------------
    int m_width; /// width of buffer (and usually textures)
    //----------------------------------------------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------------------------------
    int m_height; /// height of buffer (and usually textures)
    //----------------------------------------------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------------------------------
    std::vector<TextureAttachment>m_attachments;
    //----------------------------------------------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------------------------------
    GLuint m_depthBufferID;
    //----------------------------------------------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------------------------------
    bool m_bound=false;
    //----------------------------------------------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------------------------------
    size_t m_attachmentSize=8;
    static GLuint s_copyFBO;
    static GLuint s_defaultFBO;
};

#endif
