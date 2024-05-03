#pragma once

namespace nelems
{
  class VertexHolder
  {

  public:
    VertexHolder() : mPos(), mColor() {}
    
    // Initialize by string to be tokenized
    // TODO: Better handle outside of this class
    VertexHolder(const std::vector<std::string> tokens)
    {
    }

    //VertexHolder(const glm::vec3& pos, const glm::vec3& normal)
    VertexHolder(const glm::vec3& pos, const glm::vec3& color)
      : mPos(pos), mColor(color)
    {
    }

    ~VertexHolder() = default;


    glm::vec3 mPos;
    glm::vec3 mColor;
    //glm::vec3 mNormal;

    // TODO
    // glm::vec2 mUV;


  };
}

