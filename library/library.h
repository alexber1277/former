#pragma once
#include <string>
#include <vector>
#include "../gl/glincludes.h"

struct LibraryAsset {
    std::string name;
    std::string path;
    std::string thumbnail;
    GLuint thumbnailTex = 0;
};

class Library {
    public:

    GLuint PreviewFBO;
    GLuint PreviewTex;
    GLuint PreviewDepth;

    GLuint PreviewShader;

    std::vector<LibraryAsset> assets;

    int SizePreview = 500;

    void InitPreviewBuffer();
    void LoadElementsLib();
    void GeneratePreview(const std::string& meshPath,const std::string& thumbPath);
    GLuint LoadTexture(const std::string& path);

};