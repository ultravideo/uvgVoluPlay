#pragma once

#include "elems/camera.h"
#include "elems/mesh.h"
#include "elems/light.h"
#include "shader/shader_util.h"
#include "render/opengl_buffer_manager.h"
#include "elems/input.h"
#include "elems/pointcloud.h"
#include "communication/portal.hpp"

#include <thread> 
#include <filesystem> // Include the filesystem library for C++17 or later
#include <iostream>   // Include for std::cout (for demonstration)
#include <vector>     // Include for std::vector (for collecting filenames)
#include <mutex> 
#include <condition_variable>
#include "utils/log.hpp"

namespace nui
{
  class SceneView
  {
  public:
    SceneView() : 
      mCamera(nullptr), mFrameBuffer(nullptr), mShader(nullptr),
      mLight(nullptr), mSize(800, 600)
    {
      mFrameBuffer = std::make_unique<nrender::OpenGL_FrameBuffer>();
      mFrameBuffer->create_buffers(800, 600);
      mShader = std::make_unique<nshaders::Shader>();
      mShader->load("shaders/vs.shader", "shaders/fr_nolight.shader");
      mLight = std::make_unique<nelems::Light>();

      mCamera = std::make_unique<nelems::Camera>(glm::vec3(0, 10, 20), 45.0f, 1.3f, 0.1f, 2000.0f);

      if (!mMesh) {
          mMesh = std::make_shared<nelems::Mesh>();
      }
      mMesh->init();

      mPortal->set_data_stream(pcl_queue, mMesh);
    }

    ~SceneView()
    {
      mShader->unload();
      mFrameBuffer->delete_buffers();
    }

    nelems::Light* get_light() { return mLight.get(); }

    void resize(int32_t width, int32_t height);

    void set_input(int mode);

    void render_zmq();

    void render_sequence(); // Later
    
    void on_mouse_move(double x, double y, nelems::EInputButton button);

    void on_mouse_wheel(double delta);

    void reset_view() { mCamera->reset(); }

    void receivePointCloud();

    void stop();

    void set_pointSize(float pointSize);
  
  private:
    std::unique_ptr<nelems::Camera> mCamera;
    std::unique_ptr<nrender::OpenGL_FrameBuffer> mFrameBuffer;
    std::unique_ptr<nshaders::Shader> mShader;
    std::unique_ptr<nelems::Light> mLight;
    std::shared_ptr<nelems::Mesh> mMesh = nullptr;


    std::shared_ptr<std::queue<std::shared_ptr<nelems::Mesh>>> mesh_queue = std::make_shared<std::queue<std::shared_ptr<nelems::Mesh>>>();
    std::shared_ptr<std::queue<std::shared_ptr<nelems::GLPointCloud>>> pcl_queue = std::make_shared<std::queue<std::shared_ptr<nelems::GLPointCloud>>>();
    std::shared_ptr<Communication::Portal> mPortal = std::make_shared<Communication::Portal>();

    glm::vec2 mSize;
    int InputMode = 0;
    float mpointSize = 1.0f;

    bool parse_new_pcl = false;
  };
}

