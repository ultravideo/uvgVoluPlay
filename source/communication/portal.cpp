#include "pch.h"
#include "portal.hpp"
#include <filesystem>

template <typename T>
class Vector3 : public std::array<T, 3> {
   public:
    Vector3() : std::array<T, 3>() {}
    Vector3(T x, T y, T z) : std::array<T, 3>({x, y, z}) {}
    Vector3(std::array<T, 3>& arr) : std::array<T, 3>(arr) {}
    Vector3(std::array<T, 3>&& arr) : std::array<T, 3>(std::move(arr)) {} 
    Vector3(const std::array<T, 3>& arr) {
        std::copy(arr.begin(), arr.end(), this->begin());
    }   
    
    template <typename U>
    Vector3<T> operator+(const Vector3<U>& other) const {
        return {(*this)[0] + other[0], (*this)[1] + other[1], (*this)[2] + other[2]};
    }

    template <typename U>
    Vector3<T> operator-(const Vector3<U>& other) const {
        return {(*this)[0] - other[0], (*this)[1] - other[1], (*this)[2] - other[2]};
    }

    Vector3<double> operator-(const Vector3<double>& other) const {
        return {(*this)[0] - other[0], (*this)[1] - other[1], (*this)[2] - other[2]};
    }

    Vector3<T> operator-() const { return {-(*this)[0], -(*this)[1], -(*this)[2]}; }

    template <typename U>
    Vector3<T>& operator+=(const Vector3<U>& other) {
        (*this)[0] += other[0];
        (*this)[1] += other[1];
        (*this)[2] += other[2];
        return *this;
    }

