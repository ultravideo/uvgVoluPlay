#include "pch.h"
#include "control_panel.h"
#include "utils/log.hpp"

namespace nui
{

    void PCL_Property_Panel::render(nui::SceneView* scene_view)
    {
        ImGui::Begin("Controller");
        
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
        ImGui::SameLine(UI_configation.offset_from_start_x, UI_configation.spacing_x);
        ImGui::Text(" Render Mode");

        switch (selectedItem)
        {
        // case 0:
        //     // Action for item 1
        //     if (ImGui::CollapsingHeader("Select PLY file", ImGuiTreeNodeFlags_DefaultOpen))
        //     {

        //         if (ImGui::Button("Open..."))
        //         {
        //             mPLYFileDialog.Open();
        //         }
        //         ImGui::SameLine(UI_configation.offset_from_start_x, UI_configation.spacing_x);
        //         ImGui::Text(mCurrentPLYFile.c_str());
        //     }
        //     break;
        case 0:
            if (ImGui::Button("Open..."))
            {
                mPLYFileDialog.Open();
            }
            ImGui::SameLine(UI_configation.offset_from_start_x, UI_configation.spacing_x);
            ImGui::Text(mCurrentPLYFolder.c_str());
            break;
        case 1:
            // Action for item 3
            // ImGui::Text("Server address: ");
            // ImGui::SameLine(UI_configation.offset_from_start_x, UI_configation.spacing_x);
            // ImGui::InputText(" s", serveraddrBuffer, IM_ARRAYSIZE(serveraddrBuffer));
            ImGui::Text("Receive from Kinect(s)");
            break;
        default:
            break;
        }

        if (ImGui::Button("Start", ImVec2(100, 20)) && !StartButton_disable)
        {
            if ((mCurrentPLYFolder == "") && selectedItem == 0) {
                utilities::Logger::log(utilities::LogLevel::ERROR, "Viewport Controller", "Please select folder of the sequence\n");
            }
            else {
                startLoadPclThread = false;
                StartButton_disable = true;
                scene_view->set_render_mode(selectedItem);

                for (auto& scene_view : *mSceneView_Container)
                {
                    scene_view->set_render_mode(selectedItem);
                }

                utilities::Logger::log(utilities::LogLevel::INFO, "Viewport Controller", "Signal to start Portal\n");
            }
        }

        ImGui::SameLine(UI_configation.offset_from_start_x, UI_configation.spacing_x);

        if (ImGui::Button("Stop", ImVec2(100, 20)) && StartButton_disable)
        {
            scene_view->stop();
            // mSceneView_Container->at(0)->stop();
            StartButton_disable = false;
            utilities::Logger::log(utilities::LogLevel::INFO, "Viewport Controller", "Signal to stop Portal\n");
        }

        if (selectedItem == 0)
        {   
            if (selectedItem == 0 && StartButton_disable) {

                if (ImGui::Button("Restart", ImVec2(100, 20)))
                {
                    startLoadPclThread = false;
                }
            }
            
            ImGui::SameLine(UI_configation.offset_from_start_x, UI_configation.spacing_x);

            if (ImGui::Button("Add View", ImVec2(100, 20)))
            {
                std::shared_ptr<nui::SceneView> new_scene_view = std::make_shared<nui::SceneView>();
                new_scene_view->set_scene_name("Scene " + std::to_string(mSceneView_Container->size()));
                mSceneView_Container->push_back(new_scene_view);
                // utilities::Logger::log(utilities::LogLevel::INFO, "Viewport Controller", "Dummy for now\n");
            }
            }

        ImGui::Separator();
        if (ImGui::CollapsingHeader("View Control", ImGuiTreeNodeFlags_DefaultOpen))
        {   
            if (ImGui::SliderFloat(" ", &point_size, 1.0f, 5.0f)) {
                scene_view->set_pointSize(point_size);
            }    
            ImGui::SameLine(UI_configation.offset_from_start_x, UI_configation.spacing_x); 
            ImGui::Text("Point Size");

            if (ImGui::Button("Reset View", ImVec2(100, 20)))
            {
                scene_view->reset_view();
            }
        }

        // ImGui::Separator();
        // if (ImGui::CollapsingHeader("Scene Control", ImGuiTreeNodeFlags_DefaultOpen))
        // {
            // ImGui::Text("Scene Name:");


        // }
    
        ImGui::End();

        post_handle(scene_view);
    }

    void PCL_Property_Panel::post_handle(nui::SceneView* scene_view)
    {
        mPLYFileDialog.Display();
        if (mPLYFileDialog.HasSelected() && selectedItem == 0)
        {
            auto file_path = mPLYFileDialog.GetSelected().string();

            // Extract the directory path containing the file
            std::filesystem::path directory_path = std::filesystem::path(file_path).parent_path();

            // Convert std::filesystem::path to const char*
            mCurrentPLYFolder = directory_path.string();
        }

        if (selectedItem == 0 && !startLoadPclThread)
        {
            mPLYFileDialog.Close();

            scene_view->set_sequence_path(mCurrentPLYFolder);
            // mSceneView_Container->at(0)->set_sequence_path(mCurrentPLYFolder);
            // Clear the selected file in the file dialog
            mPLYFileDialog.ClearSelected();

            scene_view_start(scene_view);

            for (auto& scene_view : *mSceneView_Container)
            {
                scene_view->set_sequence_path(mCurrentPLYFolder);
                scene_view_start(scene_view.get());
            }

            // scene_view_start(mSceneView_Container->at(0).get());
        }
        else if (selectedItem == 1 && !startLoadPclThread)
        {
            scene_view_start(scene_view);
        }
    }

    void PCL_Property_Panel::scene_view_start(nui::SceneView* scene_view)
    {
        std::function<void()> loadGLPclFunction = [this, scene_view]() { scene_view->receivePointCloud(); };

        if (captureThread.joinable())
        {
            captureThread.join();
        }

        captureThread = std::thread(loadGLPclFunction);
        startLoadPclThread = true;
    }
}
