#include "Mesh.h"

#include "Core/Logger.h"
#include "Core/ScopedTimer.h"
#include "Core/VBOIndex.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"

Mesh::Mesh(const std::string& filename) { loadMesh(filename); }

Mesh::~Mesh() {}

void Mesh::loadMesh(const std::string& filename) {
  tinyobj::ObjReaderConfig reader_config;
  reader_config.mtl_search_path = "./";  // Path to material files

  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(filename, reader_config)) {
    if (!reader.Error().empty()) {
      Logger::error("TinyObjReader error: ", reader.Error());
    }
    throw std::runtime_error("Failed to load/parse .obj file");
    return;
  }

  if (!reader.Warning().empty()) {
    Logger::log("TinyObjReader warning: ", reader.Warning());
  }

  auto& attrib = reader.GetAttrib();
  auto& shapes = reader.GetShapes();
  auto& materials = reader.GetMaterials();

  Logger::log("Loaded ", shapes.size(), " shapes");
  Logger::log("Loaded ", materials.size(), " materials");

  // Loop over shapes
  for (size_t s = 0; s < shapes.size(); s++) {
    // Loop over faces(polygon)
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

      // Loop over vertices in the face.
      for (size_t v = 0; v < fv; v++) {
        // access to vertex
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
        tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
        tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
        tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

        vertices.emplace_back(vx, vy, vz);

        // Check if `normal_index` is zero or positive. negative = no normal data
        if (idx.normal_index >= 0) {
          tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
          tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
          tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

          normals.emplace_back(nx, ny, nz);
        }

        // Check if `texcoord_index` is zero or positive. negative = no texcoord data
        if (idx.texcoord_index >= 0) {
          tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
          tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];

          texCoords.emplace_back(tx, ty);
        }

        indices.push_back(int(indices.size()));

        // Optional: vertex colors
        // tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
        // tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
        // tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
      }
      index_offset += fv;

      // per-face material
      //   shapes[s].mesh.material_ids[f];
    }
  }
}

void Mesh::index() {
  std::vector<glm::vec3> indexed_vertices;
  std::vector<glm::vec2> indexed_texCoords;
  std::vector<glm::vec3> indexed_Normals;
  std::vector<index_t> updated_indices;

  {
    ScopedTimer _("Indexing...");
    indexVBO(vertices, texCoords, normals, updated_indices, indexed_vertices, indexed_texCoords, indexed_Normals);
  }

  Logger::log("V_In[", vertices.size(), "] V_Out[", indexed_vertices.size(), "]");

  vertices = indexed_vertices;
  texCoords = indexed_texCoords;
  normals = indexed_Normals;
  indices = updated_indices;
}

void Mesh::indexWithTangentBasis() {
  std::vector<glm::vec3> indexed_vertices;
  std::vector<glm::vec2> indexed_texCoords;
  std::vector<glm::vec3> indexed_normals;
  std::vector<glm::vec3> indexed_tangets;
  std::vector<glm::vec3> indexed_bitanget;
  std::vector<index_t> updated_indices;

  {
    ScopedTimer _("Indexing TBN...");
    computeTangentBasis();
    indexVBO_TBN(vertices, texCoords, normals, tangents, bitangents, updated_indices, indexed_vertices,
                 indexed_texCoords, indexed_normals, indexed_tangets, indexed_bitanget);
  }

  Logger::log("V_In[", vertices.size(), "] V_Out[", indexed_vertices.size(), "]");

  vertices = indexed_vertices;
  texCoords = indexed_texCoords;
  normals = indexed_normals;
  indices = updated_indices;
  tangents = indexed_tangets;
  bitangents = indexed_bitanget;
}

// https://terathon.com/blog/tangent-space.html
// https://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
void Mesh::computeTangentBasis() {
  for (unsigned int i = 0; i < vertices.size(); i += 3) {
    // Shortcuts for vertices
    glm::vec3& v0 = vertices[i + 0];
    glm::vec3& v1 = vertices[i + 1];
    glm::vec3& v2 = vertices[i + 2];

    // Shortcuts for UVs
    glm::vec2& uv0 = texCoords[i + 0];
    glm::vec2& uv1 = texCoords[i + 1];
    glm::vec2& uv2 = texCoords[i + 2];

    // Edges of the triangle : postion delta
    glm::vec3 deltaPos1 = v1 - v0;
    glm::vec3 deltaPos2 = v2 - v0;

    // UV delta
    glm::vec2 deltaUV1 = uv1 - uv0;
    glm::vec2 deltaUV2 = uv2 - uv0;

    float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
    glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
    glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

    // Set the same tangent for all three vertices of the triangle.
    // They will be merged later, in vboindexer.cpp
    tangents.push_back(tangent);
    tangents.push_back(tangent);
    tangents.push_back(tangent);

    // Same thing for binormals
    bitangents.push_back(bitangent);
    bitangents.push_back(bitangent);
    bitangents.push_back(bitangent);
  }

  for (unsigned int i = 0; i < vertices.size(); i += 1) {
    glm::vec3& n = normals[i];
    glm::vec3& t = tangents[i];
    glm::vec3& b = bitangents[i];

    // Gram-Schmidt orthogonalize
    t = glm::normalize(t - n * glm::dot(n, t));

    // Calculate handedness
    if (glm::dot(glm::cross(n, t), b) < 0.0f) {
      t = t * -1.0f;
    }
  }
}