#pragma once

#ifndef PORTAL_HPP
#define PORTAL_HPP

#include <zmq.hpp>
#include <thread>  
#include <queue>
#include <vector>

#include "elems/pointcloud.h"
#include "elems/mesh.h"
#include "utils/log.hpp"
#include "utils/threadqueue.hpp"

#include <mutex>
#include <condition_variable>
#include "ply_import/miniply.h"

namespace Communication {
    struct zmqHandler {
        // Communication
        std::string color_address = "tcp://*:5555";             // Default address for color
        std::string position_address = "tcp://*:5556";          // Default address for position
        // std::string color_address = "tcp://10.21.25.231:5555";             // Default address for color
        // std::string position_address = "tcp://10.21.25.231:5556";          // Default address for position
        std::queue<zmq::message_t> colorMessages;               // Queue for color messages
        std::queue<zmq::message_t> positionMessages;            // Queue for position messages
        std::condition_variable receive_message_cv;             // Condition variable for receiving messages

        // Data Container    
        std::shared_ptr<std::queue<std::shared_ptr<nelems::GLPointCloud>>> pcl_queue;
        
        std::shared_ptr<nelems::Mesh> mMesh = nullptr;
    };

    struct sequenceHandler {
        std::string sequence_path = "";                         // Path to the sequence
        std::shared_ptr<std::vector<std::shared_ptr<nelems::GLPointCloud>>> pcl_vector;
        std::shared_ptr<nelems::Mesh> mMesh = nullptr;
        std::shared_ptr<bool> sequence_loaded = nullptr;
    };

    enum SourceMode
    {
        SOURCE_SEQUENCE = 0,
        SOURCE_ZMQ = 1,
    };

    class Portal { 
    private:
        /*** ZMQ ***/  
        std::shared_ptr<zmqHandler> zmq_handler = nullptr;       // ZMQ Handler
        /***** *****/

        /*** Sequence ***/
        std::shared_ptr<sequenceHandler> sequence_handler = nullptr; // Sequence Handler
        /****************/

        /*** Control & Data ***/ 
        bool stop_flag = false;
        /*** ************** ***/ 
    public:
        Portal();
        ~Portal();

        void set_position_socket(std::string address);
        void set_color_socket(std::string address);
        
        void stop_signal();

        // void set_data_stream(std::shared_ptr<std::queue<std::shared_ptr<nelems::GLPointCloud>>> &pcl_queue, std::shared_ptr<nelems::Mesh> &mesh, SourceMode mode);
        template<typename T>
        void set_data_stream(std::shared_ptr<T> &pcl_queue_from_scene_view, std::shared_ptr<nelems::Mesh> &mesh_from_scene_view, SourceMode mode) {
            switch (mode) {
            case SOURCE_ZMQ:
                zmq_handler = std::make_shared<zmqHandler>();
                // check type of T
                if constexpr (std::is_same_v<T, std::queue<std::shared_ptr<nelems::GLPointCloud>>>) {
                    zmq_handler->pcl_queue = pcl_queue_from_scene_view;
                } 
                else {
                    // Handle unsupported type
                    utilities::Logger::log(utilities::LogLevel::ERROR, "Portal", "Unsupported type");
                }

                zmq_handler->mMesh = mesh_from_scene_view;
                break;
            case SOURCE_SEQUENCE:
                sequence_handler = std::make_shared<sequenceHandler>();
                if constexpr (std::is_same_v<T, std::vector<std::shared_ptr<nelems::GLPointCloud>>>) {
                    sequence_handler->pcl_vector = pcl_queue_from_scene_view;
                } else {
                    // Handle unsupported type
                    utilities::Logger::log(utilities::LogLevel::ERROR, "Portal", "Unsupported type");
                }

                sequence_handler->mMesh = mesh_from_scene_view;
                break;
        }
    }

        void set_sequence_path(std::string folder_path);

        void zmq_run();

        void signal_stop_zmq();

        void sequence_run();

        void load_pointcloud(std::string path);

        void set_load_sequence(std::shared_ptr<bool> sequence_loaded);
  };
};

#endif // PORTAL_HPP