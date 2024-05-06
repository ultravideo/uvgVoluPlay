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

    void SceneView::handleMessage(zmq::message_t Pmessage, zmq::message_t Cmessage) {
            std::shared_ptr<nelems::GLPointCloud> pointCloud = std::make_shared<nelems::GLPointCloud>();

            // size_t numPoints = Pmessage.size() / (sizeof(glm::vec3)*2);
            // std::shared_ptr<std::vector<glm::vec3>> data = std::make_shared<std::vector<glm::vec3>>(reinterpret_cast<const glm::vec3*>(message.data()), reinterpret_cast<const glm::vec3*>(message.data()) + 2* numPoints);

            // // Form the received data into vectors
            // std::shared_ptr<std::vector<glm::vec3>> positions;
            // std::shared_ptr<std::vector<glm::vec3>> attributes;
            // #pragma omp parallel sections
            // {
            //     #pragma omp section
            //     {
            //     positions = std::make_shared<std::vector<glm::vec3>>(data->begin(), data->begin() + numPoints);
            //     }
            //     #pragma omp section
            //     {
            //     attributes = std::make_shared<std::vector<glm::vec3>>(data->begin() + numPoints, data->end());
            //     }
            // }

            size_t numPoints = Pmessage.size() / (sizeof(glm::vec3));
            std::shared_ptr<std::vector<glm::vec3>> Pdata = std::make_shared<std::vector<glm::vec3>>(reinterpret_cast<const glm::vec3*>(Pmessage.data()), reinterpret_cast<const glm::vec3*>(Pmessage.data()) + numPoints);
            std::shared_ptr<std::vector<glm::vec3>> Cdata = std::make_shared<std::vector<glm::vec3>>(reinterpret_cast<const glm::vec3*>(Cmessage.data()), reinterpret_cast<const glm::vec3*>(Cmessage.data()) + numPoints);

            // Form the received data into vectors
            std::shared_ptr<std::vector<glm::vec3>> positions = std::make_shared<std::vector<glm::vec3>>(Pdata->begin(), Pdata->end());
            std::shared_ptr<std::vector<glm::vec3>> attributes = std::make_shared<std::vector<glm::vec3>>(Cdata->begin(), Cdata->end());

            // Add the received point to the point cloud
            pointCloud->parse(positions, attributes);
            add_pcl(pointCloud); 
        }

        void SceneView::receivePointCloud(std::string serveraddrBuffer)
        {
            
            // // initialize the zmq context with a single IO thread
            zmq::context_t context{1};

            zmq::socket_t colorSocket(context, ZMQ_PULL);
            zmq::socket_t positionSocket(context, ZMQ_PULL);
            colorSocket.bind("tcp://*:5555");
            positionSocket.bind("tcp://*:5556");


            // Queue for color messages
            std::queue<zmq::message_t> colorMessages;

            // Queue for position messages
            std::queue<zmq::message_t> positionMessages;

            std::thread PclProcessThread([this, &positionMessages, &colorMessages]() {
                while (true) {
                    if (positionMessages.empty() || colorMessages.empty()) {
                        continue;
                    }
                    handleMessage(std::move(positionMessages.front()), std::move(colorMessages.front()));
                    positionMessages.pop();
                    colorMessages.pop();
                }
            });

            nelems::GLPointCloud pointCloud;

            //2 thread for receiving color and position messages
            std::thread colorProcessThread([this, &colorMessages, &colorSocket]() {
                while (true) {
                    // Receive the color message from the client
                    zmq::message_t colorMessage;
                    auto colorRes = colorSocket.recv(colorMessage, zmq::recv_flags::none);
                    if (colorRes.has_value()) {
                        colorMessages.push(std::move(colorMessage));
                    }
                }
            });

            std::thread positionProcessThread([this, &positionMessages, &positionSocket]() {
                while (true) {
                    // Receive the position message from the client
                    zmq::message_t positionMessage;
                    auto positionRes = positionSocket.recv(positionMessage, zmq::recv_flags::none);
                    if (positionRes.has_value()) {
                        positionMessages.push(std::move(positionMessage));
                    }
                }
            });

            // Receive the point cloud data from the client
            while (true) {
                std::unique_lock<std::mutex> lck(mtx);
                cv.wait(lck, [this] { return stoprender_flag; });
            }

            // Close the socket (This part of code is not reachable because of the infinite loop) Modify later
            // socket.close();
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

    void SceneView::stop()
    {
        stoprender_flag = true;
        cv.notify_all();
    }

}
