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
        if (pointClouds.empty()) {
            pointCloud = nullptr;
            return;
        }
        
        pointCloud = pointClouds.front();
    }

    void Portal::clear_front_point_cloud() {
        if (pointClouds.size() < 2) {
            return;
        }

        pointClouds.pop();
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
        
        while (true) {
            std::unique_lock<std::mutex> lck(receive_message_mutex);
            receive_message_cv.wait(lck, [this] { return (!colorMessages.empty() && !positionMessages.empty()) || stop_flag; });

            if (stop_flag) {
                break;
            }

            std::shared_ptr<nelems::GLPointCloud> pointCloud = std::make_shared<nelems::GLPointCloud>();

            auto Pmessage = std::move(positionMessages.front());
            auto Cmessage = std::move(colorMessages.front());

            size_t numPoints = Pmessage.size() / (sizeof(glm::vec3));
            std::shared_ptr<std::vector<glm::vec3>> positions = std::make_shared<std::vector<glm::vec3>>(reinterpret_cast<const glm::vec3*>(Pmessage.data()), reinterpret_cast<const glm::vec3*>(Pmessage.data()) + numPoints);
            std::shared_ptr<std::vector<glm::vec3>> attributes = std::make_shared<std::vector<glm::vec3>>(reinterpret_cast<const glm::vec3*>(Cmessage.data()), reinterpret_cast<const glm::vec3*>(Cmessage.data()) + numPoints);

            // Add the received point to the point cloud
            pointCloud->parse(positions, attributes);
            pointClouds.push(pointCloud);

            positionMessages.pop();
            colorMessages.pop();
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
