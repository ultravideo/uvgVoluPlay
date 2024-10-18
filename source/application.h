#pragma once

#include "window/gl_window.h"
#include "elems/camera.h"

class Application
{

public:
  Application(const std::string& app_name);

  static Application& Instance() { return *sInstance; }

  void loop();

private:
  static Application* sInstance;

  std::unique_ptr<nwindow::GLWindow> mWindow;

  std::shared_ptr<int> FPS = std::make_shared<int>(60);
  std::shared_ptr<bool> limited_frame_rate = std::make_shared<bool>(false);
};
