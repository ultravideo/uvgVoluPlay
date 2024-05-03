#include "pointcloud.h"
#include "pch.h"
#include <algorithm>

namespace nelems
{
	void GLPointCloud::addPoint(float x, float y, float z, uint8_t r, uint8_t g, uint8_t b)
	{
		glm::vec3 position{x, y, z};
		glm::vec3 color{r/255.0f, g/255.0f, b/255.0f};

		mSize++;
		if (positions->size() < mSize){
			positions->resize(positions->size()*2+1);
			attributes->resize(attributes->size()*2+1);
		}

		(*positions)[mSize-1] = (position);
		(*attributes)[mSize-1] = (color);
	}
	
	size_t GLPointCloud::max_size() const
	{
		return positions->size();
	}

	GLPointCloud &GLPointCloud::operator+=(const GLPointCloud &cloud)
	{
		if (cloud.IsEmpty())
			return (*this);

		size_t old_vert_num = mSize;
		size_t add_vert_num = cloud.max_size();
		size_t new_vert_num = old_vert_num + add_vert_num;

		if (cloud.HasPoints())
		{
			mSize += add_vert_num;
			if (positions->size() < new_vert_num){
				positions->resize(std::max(positions->size()*2, new_vert_num));
				attributes->resize(std::max(attributes->size()*2, new_vert_num));
			}

			memcpy(positions->data()+old_vert_num, cloud.getPositionsVec().data(), add_vert_num * sizeof(glm::vec3));
			memcpy(attributes->data()+old_vert_num, cloud.getAttributesVec().data(), add_vert_num * sizeof(glm::vec3));
		}

		return *this;
	};
	
	const glm::vec3& GLPointCloud::getPosition(size_t index) const 
	{
		return (*positions)[index];
	}

	const glm::vec3& GLPointCloud::getAttribute(size_t index) const 
	{
		return (*attributes)[index];
	}

	const std::vector<glm::vec3> GLPointCloud::getPositionsVec() const 
	{
		return *positions;
	}

	const std::vector<glm::vec3> GLPointCloud::getAttributesVec() const 
	{
		return *attributes;
	}

	void GLPointCloud::Clear() 
	{
		positions->clear();
		attributes->clear();
	}

	void GLPointCloud::Finallized() 
	{
		positions->resize(mSize);
		attributes->resize(mSize);
	}

	void GLPointCloud::parse(std::shared_ptr<std::vector<glm::vec3>> position_vec, std::shared_ptr<std::vector<glm::vec3>> attribute_vec)
	{
		positions = position_vec;
		attributes = attribute_vec;
	}

	bool GLPointCloud::HasPoints() const { return positions->size() > 0; };

	bool GLPointCloud::IsEmpty() const { return !HasPoints(); };
}
