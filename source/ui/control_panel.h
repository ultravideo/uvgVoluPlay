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

        void set_scene_view_container(std::shared_ptr<std::vector<std::shared_ptr<nui::SceneView>>> &scene_view_container)
        {
            mSceneView_Container = scene_view_container;
        }

    private:
        std::string get_serialNumber(int index);
        void post_handle(nui::SceneView* scene_view);
        // void handleMessage(nui::SceneView* scene_view, zmq::message_t message );
        void scene_view_start(nui::SceneView* scene_view);

        struct UI_config {
            //Same line
            float spacing_x = 5.0f;
            float offset_from_start_x = 0.0f;
        } UI_configation;

    private:
        std::function<void(const std::string&)> mPLYLoadCallback;
        std::shared_ptr<std::thread> LoadPclThreadPtr;
        bool startLoadPclThread = true;

        //Drop down setup
        bool showDropdown = false;
        int selectedItem = 0; // Index of the selected item
        const char* items[2] = { "Folder Sequence", "Kinect(s)" }; // Example items

        // create a file browser instance
        ImGui::FileBrowser mPLYFileDialog;
        std::string mCurrentPLYFile;
        std::string mCurrentPLYFolder = "";

        //Control
        float point_size = 1.0f;
        bool StartButton_disable = false;

        //Kinect devices
        std::thread captureThread;
        char serveraddrBuffer[256] = "tcp://*:5555"; // Buffer to hold the text
        bool stopServer = false;

        //Testing
        std::shared_ptr<std::vector<std::shared_ptr<nui::SceneView>>> mSceneView_Container = nullptr;
    };
}
