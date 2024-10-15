
/**
 * @file uvgCPCTypedef.hpp
 * @brief This file contains the typedefs and structures used in the uvgCPC Geometry module.
 */

#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <unordered_map>
#include <queue>
#include <memory>
#include <cstdlib>
#include <string>
#include <glm/glm.hpp>

#ifndef UVG_CPC_TYPEDEFS_H
#define UVG_CPC_TYPEDEFS_H

namespace nelems
{
	/**
	 * @brief Class representing a OpenGL point cloud.
	 */
	class GLPointCloud
	{
	private:
		bool HasPoints() const;
		bool IsEmpty() const;

		size_t mSize = 0;
		std::shared_ptr<std::vector<glm::vec3>> positions = std::make_shared<std::vector<glm::vec3>>();
    	std::shared_ptr<std::vector<glm::vec3>> attributes = std::make_shared<std::vector<glm::vec3>>();
		
	public:
		GLPointCloud() { mSize = 0; };
		~GLPointCloud() {};

		/**
		 * @brief Adds another point cloud to the current point cloud.
		 * @param cloud The point cloud to be added.
		 * @return Reference to the updated OpenGL_PointCloud object.
		 */
		GLPointCloud &operator+=(const GLPointCloud &cloud);

		/**
		 * @brief Adds another point cloud to the current point cloud.
		 * @param cloud The point cloud to be added.
		 * @return The resulting point cloud.
		 */
		GLPointCloud operator+(const GLPointCloud &cloud) const;

		void addPoint(float x, float y, float z, uint8_t r, uint8_t g, uint8_t b);
		size_t max_size() const;
		void Finallized();
		const glm::vec3& getPosition(size_t index) const;
		const glm::vec3& getAttribute(size_t index) const;
		const std::vector<glm::vec3> getPositionsVec() const;
		const std::vector<glm::vec3> getAttributesVec() const;
		void Clear();
		void parse(std::shared_ptr<std::vector<glm::vec3>> position_vec, std::shared_ptr<std::vector<glm::vec3>> attribute_vec);
	};
		
}

#endif
