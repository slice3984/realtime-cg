//
// Created by slice on 12/26/24.
//

#ifndef CHUNKLODMESH_H
#define CHUNKLODMESH_H
#include <unordered_map>

#include "TerrainPatchLODGenerator.h"
#include "../../Linking/include/glad/glad.h"

struct ChunkMesh {
    GLuint VAO;
    GLuint elemCount;
};

struct ChunkLODMesh {
    GLuint unstitchedMeshVAO;
    GLuint unstitchedMeshElemCount;
    std::unordered_map<STITCHED_EDGE, ChunkMesh> stitchedMeshes;
};

#endif //CHUNKLODMESH_H
