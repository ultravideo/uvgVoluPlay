#pragma once

#include "elems/pointcloud.h"

#include "window/window.h"

namespace nrender
{
  class VertexIndexBuffer
  {
  public:
    VertexIndexBuffer() : mVAO_parsing_id{ 0 }//, mIBO{ 0 }
    {
    }

    /*virtual void create_buffers(const std::vector<nelems::VertexHolder>& vertices, const std::vector<unsigned int>& indices) = 0;*/
    virtual void create_buffers() = 0;

    virtual void parse_buffers(std::shared_ptr<nelems::GLPointCloud>& pointCloud) = 0;

    virtual void delete_buffers() = 0;

    virtual void bind() = 0;

    virtual void unbind() = 0;

    virtual void draw(int index_count) = 0;

    virtual void set_pointSize(float value) = 0;

  protected:
    GLuint mVBO_positions_parse_id = 1;
    GLuint mVBO_attributes_parse_id = 2;

    GLuint mVBO_positions_render_id = 0;
    GLuint mVBO_attributes_render_id = 0;

    GLuint mVAO_parsing_id;
    GLuint mVAO_rendering_id = 1;
    //GLuint mIBO;
    float pointSize = 1.0f;
  };

  class FrameBuffer
  {
  public:
    FrameBuffer() : mFBO { 0 }, mDepthId {0}
    {}

    virtual void create_buffers(int32_t width, int32_t height) = 0;

    virtual void delete_buffers() = 0;

    virtual void bind() = 0;

    virtual void unbind() = 0;

    virtual uint32_t get_texture() = 0;

  protected:
    uint32_t mFBO = 0;
    uint32_t mTexId = 0;
    uint32_t mDepthId = 0;
    int32_t mWidth = 0;
    int32_t mHeight = 0;
  };

  class RenderContext
  {

  public:

    RenderContext() : mWindow(nullptr) {}

    virtual bool init(nwindow::IWindow* window)
    {
      mWindow = window;
      return true;
    }

    virtual void pre_render() = 0;

    virtual void post_render() = 0;

    virtual void end() = 0;

  protected:
    nwindow::IWindow* mWindow;
  };
}
