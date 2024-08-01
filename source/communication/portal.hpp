#pragma once

#ifndef PORTAL_HPP
#define PORTAL_HPP

#include <zmq.hpp>
#include <thread>  
#include <queue>

#include "elems/pointcloud.h"
#include "elems/mesh.h"
#include "utils/log.hpp"
#include "utils/threadqueue.hpp"

#include <mutex>
#include <condition_variable>

namespace Communication {
    class Portal { 
    private:
        zmq::context_t context{1};
        std::string color_address = "tcp://*:5555";
        std::string position_address = "tcp://*:5556";

        // Queue for color messages
        std::queue<zmq::message_t> colorMessages;

        // Queue for position messages
        std::queue<zmq::message_t> positionMessages;

        bool stop_flag = false;
        std::condition_variable receive_message_cv;
        std::shared_ptr<std::queue<std::shared_ptr<nelems::GLPointCloud>>> pcl_queue;
        std::shared_ptr<nelems::Mesh> mMesh = nullptr;

    public:
        Portal();
        ~Portal();

        void set_position_socket(std::string address);
        void set_color_socket(std::string address);
        
        void stop_signal();

        void get_point_cloud(std::shared_ptr<nelems::GLPointCloud> &pointCloud);

        void clear_front_point_cloud();

        void zmq_run();

        void set_data_stream(std::shared_ptr<std::queue<std::shared_ptr<nelems::GLPointCloud>>> &pcl_queue, std::shared_ptr<nelems::Mesh> &mesh);

  };
};

#endif // PORTAL_HPP