#pragma once

#include "elems/light.h"
#include "ui/scene_view.h"
#include "ply_import/miniply.h"

#include "imgui.h"
#include "utils/imgui_widgets.h"
#include "3rdparty/plugins/imgui/ImFileBrowser.h"

namespace nui
{
    class PCL_Property_Panel
    {
    public:
        PCL_Property_Panel();
        ~PCL_Property_Panel();

        void render();
        void set_scene_view_container(std::shared_ptr<std::vector<std::shared_ptr<nui::SceneView>>> &scene_view_container);

    private:
        std::string get_serialNumber(int index);
        void post_handle();
        void scene_view_start(nui::SceneView* scene_view);
        void main_control_handle();
        void view_control_handle();
        void scene_view_control_handle();
        void start_button_handle();
        void stop_button_handle();

        struct UI_config {
            //Same line
            float spacing_x = 5.0f;
            float offset_from_start_x = 0.0f;
        } UI_configation;

    private:
        std::shared_ptr<std::thread> LoadPclThreadPtr;
        bool start_portal_falg = true;

        //Drop down setup
        bool showDropdown = false;
        int selected_render_mode = 1; // Index of the selected item
        const char* render_mode_items[2] = { "Folder Sequence", "ZMQ sender" }; // Example items

        // create a file browser instance
        ImGui::FileBrowser mPLYFileDialog;
        std::string mCurrentPLYFile;
        std::string mCurrentPLYFolder = "";

        //Control
        float point_size = 1.0f;
        bool StartButton_disable = false;

        //Kinect devices
        std::thread captureThread;
        bool stopServer = false;

        //SceneView container
        size_t selected_scene_index = 0;
        std::shared_ptr<std::vector<std::shared_ptr<nui::SceneView>>> mSceneView_Container = nullptr;
        std::vector<std::pair<std::string, std::string>> mSceneView_Names; // <SceneName, Sequence_Path>
        
        int current_frame = 0;
        int total_frames = 0;
        float bg_color[3] = { 1.0f, 1.0f, 1.0f };

        char zmq_position_address[64] = "tcp://*:5556";
        char zmq_color_address[64] = "tcp://*:5555";
    };
}
