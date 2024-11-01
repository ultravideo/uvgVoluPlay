#include "pch.h"
#include "opengl_buffer_manager.h"
#include <thread>
#include <chrono>

namespace nrender
{
  // Currently, we only render the pointcloud so we dont need indices vector.
  //void OpenGL_VertexIndexBuffer::create_buffers(const std::vector<nelems::VertexHolder>& vertices, const std::vector<unsigned int>& indices)
  void OpenGL_VertexIndexBuffer::create_buffers()
  {
    glGenVertexArrays(1, &mVAO_parsing_id);
    //glGenBuffers(1, &mIBO);
    glGenBuffers(1, &mVBO_positions_parse_id);
    glGenBuffers(1, &mVBO_attributes_parse_id);
  }

  void OpenGL_VertexIndexBuffer::parse_buffers(std::shared_ptr<nelems::GLPointCloud>& pointCloud)
  {
    if (pointCloud->getPositionsVec().empty())
    {
      return;
    }


    // Calculate the total size needed for both position and attribute data
    size_t positionSize = (pointCloud->getPositionsVec().size()) * sizeof(glm::vec3);
    size_t attributeSize = (pointCloud->getAttributesVec().size()) * sizeof(glm::vec3);

    // Bind the VAO
    glBindVertexArray(mVAO_parsing_id);

    // Bind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, mVBO_positions_parse_id);
    // Allocate memory for both position and attribute data
    glBufferData(GL_ARRAY_BUFFER, positionSize, pointCloud->getPositionsVec().data(), GL_STREAM_DRAW);
    // Specify the layout of the position data
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

   // Bind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, mVBO_attributes_parse_id);
    // Allocate memory for both position and attribute data
    glBufferData(GL_ARRAY_BUFFER, attributeSize, pointCloud->getAttributesVec().data(), GL_STREAM_DRAW);
    // Specify the layout of the attribute data
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)(0));

    glBindVertexArray(0);

  }

  void OpenGL_VertexIndexBuffer::draw(int index_count)
  {
    bind();

    glPointSize(pointSize);
    glDrawArrays(GL_POINTS, 0, index_count);
    
    unbind();
  }

  void OpenGL_VertexIndexBuffer::delete_buffers()
  {
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &mVBO_positions_parse_id);
    glDeleteBuffers(1, &mVBO_attributes_parse_id);
    glDeleteVertexArrays(1, &mVAO_parsing_id);
  }

  void OpenGL_VertexIndexBuffer::bind()
  {
    glBindVertexArray(mVAO_parsing_id);
  }

  void OpenGL_VertexIndexBuffer::unbind()
  {
     glBindVertexArray(0);
  }

  void OpenGL_VertexIndexBuffer::set_pointSize(float value)
  {
      pointSize = value;
  }

  void OpenGL_FrameBuffer::create_buffers(int32_t width, int32_t height)
  {
    mWidth = width;
    mHeight = height;

    if (mFBO)
    { 
      delete_buffers();
    }

    glGenFramebuffers(1, &mFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

    // Platform-specific texture creation
    #ifdef __APPLE__
      // Use glGenTextures and glBindTexture on macOS
      glGenTextures(1, &mTexId);
      glBindTexture(GL_TEXTURE_2D, mTexId);
    #else
      // Use glCreateTextures on other platforms (Windows, Linux)
      glCreateTextures(GL_TEXTURE_2D, 1, &mTexId);
      glBindTexture(GL_TEXTURE_2D, mTexId);
    #endif

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexId, 0);

    // Platform-specific depth buffer creation
    #ifdef __APPLE__
      // Use glGenTextures and glBindTexture on macOS
      glGenTextures(1, &mDepthId);
      glBindTexture(GL_TEXTURE_2D, mDepthId);
    #else
      // Use glCreateTextures on other platforms (Windows, Linux)
      glCreateTextures(GL_TEXTURE_2D, 1, &mDepthId);
      glBindTexture(GL_TEXTURE_2D, mDepthId);
    #endif

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, mWidth, mHeight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mDepthId, 0);

    // Specify that we're drawing to the color attachment
    GLenum buffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, buffers);  // Corrected call

    unbind();
  }

  void OpenGL_FrameBuffer::delete_buffers()
  {
    if (mFBO)
    {
      glDeleteFramebuffers(1, &mFBO);
      glDeleteTextures(1, &mTexId);
      glDeleteTextures(1, &mDepthId);
      mTexId = 0;
      mDepthId = 0;
      mFBO = 0;
    }

  }

  void OpenGL_FrameBuffer::bind()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glViewport(0, 0, mWidth, mHeight);

    glClearColor(background_color[0], background_color[1], background_color[2], background_color[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void OpenGL_FrameBuffer::unbind()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  uint32_t OpenGL_FrameBuffer::get_texture()
  {
    return mTexId;
  }

  void OpenGL_FrameBuffer::set_background_color(float r, float g, float b)
  {
    background_color[0] = r;
    background_color[1] = g;
    background_color[2] = b;
  }
}