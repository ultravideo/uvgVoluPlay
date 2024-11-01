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

    mPCLPropertyPanel = std::make_unique<Control_Panel>();
    mPCLPropertyPanel->set_scene_view_container(mSceneView_Container);

    mKeyboardHelperPanel = std::make_unique<KeyboardHelperPanel>();

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
    for (auto& scene_view : *mSceneView_Container)
    {
      scene_view->on_mouse_wheel(delta);
    }
  }

  void GLWindow::on_key(int key, int scancode, int action, int mods)
  {
    if (action == GLFW_PRESS) {
       handle_input(key, false);
    } 

    // Long press key
    if (action == GLFW_REPEAT) {
       handle_input(key, true);
    }

  }

  void GLWindow::handle_input(int key, bool is_long_press)
  {
      if (!is_long_press) {
        if (key == GLFW_KEY_ESCAPE)
        {
          on_close();
        }

        else if (key == GLFW_KEY_SPACE && mSceneView_Container->size() > 0)
        {
          auto current_frame = mSceneView_Container->at(0)->get_current_frame();
          current_frame = (current_frame > 0) ? current_frame - 1 : 0;
          for (auto& scene_view : *mSceneView_Container)
          {
            scene_view->set_frame_idx(current_frame);
            scene_view->set_pause(!scene_view->is_paused());
          }
        }

        else if (glfwGetKey(mWindow, GLFW_KEY_F))
        {
          for (auto& scene_view : *mSceneView_Container)
          {
            scene_view->reset_view();
          }
        }

        else if (glfwGetKey(mWindow, GLFW_KEY_R))
        {
          mPCLPropertyPanel->set_auto_rotate(!mPCLPropertyPanel->get_auto_rotate());
        }

        else if (glfwGetKey(mWindow, GLFW_KEY_1))
        {
          mPCLPropertyPanel->set_limited_frame_rate(!mPCLPropertyPanel->get_limited_frame_rate());
        }

        else if (glfwGetKey(mWindow, GLFW_KEY_O))
        {
          mPCLPropertyPanel->open_json_dialog();
        }
      }

      /* Camera control */
      if (glfwGetKey(mWindow, GLFW_KEY_W))
      {
        for (auto& scene_view : *mSceneView_Container)
        {
          scene_view->on_mouse_wheel(-10.0f);
        }
      }

      else if (glfwGetKey(mWindow, GLFW_KEY_S))
      {
        for (auto& scene_view : *mSceneView_Container)
        {
          scene_view->on_mouse_wheel(10.0f);
        }
      }

      else if (glfwGetKey(mWindow, GLFW_KEY_A))
      {
        for (auto& scene_view : *mSceneView_Container)
        {
          scene_view->orbit_camera(false);
        }
      }

      else if (glfwGetKey(mWindow, GLFW_KEY_D))
      {
        for (auto& scene_view : *mSceneView_Container)
        {
          scene_view->orbit_camera(true);
        }
      }

      /* Rotate model */
      else if (glfwGetKey(mWindow, GLFW_KEY_Q))
      {
        for (auto& scene_view : *mSceneView_Container)
        {
          scene_view->rotate_model(true, 0.1f);
        }
      }

      else if (glfwGetKey(mWindow, GLFW_KEY_E))
      {
        for (auto& scene_view : *mSceneView_Container)
        {
          scene_view->rotate_model(true, -0.1f);
        }
      }

      else if (glfwGetKey(mWindow, GLFW_KEY_Z))
      {
        for (auto& scene_view : *mSceneView_Container)
        {
          scene_view->rotate_model(false, 0.1f);
        }
      }

      else if (glfwGetKey(mWindow, GLFW_KEY_C))
      {
        for (auto& scene_view : *mSceneView_Container)
        {
          scene_view->rotate_model(false, -0.1f);
        }
      }

      else if (glfwGetKey(mWindow, GLFW_KEY_F1))
      {
        show_helper_panel = !show_helper_panel;
      }

      else if (glfwGetKey(mWindow, GLFW_KEY_F2))
      {
        show_control_panel = !show_control_panel;
      }
      /* ############################################## */

      /* Pan control */
      else if (glfwGetKey(mWindow, GLFW_KEY_LEFT))
      {
        for (auto& scene_view : *mSceneView_Container)
        {
          scene_view->camera_horizontal_pan(false);
        }
      }

      else if (glfwGetKey(mWindow, GLFW_KEY_RIGHT))
      {
        for (auto& scene_view : *mSceneView_Container)
        {
          scene_view->camera_horizontal_pan(true);
        }
      }

      else if (glfwGetKey(mWindow, GLFW_KEY_UP))
      {
        for (auto& scene_view : *mSceneView_Container)
        {
          scene_view->camera_vertical_pan(true);
        }
      }

      else if (glfwGetKey(mWindow, GLFW_KEY_DOWN))
      {
        for (auto& scene_view : *mSceneView_Container)
        {
          scene_view->camera_vertical_pan(false);
        }
      }
      /* ############################################## */

      /* FPS control */
      else if (glfwGetKey(mWindow, GLFW_KEY_2))
      {
        mPCLPropertyPanel->adjust_frame_rate(true);
      }

      else if (glfwGetKey(mWindow, GLFW_KEY_3))
      {
        mPCLPropertyPanel->adjust_frame_rate(false);
      }
      /* ############################################## */

      /* Playback control */
      else if (glfwGetKey(mWindow, GLFW_KEY_L))
      {
        mPCLPropertyPanel->playback(true);
      }

      else if (glfwGetKey(mWindow, GLFW_KEY_J))
      {
        mPCLPropertyPanel->playback(false);
      }
      /* ############################################## */

      /* Point size */
      else if (glfwGetKey(mWindow, GLFW_KEY_I))
      {
        mPCLPropertyPanel->increase_point_size(true);
      }

      else if (glfwGetKey(mWindow, GLFW_KEY_K))
      {
        mPCLPropertyPanel->increase_point_size(false);
      }
      /* ############################################## */
  }

  void GLWindow::on_close()
  {
    mIsRunning = false;
  }

  void GLWindow::render()
  {
    // // Clear the view
    mRenderCtx->pre_render();

    // // Initialize UI components
    mUICtx->pre_render();

    if (!is_minized())
    {
      // bool show_demo_window = true;
      // ImGui::ShowDemoWindow(&show_demo_window);

      if (show_control_panel)
      {
        mPCLPropertyPanel->render();
      } else {
        if (mPCLPropertyPanel->get_auto_rotate()) {
          for (auto& scene_view : *mSceneView_Container)
                {
                    scene_view->set_auto_rotate();
                }
           }
      }

      for (auto& scene_view : *mSceneView_Container)
      {
        scene_view->render();
      }

      if (show_helper_panel)
      {
        mKeyboardHelperPanel->render();
      }
    }
    
    // Render the UI 
    mUICtx->post_render();

    // // Render end, swap buffers
    mRenderCtx->post_render();

    handle_input();
  }

  void GLWindow::handle_input()
  {
    // TODO: move this and camera to scene UI component?    
    double x, y;
    glfwGetCursorPos(mWindow, &x, &y);

    // mSceneView->on_mouse_move(x, y, Input::GetPressedButton(mWindow));
    for (auto& scene_view : *mSceneView_Container)
    {
      scene_view->on_mouse_move(x, y, Input::GetPressedButton(mWindow));
    }
  }

  bool GLWindow::is_minized()
  {
    if (glfwGetWindowAttrib(mWindow, GLFW_ICONIFIED))
    {
      return true;
    } 
    return false;

  }
}
