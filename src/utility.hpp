#pragma once

#include "imgui.h"

static void HelpMarker(const char *desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

#include <iostream>
#include <vector>
#include <string>

#ifdef _WIN32
#include <windows.h>
#include <tchar.h> // For _T macro and TCHAR type
#else
#include <dirent.h>
#include <sys/types.h>
#endif

std::vector<std::string> getFilenamesInDir(const std::string &directoryPath)
{
    std::vector<std::string> filenames;

#ifdef _WIN32
    std::wstring searchPath = std::wstring(directoryPath.begin(), directoryPath.end()) + L"\\*.*";
    WIN32_FIND_DATAW findFileData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Error: Unable to open directory " << directoryPath << std::endl;
        return filenames;
    }
    do
    {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            filenames.push_back(std::string(findFileData.cFileName, findFileData.cFileName + wcslen(findFileData.cFileName)));
        }
    } while (FindNextFileW(hFind, &findFileData) != 0);
    FindClose(hFind);

#else
    DIR *dirp = opendir(directoryPath.c_str());
    if (dirp == nullptr)
    {
        std::cerr << "Error: Unable to open directory " << directoryPath << std::endl;
        return filenames;
    }

    struct dirent *dp;
    while ((dp = readdir(dirp)) != nullptr)
    {
        if (dp->d_type == DT_REG)
        { // Check if it's a regular file
            filenames.push_back(dp->d_name);
        }
    }
    closedir(dirp);
#endif

    return filenames;
}

ImVector<char *> getFilenamesInDirC(const std::string &directoryPath)
{
    ImVector<char *> filenames;

    std::vector<std::string> filenamesStr = getFilenamesInDir(directoryPath);
    for (const std::string &filename : filenamesStr)
    {
        char *filenameC = new char[filename.size() + 1];
        strcpy(filenameC, filename.c_str());
        filenames.push_back(filenameC);
    }

    return filenames;
}

#define STB_IMAGE_IMPLEMENTATION
#include "../stb-master/stb_image.h"

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char *filename, GLuint *out_texture, int *out_width, int *out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char *image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}
