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
        if (currnet_points == 0) {
            return;
        }
        
        bind();
        mRenderBufferMgr->draw(currnet_points);
        unbind();
    }

    void Mesh::set_pointSize(float pointSize)
    {
        mRenderBufferMgr->set_pointSize(pointSize);
    }

}