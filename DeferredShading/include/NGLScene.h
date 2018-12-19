#ifndef NGLSCENE_H_
#define NGLSCENE_H_
#include <ngl/Transformation.h>
#include <ngl/Text.h>
#include <ngl/Mat4.h>
#include <ngl/AbstractVAO.h>
#include <QElapsedTimer>
#include "FrameBufferObject.h"
#include "FirstPersonCamera.h"
#include <QOpenGLWindow>
#include <QSet>
#include <memory>
#include "WindowParams.h"

//----------------------------------------------------------------------------------------------------------------------
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
//----------------------------------------------------------------------------------------------------------------------

class NGLScene : public QOpenGLWindow
{
  public:
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief ctor for our NGL drawing class
    /// @param [in] parent the parent window to the class
    //----------------------------------------------------------------------------------------------------------------------
    NGLScene();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief dtor must close down ngl and release OpenGL resources
    //----------------------------------------------------------------------------------------------------------------------
    ~NGLScene() override;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief the initialize class is called once when the window is created and we have a valid GL context
    /// use this to setup any default GL stuff
    //----------------------------------------------------------------------------------------------------------------------
    void initializeGL() override;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief this is called everytime we want to draw the scene
    //----------------------------------------------------------------------------------------------------------------------
    void paintGL() override;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief this is called everytime we resize
    //----------------------------------------------------------------------------------------------------------------------
    void resizeGL(int _w, int _h) override;

private:
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief the windows params such as mouse and rotations etc
    //----------------------------------------------------------------------------------------------------------------------
    WinParams m_win;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief used to store the global mouse transforms
    //----------------------------------------------------------------------------------------------------------------------
    ngl::Mat4 m_mouseGlobalTX;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Our Camera
    //----------------------------------------------------------------------------------------------------------------------
//    ngl::Mat4 m_view;
//    ngl::Mat4 m_project;
      FirstPersonCamera m_cam;

    //----------------------------------------------------------------------------------------------------------------------
    /// @brief transformation stack for the gl transformations etc
    //----------------------------------------------------------------------------------------------------------------------
    ngl::Transformation m_transform;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief the model position for mouse movement
    //----------------------------------------------------------------------------------------------------------------------
    ngl::Vec3 m_modelPos;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief method to load transform matrices to the shader
    //----------------------------------------------------------------------------------------------------------------------
    void loadMatricesToShader(const ngl::Mat4 &_mouse);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our texture id used by the FBO
    //----------------------------------------------------------------------------------------------------------------------
    GLuint m_textureID;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our FBO id used by the FBO
    //----------------------------------------------------------------------------------------------------------------------
    GLuint m_fboID;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Qt Event called when a key is pressed
    /// @param [in] _event the Qt event to query for size etc
    //----------------------------------------------------------------------------------------------------------------------
    void keyPressEvent(QKeyEvent *_event) override;
    void keyReleaseEvent(QKeyEvent *_event) override;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief this method is called every time a mouse is moved
    /// @param _event the Qt Event structure
    //----------------------------------------------------------------------------------------------------------------------
    void mouseMoveEvent (QMouseEvent * _event ) override;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief this method is called everytime the mouse button is pressed
    /// inherited from QObject and overridden here.
    /// @param _event the Qt Event structure
    //----------------------------------------------------------------------------------------------------------------------
    void mousePressEvent ( QMouseEvent *_event) override;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief this method is called everytime the mouse button is released
    /// inherited from QObject and overridden here.
    /// @param _event the Qt Event structure
    //----------------------------------------------------------------------------------------------------------------------
    void mouseReleaseEvent ( QMouseEvent *_event ) override;

    //----------------------------------------------------------------------------------------------------------------------
    /// @brief this method is called everytime the mouse wheel is moved
    /// inherited from QObject and overridden here.
    /// @param _event the Qt Event structure
    //----------------------------------------------------------------------------------------------------------------------
    void wheelEvent( QWheelEvent *_event) override;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief this method is called everytime the timer triggers
    //----------------------------------------------------------------------------------------------------------------------
    void timerEvent(QTimerEvent *) override;

    void createScreenQuad();
    void editLightShader();
    void geometryPass();
    void lightingPass();
    void forwardPass();
    void ssaoPass();
    void bloomBlurPass();
    void finalPass();
    void drawUI();
    void loadDOFUniforms();
    void createSSAOKernel();
    void debugBlit(GLuint id);
    std::unique_ptr<ngl::AbstractVAO>  m_screenQuad;
    std::unique_ptr<FrameBufferObject> m_renderFBO;
    std::unique_ptr<FrameBufferObject> m_lightingFBO;
    std::unique_ptr<FrameBufferObject> m_forwardPass;
    std::unique_ptr<FrameBufferObject> m_ssaoPass;
    std::unique_ptr<FrameBufferObject> m_dofPass;
    std::unique_ptr<FrameBufferObject> m_dofTarget;

    std::array<std::unique_ptr<FrameBufferObject>,2> m_pingPongBuffer;
    GLuint m_floorNormalTexture;
    GLuint m_noiseTexture=0;
    bool m_debugOn=false;
    int m_debugAttachment=0;
    void createLights();
    float m_fstop=2.8f;
    int m_av=3; // used in f-stop calc where fstop=sqrtf(2^m_av)
    float m_focalLenght=1.0f;
    float m_focalDistance=8.0f;
    float m_focusDistance=0.55f;

    struct Light
    {
        ngl::Vec3 position;
        ngl::Vec3 colour;
        float linear= 0.7f;
        float quadratic=1.8f;
    };
    int m_numLights=24;
    bool m_showLights=true;
    std::vector<Light> m_lights;
    std::vector<ngl::Vec3> m_ssaoKernel;

    float m_freq=1.0f;
    float m_lightRadius=4.0f;
    float m_lightYOffset=1.0f;
    bool m_lightRandom=false;
    bool m_useAO=true;
    int m_randomUpdateTimer;

    long m_geoPassDuration=0;
    long m_ssaoPassDuration=0;
    long m_lightingPassDuration=0;
    long m_forwardPassDuration=0;
    long m_bloomBlurPassDuration=0;
    long m_finalPassDuration=0;
    long m_totalDuration=0;
    float m_gamma=2.2f;
    bool m_useDOF=true;
    bool m_useBloom=true;
    float m_exposure=1.0f;
    float m_lightMaxIntensity=4.0f;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief the keys being pressed
    //----------------------------------------------------------------------------------------------------------------------
    QSet<Qt::Key> m_keysPressed;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief timing for camera update
    //----------------------------------------------------------------------------------------------------------------------
    float m_deltaTime = 0.0f;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief frame time for camera update
    //----------------------------------------------------------------------------------------------------------------------
    float m_lastFrame = 0.0f;
    QElapsedTimer m_timer;
    bool m_textureDebug=false;
    GLuint m_debugTextureID=1;
    bool m_showUI=true;
    int m_fpsTimer;
    int m_fps=0;
    int m_frames=0;


};



#endif
