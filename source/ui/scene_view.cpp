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

    void SceneView::render_zmq()
    {
        mShader->use();

        //mLight->update(mShader.get());

        mFrameBuffer->bind();

        if (mMesh)
        {
            if (!pcl_queue->empty())
            {                
                mMesh->parse_data(pcl_queue->front());
            }
            mMesh->render();
        }

        // Make sure that we always have 1 pcl left to visualize, otherwise pcl will be deleted 
        // due to differnet in speed of rendering and receiving
        if (pcl_queue->size() >= 2) 
        {
            pcl_queue->pop();
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

    void SceneView::set_pointSize(float pointSize)
    {
        mpointSize = pointSize;
    }


    void SceneView::receivePointCloud()
    {
        mPortal->zmq_run();
    }

    void SceneView::stop()
    {
        mPortal->stop_signal();
    }
}
