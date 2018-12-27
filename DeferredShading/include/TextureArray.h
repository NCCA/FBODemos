#ifndef TEXTUREARRAY_H_
#define TEXTUREARRAY_H_
#include <ngl/Types.h>
#include <unordered_map>
#include <string>
#include <vector>

class TextureArray
{
  public :
    TextureArray()=default;
    ~TextureArray()=default;
    TextureArray(std::string _name, std::vector<std::string> &_textures, int _width, int _height);
    static GLuint getID(const std::string &_name);
  private :
      static std::unordered_map<std::string,GLuint> m_arrays;

};


#endif
