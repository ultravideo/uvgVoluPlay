#include "pch.h"
#include "pcl_property_panel.h"
#include "utils/log.hpp"

namespace nui
{

    void PCL_Property_Panel::render(nui::SceneView* scene_view)
    {
        //auto mesh = scene_view->get_mesh();

        ImGui::Begin("Input");

        if (ImGui::BeginCombo("##inputlist", items[selectedItem])) // The ##combo is a unique identifier
        {
            for (int i = 0; i < IM_ARRAYSIZE(items); i++)
            {
                bool isSelected = (selectedItem == i);
                if (ImGui::Selectable(items[i], isSelected))
                {
                    selectedItem = i;
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        switch (selectedItem)
        {
        case 0:
            // Action for item 1
            if (ImGui::CollapsingHeader("Select PLY file", ImGuiTreeNodeFlags_DefaultOpen))
            {

                if (ImGui::Button("Open..."))
                {
                    mPLYFileDialog.Open();
                }
                ImGui::SameLine(0, 5.0f);
                ImGui::Text(mCurrentPLYFile.c_str());
                set_mesh_load_callback([this, scene_view](std::string filepath) { scene_view->load_mesh(filepath); });
                scene_view->set_input(selectedItem);
            }
            break;
        case 1:
            if (ImGui::CollapsingHeader("Select Sequence Folder", ImGuiTreeNodeFlags_DefaultOpen))
            {

                if (ImGui::Button("Open..."))
                {
                    mPLYFileDialog.Open();
                }
                ImGui::SameLine(0, 5.0f);
                ImGui::Text(mCurrentPLYFolder.c_str());
                set_mesh_load_callback([this, scene_view](std::string filepath) { scene_view->load_sequence(filepath); });
                scene_view->set_input(selectedItem);
            }
            break;
        case 2:
            // Action for item 3
            ImGui::Text("Server address: ");
            ImGui::SameLine();
            ImGui::InputText(" s", serveraddrBuffer, IM_ARRAYSIZE(serveraddrBuffer));

            if (ImGui::Button("Start", ImVec2(100, 20)))
            {
                startLoadPclThread = false;
                scene_view->set_input(selectedItem);
                StartButton_disable = true;
                utilities::Logger::log(utilities::LogLevel::INFO, "Viewport Controller", "Signal to start Portal\n");
                break;
            }

            ImGui::SameLine();

            if (ImGui::Button("Stop", ImVec2(100, 20)))
            {
                scene_view->stop();
                StartButton_disable = false;
                utilities::Logger::log(utilities::LogLevel::INFO, "Viewport Controller", "Signal to stop Portal\n");
            }

            break;
        default:
            break;
        }

        ImGui::Separator();
        if (ImGui::CollapsingHeader("Control"))
        {        
            ImGui::Text("Point Size:");
            ImGui::SameLine();
            if (ImGui::SliderFloat(" ", &point_size, 1.0f, 5.0f)) {
                scene_view->set_pointSize(point_size);
            }
            if (ImGui::Button("Reset View", ImVec2(100, 20)))
            {
                scene_view->reset_view();
            }

            ImGui::SameLine();

            if (ImGui::Button("Clear queue", ImVec2(100, 20)))
            {
                scene_view->clean_pclqueue();
            }
        }
    
        ImGui::End();

        post_handle(scene_view);

    }

    void PCL_Property_Panel::post_handle(nui::SceneView* scene_view)
    {
        mPLYFileDialog.Display();
        if (mPLYFileDialog.HasSelected() && selectedItem == 0)
        {
            auto file_path = mPLYFileDialog.GetSelected().string();
            mCurrentPLYFile = file_path.substr(file_path.find_last_of("\\") + 1);

            mPLYLoadCallback(file_path);

            mPLYFileDialog.ClearSelected();
        }
        else if (mPLYFileDialog.HasSelected() && selectedItem == 1)
        {

            mPLYFileDialog.Close();
            auto file_path = mPLYFileDialog.GetSelected().string();

            // Extract the directory path containing the file
            std::filesystem::path directory_path = std::filesystem::path(file_path).parent_path();

            // Convert std::filesystem::path to const char*
            mCurrentPLYFolder = directory_path.string();

            // Call your load callback in a separate thread
            std::thread loadThread([this]() {
               mPLYLoadCallback(mCurrentPLYFolder);
                });
            loadThread.detach();

            // Clear the selected file in the file dialog
            mPLYFileDialog.ClearSelected();
        }
        else if (selectedItem == 2 && !startLoadPclThread)
        {
            
            std::function<void()> loadGLPclFunction = [this, scene_view]() { scene_view->receivePointCloud(); };

            if (captureThread.joinable())
            {
                captureThread.join();
            }
            // Create the thread using the stored std::function object
            // std::thread captureThread(loadGLPclFunction);
            captureThread = std::thread(loadGLPclFunction);
            // captureThread.detach();
std::cout << "Triggered\n";
            startLoadPclThread = true;
        }
    }
}
