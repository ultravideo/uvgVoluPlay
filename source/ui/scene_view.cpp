#include "pch.h"
#include "scene_view.h"

#include "imgui.h"

namespace nui
{
    SceneView::SceneView() : 
      mCamera(nullptr), mFrameBuffer(nullptr), mShader(nullptr),
      mLight(nullptr), mSize(800, 600)
    {
      mFrameBuffer = std::make_unique<nrender::OpenGL_FrameBuffer>();
      mFrameBuffer->create_buffers(800, 600);
      mShader = std::make_unique<nshaders::Shader>();
      mShader->load("shaders/vs.shader", "shaders/fr_nolight.shader");
      // mLight = std::make_unique<nelems::Light>();
      mCamera = std::make_unique<nelems::Camera>(glm::vec3(-94, 272, -251), 45.0f, 1.3f, 0.1f, 2000.0f);
      // mCamera = std::make_unique<nelems::Camera>(glm::vec3(10, 100, 200), 45.0f, 1.3f, 0.1f, 2000.0f);

      if (!mMesh) {
          mMesh = std::make_shared<nelems::Mesh>();
      }
      mMesh->init();
      temp_repeat_time = repeat_time;
    }

    SceneView::~SceneView()
    {
      mShader->unload();
      mFrameBuffer->delete_buffers();
    }

    void SceneView::resize(int32_t width, int32_t height)
    {
        if (width <= 0 || height <= 0)
        {
            return;
        }

        mSize.x = width;
        mSize.y = height;

        mFrameBuffer->create_buffers((int32_t)mSize.x, (int32_t) mSize.y);
    }

    void SceneView::on_mouse_move(double x, double y, nelems::EInputButton button)
    {
        mCamera->on_mouse_move(x, y, button);
    }

    void SceneView::on_mouse_wheel(double delta)
    {
        mCamera->on_mouse_wheel(delta);
    }

    void SceneView::set_focus_on_fisrt_frame(glm::vec3 focus)
    {
        mCamera->set_focus(focus);
    }

    void SceneView::render_zmq()
    {
        mShader->use();

        //mLight->update(mShader.get());

        mFrameBuffer->bind();

        if (mMesh)
        {
            if (!pcl_queue->empty() && parse_new_pcl)
            {                
                if (frame_idx == 0)
                {
                    set_focus_on_fisrt_frame(pcl_queue->front()->getPosition(0));
                }
                mMesh->parse_data(pcl_queue->front());
                parse_new_pcl = false;
                frame_idx++;
            }
            mMesh->render();
        }

        // // Make sure that we always have 1 pcl left to visualize, otherwise pcl will be deleted 
        // // due to differnet in speed of rendering and receiving
        if (pcl_queue->size() >= 2) 
        {
            pcl_queue->pop();
            parse_new_pcl = true;
        }

        mFrameBuffer->unbind();

        ImGui::Begin(scene_name.c_str());

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        mSize = { viewportPanelSize.x, viewportPanelSize.y };

        mCamera->set_aspect(mSize.x / mSize.y);
        mCamera->update(mShader.get());

        // add rendered texture to ImGUI scene window
        uint64_t textureID = mFrameBuffer->get_texture();
        ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ mSize.x, mSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

        ImGui::End();
    }

    void SceneView::render_sequence()
    {
        mShader->use();

        //mLight->update(mShader.get());

        mFrameBuffer->bind();

        if (mMesh)
        {
            if (*sequence_loaded && parse_new_pcl && (frame_idx < (pcl_vector->size())))
            {        
                // Use mutex to avoid race condition
                std::lock_guard<std::mutex> lock(frame_idx_mutex);                  
                mMesh->parse_data(pcl_vector->at(frame_idx));
                parse_new_pcl = false;
            }

            mMesh->render();

            // // Make sure that we always have 1 pcl left to visualize, otherwise pcl will be deleted 
            // // due to differnet in speed of rendering and receiving
            if (*sequence_loaded && (frame_idx < (pcl_vector->size() - 1)) && !paused_flag)
            {
                // Use mutex to avoid race condition
                auto frameEnd = std::chrono::steady_clock::now();
                std::chrono::duration<double, std::milli> elapsed = frameEnd - frameStart;
                int remainingTime = frameDuration - static_cast<int>(elapsed.count());

                if (remainingTime > 0) {
                    
                } else {
                    std::lock_guard<std::mutex> lock(frame_idx_mutex);
                    
                    if (!Sequence_length.empty()) {
                        if (temp_repeat_time != 1 && frame_idx+1 == (Sequence_length.at(curr_sequence_idx)*(curr_sequence_idx+1)-1)) {
                            frame_idx = Sequence_length.at(curr_sequence_idx)*(curr_sequence_idx)-1;
                            temp_repeat_time--;
                        } else if (temp_repeat_time == 1 && frame_idx+1 == (Sequence_length.at(curr_sequence_idx)*(curr_sequence_idx+1)-1)) {
                            curr_sequence_idx = (curr_sequence_idx + 1) % Sequence_length.size();

                            frame_idx = Sequence_length.at(curr_sequence_idx)*(curr_sequence_idx)-1;
                            temp_repeat_time = repeat_time;
                        }
                    }
                    
                    frame_idx++;

                    parse_new_pcl = true;
                    frameStart = std::chrono::steady_clock::now();
                }
            } else if (*sequence_loaded && (frame_idx >= (pcl_vector->size() - 1)) && !paused_flag) {
                frame_idx = 0;
            }
        }

        mFrameBuffer->unbind();

        ImGui::Begin(scene_name.c_str());
 
        if (description != "") {
            ImGui::SetWindowFontScale(1.5f);
            ImGui::Text("%s", description.c_str());
            ImGui::SetWindowFontScale(1.0f);
        }


        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        mSize = { viewportPanelSize.x, viewportPanelSize.y };

        mCamera->set_aspect(mSize.x / mSize.y);
        mCamera->update(mShader.get());

        // add rendered texture to ImGUI scene window
        uint64_t textureID = mFrameBuffer->get_texture();
        ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ mSize.x, mSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
        ImGui::End();
    }

