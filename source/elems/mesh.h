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
#include "elems/vertex_holder.h"
#include "elems/element.h"

namespace nelems
{
  class Mesh : public Element
  {
    
  public:

    Mesh() = default;

    virtual ~Mesh();

    void load(const std::string& filepath);

    void add_vertex(const VertexHolder& vertex) { mVertices.push_back(vertex);  }

    void add_vertex_index(unsigned int vertex_idx) { mVertexIndices.push_back(vertex_idx); }

    std::vector<unsigned int> get_vertex_indices() { return mVertexIndices; }

    void update(nshaders::Shader* shader) override
    {
      // pbr color
      //shader->set_vec3(mColor, "albedo");

      //shader->set_f1(mRoughness, "roughness");
      //shader->set_f1(mMetallic, "metallic");
      //shader->set_f1(1.0f, "ao");

      if (!pcl_queue.empty()) 
      {       
          if (pcl_queue.size() == 2 )
          {
              pcl_queue.pop();
              parse_data();
              total_frames++;
          }
          else if (pcl_queue.size() > 2)
          {
              clear_queue();
              total_frames--;
          }  
      }
    }
    
    glm::vec3 mColor = { 1.0f, 0.0f, 0.0f };
    float mRoughness = 0.2f;
    float mMetallic = 0.1f;

    void init();

    void parse_data();

    void create_buffers();

    void delete_buffers();

    void render();

    void bind();

    void unbind();

    void clear_queue();

    void set_point_size(float value);

    void add_pcl(std::shared_ptr<nelems::GLPointCloud> pointCloud);

    int get_total_frames();

  private:
    
    // Buffers manager
    std::unique_ptr<nrender::VertexIndexBuffer> mRenderBufferMgr;
    
    // Vertices and indices
    std::vector<VertexHolder> mVertices;
    std::vector<unsigned int> mVertexIndices;
    std::queue<std::shared_ptr<nelems::GLPointCloud>> pcl_queue;

    std::mutex mtx;
    std::condition_variable cv;
    bool pcl_ready_flag = false;

    int total_frames = 0;
  };
}

