#pragma once

#include "imgui.h"
#include "utils/imgui_widgets.h"

namespace nui {
    class KeyboardHelperPanel {
    public:
        KeyboardHelperPanel() {

        }

        void render() {
            ImGui::Begin("Keyboard Helper");

            ImGui::BulletText("Application Control:");
            ImGui::Text("+ ESC: Exit Application");
            ImGui::Text("+ F1: Show/Hide Keyboard Helper");
            ImGui::Text("+ F2: Show/Hide Control Panel");

            ImGui::Separator();

            ImGui::BulletText("PointCloud Control:");
            ImGui::Text("+ I: Increase Point Size");
            ImGui::Text("+ K: Decrease Point Size");

            ImGui::Separator();

            ImGui::BulletText("Playback Control:");
            ImGui::Text("+ L: Forward Playback");
            ImGui::Text("+ J: Backward Playback");

            ImGui::Separator();
            
            ImGui::BulletText("FPS Control:");
            ImGui::Text("+ 1: Enable/Disable FPS Limit");
            ImGui::Text("+ 2: Increase FPS");
            ImGui::Text("+ 3: Decrease FPS");
            
            ImGui::Separator();

            ImGui::BulletText("Camera Control:");
            ImGui::Text("+ W: Zoom In");
            ImGui::Text("+ S: Zoom Out");
            ImGui::Text("+ A: Rotate Left");
            ImGui::Text("+ D: Rotate Right");
            ImGui::Text("+ Up Arrow: Pan Up");
            ImGui::Text("+ Down Arrow: Pan Down");
            ImGui::Text("+ Left Arrow: Pan Left");
            ImGui::Text("+ Right Arrow: Pan Right");

            ImGui::End();
        }
    };
}