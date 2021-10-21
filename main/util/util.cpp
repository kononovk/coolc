#include <filesystem>
#include <fstream>
#include <string>

std::string ReadAllFile(std::string filename) {
  std::ifstream is(std::filesystem::path{filename});
  if (!is.is_open()) {
    throw std::runtime_error{"File" + filename + "does not exist"};
  }
  return std::string(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>());
}
