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
    glGenVertexArrays(1, &mVAO);

    //glGenBuffers(1, &mIBO);
    glGenBuffers(1, &mVBO);
    glGenBuffers(1, &mVCO);

    // glGenBuffers(1, &mVBO_odd);
    // glGenBuffers(1, &mVCO_odd);

    glBindVertexArray(mVAO);
  }

  void OpenGL_VertexIndexBuffer::parse_buffers(std::shared_ptr<nelems::GLPointCloud>& pointCloud)
  {
      // glBindBuffer(GL_ARRAY_BUFFER, mVBO);
      // glBufferData(GL_ARRAY_BUFFER, std::min(vertices->size(), (size_t)1500000) * sizeof((*vertices)[0]), vertices->data(), GL_DYNAMIC_DRAW);

      // // Specify the layout of the position data
      // glEnableVertexAttribArray(0);
      // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);

      // // Specify the layout of the color data
      // glEnableVertexAttribArray(1);
      // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(3 * sizeof(float)));

      // // Unbind the vertex array object to prevent accidental modifications
      // glBindVertexArray(0);

    //start timer
    auto start = std::chrono::high_resolution_clock::now();

    // Calculate the total size needed for both position and attribute data
    size_t positionSize = (pointCloud->getPositionsVec().size()) * sizeof(glm::vec3);
    size_t attributeSize = (pointCloud->getAttributesVec().size()) * sizeof(glm::vec3);

    // Bind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    // Allocate memory for both position and attribute data
    glBufferData(GL_ARRAY_BUFFER, positionSize, pointCloud->getPositionsVec().data(), GL_DYNAMIC_DRAW);
    // Specify the layout of the position data
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    // Unbind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);

   // Bind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, mVCO);
    // Allocate memory for both position and attribute data
    glBufferData(GL_ARRAY_BUFFER, attributeSize, pointCloud->getAttributesVec().data(), GL_DYNAMIC_DRAW);
    // Specify the layout of the attribute data
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)(0));
    // Unbind the VCO
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Time taken to parse buffers: " << elapsed.count() << "s\n";

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
    //glDeleteBuffers(1, &mIBO);
    glDeleteBuffers(1, &mVBO);
    glDeleteBuffers(1, &mVCO);
    glDeleteVertexArrays(1, &mVAO);
  }

  void OpenGL_VertexIndexBuffer::bind()
  {
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mVCO);
    glBindVertexArray(mVAO);
    glEnableClientState(GL_VERTEX_ARRAY);
  }

  void OpenGL_VertexIndexBuffer::unbind()
  {
     glDisableClientState(GL_VERTEX_ARRAY);
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
    glCreateTextures(GL_TEXTURE_2D, 1, &mTexId);
    glBindTexture(GL_TEXTURE_2D, mTexId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexId, 0);

    glCreateTextures(GL_TEXTURE_2D, 1, &mDepthId);
    glBindTexture(GL_TEXTURE_2D, mDepthId);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, mWidth, mHeight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mDepthId, 0);

    GLenum buffers[4] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(mTexId, buffers);

    unbind();
  }

  void OpenGL_FrameBuffer::delete_buffers()
  {
    if (mFBO)
    {
      glDeleteFramebuffers(GL_FRAMEBUFFER, &mFBO);
      glDeleteTextures(1, &mTexId);
      glDeleteTextures(1, &mDepthId);
      mTexId = 0;
      mDepthId = 0;
    }

  }

  void OpenGL_FrameBuffer::bind()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glViewport(0, 0, mWidth, mHeight);
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


}