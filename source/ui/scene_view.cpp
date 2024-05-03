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

  void SceneView::load_mesh(const std::string& filepath)
  {
    mMesh->clear_queue();
    mMesh->load(filepath);
    mMesh->parse_data();
  }

  void SceneView::load_sequence(const std::string& folderpath)
  {
      load_sequence_flag = false;
      // Iterate over the directory
      for (const auto& entry : std::filesystem::directory_iterator(folderpath))
      {
          // Check if the entry is a regular file
          if (entry.is_regular_file())
          {
              if (entry.path().extension() == ".ply")
              {
                  // Add the file path to the list of files
                  mMesh->load(entry.path().string().c_str());
              }
          }
      }

      load_sequence_flag = true;
  }

  void SceneView::render()
  {
    mShader->use();

    //mLight->update(mShader.get());

    mFrameBuffer->bind();

    if (mMesh)
    {
        if (InputMode > 0 && load_sequence_flag)
        {
            // std::this_thread::sleep_for(std::chrono::microseconds(1000/30));
            mMesh->update(mShader.get()); 
        }
        mMesh->render();
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

  void SceneView::handleMessage(zmq::message_t message) {
        std::shared_ptr<nelems::GLPointCloud> pointCloud = std::make_shared<nelems::GLPointCloud>();

        size_t numPoints = message.size() / (2 * sizeof(glm::vec3));
        std::shared_ptr<std::vector<glm::vec3>> data = std::make_shared<std::vector<glm::vec3>>(reinterpret_cast<const glm::vec3*>(message.data()), reinterpret_cast<const glm::vec3*>(message.data()) + 2* numPoints);

        // Form the received data into vectors
        std::shared_ptr<std::vector<glm::vec3>> positions = std::make_shared<std::vector<glm::vec3>>(data->begin(), data->begin() + numPoints);
        std::shared_ptr<std::vector<glm::vec3>> attributes = std::make_shared<std::vector<glm::vec3>>(data->begin() + numPoints, data->end());

        // Add the received point to the point cloud
        pointCloud->parse(positions, attributes);
        add_pcl(pointCloud); 
  }

    void SceneView::receivePointCloud(std::string serveraddrBuffer)
    {
        
        // // initialize the zmq context with a single IO thread
        zmq::context_t context{1};
        // construct a REP (reply) socket and bind to interface
        zmq::socket_t socket{context, zmq::socket_type::rep};
        socket.bind(serveraddrBuffer.c_str());
        // Queue for messages
        std::queue<zmq::message_t> messages;

        std::thread processThread([this, &messages]() {
                while (true) {
                    if (messages.empty()) {

                        continue;
                    }
                    handleMessage(std::move(messages.front()));
                    messages.pop();
                }
            });

        nelems::GLPointCloud pointCloud;
        //print byte size of pointCloud
        std::cout << "Size of pointCloud: " << sizeof(pointCloud) << std::endl;

        // Receive the point cloud data from the client
        while (true) {

            // Receive the message from the client
            zmq::message_t message;
            auto res = socket.recv(message, zmq::recv_flags::none);
            if (!res.has_value()) {
                continue;
            }

            messages.push(std::move(message));

            // Send a confirmation reply back to the client
            socket.send(zmq::buffer("Received point"), zmq::send_flags::none);
        }

        // Close the socket (This part of code is not reachable because of the infinite loop)
        socket.close();
    }

  void SceneView::add_pcl(std::shared_ptr<nelems::GLPointCloud> pointCloud) { 
      load_sequence_flag = false;
      mMesh->add_pcl(pointCloud);
      load_sequence_flag = true; 
  }

  int SceneView::get_render_frames()
  {
      return mMesh->get_total_frames();
  }

}
