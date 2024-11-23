//
// Created by slice on 11/22/24.
//

#include "GltfLoader.h"
#include <filesystem>
#include <iostream>

GltfLoader::GltfLoader() = default;

GltfScene GltfLoader::loadModel(const std::string &path) {
    Model model;

    TinyGLTF loader;
    std::string err;
    std::string warn;

    std::filesystem::path filePath(path);
    std::string extension = filePath.extension().string();

    bool success = false;

    if (extension == ".gltf") {
        success = loader.LoadASCIIFromFile(&model, &err, &warn, path);
    } else if (extension == ".glb") {
        success = loader.LoadBinaryFromFile(&model, &err, &warn, path);
    } else {
        std::cerr << "Unsupported file extension: " << extension << std::endl;
        return GltfScene{};
    }

    if (!success) {
        std::cerr << "Error loading model: " << err << std::endl;
        return GltfScene{};
    }

    GltfScene gltfScene;

    processScenes(gltfScene, model);

    return std::move(gltfScene);
}


void GltfLoader::processScenes(GltfScene &gltfScene, Model &model) {
    for (const Scene &scene : model.scenes) {
        gltfScene.name = scene.name;

        // Each node in a scene is a root node / represents an object
        for (int nodeIdx : scene.nodes) {
            const Node &node = model.nodes[nodeIdx];

            if (node.mesh >= 0) {
                GltfObject object;
                std::vector<GltfMesh> meshes;

                // Process the mesh attached to the node
                processNode(model, meshes, node);
                object.name = node.name;
                object.meshes = std::move(meshes);
                gltfScene.objects.emplace_back(std::move(object));
            }
            else if (node.mesh == -1 && !node.children.empty()) {
                GltfObject object;
                std::vector<GltfMesh> meshes;

                // Recursively process this node and its children as an object with geometry
                processNode(model, meshes, node);
                object.name = node.name;
                object.meshes = std::move(meshes);
                gltfScene.objects.emplace_back(std::move(object));
            }
        }
    }
}


void GltfLoader::processNode(Model &model, std::vector<GltfMesh> &meshes, const Node &node) {
    // A node could be a camera, ignore in this case
    if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
        processMesh(model, meshes, model.meshes[node.mesh]);
    }

    for (int nodeChildrenIdx : node.children) {
        processNode(model, meshes, model.nodes[nodeChildrenIdx]);
    }
}

void GltfLoader::processMesh(Model &model, std::vector<GltfMesh> &meshes, const Mesh &mesh) {
    GltfMesh gltfMesh;
    gltfMesh.name = mesh.name;

    for (const Primitive &primitive : mesh.primitives) {
        GltfPrimitive gltfPrimitive;

        gltfPrimitive.mode = primitive.mode;

        for (auto &attrib : primitive.attributes) {
            GltfVertexAttrib gltfVertexAttrib;

            Accessor &accessor = model.accessors[attrib.second];
            BufferView &bufferView = model.bufferViews[accessor.bufferView];
            Buffer &buffer = model.buffers[bufferView.buffer];

            gltfVertexAttrib.attribName = attrib.first;
            gltfVertexAttrib.componentType = accessor.componentType;
            gltfVertexAttrib.elemCount = accessor.count;
            gltfVertexAttrib.datatype = accessor.type;
            gltfVertexAttrib.bufferSize = bufferView.byteLength;

            gltfVertexAttrib.buffer = new std::byte[bufferView.byteLength];
            memcpy(gltfVertexAttrib.buffer, &buffer.data.at(bufferView.byteOffset), bufferView.byteLength);

            if (attrib.first == "POSITION") {
                gltfPrimitive.attributes[GltfAttribute::POSITION] = std::move(gltfVertexAttrib);
            } else if (attrib.first == "NORMAL") {
                gltfPrimitive.attributes[GltfAttribute::NORMAL] = std::move(gltfVertexAttrib);
            } else if (attrib.first == "TANGENT") {
                gltfPrimitive.attributes[GltfAttribute::TANGENT] = std::move(gltfVertexAttrib);
            } else if (attrib.first == "TEXCOORD_0") {
                gltfPrimitive.attributes[GltfAttribute::TEXCOORD_0] = std::move(gltfVertexAttrib);
            } else if (attrib.first == "COLOR_0") {
                gltfPrimitive.attributes[GltfAttribute::COLOR_0] = std::move(gltfVertexAttrib);
            } else {
                std::cerr << "Unknown attribute " << attrib.first << std::endl;
            }
        }

        if (primitive.indices >= 0) {
            Accessor &accessorEbo = model.accessors[primitive.indices];
            BufferView &bufferView = model.bufferViews[accessorEbo.bufferView];
            Buffer &buffer = model.buffers[bufferView.buffer];

            gltfPrimitive.indexBuffer = new std::byte[bufferView.byteLength];
            gltfPrimitive.elemCount = accessorEbo.count;
            gltfPrimitive.indexBufferSize = bufferView.byteLength;
            gltfPrimitive.componentType = accessorEbo.componentType;

            memcpy(gltfPrimitive.indexBuffer, &buffer.data.at(bufferView.byteOffset), bufferView.byteLength);
        }

        gltfMesh.primitives.emplace_back(std::move(gltfPrimitive));
    }

    meshes.emplace_back(std::move(gltfMesh));

    // TODO: Materials, textures..
}

