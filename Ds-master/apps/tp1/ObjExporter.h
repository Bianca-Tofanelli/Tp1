#ifndef __ObjExporter_h
#define __ObjExporter_h

#include "geometry/TriangleMesh.h"
#include "graphics/Application.h" 
#include <string>
#include <fstream>
#include <iostream>

namespace cg {

inline bool exportMeshToOBJ(const TriangleMesh& mesh, const std::string& meshName) {
    // 1. Juntamos os textos primeiro
    std::string filename = "meshes/" + meshName + ".obj";
    
    // 2. Passamos com .c_str() para agradar a função do professor
    std::string filepath = Application::assetFilePath(filename.c_str());
    
    std::ofstream file(filepath);
    
    if (!file.is_open()) {
        std::cerr << "Erro: Nao foi possivel salvar o arquivo em: " << filepath << "\n";
        return false;
    }

    const auto& data = mesh.data();
    file << "# Malha exportada (Trabalho Pratico 1 - CG)\n";
    file << "o " << meshName << "\n";

    for (int i = 0; i < data.vertexCount; ++i) {
        file << "v " << data.vertices[i].x << " " << data.vertices[i].y << " " << data.vertices[i].z << "\n";
    }

    if (data.vertexNormals) {
        for (int i = 0; i < data.vertexCount; ++i) {
            file << "vn " << data.vertexNormals[i].x << " " << data.vertexNormals[i].y << " " << data.vertexNormals[i].z << "\n";
        }
    }

    for (int i = 0; i < data.triangleCount; ++i) {
        int v0 = data.triangles[i].v[0] + 1;
        int v1 = data.triangles[i].v[1] + 1;
        int v2 = data.triangles[i].v[2] + 1;

        if (data.vertexNormals) {
            file << "f " << v0 << "//" << v0 << " " << v1 << "//" << v1 << " " << v2 << "//" << v2 << "\n";
        } else {
            file << "f " << v0 << " " << v1 << " " << v2 << "\n";
        }
    }

    file.close();
    std::cout << "\nSUCESSO! O arquivo OBJ foi salvo em: " << filepath << "\n";
    return true;
}

} // namespace cg

#endif // __ObjExporter_h