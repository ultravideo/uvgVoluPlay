#include "pch.h"
#include "portal.hpp"
#include <filesystem>

namespace Communication {
    Portal::Portal() {
    }

    Portal::~Portal() {
    }

    void Portal::set_position_socket(std::string address) {
        zmq_handler->position_address = address;
    }

    void Portal::set_color_socket(std::string address) {
        zmq_handler->color_address = address;
    }

    void Portal::stop_signal() {
        stop_flag = true;
        zmq_handler->receive_message_cv.notify_one();
    }

    void Portal::signal_stop_zmq() {
        zmq::context_t stopcontext{1};

        zmq::socket_t stopcolorSocket(stopcontext, ZMQ_PUSH);
        stopcolorSocket.connect("tcp://localhost:5555");

        zmq::socket_t stoppositionSocket(stopcontext, ZMQ_PUSH);
        stoppositionSocket.connect("tcp://localhost:5556");

        zmq::message_t stopMessage(5);
        memcpy(stopMessage.data(), "stop", 5);

        stopcolorSocket.send(stopMessage, zmq::send_flags::none);
        stoppositionSocket.send(stopMessage, zmq::send_flags::none);

        // Disconnect the sockets
        stopcolorSocket.disconnect("tcp://localhost:5555");
        stoppositionSocket.disconnect("tcp://localhost:5556");

        stopcolorSocket.close();
    }

    // void Portal::set_data_stream(   std::shared_ptr<std::queue<std::shared_ptr<nelems::GLPointCloud>>> &pcl_queue_from_scene_view,
    //                                 std::shared_ptr<nelems::Mesh> &mesh_from_scene_view,
    //                                 SourceMode mode
    //                                 ) {

    //     switch (mode) {
    //     case SOURCE_ZMQ:
    //         zmq_handler = std::make_shared<zmqHandler>();
    //         zmq_handler->mMesh = mesh_from_scene_view;
    //         zmq_handler->pcl_queue = pcl_queue_from_scene_view;
    //         break;
    //     case SOURCE_SEQUENCE:
    //         utilities::Logger::log(utilities::LogLevel::ERROR, "Portal", "setup stream Sequnce need to fix\n");
    //         break;
    //     }
    // }

    void Portal::set_sequence_path(std::string folder_path) {
        sequence_handler->sequence_path = folder_path;
    }

