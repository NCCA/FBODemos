#ifndef SCOPEDBIND_H_
#define SCOPEDBIND_H_

template<class T>
class ScopedBind
{
public:
  ScopedBind(T* ptr)
    : m_ptr(ptr)
  {
    m_ptr->bind();
  }
  ~ScopedBind()
  {
    if(m_ptr !=nullptr)
      m_ptr->unbind();
  }


private:
  T* m_ptr;
};

#endif
