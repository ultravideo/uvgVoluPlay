#pragma once

#include "pch.h"

#include "render_base.h"

namespace nrender
{
  class OpenGL_VertexIndexBuffer : public VertexIndexBuffer
  {
  private:
    size_t mFrameParse_count = 0;
    size_t mFrameRender_count = 0;

  public:
    OpenGL_VertexIndexBuffer() : VertexIndexBuffer()
    {}

    //void create_buffers(const std::vector<nelems::VertexHolder>& vertices, const std::vector<unsigned int>& indices) override;
    void create_buffers() override;

    void parse_buffers(std::shared_ptr<nelems::GLPointCloud>& pointCloud) override;

    void delete_buffers() override;

    void bind() override;

    void unbind() override;

    void draw(int index_count) override;

    void set_pointSize(float value) override;
  };

  class OpenGL_FrameBuffer : public FrameBuffer
  {
  public:

    void create_buffers(int32_t width, int32_t height) override;

    void delete_buffers() override;

    void bind() override;

    void unbind() override;
    
    uint32_t get_texture() override;
  };
}
