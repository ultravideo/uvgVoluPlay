#pragma once
#include <cstdint>
#include <array>
#include <stdexcept>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "pch.h"

#include "render/render_base.h"
#include "elems/pointcloud.h"
// #include "elems/vertex_holder.h"
#include "elems/element.h"

namespace nelems
{
  class Mesh : public Element
  {
    
  public:

    Mesh() = default;

    virtual ~Mesh();

    void load(const std::string& filepath);

    void update(nshaders::Shader* shader) override
    {
      // pbr color
      // shader->set_vec3(mColor, "albedo");

      // shader->set_f1(mRoughness, "roughness");
      // shader->set_f1(mMetallic, "metallic");
      // shader->set_f1(1.0f, "ao");
    }
    
    // glm::vec3 mColor = { 1.0f, 0.0f, 0.0f };
    // float mRoughness = 0.2f;
    // float mMetallic = 0.1f;

    void init();

    void parse_data(std::shared_ptr<nelems::GLPointCloud> &pointCloud);

    void create_buffers();

    void delete_buffers();

    void render();

    void bind();

    void unbind();

    void set_pointSize(float pointSize);

  private:  
    // Buffers manager
    std::unique_ptr<nrender::VertexIndexBuffer> mRenderBufferMgr;
    int currnet_points = 0;

    std::shared_ptr<nelems::GLPointCloud> pcl = nullptr;
  };
}

