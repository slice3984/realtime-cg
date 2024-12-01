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
    processImages(gltfScene, model);
    processMaterials(gltfScene, model);

    processScenes(gltfScene, model);

    return std::move(gltfScene);
}

void GltfLoader::processImages(GltfScene &gltfScene, Model &model) {
    for (const Image &image : model.images) {
        gltfScene.images.emplace_back(GltfImage{
            image.component,
            image.pixel_type,
            image.bits,
            image.width,
            image.height,
            image.image
        });
    }
}

void GltfLoader::processTextures(GltfScene &gltfScene, Model &model) {
    for (const Texture &texture : model.textures) {
        // We ignore sampler settings
        // texture.
    }
}

void GltfLoader::processMaterials(GltfScene &gltfScene, Model &model) {
    // We ignore sampler settings, retrieve the image idx directly
    auto retrieveImageIdx = [&](const int index) -> int {
        return model.textures[index].source;
    };

    for (const Material &material : model.materials) {
        GltfMaterial gltfMaterial;
        std::vector<double> baseColorFactor = material.pbrMetallicRoughness.baseColorFactor;
        gltfMaterial.baseColorFactor = glm::vec4{baseColorFactor.at(0), baseColorFactor.at(1), baseColorFactor.at(2), baseColorFactor.at(3)};

        // Processing textures
        if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) {
            GltfTextureProperties properties;
            properties.type = GltfTextureType::DIFFUSE;
            int idxTexture = material.pbrMetallicRoughness.baseColorTexture.index;

            properties.index = retrieveImageIdx(idxTexture);
            gltfMaterial.textureProperties.push_back(properties);
        }

        if (material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) {
            GltfTextureProperties properties;
            properties.type = GltfTextureType::METALLIC_ROUGHNESS;
            const int idxTexture = material.pbrMetallicRoughness.metallicRoughnessTexture.index;

            properties.index = retrieveImageIdx(idxTexture);
            gltfMaterial.textureProperties.push_back(properties);
        }

        if (material.normalTexture.index >= 0) {
            GltfTextureProperties properties;
            properties.type = GltfTextureType::NORMAL;
            const int idxTexture = material.normalTexture.index;

            properties.index = retrieveImageIdx(idxTexture);
            properties.property = material.normalTexture.scale;
            gltfMaterial.textureProperties.push_back(properties);
        }

        if (material.occlusionTexture.index >= 0) {
            GltfTextureProperties properties;
            properties.type = GltfTextureType::OCCLUSION;
            const int idxTexture = material.occlusionTexture.index;

            properties.index = retrieveImageIdx(idxTexture);
            properties.property = material.occlusionTexture.strength;
            gltfMaterial.textureProperties.push_back(properties);
        }

        if (material.emissiveTexture.index >= 0) {
            GltfTextureProperties properties;
            properties.type = GltfTextureType::EMISSIVE;
            const int idxTexture = material.emissiveTexture.index;

            properties.index = retrieveImageIdx(idxTexture);
            gltfMaterial.textureProperties.push_back(properties);
        }

        gltfScene.materials.push_back(std::move(gltfMaterial));
    }
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

        // Handle materials
        gltfPrimitive.materialIdx = primitive.material;

        gltfMesh.primitives.emplace_back(std::move(gltfPrimitive));
    }

    meshes.emplace_back(std::move(gltfMesh));
}

