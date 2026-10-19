#ifndef __ObjExporter_h
#define __ObjExporter_h

#include "geometry/TriangleMesh.h"
#include "graphics/Application.h"
#include <string>
#include <fstream>
#include <Windows.h> // Inclui os Popups do Windows!

namespace cg {

inline bool exportMeshToOBJ(const TriangleMesh& mesh, const std::string& meshName) {
    // 1. Tenta usar o caminho oficial do professor
    std::string filename = "meshes/" + meshName + ".obj";
    std::string filepath = Application::assetFilePath(filename.c_str());
    std::ofstream file(filepath.c_str());
    
    // 2. Se o Windows/OneDrive bloquear, tenta forçar o caminho local
    if (!file.is_open()) {
        filepath = "apps/tp1/assets/meshes/" + meshName + ".obj";
        file.open(filepath.c_str());
    }
    
    // 3. Em último recurso de emergência, salva na mesma pasta do executável
    if (!file.is_open()) {
        filepath = meshName + ".obj";
        file.open(filepath.c_str());
    }

    // Se tudo falhar, exibe uma mensagem de erro na tela
    if (!file.is_open()) {
        MessageBoxA(NULL, "Erro: O Windows ou o OneDrive bloqueou a criacao do arquivo!", "Falha na Exportacao", MB_ICONERROR | MB_OK);
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
    
    // POPUP DE SUCESSO! Aparecerá no meio da tela com o caminho exato
    std::string mensagem = "Arquivo OBJ criado e salvo com sucesso em:\n\n" + filepath;
    MessageBoxA(NULL, mensagem.c_str(), "Sucesso!", MB_ICONINFORMATION | MB_OK);
    
    return true;
}

} // namespace cg

#endif // __ObjExporter_h