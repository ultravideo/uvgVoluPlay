#pragma once

#include "elems/camera.h"
#include "elems/mesh.h"
#include "elems/light.h"
#include "shader/shader_util.h"
#include "render/opengl_buffer_manager.h"
#include "elems/input.h"
#include "elems/pointcloud.h"
#include "elems/vertex_holder.h"

#include <zmq.hpp>
#include <thread> 
#include <filesystem> // Include the filesystem library for C++17 or later
#include <iostream>   // Include for std::cout (for demonstration)
#include <vector>     // Include for std::vector (for collecting filenames)
#include <mutex> 
#include <condition_variable>

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
      if (!mMesh)
          mMesh = std::make_shared<nelems::Mesh>();
      mMesh->init();
    }

    ~SceneView()
    {
      mShader->unload();
      mFrameBuffer->delete_buffers();
    }

    nelems::Light* get_light() { return mLight.get(); }

    void resize(int32_t width, int32_t height);

    void set_input(int mode);

    void render();

    void load_mesh(const std::string& filepath);

    void load_sequence(const std::string& folderpath);

    std::shared_ptr<nelems::Mesh> get_mesh() { return mMesh; }
    
    void on_mouse_move(double x, double y, nelems::EInputButton button);

    void on_mouse_wheel(double delta);

    void clear_mesh() { mMesh->delete_buffers(); }

    void set_mesh(std::shared_ptr<nelems::Mesh> mesh) { mMesh = mesh;}

    void reset_view() { mCamera->reset(); }

    void clean_pclqueue() { mMesh->clear_queue(); }

    void set_pointSize(float value) { mMesh->set_point_size(value); };

    void add_pcl(std::shared_ptr<nelems::GLPointCloud> pointCloud);

    int get_render_frames();

    void receivePointCloud(std::string serveraddrBuffer);

    void stop();

  private:
    void handleMessage(zmq::message_t Pmessage, zmq::message_t Cmessage);

    std::unique_ptr<nelems::Camera> mCamera;
    std::unique_ptr<nrender::OpenGL_FrameBuffer> mFrameBuffer;
    std::unique_ptr<nshaders::Shader> mShader;
    std::unique_ptr<nelems::Light> mLight;
    std::shared_ptr<nelems::Mesh> mMesh;
    glm::vec2 mSize;
    int InputMode = 0;
    bool load_sequence_flag = false;

    std::mutex mtx;
    std::condition_variable cv;
    bool stoprender_flag = false;
  };
}

