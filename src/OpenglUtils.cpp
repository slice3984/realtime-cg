//
// Created by slice on 11/13/24.
//

#include "OpenglUtils.h"

namespace opengl_utils {
    std::vector<const GltfPrimitive*> unpackGltfScene(const GltfScene &scene) {
        std::vector<const GltfPrimitive*> primitives;

        for (const GltfObject& object : scene.objects) {
            for (const GltfMesh& mesh : object.meshes) {
                for (const GltfPrimitive &primitive : mesh.primitives) {
                    primitives.push_back(&primitive);
                }
            }
        }

        return primitives;
    }
}
