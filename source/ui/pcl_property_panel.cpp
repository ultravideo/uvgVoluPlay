#include "pch.h"
#include "pcl_property_panel.h"

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

            if (StartButton_disable)
            {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
            }
            if (ImGui::Button("Start", ImVec2(100, 20)))
            {
                startLoadPclThread = false;
                scene_view->set_input(selectedItem);
                StartButton_disable = true;
                fps_data.start_time = std::chrono::high_resolution_clock::now();
                
                fps_data.values[90] = {};
                fps_data.curFPS = 0.0f;
                fps_data.values_offset = 0;
                break;
            }
            if (StartButton_disable)
            {
                ImGui::PopItemFlag();
                ImGui::PopStyleVar();
            }

            ImGui::SameLine();

            if (ImGui::Button("Stop", ImVec2(100, 20)))
            {
                // stopServer = true;
                scene_view->stop();
                StartButton_disable = false;
                captureThread.join();
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
        
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Statistics"))
        {

            char overlay[32];
            if (StartButton_disable)
            {
                std::chrono::duration<double> durationProcess = std::chrono::high_resolution_clock::now() - fps_data.start_time;
                
                if (durationProcess.count() != 0.0)
                {
                    fps_data.curFPS = static_cast<float>(scene_view->get_render_frames())/durationProcess.count();
                    // std::cerr << "FPS: " << fps_data.curFPS << std::endl;
                    fps_data.values[fps_data.values_offset] = fps_data.curFPS;
                    fps_data.values_offset = (fps_data.values_offset + 1) % IM_ARRAYSIZE(fps_data.values);
                }
                
            }
            sprintf_s(overlay, "avg rate: %f FPS", fps_data.curFPS);
            ImGui::Text("Frame rate:");
            ImGui::SameLine();
            ImGui::PlotLines("", fps_data.values, IM_ARRAYSIZE(fps_data.values), fps_data.values_offset, overlay, 0.0f, 30.0f, ImVec2(0, 100.0f));
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
            loadGLPclFunction = [this, scene_view]() {
                scene_view->receivePointCloud(serveraddrBuffer);
            };
            
            // Create the thread using the stored std::function object
            // std::thread captureThread(loadGLPclFunction);
            captureThread = std::thread(loadGLPclFunction);
            captureThread.detach();

            startLoadPclThread = true;
        }
    }

    // void PCL_Property_Panel::handleMessage(nui::SceneView* scene_view, zmq::message_t message ) {
    //     nelems::GLPointCloud pointCloud;

    //     size_t numPoints = message.size() / (2 * sizeof(glm::vec3));
    //     std::shared_ptr<std::vector<glm::vec3>> data = std::make_shared<std::vector<glm::vec3>>(reinterpret_cast<const glm::vec3*>(message.data()), reinterpret_cast<const glm::vec3*>(message.data()) + 2* numPoints);

    //     // Form the received data into vectors
    //     std::shared_ptr<std::vector<glm::vec3>> positions = std::make_shared<std::vector<glm::vec3>>(data->begin(), data->begin() + numPoints);
    //     std::shared_ptr<std::vector<glm::vec3>> attributes = std::make_shared<std::vector<glm::vec3>>(data->begin() + numPoints, data->end());

    //     // Add the received point to the point cloud
    //     pointCloud.parse(positions, attributes);
    //     pointCloud.Clear();
    // }

    // void PCL_Property_Panel::capturePointCloud(nui::SceneView* scene_view)
    // {
        
    //     // // initialize the zmq context with a single IO thread
    //     zmq::context_t context{1};
    //     // construct a REP (reply) socket and bind to interface
    //     zmq::socket_t socket{context, zmq::socket_type::rep};
    //     socket.bind(serveraddrBuffer);

    //     // Queue for messages
    //     std::queue<zmq::message_t> messages;

    //     std::thread processThread([this, &messages, scene_view = std::move(scene_view)]() {
    //             while (true) {
    //                 if (messages.empty()) {

    //                     continue;
    //                 }
    //                 handleMessage(scene_view , std::move(messages.front()));
    //                 messages.pop();
    //             }
    //         });

    //     // Receive the point cloud data from the client
    //     while (true) {

    //         // Receive the message from the client
    //         zmq::message_t message;
    //         auto res = socket.recv(message, zmq::recv_flags::none);
    //         if (!res.has_value()) {
    //             continue;
    //         }

    //         messages.push(std::move(message));

    //         // Send a confirmation reply back to the client
    //         socket.send(zmq::buffer("Received point"), zmq::send_flags::none);
    //     }

    //     // Close the socket (This part of code is not reachable because of the infinite loop)
    //     socket.close();
         
    //     StartButton_disable = false;
    // }
}
