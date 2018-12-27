#include "TextureArray.h"
#include <ngl/Image.h>
#include <iostream>
std::unordered_map<std::string,GLuint> TextureArray::m_arrays;


TextureArray::TextureArray(std::string _name,std::vector<std::string> &_textures, int _width, int _height)
{
  GLuint id;
  glGenTextures(1,&id);
  glBindTexture(GL_TEXTURE_2D_ARRAY,id);
  int layerCount=_textures.size();
  glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGB8, _width, _height, layerCount);
  std::vector<ngl::Image> images;

  std::unique_ptr <unsigned char[] > data;

  for(auto t : _textures)
  {
    images.push_back(ngl::Image(t));
  }

  data=std::make_unique<unsigned char[]>(images[0].width()*
                                         images[0].height()*
                                         images[0].channels() *
                                         images.size());
  size_t offset=0;
  size_t dataSize=images[0].width()*images[0].height()*images[0].channels();
  for(size_t i=0; i<images.size(); ++i)
  {
    std::memcpy(&data[offset],images[i].getPixels(),dataSize);
    offset+=dataSize;
  }


    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, _width, _height, layerCount, GL_RGB, GL_UNSIGNED_BYTE, &data[0]);

    glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);


  m_arrays[_name]=id;


}
GLuint TextureArray::getID(const std::string &_name)
{
  auto it=m_arrays.find(_name);
  if(it !=m_arrays.end())
  {
    return it->second;
  }

  else return 0;
}