    void Portal::zmq_run() {
        stop_flag = false;
        zmq::context_t context{1};
        zmq::socket_t colorSocket(context, ZMQ_PULL);
        zmq::socket_t positionSocket(context, ZMQ_PULL);
        colorSocket.bind(zmq_handler->color_address);
        positionSocket.bind(zmq_handler->position_address);

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
                    zmq_handler->colorMessages.push(std::move(colorMessage));
                    zmq_handler->receive_message_cv.notify_one();
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
                    zmq_handler->positionMessages.push(std::move(positionMessage));
                    zmq_handler->receive_message_cv.notify_one();
                }
            }
        }); 

        std::unique_lock<std::mutex> lck(receive_message_mutex);
        
        while (true) {
            zmq_handler->receive_message_cv.wait(lck, [this] { return (!zmq_handler->colorMessages.empty() && !zmq_handler->positionMessages.empty()) || stop_flag; });

            if (stop_flag) {
                break;
            }
            // auto start = std::chrono::high_resolution_clock::now();
            std::shared_ptr<nelems::GLPointCloud> pointCloud = std::make_shared<nelems::GLPointCloud>();

            auto Pmessage = std::move(zmq_handler->positionMessages.front());
            auto Cmessage = std::move(zmq_handler->colorMessages.front());

            size_t numPoints = Pmessage.size() / (sizeof(glm::vec3));
            std::shared_ptr<std::vector<glm::vec3>> positions = std::make_shared<std::vector<glm::vec3>>(reinterpret_cast<const glm::vec3*>(Pmessage.data()), reinterpret_cast<const glm::vec3*>(Pmessage.data()) + numPoints);
            std::shared_ptr<std::vector<glm::vec3>> attributes = std::make_shared<std::vector<glm::vec3>>(reinterpret_cast<const glm::vec3*>(Cmessage.data()), reinterpret_cast<const glm::vec3*>(Cmessage.data()) + numPoints);

            // Add the received point to the point cloud
            pointCloud->parse(positions, attributes);
            zmq_handler->pcl_queue->push(pointCloud);   
            
            zmq_handler->positionMessages.pop();
            zmq_handler->colorMessages.pop();

            // auto end = std::chrono::high_resolution_clock::now();
            // std::chrono::duration<double> elapsed = end - start;
            // std::cout << "Time taken to create PCL: " << elapsed.count() << "s\n";
        }

        utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Stopping Components Threads\n");

        // Sleep for 5 seconds to allow the components to stop
        std::this_thread::sleep_for(std::chrono::seconds(5));
        signal_stop_zmq();

        colorProcessThread.join();
        positionProcessThread.join();

        utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Components Threads stopped\n");

        // colorSocket.unbind(color_address); 
        // positionSocket.unbind(position_address);

        colorSocket.close();
        positionSocket.close();
        context.close();

        utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Context closed\n");
    }

    void Portal::load_pointcloud(std::string filepath) {
        miniply::PLYReader reader(filepath.c_str());

        if (!reader.valid()) {
            throw std::runtime_error("miniply : Failed to open " + filepath);
        }

        bool vertexElementFound = false;
        while (reader.has_element()) {
            if (reader.element_is(miniply::kPLYVertexElement)) {
                vertexElementFound = true;
                break;  // Ensure that the current element is the vertex element for what follow
            }
            reader.next_element();
        }

        if (!vertexElementFound) {
            throw std::runtime_error("miniply : No vertex element (miniply::kPLYVertexElement) was found in this file : " + filepath);
        }

        if (!reader.load_element()) {
            throw std::runtime_error("miniply : Vertex element did not load correctly (file: " + filepath + ")");
        }
        std::array<uint32_t, 3> indicesPos{};  // Indices of position properties in the vertex line
        if (!reader.find_pos(indicesPos.data())) {
            throw std::runtime_error(
                "miniply : Position properties (x,y,z) were not located in the vertex element (file: " + filepath + ")");
        }
        std::array<uint32_t, 3> indicesCol{};  // Indices of color properties in the vertex line
        if (!reader.find_color(indicesCol.data())) {
            throw std::runtime_error("miniply : Color properties (r,g,b or red,green,blue) were not located in the vertex element (file: " +
                filepath + ")");
        }

        std::vector<std::array<float, 3>> geoVec;
        std::vector<std::array<uint8_t, 3>> attVec;

        const uint32_t vertexCount = reader.element()->count;
        if (vertexCount > 0)
        {
            geoVec.resize(vertexCount);
            attVec.resize(vertexCount);
        
            reader.extract_properties(indicesPos.data(), 3, miniply::PLYPropertyType::Float, geoVec.data());
            reader.extract_properties(indicesCol.data(), 3, miniply::PLYPropertyType::UChar, attVec.data());

            std::shared_ptr<nelems::GLPointCloud> pcl = std::make_shared<nelems::GLPointCloud>();

            for (size_t i = 0; i < vertexCount; ++i)
            {
                pcl->addPoint(geoVec[i][0], geoVec[i][1], geoVec[i][2], attVec[i][0], attVec[i][1], attVec[i][2]);
            }

            pcl->Finallized();
            sequence_handler->pcl_vector->push_back(pcl);
            utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Loaded  sequence_handler->pcl_vector size " + std::to_string(sequence_handler->pcl_vector->size()) + "\n");
        } 
    }

    void Portal::set_load_sequence(std::shared_ptr<bool> load_sequence) {
        sequence_handler->sequence_loaded = load_sequence;
    }

    void Portal::sequence_run() {

        if (!sequence_handler->pcl_vector->empty()) {
            utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Restart to run the sequence at path: " + sequence_handler->sequence_path + "\n");
            return;
        }
        *sequence_handler->sequence_loaded = false;

        // Go to the folder of the sequence and check if there are any files
        std::string folder_path = sequence_handler->sequence_path;
        std::vector<std::string> files;
        for (const auto & entry : std::filesystem::directory_iterator(folder_path)) {
            files.push_back(entry.path().string());
        }

        // Load the point clouds from the files
        for (const auto & file : files) {
            load_pointcloud(file);
        }

        *sequence_handler->sequence_loaded = true;

        utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Loaded " + std::to_string(files.size()) + " files\n");
    }

}; // namespace Communication
