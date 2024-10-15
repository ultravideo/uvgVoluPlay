#include "pch.h"
#include "opengl_context.h"

void APIENTRY glDebugOutput(GLenum source, 
                            GLenum type, 
                            unsigned int id, 
                            GLenum severity, 
                            GLsizei length, 
                            const char *message, 
                            const void *userParam)
  {
      // ignore non-significant error/warning codes
      if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

      std::cout << "---------------" << std::endl;
      std::cout << "Debug message (" << id << "): " <<  message << std::endl;

      switch (source)
      {
          case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
          case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
          case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
          case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
          case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
          case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
      } std::cout << std::endl;

      switch (type)
      {
          case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
          case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
          case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break; 
          case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
          case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
          case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
          case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
          case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
          case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
      } std::cout << std::endl;
      
      switch (severity)
      {
          case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
          case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
          case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
          case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
      } std::cout << std::endl;
      std::cout << std::endl;
  }

namespace nrender
{
  static void on_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
  {
    auto pWindow = static_cast<nwindow::IWindow*>(glfwGetWindowUserPointer(window));
    pWindow->on_key(key, scancode, action, mods);
  }

  static void on_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
  {
    auto pWindow = static_cast<nwindow::IWindow*>(glfwGetWindowUserPointer(window));
    pWindow->on_scroll(yoffset);
  }

  static void on_window_size_callback(GLFWwindow* window, int width, int height)
  {
    auto pWindow = static_cast<nwindow::IWindow*>(glfwGetWindowUserPointer(window));
    pWindow->on_resize(width, height);
  }

  static void on_window_close_callback(GLFWwindow* window)
  {
    nwindow::IWindow* pWindow = static_cast<nwindow::IWindow*>(glfwGetWindowUserPointer(window));
    pWindow->on_close();
  }

  bool OpenGL_Context::init(nwindow::IWindow* window)
  {
    RenderContext::init(window);
#ifdef __APPLE__
    glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
    glfwInitHint(GLFW_COCOA_MENUBAR, GLFW_FALSE);
#endif
    /* Initialize the library */
    if (!glfwInit())
    {
      fprintf(stderr, "Error: GLFW Window couldn't be initialized\n");
      return false;
    }
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#endif
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true); 

    // Create the window and store this window as window pointer
    // so that we can use it in callback functions
    auto glWindow = glfwCreateWindow(window->Width, window->Height, window->Title.c_str(), nullptr, nullptr);
    window->set_native_window(glWindow);

    if (!glWindow)
    {
      fprintf(stderr, "Error: GLFW Window couldn't be created\n");
      return false;
    }

    glfwSetWindowUserPointer(glWindow, window);
    glfwSetKeyCallback(glWindow, on_key_callback);
    glfwSetScrollCallback(glWindow, on_scroll_callback);
    glfwSetWindowSizeCallback(glWindow, on_window_size_callback);
    glfwSetWindowCloseCallback(glWindow, on_window_close_callback);
    glfwMakeContextCurrent(glWindow);

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
      /* Problem: glewInit failed, something is seriously wrong. */
      fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
      return false;
    }

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
#ifndef __APPLE__
    glDebugMessageCallback(glDebugOutput, nullptr);
#endif

    return true;
  }

  void OpenGL_Context::pre_render()
  {
    glViewport(0, 0, mWindow->Width, mWindow->Height);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void OpenGL_Context::post_render()
  {
    glfwPollEvents();
    glfwSwapBuffers((GLFWwindow*) mWindow->get_native_window());
  }

  void OpenGL_Context::end()
  {
    glfwDestroyWindow((GLFWwindow*)mWindow->get_native_window());
    glfwTerminate();
  }
}