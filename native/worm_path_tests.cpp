#include "worm_path.hpp"
#include <iostream>
#include <stdexcept>
using namespace rdzen;

void require(bool value, const char* message) {
  if (!value)
    throw std::runtime_error(message);
}
void followCorners(WormPath& path, int steps) {
  Vec3 start{};
  for (Vec3 end : {Vec3{14, 0, 0}, Vec3{14, 0, 14}, Vec3{0, 0, 14}, Vec3{0, 0, 7}}) {
    for (int n = 1; n <= steps; ++n)
      path.advance(start + (end - start) * (float(n) / steps));
    start = end;
  }
}
int main() {
  try {
    WormPath path;
    path.reset({}, {-1, 0, 0});
    followCorners(path, 7);
    constexpr std::array<Vec3, 8> expected{{{0, 0, 14},
                                            {7, 0, 14},
                                            {14, 0, 14},
                                            {14, 0, 7},
                                            {14, 0, 0},
                                            {7, 0, 0},
                                            {0, 0, 0},
                                            {-7, 0, 0}}};
    for (size_t i = 0; i < expected.size(); ++i)
      require(length(path.segments[i] - expected[i]) < .001f,
              "Body cuts a corner instead of retracing the head");
    WormPath fine;
    fine.reset({}, {-1, 0, 0});
    followCorners(fine, 112);
    for (size_t i = 0; i < expected.size(); ++i)
      require(length(fine.segments[i] - path.segments[i]) < .001f,
              "Body spacing depends on frame rate or movement speed");
    auto stopped = path.segments;
    for (int n = 0; n < 1000; ++n)
      path.advance({0, 0, 7});
    for (size_t i = 0; i < stopped.size(); ++i)
      require(length(stopped[i] - path.segments[i]) == 0,
              "Stationary head still drags or relaxes its body");
    std::cout << "PASS Snake turns without corner cutting, frame independence and stopped body\n";

    path.reset({}, {-1, 0, 0});
    for (int n = 1; n <= 10000; ++n)
      path.advance({n * .01f, 0, 0});
    for (size_t i = 0; i < expected.size(); ++i)
      require(length(path.segments[i] - Vec3{100 - 7 * float(i + 1), 0, 0}) < .001f,
              "Slow movement overwrites history before the tail passes");
    path.advance({1000000, 0, 0});
    for (size_t i = 0; i < expected.size(); ++i)
      require(length(path.segments[i] - Vec3{1000000 - 7 * float(i + 1), 0, 0}) < .1f,
              "Large movement breaks bounded history");
    std::cout << "PASS distance-based history across 10000 tiny steps and a large debug jump\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL " << e.what() << '\n';
    return 1;
  }
}
