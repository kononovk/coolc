#include <filesystem>
#include <fstream>
#include <string>

std::string ReadAllFile(std::string filename) {
  std::ifstream is(std::filesystem::path{filename});
  return std::string(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>());
}
