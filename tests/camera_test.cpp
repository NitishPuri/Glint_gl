#include "core/camera.h"

#include "../test_cfg.h"

void camera_test() {
  using namespace boost::ut;

  "camera_matrix_tests"_test = [] {
    // Test default position
    {
      const auto result = camera(1.0f, glm::vec2(0.0f, 0.0f));
      expect(glm::determinant(result) != 0.0f) << "Matrix should be invertible";
      expect(std::abs(result[1][1] / result[0][0]) - (3.0f / 4.0f) < 0.01f) << "Aspect ratio should be maintained";
    }

    // Test translation
    {
      const auto near = camera(0.1f, glm::vec2(0.0f, 0.0f));
      const auto far = camera(100.0f, glm::vec2(0.0f, 0.0f));
      expect(near[3][2] != far[3][2]) << "Near and far translations should differ";
    }

    // Test rotation
    {
      const auto rotated = camera(1.0f, glm::vec2(glm::pi<float>(), 0.0f));
      const auto identity = camera(1.0f, glm::vec2(0.0f, 0.0f));
      expect(rotated != identity) << "Rotation should change matrix";
    }

    // Test combined transformations
    {
      const auto result = camera(2.0f, glm::vec2(1.0f, 0.5f));
      expect(glm::determinant(result) != 0.0f) << "Complex transformation should be valid";
    }
  };
}