    void SceneView::set_pointSize(float pointSize)
    {
        mMesh->set_pointSize(pointSize);
    }

    std::string SceneView::get_scene_name()
    {
        return scene_name;
    }

    void SceneView::receivePointCloud()
    {
            if (mRenderMode == RENDER_ZMQ) {
                frame_idx = 0;
                mPortal->zmq_run();
            } 
            else if (mRenderMode == RENDER_SEQUENCE)
            {
                std::thread([this]() {
                    mPortal->sequence_run();
                    if (!pcl_vector->empty() && *sequence_loaded && frame_idx == (pcl_vector->size() - 1))
                    {
                        frame_idx = 0;
                        paused_flag = true;
                    }
                        
                }).detach();
            }
    }

    void SceneView::stop()
    {
        mMesh = nullptr;
        switch (this->mRenderMode)
        {
        case RENDER_ZMQ:
            mPortal->stop_signal();
            frame_idx = 0;
            break;
        case RENDER_SEQUENCE:
            pcl_vector->clear();
            sequence_loaded = std::make_shared<bool>(false);
            frame_idx = 0;
            break;
        default:
            break;
        }
    }

    void SceneView::set_scene_name(std::string name)
    {
        scene_name = name;
    }

    void SceneView::set_sequence_path(std::string path)
    {
        mPortal->set_sequence_path(path);
    }

    void SceneView::render()
    {
        if (render_mode_ptr)
            render_mode_ptr->operator()();
    }

    void SceneView::set_render_mode(int mode)
    {
        switch (mode)
        {
        case RENDER_ZMQ:
            this->mRenderMode = RENDER_ZMQ;
            mPortal->set_data_stream(pcl_queue, mMesh, Communication::SourceMode::SOURCE_ZMQ);
            break;
        case RENDER_SEQUENCE:
            this->mRenderMode = RENDER_SEQUENCE;
            if (pcl_vector->empty())
            {
                mPortal->set_data_stream(pcl_vector, mMesh, Communication::SourceMode::SOURCE_SEQUENCE);
                frame_idx = 0;
                mPortal->set_load_sequence(sequence_loaded);
            }
            break;
        default:
            break;
        }

        utilities::Logger::log(utilities::LogLevel::INFO, "SceneView", "Render mode set to: " + std::to_string(mode) + "\n");
    }

    void SceneView::run() {
        switch (this->mRenderMode)
        {
        case RENDER_ZMQ:
            render_mode_ptr.reset(new std::function<void()>(std::bind(&SceneView::render_zmq, this)));
            break;
        case RENDER_SEQUENCE:
            render_mode_ptr.reset(new std::function<void()>(std::bind(&SceneView::render_sequence, this)));
            break;
        default:
            break;
        }
    }

    int SceneView::get_total_frames()
    {
        return static_cast<int>(pcl_vector->size());
    }

    int SceneView::get_current_frame()
    {
        return frame_idx;
    }

    void SceneView::set_frame_idx(size_t idx)
    {
        // Use mutex to avoid race condition
        std::lock_guard<std::mutex> lock(frame_idx_mutex);
        frame_idx = idx;
    }

    void SceneView::set_pause(bool pause)
    {
        paused_flag = pause;
        parse_new_pcl = true;
    }

    bool SceneView::is_paused()
    {
        return paused_flag;
    }

    void SceneView::set_background_color(float r, float g, float b)
    {
        mFrameBuffer->set_background_color(r, g, b);
    }

    void SceneView::setup_socket(char * position_socket, char * color_socket)
    {
        if (mPortal)
        {
            mPortal->set_position_socket(position_socket);
            mPortal->set_color_socket(color_socket);
        }
        else {
            utilities::Logger::log(utilities::LogLevel::ERROR, "SceneView", "Portal is not initialized\n");
        }
    }

    void SceneView::reset_view() { 
      mCamera->reset(); 
    }

    void SceneView::set_FPS(int fps) {
        FPS = fps;
        frameDuration = 1000 / FPS;
    }

    void SceneView::reserve_sequence_length(std::vector<int> _sequence_length) {
        Sequence_length = _sequence_length;
    }

    void SceneView::set_repeat_time(int _repeat_time) {
        repeat_time = _repeat_time;
        temp_repeat_time = repeat_time;
    }

    void SceneView::set_description(std::string _description) {
        description = _description;
    }

    void SceneView::export_camera_data(glm::vec3 &position, glm::vec3 &focus, float &distance, glm::quat &orientation) {
        mCamera->get_camera_data(position, focus, distance, orientation);
    }

    void SceneView::set_camera(glm::vec3 &position, glm::vec3 &focus, float &distance, glm::quat &orientation) {
        mCamera->set_camera_data(position, focus, distance, orientation);
    }

    void SceneView::set_auto_rotate() {
        mCamera->auto_rotate();
    }

    void SceneView::camera_horizontal_pan(bool right) {
        mCamera->horizontal_pan(right);
    }

    void SceneView::camera_vertical_pan(bool up) {
        mCamera->vertical_pan(up);
    }

    void SceneView::rotate_model(bool horizontal, float angle) {
        mCamera->rotate_model(horizontal, angle);
    }

    void SceneView::orbit_camera(bool right) {
        mCamera->orbit_move(right);
    }
}
