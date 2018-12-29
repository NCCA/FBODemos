#ifndef MULTISAMPLEFRAMEBUFFEROBJECT_H_
#define MULTISAMPLEFRAMEBUFFEROBJECT_H_
#include "FrameBufferObject.h"

class MultisampleFrameBufferObject : public FrameBufferObject
{
  public :
    static std::unique_ptr<MultisampleFrameBufferObject> create(int _w, int _h, size_t _numAttatchments=8) noexcept;
    //----------------------------------------------------------------------------------------------------------------------
    // make non-copyable using rule of 5
    //----------------------------------------------------------------------------------------------------------------------
    MultisampleFrameBufferObject(const MultisampleFrameBufferObject &)=delete;
    MultisampleFrameBufferObject & operator=(const MultisampleFrameBufferObject &)=delete;
    MultisampleFrameBufferObject(MultisampleFrameBufferObject &&)=delete;
    MultisampleFrameBufferObject & operator=(MultisampleFrameBufferObject &&)=delete;
    //----------------------------------------------------------------------------------------------------------------------
    // Dtor will release all textures associated with the FBO so be careful if sharing them.
    //----------------------------------------------------------------------------------------------------------------------
    virtual ~MultisampleFrameBufferObject() noexcept;
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
                             bool _immutable=false)  noexcept override;

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
    virtual bool addDepthBuffer(GLTextureDepthFormats _format,  GLTextureMinFilter _min,
                        GLTextureMagFilter _mag,GLTextureWrap _swrap,
                        GLTextureWrap _twrap, bool _immutable=false)  noexcept override;


  private :

    MultisampleFrameBufferObject(int _w, int _h, size_t _numAttatchments) noexcept;

};


#endif
