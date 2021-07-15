#ifndef __ISINGLETON_H_
#define __ISINGLETON_H_

template<typename _T>
class ISingleton
{
protected:
  ISingleton(void) {};
  virtual ~ISingleton(void) {};

protected:
  static _T* m_pInstance;

public:
  inline static _T* getInstance()
  {
    if (m_pInstance == NULL)
    {
      m_pInstance = new _T;
    }

    return m_pInstance;
  }
};

template <typename _T>
_T* ISingleton<_T>::m_pInstance = NULL;

#endif
