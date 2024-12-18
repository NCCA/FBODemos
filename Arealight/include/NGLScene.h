#ifndef NGLSCENE_H_
#define NGLSCENE_H_
#include <ngl/Text.h>
#include <ngl/Vec3.h>
#include <ngl/Vec4.h>
#include <ngl/Mat4.h>
#include "WindowParams.h"
#include <QOpenGLWindow>

/// @file NGLScene.h
/// @brief this class inherits from the Qt OpenGLWindow and allows us to use NGL to draw OpenGL
/// @author Jonathan Macey
/// @version 1.0
/// @date 10/9/13
/// Revision History :
/// This is an initial version used for the new NGL6 / Qt 5 demos
/// @class NGLScene
/// @brief our main glwindow widget for NGL applications all drawing elements are
/// put in this file


class NGLScene : public QOpenGLWindow
{
  Q_OBJECT
public:
  
  /// @brief ctor for our NGL drawing class
  /// @param [in] parent the parent window to the class
  
  NGLScene();
  
  /// @brief dtor must close down ngl and release OpenGL resources
  
  ~NGLScene() override;
  /// @brief the initialize class is called once when the window is created and we have a valid GL context
  /// use this to setup any default GL stuff
  void initializeGL() override;
  /// @brief this is called everytime we want to draw the scene
  void paintGL() override;
  
  /// @brief this is called everytime we want to draw the scene
  
  void resizeGL(int _w, int _h) override;

private:
  
  /// @brief the windows params such as mouse and rotations etc
  
  WinParams m_win;
  
  /// @brief Qt Event called when a key is pressed
  /// @param [in] _event the Qt event to query for size etc  
  void keyPressEvent(QKeyEvent *_event) override;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief this method is called every time a mouse is moved
  /// @param _event the Qt Event structure
  //----------------------------------------------------------------------------------------------------------------------
  void mouseMoveEvent(QMouseEvent *_event) override;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief this method is called everytime the mouse button is pressed
  /// inherited from QObject and overridden here.
  /// @param _event the Qt Event structure
  //----------------------------------------------------------------------------------------------------------------------
  void mousePressEvent(QMouseEvent *_event) override;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief this method is called everytime the mouse button is released
  /// inherited from QObject and overridden here.
  /// @param _event the Qt Event structure
  //----------------------------------------------------------------------------------------------------------------------
  void mouseReleaseEvent(QMouseEvent *_event) override;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief this method is called everytime the mouse wheel is moved
  /// inherited from QObject and overridden here.
  /// @param _event the Qt Event structure
  //----------------------------------------------------------------------------------------------------------------------
  void wheelEvent(QWheelEvent *_event) override;

  void timerEvent(QTimerEvent *) override;
  bool m_animate = false;
  void generateFBO();
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief used to store the global mouse transforms
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Mat4 m_mouseGlobalTX;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief the model position for mouse movement
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Vec3 m_modelPos;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief the view matrix for camera
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Mat4 m_view;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief the projection matrix for camera
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Mat4 m_projection;
  // Texture ID for our colour attachments 
  GLuint m_position;
  GLuint m_normal;
  GLuint m_albedo;
  // vbo id for screen tri
  GLuint m_vao=0;
  GLuint m_fboID;
  float m_lightScaleU=1.0f;
  float m_lightScaleV=1.0f;
  
  float m_lightIntensity=10.0f;
  ngl::Vec3 m_lightRotation;
  // position of the lights
  ngl::Vec3 m_lightPos={0.0f,1.0f,0.0f};
  // display modes for debugging
  enum class DisplayModes: int {Shaded=0,Position=1,Normal=2,Albedo=3} ;
  DisplayModes m_displayMode = DisplayModes::Shaded;



};


#endif
