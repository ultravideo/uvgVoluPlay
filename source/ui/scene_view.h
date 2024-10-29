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
#include <filesystem> 
#include <iostream>   
#include <vector>     
#include <mutex> 
#include <condition_variable>
#include "utils/log.hpp"

namespace nui
{
  enum RenderMode
  {
    RENDER_SEQUENCE = 0,
    RENDER_ZMQ = 1,
  };

  class SceneView
  {
  public:
    SceneView();

    ~SceneView();

    nelems::Light* get_light() { return mLight.get(); }

    void resize(int32_t width, int32_t height);
    
    void on_mouse_move(double x, double y, nelems::EInputButton button);

    void on_mouse_wheel(double delta);

    void set_focus_on_fisrt_frame(glm::vec3 focus);

    void reset_view();

    void receivePointCloud();

    void stop();

    void set_pointSize(float pointSize);

    void set_scene_name(std::string name);

    void set_sequence_path(std::string path);

    void render();

    void set_render_mode(int mode);

    std::string get_scene_name();

    void run();

    int get_total_frames();

    int get_current_frame();

    void set_frame_idx(size_t idx);

    void set_pause(bool pause);

    void set_background_color(float r, float g, float b);

    void set_FPS(int fps);

    void reserve_sequence_length(std::vector<int> _sequence_length);

    void setup_socket(char * position_socket, char * color_socket);

    void set_repeat_time(int repeat_time);

    void set_description(std::string _description);

  private:
    void render_zmq();

    void render_sequence(); // Later
  
  private:
    std::unique_ptr<nelems::Camera> mCamera;
    std::unique_ptr<nrender::OpenGL_FrameBuffer> mFrameBuffer;
    std::unique_ptr<nshaders::Shader> mShader;
    std::unique_ptr<nelems::Light> mLight;
    std::shared_ptr<nelems::Mesh> mMesh = nullptr;

    std::shared_ptr<std::vector<std::shared_ptr<nelems::GLPointCloud>>> pcl_vector = std::make_shared<std::vector<std::shared_ptr<nelems::GLPointCloud>>>();
    std::shared_ptr<std::queue<std::shared_ptr<nelems::GLPointCloud>>> pcl_queue = std::make_shared<std::queue<std::shared_ptr<nelems::GLPointCloud>>>();
    std::shared_ptr<Communication::Portal> mPortal = std::make_shared<Communication::Portal>();

    glm::vec2 mSize;
    float mpointSize = 3.0f;

    bool parse_new_pcl = false;
    std::string scene_name = "Scene 0";
    std::shared_ptr<std::function<void()>> render_mode_ptr = nullptr;

    RenderMode mRenderMode = RENDER_SEQUENCE;
    size_t frame_idx = 0;
    std::shared_ptr<bool> sequence_loaded = std::make_shared<bool>(false);
    std::mutex frame_idx_mutex;
    bool is_paused = false;

    std::chrono::steady_clock::time_point frameStart = std::chrono::steady_clock::now();
    int FPS = 25;
    int frameDuration = 1000 / FPS;

    int repeat_time = 3;
    int temp_repeat_time = 0;
    int curr_sequence_idx = 0;
    std::vector<int> Sequence_length;

    std::string description = "";
  };
}

