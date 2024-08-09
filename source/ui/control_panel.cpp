#include "pch.h"
#include "control_panel.h"
#include "utils/log.hpp"

namespace nui
{
    PCL_Property_Panel::PCL_Property_Panel() {
        mCurrentPLYFile = "< ... >";
        mPLYFileDialog.SetTitle("Open PLY file");
        mPLYFileDialog.SetFileFilters({ ".ply" });
    }

    PCL_Property_Panel::~PCL_Property_Panel() {
        if (captureThread.joinable())
        {
            captureThread.join();
        }
    }

    void PCL_Property_Panel::render()
    {
        ImGui::Begin("Controller");
        
        if (ImGui::BeginCombo("##inputlist_rendermode", render_mode_items[selected_render_mode])) // The ##combo is a unique identifier
        {
            for (int i = 0; i < IM_ARRAYSIZE(render_mode_items); i++)
            {
                bool isSelected = (selected_render_mode == i);
                if (ImGui::Selectable(render_mode_items[i], isSelected))
                {
                    selected_render_mode = i;
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

        switch (selected_render_mode)
        {
        case 0:
            if (ImGui::BeginCombo("##inputlist_scenelist", mSceneView_Names.at(selected_scene_index).c_str())) // The ##combo is a unique identifier
            {
                for (size_t i = 0; i < mSceneView_Container->size(); i++)
                {
                    bool isSelected = (selected_scene_index == i);
                    if (ImGui::Selectable(mSceneView_Names.at(i).c_str(), isSelected))
                    {
                        selected_scene_index = i;
                    }
                    if (isSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine(UI_configation.offset_from_start_x, UI_configation.spacing_x);
            ImGui::Text(" Select Scene");
        
            if (ImGui::Button("Open..."))
            {
                mPLYFileDialog.Open();
            }
            ImGui::SameLine(UI_configation.offset_from_start_x, UI_configation.spacing_x);
            ImGui::Text("Sequence Folder: %s", mCurrentPLYFolder.c_str());
            
            break;
        case 1:
            ImGui::Text("Receive from Kinect(s)");
            break;
        default:
            break;
        }

        if (ImGui::Button("Start", ImVec2(100, 20)) && !StartButton_disable)
        {
            if ((mCurrentPLYFolder == "") && selected_render_mode == 0) {
                utilities::Logger::log(utilities::LogLevel::ERROR, "Viewport Controller", "Please select folder of the sequence\n");
            }
            else {
                startLoadPclThread = false;
                StartButton_disable = true;

                for (auto& scene_view : *mSceneView_Container)
                {
                    scene_view->run();
                }

                utilities::Logger::log(utilities::LogLevel::INFO, "Viewport Controller", "Signal to start Portal\n");
            }
        }

        ImGui::SameLine(UI_configation.offset_from_start_x, UI_configation.spacing_x);

        if (ImGui::Button("Stop", ImVec2(100, 20)) && StartButton_disable)
        {
            for (auto& scene_view : *mSceneView_Container)
            {
                scene_view->stop();
            }

            StartButton_disable = false;
            utilities::Logger::log(utilities::LogLevel::INFO, "Viewport Controller", "Signal to stop Portal\n");
        }

        if (selected_render_mode == 0)
        {   
            if (selected_render_mode == 0 && StartButton_disable) {

                if (ImGui::Button("Restart", ImVec2(100, 20)))
                {
                    startLoadPclThread = false;
                }
            }
        }

        ImGui::Separator();
        if (ImGui::CollapsingHeader("View Control", ImGuiTreeNodeFlags_DefaultOpen))
        {   
            if (ImGui::SliderFloat("Point Size", &point_size, 1.0f, 5.0f)) {
                for (auto& scene_view : *mSceneView_Container)
                {
                    scene_view->set_pointSize(point_size);
                }
            }    

            if (ImGui::Button("Reset View", ImVec2(100, 20)))
            {
                for (auto& scene_view : *mSceneView_Container)
                {
                    scene_view->reset_view();
                }
            }

            ImGui::SameLine(UI_configation.offset_from_start_x, UI_configation.spacing_x);

            if (ImGui::Button("Add Views", ImVec2(100, 20)))
            {
                std::shared_ptr<nui::SceneView> new_scene_view = std::make_shared<nui::SceneView>();
                new_scene_view->set_scene_name("Scene " + std::to_string(mSceneView_Container->size()));
                mSceneView_Container->push_back(new_scene_view);
                mSceneView_Names.push_back(new_scene_view->get_scene_name());
            }
        }

        ImGui::Separator();
        if (ImGui::CollapsingHeader("Scene Control", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (total_frames != 0) { 
                if (ImGui::SliderInt("Playback", &current_frame, 0, total_frames))
                {

                }
            }

        }
    
        ImGui::End();

        post_handle();
    }

    void PCL_Property_Panel::set_scene_view_container(std::shared_ptr<std::vector<std::shared_ptr<nui::SceneView>>> &scene_view_container)
    {
        mSceneView_Container = scene_view_container;

        for (auto& scene_view : *mSceneView_Container)
        {
            mSceneView_Names.push_back(scene_view->get_scene_name());
        }
    }

    void PCL_Property_Panel::post_handle()
    {
        mPLYFileDialog.Display();
        if (mPLYFileDialog.HasSelected() && selected_render_mode == 0)
        {
            auto file_path = mPLYFileDialog.GetSelected().string();

            // Extract the directory path containing the file
            std::filesystem::path directory_path = std::filesystem::path(file_path).parent_path();

            // Convert std::filesystem::path to const char*
            mCurrentPLYFolder = directory_path.string();
            for (auto& scene_view : *mSceneView_Container)
            {
                if (mSceneView_Names.at(selected_scene_index) == scene_view->get_scene_name())
                {
                    scene_view->set_render_mode(selected_render_mode);
                    scene_view->set_sequence_path(mCurrentPLYFolder);
                    utilities::Logger::log(utilities::LogLevel::INFO, "Viewport Controller", "Set sequence path to: " + mCurrentPLYFolder + " for " + scene_view->get_scene_name() + "\n");
                }
            }

            mPLYFileDialog.ClearSelected();
        }

        if (selected_render_mode == 0 && !startLoadPclThread)
        {
            mPLYFileDialog.Close();

            // mPLYFileDialog.ClearSelected();

            for (auto& scene_view : *mSceneView_Container)
            {
                // scene_view_start(scene_view.get()); 
                scene_view->receivePointCloud();
                std::cout << "Start loading PCL thread for " << scene_view->get_scene_name() << std::endl;
            }
            startLoadPclThread = true;
        }
        else if (selected_render_mode == 1 && !startLoadPclThread)
        {
            mSceneView_Container->front()->set_render_mode(selected_render_mode);
            scene_view_start(mSceneView_Container->front().get());
            startLoadPclThread = true;
        }
    }

    void PCL_Property_Panel::scene_view_start(nui::SceneView* scene_view)
    {
        // std::function<void()> loadGLPclFunction = [this, scene_view]() { scene_view->receivePointCloud(); };

        // captureThread = std::thread(loadGLPclFunction);
        std::thread([this, scene_view]() { scene_view->receivePointCloud(); });
    }
}
