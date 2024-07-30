#include "pch.h"
#include "mesh.h"
#include "ply_import/miniply.h"

#include "render/opengl_buffer_manager.h"

namespace nelems
{
    void Mesh::init()
    {
        mRenderBufferMgr = std::make_unique<nrender::OpenGL_VertexIndexBuffer>();
        create_buffers();
    }

    void Mesh::parse_data(std::shared_ptr<nelems::GLPointCloud> &pointCloud)
    {
        mRenderBufferMgr->parse_buffers(pointCloud);
        currnet_points = pointCloud->max_size();
    }

    Mesh::~Mesh()
    {
        delete_buffers();
    }

    void Mesh::load(const std::string& filepath)
    {
        // miniply::PLYReader reader(filepath.c_str());

        // if (!reader.valid()) {
        //     throw std::runtime_error("miniply : Failed to open " + filepath);
        // }

        // bool vertexElementFound = false;
        // while (reader.has_element()) {
        //     if (reader.element_is(miniply::kPLYVertexElement)) {
        //         vertexElementFound = true;
        //         break;  // Ensure that the current element is the vertex element for what follow
        //     }
        //     reader.next_element();
        // }

        // if (!vertexElementFound) {
        //     throw std::runtime_error("miniply : No vertex element (miniply::kPLYVertexElement) was found in this file : " + filepath);
        // }

        // if (!reader.load_element()) {
        //     throw std::runtime_error("miniply : Vertex element did not load correctly (file: " + filepath + ")");
        // }
        // std::array<uint32_t, 3> indicesPos{};  // Indices of position properties in the vertex line
        // if (!reader.find_pos(indicesPos.data())) {
        //     throw std::runtime_error(
        //         "miniply : Position properties (x,y,z) were not located in the vertex element (file: " + filepath + ")");
        // }
        // std::array<uint32_t, 3> indicesCol{};  // Indices of color properties in the vertex line
        // if (!reader.find_color(indicesCol.data())) {
        //     throw std::runtime_error("miniply : Color properties (r,g,b or red,green,blue) were not located in the vertex element (file: " +
        //         filepath + ")");
        // }

        // std::vector<std::array<float, 3>> geoVec;
        // std::vector<std::array<uint8_t, 3>> attVec;

        // const uint32_t vertexCount = reader.element()->count;
        // if (vertexCount > 0)
        // {
        //     geoVec.resize(vertexCount);
        //     attVec.resize(vertexCount);
            
        //     reader.extract_properties(indicesPos.data(), 3, miniply::PLYPropertyType::Float, geoVec.data());
        //     reader.extract_properties(indicesCol.data(), 3, miniply::PLYPropertyType::UChar, attVec.data());

        //     //mVertexIndices.clear();
        //     mVertices.clear();

        //     for (size_t i = 0; i < vertexCount; ++i)
        //     {
               
        //         VertexHolder vh;
   
        //         vh.mPos = { geoVec[i][0], geoVec[i][1] , geoVec[i][2] };

        //         vh.mColor = { (attVec[i][0])/255.0f, (attVec[i][1]) / 255.0f , (attVec[i][2]) / 255.0f };
        //     }

        //     // Push the vertices into the pcl_queue
        //     pcl_queue.push(std::make_shared<std::vector<nelems::VertexHolder>>(mVertices));
        // }
    }

    void Mesh::create_buffers()
    {
         mRenderBufferMgr->create_buffers();
    }

    void Mesh::delete_buffers()
    {
        mRenderBufferMgr->delete_buffers();
    }

    void Mesh::bind()
    {
        mRenderBufferMgr->bind();
    }

    void Mesh::unbind()
    {
        mRenderBufferMgr->unbind();
    }

    void Mesh::render()
    { 
        if (currnet_points == 0)
            return;
        bind();
        mRenderBufferMgr->draw(currnet_points);
        unbind();
    }

    void Mesh::set_data(std::shared_ptr<nelems::GLPointCloud> &pointCloud)
    {
        mRenderBufferMgr->parse_buffers(pointCloud);
    }

    void Mesh::set_pointSize(float pointSize)
    {
        mRenderBufferMgr->set_pointSize(pointSize);
    }

}