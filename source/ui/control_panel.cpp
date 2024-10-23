#include "pch.h"
#include "control_panel.h"
#include "utils/log.hpp"

namespace nui
{
    Control_Panel::Control_Panel(std::shared_ptr<int> _FPS, std::shared_ptr<bool> _limited_frame_rate) {
        mCurrentPLYFile = "< ... >";
        mPLYFileDialog.SetTitle("Open PLY file");
        mPLYFileDialog.SetFileFilters({ ".ply" });

        mFPS = _FPS;
        mlimited_frame_rate = _limited_frame_rate;
    }

    Control_Panel::~Control_Panel() {
        if (captureThread.joinable())
        {
            captureThread.join();
        }
    }

    void Control_Panel::render()
    {
        ImGui::Begin("Controller");
        
        this->main_control_handle();

        ImGui::Separator();

        this->view_control_handle();

        ImGui::Separator();

        this->scene_view_control_handle();

        ImGui::End();

        post_handle();
    }

    void Control_Panel::main_control_handle() {
        if (ImGui::CollapsingHeader("Main Control", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::AlignTextToFramePadding();
            ImGui::BulletText("Render Mode ");
            ImGui::SameLine(UI_configation.offset_from_start_x, UI_configation.spacing_x);
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

            switch (selected_render_mode)
            {
            case 0:
                ImGui::AlignTextToFramePadding();
                ImGui::BulletText("Select sequence Folder: ");

                ImGui::BulletText("Scenes Information:");
                ImGui::Text("Note: Click respective scene to select folder");
                ImGui::BeginTable("##scene_table", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);
                ImGui::TableSetupColumn("Scene Name", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                ImGui::TableSetupColumn("Sequence Name", ImGuiTableColumnFlags_WidthStretch, 100.0f);
                ImGui::TableSetupColumn("Total Frames", ImGuiTableColumnFlags_WidthStretch, 100.0f);
                ImGui::TableHeadersRow();
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(60, 136, 158, 255));

                if (mSceneView_Container->size() > 0 && mSceneView_Names.size() > 0)
                {
                    for (size_t i = 0; i < mSceneView_Container->size(); i++)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", mSceneView_Names.at(i).first.c_str());
                        ImGui::TableNextColumn();

                        if (ImGui::Selectable(mSceneView_Names.at(i).second.c_str()))
                        {
                            mPLYFileDialog.Open();
                            selected_scene_index = i;
                            ImGui::Text("%s", mSceneView_Names.at(i).second.c_str());
                        }
                        
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", mSceneView_Container->at(i)->get_total_frames());
                    }
                }

                ImGui::EndTable();
                ImGui::Separator();
                
                break;
            case 1:
                ImGui::Text("Receive from Depth Sensor(s)");
                // Textbox for the IP addresses
                ImGui::AlignTextToFramePadding();
                ImGui::BulletText("Position Socket: ");
                ImGui::SameLine(UI_configation.offset_from_start_x, UI_configation.spacing_x);
                ImGui::InputText("##position_socket", zmq_position_address, 64);

                ImGui::AlignTextToFramePadding();
                ImGui::BulletText("Color Socket:    ");
                ImGui::SameLine(UI_configation.offset_from_start_x, UI_configation.spacing_x);
                ImGui::InputText("##color_socket", zmq_color_address, 64);
                break;
            default:
                break;
            }

            ImGui::AlignTextToFramePadding();
            ImGui::BulletText("Mode Control: ");
            ImGui::SameLine(UI_configation.offset_from_start_x, UI_configation.spacing_x);

            if (ImGui::Button("Start", ImVec2(100, 20)) && !StartButton_disable)
            {
                this->start_button_handle();
            }

            ImGui::SameLine(UI_configation.offset_from_start_x, UI_configation.spacing_x);

            if (ImGui::Button("Stop", ImVec2(100, 20)) && StartButton_disable)
            {
                this->stop_button_handle();
            }
        }
    }

    void Control_Panel::view_control_handle()
    {
        if (ImGui::CollapsingHeader("View Control", ImGuiTreeNodeFlags_DefaultOpen))
            {   
                ImGui::AlignTextToFramePadding();
                ImGui::BulletText("Number of Views: %d --- ", static_cast<int>(mSceneView_Container->size()));
                ImGui::SameLine(UI_configation.offset_from_start_x, UI_configation.spacing_x);
                if (ImGui::Button("Add Views", ImVec2(100, 20)))
                {
                    if (selected_render_mode == 1 && mSceneView_Container->size() >= 1){
                        utilities::Logger::log(utilities::LogLevel::ERROR, "Viewport Controller", "ZMQ mode only support 1 scene view\n");
                    } else {
                        std::shared_ptr<nui::SceneView> new_scene_view = std::make_shared<nui::SceneView>();
                        new_scene_view->set_scene_name("Scene " + std::to_string(mSceneView_Container->size()));
                        mSceneView_Container->push_back(new_scene_view);
                        mSceneView_Names.push_back(std::make_pair(new_scene_view->get_scene_name(), ""));
                    }
                }

                ImGui::Text(" ");
                ImGui::BulletText("Background Color:");

                ImGui::ColorPicker3("Background Color", bg_color, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);

                for (auto& scene_view : *mSceneView_Container)
                {
                    scene_view->set_background_color(bg_color[0], bg_color[1], bg_color[2]);
                }
                
                ImGui::Text(" ");
            }

    }

