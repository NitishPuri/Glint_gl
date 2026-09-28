#pragma once

#include <glm/glm.hpp>
#include <vector>

using index_t = unsigned int;

void indexVBO(const std::vector<glm::vec3>& in_vertices, const std::vector<glm::vec2>& in_uvs,
              const std::vector<glm::vec3>& in_normals,
              // outputs
              std::vector<index_t>& out_indices, std::vector<glm::vec3>& out_vertices, std::vector<glm::vec2>& out_uvs,
              std::vector<glm::vec3>& out_normals);

void indexVBO_TBN(std::vector<glm::vec3>& in_vertices, std::vector<glm::vec2>& in_uvs,
                  std::vector<glm::vec3>& in_normals, std::vector<glm::vec3>& in_tangents,
                  std::vector<glm::vec3>& in_bitangents,
                  // outputs
                  std::vector<index_t>& out_indices, std::vector<glm::vec3>& out_vertices,
                  std::vector<glm::vec2>& out_uvs, std::vector<glm::vec3>& out_normals,
                  std::vector<glm::vec3>& out_tangents, std::vector<glm::vec3>& out_bitangents);
