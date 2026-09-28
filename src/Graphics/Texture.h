// Texture.h
#pragma once
#include <string>

class Texture {
 public:
  Texture(int width, int height);
  Texture(const std::string& path);
  ~Texture();

  void bind(unsigned int slot = 0) const;
  void unbind() const;

  std::string getFilePath() const { return m_FilePath; }

  unsigned int getID() const { return m_ID; }

 private:
  unsigned int m_ID;
  int m_Width, m_Height, m_BPP;
  std::string m_FilePath;
};
