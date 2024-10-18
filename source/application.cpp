#include "pch.h"

#include "application.h"

#include "window/gl_window.h"

Application::Application(const std::string& app_name)
{
  mWindow = std::make_unique<nwindow::GLWindow>();
  mWindow->init(1680, 1050, app_name, FPS, limited_frame_rate);
}

void Application::loop()
{
  while (mWindow->is_running())
  {
    // Record the start time of the frame
    int frameDuration = 1000 / *FPS; // Frame time in milliseconds (16.67ms)
    auto frameStart = std::chrono::steady_clock::now();

    mWindow->render();

    if (*limited_frame_rate)
    {
      auto frameEnd = std::chrono::steady_clock::now();
      std::chrono::duration<double, std::milli> elapsed = frameEnd - frameStart;
      int remainingTime = frameDuration - static_cast<int>(elapsed.count());

      if (remainingTime > 0) {
          std::this_thread::sleep_for(std::chrono::milliseconds(remainingTime));
      }
    }

  }
}

// control_panel