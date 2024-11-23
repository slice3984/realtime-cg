//
// Created by slice on 11/22/24.
//

// https://www.khronos.org/files/gltf20-reference-guide.pdf

#ifndef GLTFLOADER_H
#define GLTFLOADER_H
#include <iostream>

#include "tiny_gltf.h"

using namespace tinygltf;

enum class GltfAttribute {
    POSITION, NORMAL, TANGENT, TEXCOORD_0, COLOR_0
};

struct GltfVertexAttrib {
    std::string attribName;
    void *buffer;
    int componentType; // OpenGL data type
    int datatype; // VEC2..
    std::size_t elemCount;
    std::size_t bufferSize;
};
struct GltfPrimitive {
    std::unordered_map<GltfAttribute, GltfVertexAttrib> attributes;
    void *indexBuffer;
    std::size_t indexBufferSize;
    int componentType;
    std::size_t elemCount;
    int mode;

    const GltfVertexAttrib& get(GltfAttribute attribName) const {
        if (attributes.find(attribName) == attributes.end()) {
            throw std::out_of_range("Attribute not found");
        }

        return attributes.find(attribName)->second;
    }

    // TODO: Figure out how to clear this, i should maybe look into smart pointers
};

struct GltfMesh {
    std::string name;
    std::vector<GltfPrimitive> primitives;
};

struct GltfObject {
    std::string name;
    std::vector<GltfMesh> meshes;
};

struct GltfScene {
    std::string name;
    std::vector<GltfObject> objects;
};

class GltfLoader {
private:
    void processScenes(GltfScene &gltfScene, Model &model);
    void processNode(Model &model, std::vector<GltfMesh> &meshes, const Node &node);
    void processMesh(Model &model, std::vector<GltfMesh> &meshes, const Mesh &mesh);

public:
    explicit GltfLoader();
    GltfScene loadModel(const std::string &path);
};

#endif //GLTFLOADER_H