    template <typename U>
    Vector3<T>& operator/=(const U& val) {
        (*this)[0] /= val;
        (*this)[1] /= val;
        (*this)[2] /= val;
        return *this;
    }
};

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
        zmq_handler->receive_message_cv.notify_all();
    }

    void Portal::set_sequence_path(std::string folder_path) {
        sequence_handler->sequence_paths.push_back(folder_path);
    }

    void Portal::zmq_run() {
        stop_flag = false;
        zmq::context_t context{1};
        zmq::socket_t colorSocket(context, ZMQ_PULL);
        zmq::socket_t positionSocket(context, ZMQ_PULL);
        colorSocket.bind(zmq_handler->color_address);
        positionSocket.bind(zmq_handler->position_address);

        // colorSocket.set(zmq::sockopt::rcvtimeo, 6000);
        // positionSocket.set(zmq::sockopt::rcvtimeo, 6000);

        std::mutex receive_message_mutex;
        int counter = 0;

        utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Context created\n");

        //2 thread for receiving color and position messages
        std::thread colorProcessThread([this, &colorSocket, &counter]() {
            utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Color thread started\n");
            while (!stop_flag) {
                // Receive the color message from the client
                zmq::message_t colorMessage;
                auto colorRes = colorSocket.recv(colorMessage, zmq::recv_flags::none);

                if (colorRes && colorRes.has_value()) { 
                    zmq_handler->colorMessages.push(std::move(colorMessage));
                    zmq_handler->receive_message_cv.notify_one();
                    counter++;
                }
                else if (!colorRes && counter != 0) {
                    utilities::Logger::log(utilities::LogLevel::WARNING, "Portal", "Timeout: No message received.\n");                    
                    stop_signal();
                }

                //if the message is "DISCONNECT", stop the loop
                if (colorMessage.to_string() == "DISCONNECT") {
                    utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Color socker is on hold\n");
                    break;
                }

            }
            utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Color thread stopped\n");
        });

        std::thread positionProcessThread([this, &positionSocket, counter]() {
            utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Position thread started\n");
            
             while (!stop_flag) {
                // Receive the position message from the client
                zmq::message_t positionMessage;
                auto positionRes = positionSocket.recv(positionMessage, zmq::recv_flags::none);

                if (positionRes && positionRes.has_value()) {   
                    zmq_handler->positionMessages.push(std::move(positionMessage));
                    zmq_handler->receive_message_cv.notify_one();
                }
                else if (!positionRes && counter != 0) {
                    utilities::Logger::log(utilities::LogLevel::WARNING, "Portal", "Timeout: No message received.\n");
                    stop_signal();
                }
            }
            utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Position thread stopped\n");
        }); 

        std::unique_lock<std::mutex> lck(receive_message_mutex);
        
        int frame_num = 0;
        while (true) {
            zmq_handler->receive_message_cv.wait(lck, [this] { return (!zmq_handler->colorMessages.empty() && !zmq_handler->positionMessages.empty()) || stop_flag; });

            if (stop_flag) {
                break;
            }
            // auto start = std::chrono::high_resolution_clock::now();
            std::shared_ptr<nelems::GLPointCloud> pointCloud = std::make_shared<nelems::GLPointCloud>();

            auto Pmessage = std::move(zmq_handler->positionMessages.front());
            auto Cmessage = std::move(zmq_handler->colorMessages.front());

            size_t numPoints = Pmessage.size() / (sizeof(Vector3<uint16_t>));
            // std::shared_ptr<std::vector<glm::vec3>> positions = std::make_shared<std::vector<glm::vec3>>(reinterpret_cast<const glm::vec3*>(Pmessage.data()), reinterpret_cast<const glm::vec3*>(Pmessage.data()) + numPoints);
            // std::shared_ptr<std::vector<glm::vec3>> attributes = std::make_shared<std::vector<glm::vec3>>(reinterpret_cast<const glm::vec3*>(Cmessage.data()), reinterpret_cast<const glm::vec3*>(Cmessage.data()) + numPoints);


            const auto* attributePointer = reinterpret_cast<const Vector3<uint8_t>*>(Cmessage.data());
            std::vector<Vector3<uint8_t>> rawAttrTmp(attributePointer, attributePointer + numPoints);

            const auto* geometryPointer = reinterpret_cast<const Vector3<uint16_t>*>(Pmessage.data());
            std::vector<Vector3<uint16_t>> rawGeoTmp(geometryPointer, geometryPointer + numPoints);

            std::shared_ptr<std::vector<glm::vec3>> positions = std::make_shared<std::vector<glm::vec3>>(numPoints);
            std::shared_ptr<std::vector<glm::vec3>> attributes = std::make_shared<std::vector<glm::vec3>>(numPoints);

            for (size_t i = 0; i < numPoints; ++i) {
                (*positions)[i] = glm::vec3(
                    static_cast<float>(rawGeoTmp[i][0]),
                    static_cast<float>(rawGeoTmp[i][1]),
                    static_cast<float>(rawGeoTmp[i][2])
                );

                // (*attributes)[i][0] = static_cast<float>(rawAttrTmp[i][0]) / 255.0f;
                // (*attributes)[i][1] = static_cast<float>(rawAttrTmp[i][1]) / 255.0f;
                // (*attributes)[i][2] = static_cast<float>(rawAttrTmp[i][2]) / 255.0f;

                (*attributes)[i] = glm::vec3(
                    static_cast<float>(rawAttrTmp[i][0])/ 255.0f,
                    static_cast<float>(rawAttrTmp[i][1])/ 255.0f,
                    static_cast<float>(rawAttrTmp[i][2])/ 255.0f
                );
            }

            // printf("Frame count: %d\n", (int)frame_num++);

            // Add the received point to the point cloud
            pointCloud->parse(positions, attributes);
            zmq_handler->pcl_queue->push(pointCloud);   
            
            zmq_handler->positionMessages.pop();
            zmq_handler->colorMessages.pop();
        }

        utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Stopping Components Threads\n");

        colorProcessThread.join();
        positionProcessThread.join();

        utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Components Threads stopped\n");

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
            // utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Loaded  sequence_handler->pcl_vector size " + std::to_string(sequence_handler->pcl_vector->size()) + "\n");
        } 
    }

    void Portal::set_load_sequence(std::shared_ptr<bool> load_sequence) {
        sequence_handler->sequence_loaded = load_sequence;
    }

    void Portal::sequence_run() {

        if (!sequence_handler->pcl_vector->empty()) {
            utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Restart to run other sequence\n");
            return;
        }
        *sequence_handler->sequence_loaded = false;

        for (const auto folder_path : sequence_handler->sequence_paths) {
            if (!std::filesystem::exists(folder_path)) {
                utilities::Logger::log(utilities::LogLevel::ERROR, "Portal", "Invalid folder path\n");
                return;
            }

            std::vector<std::string> files;
            for (const auto & entry : std::filesystem::directory_iterator(folder_path)) {
                files.push_back(entry.path().string());
            }

            // Load the point clouds from the files
            for (const auto & file : files) {
                load_pointcloud(file);
            }
            std::cout << "Loaded " << sequence_handler->pcl_vector->size() << " files\n";
        }

        // std::string folder_path = sequence_handler->sequence_paths;
        
        // for (const auto & entry : std::filesystem::directory_iterator(folder_path)) {
        //     files.push_back(entry.path().string());
        // }

        // // Load the point clouds from the files
        // for (const auto & file : files) {
        //     load_pointcloud(file);
        // }

        *sequence_handler->sequence_loaded = true;

        utilities::Logger::log(utilities::LogLevel::INFO, "Portal", "Loaded " + std::to_string(sequence_handler->pcl_vector->size()) + " files\n");
    }

}; // namespace Communication