    void Control_Panel::scene_view_control_handle() {
        
        if (ImGui::CollapsingHeader("Scene Control", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::BulletText("Limit square Frame Rate:");
            ImGui::Checkbox("Enable Frame Rate Limit", mlimited_frame_rate.get());
            static int frame_rate = 60;
            if (ImGui::SliderInt(" Frame Rate", &frame_rate, 5, 120) && *mlimited_frame_rate)
            {
                // Force the frame rate to be a multiple of 5
                // *mFPS = frame_rate;
                for (auto& scene_view : *mSceneView_Container)
                {
                    scene_view->set_FPS(frame_rate);
                }
            }
            frame_rate = ((frame_rate / 5) * 5 > 0) ? (frame_rate / 5) * 5 : 5;

            if (!StartButton_disable)
            {
                return;
            }

            ImGui::BulletText("Set point size:");
            if (ImGui::SliderFloat(" Size", &point_size, 1.0f, 10.0f)) {
                for (auto& scene_view : *mSceneView_Container)
                {
                    scene_view->set_pointSize(point_size);
                }
            }

            ImGui::BulletText("Timestamp control:");
            for (auto& scene_view : *mSceneView_Container)
            {
                if (scene_view->get_scene_name() == mSceneView_Names.at(selected_scene_index).first && scene_view->get_total_frames() > 0) 
                {
                    total_frames = scene_view->get_total_frames() - 1;
                    break;
                }
            }
            
            if (ImGui::SliderInt("Current Frame", &current_frame, 0, total_frames))
            {
                for (auto& scene_view : *mSceneView_Container)
                {
                    scene_view->set_frame_idx(current_frame);
                    scene_view->set_pause(true);
                }
            }


            for (auto& scene_view : *mSceneView_Container)
            {
                if (scene_view->get_scene_name() == mSceneView_Names.at(selected_scene_index).first)
                {
                    if (scene_view->get_current_frame() != current_frame)
                    {
                        current_frame = scene_view->get_current_frame();
                    }
                    break;
                }
            }


            ImGui::BulletText("Playback control:");
            // Backward and Forward buttons and play button
            if (ImGui::Button("<<", ImVec2(50, 20)))
            {
                if (current_frame > 0)
                {
                    current_frame = current_frame - 1;
                    for (auto& scene_view : *mSceneView_Container)
                    {
                        scene_view->set_frame_idx(current_frame);
                        scene_view->set_pause(true);
                    }
                }
            }
            ImGui::SameLine(UI_configation.offset_from_start_x, UI_configation.spacing_x);
            if (ImGui::Button("|>", ImVec2(50, 20)))
            {
                for (auto& scene_view : *mSceneView_Container)
                {
                    scene_view->set_frame_idx(current_frame);
                    scene_view->set_pause(false);
                }
            }
            ImGui::SameLine(UI_configation.offset_from_start_x, UI_configation.spacing_x);
            if (ImGui::Button(">>", ImVec2(50, 20)))
            {
                if (current_frame < total_frames)
                {
                    current_frame = current_frame + 1;
                    for (auto& scene_view : *mSceneView_Container)
                    {
                        scene_view->set_frame_idx(current_frame);
                        scene_view->set_pause(true);
                    }
                }

            }

            ImGui::SameLine(UI_configation.offset_from_start_x, UI_configation.spacing_x);
            if (ImGui::Button("Pause", ImVec2(100, 20)))
            {
                for (auto& scene_view : *mSceneView_Container)
                {
                    scene_view->set_pause(true);
                }
            }

            if (ImGui::Button("Reset Camera View", ImVec2(160, 20)))
            {
                for (auto& scene_view : *mSceneView_Container)
                {
                    scene_view->reset_view();
                }
            }
            ImGui::SameLine(UI_configation.offset_from_start_x, UI_configation.spacing_x);
            if (ImGui::Button("Restart", ImVec2(100, 20)))
            {
                if (selected_render_mode == 0 && StartButton_disable) {
                    start_portal_falg = false;
                    current_frame = 0;
                    for (auto& scene_view : *mSceneView_Container)
                    {
                        scene_view->set_frame_idx(current_frame);
                        scene_view->set_pause(false);
                    }
                }
            }
        }
    }

    void Control_Panel::set_scene_view_container(std::shared_ptr<std::vector<std::shared_ptr<nui::SceneView>>> &scene_view_container)
    {
        mSceneView_Container = scene_view_container;

        for (auto& scene_view : *mSceneView_Container)
        {
            mSceneView_Names.push_back(std::make_pair(scene_view->get_scene_name(), ""));
        }
    }

    void Control_Panel::start_button_handle() {
        bool all_scene_view_has_sequence = true;
        
        if (selected_render_mode == 0) {
            if (mSceneView_Container->empty())
            {
                utilities::Logger::log(utilities::LogLevel::ERROR, "Viewport Controller", "Please add at least 1 scene view to render PLY sequnece.\n");
                return;
            }

            for (auto& scene_view : mSceneView_Names)
            {
                // Verify the path to sequence folder is valid
                if (scene_view.second.empty())
                {
                    all_scene_view_has_sequence = false;
                    break;
                }
            }

            if (all_scene_view_has_sequence) {
                start_portal_falg = false;
                StartButton_disable = true;

                for (auto& scene_view : *mSceneView_Container)
                {
                    scene_view->run();
                }
            } else {
                utilities::Logger::log(utilities::LogLevel::ERROR, "Viewport Controller", "Please select folder of the sequence\n");
                return;
            }
        } else if (selected_render_mode == 1) {
            if (mSceneView_Container->empty())
            {
                std::shared_ptr<nui::SceneView> new_scene_view = std::make_shared<nui::SceneView>();
                new_scene_view->set_scene_name("Scene " + std::to_string(0));
                mSceneView_Container->push_back(new_scene_view);
                mSceneView_Names.push_back(std::make_pair(new_scene_view->get_scene_name(), ""));
            }

            start_portal_falg = false;
            StartButton_disable = true;

            for (auto& scene_view : *mSceneView_Container)
            {
                if (selected_render_mode == 1) {
                    scene_view->set_render_mode(selected_render_mode);
                    scene_view->setup_socket(zmq_position_address, zmq_color_address);
                }
                scene_view->run();
            }
        }

        utilities::Logger::log(utilities::LogLevel::INFO, "Viewport Controller", "Signal to start Portal\n");
    } 

    void Control_Panel::stop_button_handle() {
        for (auto& scene_view : *mSceneView_Container)
        {
            scene_view->stop();
        }
                
        if (captureThread.joinable())
        {
            captureThread.join();
        }

        mSceneView_Container->clear();
        mSceneView_Names.clear();

        StartButton_disable = false;
        
        utilities::Logger::log(utilities::LogLevel::INFO, "Viewport Controller", "Signal to stop Portal\n");
    }

    void Control_Panel::post_handle()
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
                if (mSceneView_Names.at(selected_scene_index).first == scene_view->get_scene_name())
                {
                    scene_view->set_render_mode(selected_render_mode);
                    scene_view->set_sequence_path(mCurrentPLYFolder);
                    std::string folder_name = mCurrentPLYFolder.substr(mCurrentPLYFolder.find_last_of("/\\") + 1);

                    // Verify the folder is valid
                    if (std::filesystem::exists(mCurrentPLYFolder) &&  std::filesystem::is_directory(mCurrentPLYFolder)) {
                        mSceneView_Names.at(selected_scene_index).second = folder_name;
                        utilities::Logger::log(utilities::LogLevel::INFO, "Viewport Controller", "Set sequence path to: " + mCurrentPLYFolder + " for " + scene_view->get_scene_name() + "\n");
                    }  else {
                        utilities::Logger::log(utilities::LogLevel::ERROR, "Viewport Controller", "Invalid folder path or folder is empty\n");
                    }
                    break;  
                }
            }

            mPLYFileDialog.ClearSelected();
        }

        if (selected_render_mode == 0 && !start_portal_falg)
        {
            mPLYFileDialog.Close();

            for (auto& scene_view : *mSceneView_Container)
            {
                // scene_view_start(scene_view.get()); 
                // Cannot parallelize the scene_view_start due to the miniply library reading file corrupts when multiple threads are reading the same file
                scene_view->receivePointCloud();
            }
            start_portal_falg = true;
        }
        else if (selected_render_mode == 1 && !start_portal_falg)
        {
            scene_view_start(mSceneView_Container->front().get());
            start_portal_falg = true;
        }
    }

    void Control_Panel::scene_view_start(nui::SceneView* scene_view)
    {
        captureThread = std::thread([this, scene_view]() { scene_view->receivePointCloud(); });
    }
}
