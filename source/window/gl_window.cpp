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

    mSceneView = std::make_unique<SceneView>();

    mPCLPropertyPanel = std::make_unique<PCL_Property_Panel>();
    // mPCLPropertyPanel->set_scene_view_container(mSceneView_Container);

    //mPCLPropertyPanel->set_mesh_load_callback(
    //  [this](std::string filepath) { mSceneView->load_mesh(filepath); });

    // mStatPanel = std::make_unique<Stat_Panel>();

    //mPropertyPanel = std::make_unique<Property_Panel>();

    //mPropertyPanel->set_mesh_load_callback(
    //  [this](std::string filepath) { mSceneView->load_mesh(filepath); });

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

    mSceneView->resize(Width, Height);

    // for (auto& scene_view : *mSceneView_Container)
    // {
    //   scene_view->resize(Width, Height);
    // }

    render();
  }

  void GLWindow::on_scroll(double delta)
  {
    mSceneView->on_mouse_wheel(delta);

    // for (auto& scene_view : *mSceneView_Container)
    // {
    //   scene_view->on_mouse_wheel(delta);
    // }
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

    // render scene to framebuffer and add it to scene view
    mSceneView->render();

    mPCLPropertyPanel->render(mSceneView.get());

    // mStatPanel->render();

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
      mSceneView->on_mouse_wheel(-0.4f);
    }

    if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS)
    {
      mSceneView->on_mouse_wheel(0.4f);
    }

    if (glfwGetKey(mWindow, GLFW_KEY_F) == GLFW_PRESS)
    {
      mSceneView->reset_view();
    }

    double x, y;
    glfwGetCursorPos(mWindow, &x, &y);

    mSceneView->on_mouse_move(x, y, Input::GetPressedButton(mWindow));
  }
}
