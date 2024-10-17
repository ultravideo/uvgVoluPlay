#include "pch.h"
#include "scene_view.h"

#include "imgui.h"

namespace nui
{
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
                if (frame_sequence_idx == 0)
                {
                    set_focus_on_fisrt_frame(pcl_queue->front()->getPosition(0));
                }
                mMesh->parse_data(pcl_queue->front());
                parse_new_pcl = false;
                frame_sequence_idx++;
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
            if (*sequence_loaded && parse_new_pcl && (frame_sequence_idx < (pcl_vector->size())))
            {        
                // Use mutex to avoid race condition
                std::lock_guard<std::mutex> lock(frame_idx_mutex);                  
                mMesh->parse_data(pcl_vector->at(frame_sequence_idx));
                parse_new_pcl = false;
            }

            mMesh->render();

            // // Make sure that we always have 1 pcl left to visualize, otherwise pcl will be deleted 
            // // due to differnet in speed of rendering and receiving
            if (*sequence_loaded && (frame_sequence_idx < (pcl_vector->size() - 1)) && !is_paused)
            {
                // Use mutex to avoid race condition
                std::lock_guard<std::mutex> lock(frame_idx_mutex);
                frame_sequence_idx++;
                parse_new_pcl = true;
            } else if (*sequence_loaded && (frame_sequence_idx >= (pcl_vector->size() - 1)) && !is_paused) {
                frame_sequence_idx = 0;
            }
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
            frame_sequence_idx = 0;
            mPortal->zmq_run();
        } 
        else if (mRenderMode == RENDER_SEQUENCE)
        {
            mPortal->sequence_run();
            if (!pcl_vector->empty() && *sequence_loaded && frame_sequence_idx == (pcl_vector->size() - 1))
            {
                frame_sequence_idx = 0;
                is_paused = true;
            }
        }
    }

    void SceneView::stop()
    {
        switch (this->mRenderMode)
        {
        case RENDER_ZMQ:
            mPortal->stop_signal();
            frame_sequence_idx = 0;
            break;
        case RENDER_SEQUENCE:
            pcl_vector->clear();
            sequence_loaded = std::make_shared<bool>(false);
            frame_sequence_idx = 0;
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
                frame_sequence_idx = 0;
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
        return frame_sequence_idx;
    }

    void SceneView::set_frame_idx(size_t idx)
    {
        // Use mutex to avoid race condition
        std::lock_guard<std::mutex> lock(frame_idx_mutex);
        frame_sequence_idx = idx;
    }

    void SceneView::set_pause(bool pause)
    {
        is_paused = pause;
        parse_new_pcl = true;
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
}
