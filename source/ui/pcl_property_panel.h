#pragma once

#include "elems/light.h"
#include "ui/scene_view.h"
#include "ply_import/miniply.h"

#include "imgui.h"
#include "utils/imgui_widgets.h"
#include <ImFileBrowser.h>

namespace nui
{
    class PCL_Property_Panel
    {
    public:

        PCL_Property_Panel()
        {
            mCurrentPLYFile = "< ... >";

            mPLYFileDialog.SetTitle("Open PLY file");
            mPLYFileDialog.SetFileFilters({ ".ply" });
        }

        void render(nui::SceneView* mScene);

        void set_mesh_load_callback(const std::function<void(const std::string&)>& callback)
        {
            mPLYLoadCallback = callback;
        }

        void capturePointCloud(nui::SceneView* scene_view);

    private:
        std::string get_serialNumber(int index);
        void post_handle(nui::SceneView* scene_view);
        void handleMessage(nui::SceneView* scene_view, zmq::message_t message );

        std::function<void(const std::string&)> mPLYLoadCallback;
        std::function<void()> loadGLPclFunction;
        std::shared_ptr<std::thread> LoadPclThreadPtr;
        bool startLoadPclThread = true;

        //Drop down setup
        bool showDropdown = false;
        int selectedItem = 2;
        const char* items[3] = { "PLY file", "Folder Sequence", "Kinect(s)" }; // Example items

        // create a file browser instance
        ImGui::FileBrowser mPLYFileDialog;
        std::string mCurrentPLYFile;
        std::string mCurrentPLYFolder;

        //Control
        float point_size = 1.0f;
        bool StartButton_disable = false;

        //Kinect devices
        std::thread captureThread = std::thread();
        char serveraddrBuffer[256] = "tcp://*:5555"; // Buffer to hold the text
        bool stopServer = false;


        struct FPS_data
        {
            float values[90] = {};
            float curFPS = 0.0f;
            int values_offset = 0;
            std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
        } fps_data;

    };
}
