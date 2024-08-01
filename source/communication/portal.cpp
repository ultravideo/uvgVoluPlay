#include "pch.h"
#include "portal.hpp"

namespace Communication {
    Portal::Portal() {
    }

    Portal::~Portal() {
    }

    void Portal::set_position_socket(std::string address) {
        position_address = address;
    }

    void Portal::set_color_socket(std::string address) {
        color_address = address;
    }

    void Portal::stop_signal() {
        stop_flag = true;
        receive_message_cv.notify_one();
    }

    void Portal::get_point_cloud(std::shared_ptr<nelems::GLPointCloud> &pointCloud) {
        // if (pointClouds.empty()) {
        //     pointCloud = nullptr;
        //     return;
        // }
        
        // pointCloud = pointClouds.front();
    }

    void Portal::clear_front_point_cloud() {
        // if (pointClouds.size() < 2) {
        //     return;
        // }

        // pointClouds.pop();
    }

    void Portal::set_data_stream(std::shared_ptr<std::queue<std::shared_ptr<nelems::GLPointCloud>>> &pcl_queue_from_scene_view,
        std::shared_ptr<nelems::Mesh> &mesh_from_scene_view) {
        this->pcl_queue = pcl_queue_from_scene_view;
        this->mMesh = mesh_from_scene_view;
    }

    void Portal::zmq_run() {
        zmq::socket_t colorSocket(context, ZMQ_PULL);
        zmq::socket_t positionSocket(context, ZMQ_PULL);
        colorSocket.bind(color_address);
        positionSocket.bind(position_address);

        std::mutex receive_message_mutex;

        utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Context created\n");

        //2 thread for receiving color and position messages
        std::thread colorProcessThread([this, &colorSocket]() {
            utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Color thread started\n");
            while (!stop_flag) {
                // Receive the color message from the client
                zmq::message_t colorMessage;
                auto colorRes = colorSocket.recv(colorMessage, zmq::recv_flags::none);
                if (colorRes.has_value()) {
                    colorMessages.push(std::move(colorMessage));
                    receive_message_cv.notify_one();
                }
            }
        });

        std::thread positionProcessThread([this, &positionSocket]() {
            utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Position thread started\n");
            
            while (!stop_flag) {
                // Receive the position message from the client
                zmq::message_t positionMessage;
                auto positionRes = positionSocket.recv(positionMessage, zmq::recv_flags::none);
                if (positionRes.has_value()) {
                    positionMessages.push(std::move(positionMessage));
                    receive_message_cv.notify_one();
                }
            }
        });

        std::mutex mesh_mutex;
        std::condition_variable mesh_cv;
        std::thread meshLoaderThread([this, &mesh_mutex, &mesh_cv]() { 
          std::unique_lock<std::mutex> lck(mesh_mutex);
          while (!stop_flag)
          {
            mesh_cv.wait(lck, [this] { return !pcl_queue->empty(); });
            // Make sure that we always have 1 pcl left to visualize, otherwise pcl will be deleted 
            // due to differnet in speed of rendering and receiving
            // if (pcl_queue->size() >= 2) 
            // {
            //     pcl_queue->pop();
                // std::shared_ptr<nelems::Mesh> mMesh = std::make_shared<nelems::Mesh>();
                // mMesh->init();
                // mMesh->parse_data(pcl_queue->front());
            // }
          }
          
       }); 

        std::unique_lock<std::mutex> lck(receive_message_mutex);
        
        while (true) {
            receive_message_cv.wait(lck, [this] { return (!colorMessages.empty() && !positionMessages.empty()) || stop_flag; });

            if (stop_flag) {
                break;
            }
            // auto start = std::chrono::high_resolution_clock::now();
            std::shared_ptr<nelems::GLPointCloud> pointCloud = std::make_shared<nelems::GLPointCloud>();

            auto Pmessage = std::move(positionMessages.front());
            auto Cmessage = std::move(colorMessages.front());

            size_t numPoints = Pmessage.size() / (sizeof(glm::vec3));
            std::shared_ptr<std::vector<glm::vec3>> positions = std::make_shared<std::vector<glm::vec3>>(reinterpret_cast<const glm::vec3*>(Pmessage.data()), reinterpret_cast<const glm::vec3*>(Pmessage.data()) + numPoints);
            std::shared_ptr<std::vector<glm::vec3>> attributes = std::make_shared<std::vector<glm::vec3>>(reinterpret_cast<const glm::vec3*>(Cmessage.data()), reinterpret_cast<const glm::vec3*>(Cmessage.data()) + numPoints);

            // Add the received point to the point cloud
            pointCloud->parse(positions, attributes);
            pcl_queue->push(pointCloud);
            mesh_cv.notify_one();

            // mMesh->parse_data(pcl_queue->back());   
            
            positionMessages.pop();
            colorMessages.pop();

            // auto end = std::chrono::high_resolution_clock::now();
            // std::chrono::duration<double> elapsed = end - start;
            // std::cout << "Time taken to create PCL: " << elapsed.count() << "s\n";
        }

        utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Stopping Components Threads\n");

        colorProcessThread.join();
        positionProcessThread.join();

        utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Components Threads stopped\n");

        colorSocket.unbind(color_address);
        positionSocket.unbind(position_address);

        colorSocket.close();
        positionSocket.close();
        context.close();

        utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Context closed\n");
    }
}; // namespace Communication
