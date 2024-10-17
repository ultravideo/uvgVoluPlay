#include "pch.h"

#include "gl_window.h"
#include "elems/input.h"

#include "ply_import/miniply.h"
#include "string"

namespace nwindow
{
  bool GLWindow::init(int width, int height, const std::string& title)
  {
    Width = width;
    Height = height;
    Title = title;

    mRenderCtx->init(this);

    mUICtx->init(this);

    std::shared_ptr<SceneView> mSceneView = std::make_shared<SceneView>();
    mSceneView_Container->push_back(mSceneView);

    mPCLPropertyPanel = std::make_unique<PCL_Property_Panel>();
    mPCLPropertyPanel->set_scene_view_container(mSceneView_Container);

    return mIsRunning;
  }

  GLWindow::~GLWindow()
  {
    mUICtx->end();

    mRenderCtx->end();
  }

  void GLWindow::on_resize(int width, int height)
  {
    Width = width;
    Height = height;

    for (auto& scene_view : *mSceneView_Container)
    {
      scene_view->resize(Width, Height);
    }

    render();
  }

  void GLWindow::on_scroll(double delta)
  {
    // mSceneView->on_mouse_wheel(delta);

    for (auto& scene_view : *mSceneView_Container)
    {
      scene_view->on_mouse_wheel(delta);
    }
  }

  void GLWindow::on_key(int key, int scancode, int action, int mods)
  {
    if (action == GLFW_PRESS)
    {
    }
  }

  void GLWindow::on_close()
  {
    mIsRunning = false;
  }

  void GLWindow::render()
  {
    // Clear the view
    mRenderCtx->pre_render();

    // Initialize UI components
    mUICtx->pre_render();

    bool show_demo_window = true;
    ImGui::ShowDemoWindow(&show_demo_window);

    mPCLPropertyPanel->render();

    for (auto& scene_view : *mSceneView_Container)
    {
      scene_view->render();
    }

    // Render the UI 
    mUICtx->post_render();

    // Render end, swap buffers
    mRenderCtx->post_render();

    handle_input();
  }

  void GLWindow::handle_input()
  {
    // TODO: move this and camera to scene UI component?

    if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS)
    {
      // mSceneView->on_mouse_wheel(-0.4f);
      for (auto& scene_view : *mSceneView_Container)
      {
        scene_view->on_mouse_wheel(-0.4f);
      }
    }

    if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS)
    {
      for (auto& scene_view : *mSceneView_Container)
      {
        scene_view->on_mouse_wheel(0.4f);
      }
    }

    if (glfwGetKey(mWindow, GLFW_KEY_F) == GLFW_PRESS)
    {
      for (auto& scene_view : *mSceneView_Container)
      {
        scene_view->reset_view();
      }
    }

    double x, y;
    glfwGetCursorPos(mWindow, &x, &y);

    // mSceneView->on_mouse_move(x, y, Input::GetPressedButton(mWindow));
    for (auto& scene_view : *mSceneView_Container)
    {
      scene_view->on_mouse_move(x, y, Input::GetPressedButton(mWindow));
    }
  }
}
