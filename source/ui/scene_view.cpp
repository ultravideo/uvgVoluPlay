#include "pch.h"
#include "scene_view.h"

#include "imgui.h"
#include <chrono>
#include <thread>

namespace nui
{
    void SceneView::resize(int32_t width, int32_t height)
    {
        mSize.x = width;
        mSize.y = height;

        mFrameBuffer->create_buffers((int32_t)mSize.x, (int32_t) mSize.y);
    }

    void SceneView::set_input(int mode)
    {
        InputMode = mode;
    }

    void SceneView::on_mouse_move(double x, double y, nelems::EInputButton button)
    {
        mCamera->on_mouse_move(x, y, button);
    }

    void SceneView::on_mouse_wheel(double delta)
    {
        mCamera->on_mouse_wheel(delta);
    }

    void SceneView::load_mesh(const std::string& filepath)
    {
        // mMesh->clear_queue();
        // mMesh->load(filepath);
        // // mMesh->parse_data();
    }

    void SceneView::load_sequence(const std::string& folderpath)
    {
        load_sequence_flag = false;
        // Iterate over the directory
        for (const auto& entry : std::filesystem::directory_iterator(folderpath))
        {
            // Check if the entry is a regular file
            if (entry.is_regular_file())
            {
                if (entry.path().extension() == ".ply")
                {
                    // Add the file path to the list of files
                    mMesh->load(entry.path().string().c_str());
                }
            }
        }

        load_sequence_flag = true;
    }

    void SceneView::render()
    {
        mShader->use();

        //mLight->update(mShader.get());

        mFrameBuffer->bind();

        if (mMesh)
        {
            // if (InputMode > 0)// && load_sequence_flag)
            // {
            //     mMesh->update(mShader.get()); 
            // }

            std::shared_ptr<nelems::GLPointCloud> pointCloud = nullptr;
             
            mPortal->clear_front_point_cloud();
            mPortal->get_point_cloud(pointCloud);
            if (pointCloud)
            {                
                mMesh->parse_data(pointCloud);
                draw_points = static_cast<int>(pointCloud->max_size());
            }
            mMesh->render(draw_points);
        }

        mFrameBuffer->unbind();

        ImGui::Begin("Scene");

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        mSize = { viewportPanelSize.x, viewportPanelSize.y };

        mCamera->set_aspect(mSize.x / mSize.y);
        mCamera->update(mShader.get());

        // add rendered texture to ImGUI scene window
        uint64_t textureID = mFrameBuffer->get_texture();
        ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ mSize.x, mSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

        ImGui::End();
    }


    void SceneView::receivePointCloud()
    {
        mPortal->zmq_run();
    }

    // void SceneView::add_pcl(std::shared_ptr<nelems::GLPointCloud> pointCloud) { 
    //     // load_sequence_flag = false;
    //     mMesh->add_pcl(pointCloud);
    //     // load_sequence_flag = true; 
    // }

    void SceneView::stop()
    {
        mPortal->stop_signal();
    }
}
