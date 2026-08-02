
#include "model_obj.hpp"

sModelData* gLoadOBJ(const std::string &_fileName) {
    std::ifstream objFile(_fileName);
    if (!objFile.is_open()) {
        std::cout << "Failed to open file: " << _fileName << std::endl;
        return nullptr;
    }

    // Global temporary storage
    std::vector<glm::vec3>  globalPos;
    std::vector<glm::vec2>  globalTex;
    std::vector<glm::vec3>  globalNorm;

    struct FaceVertex {
        int p, t, n; // indices (1‑based)
    };
    struct TempMesh {
        std::string name;
        std::vector<FaceVertex> faces; // flattened triangle vertices
    };
    std::vector<TempMesh> meshes;
    meshes.emplace_back(); // default mesh for data before any 'o'

    std::string line;
    while (std::getline(objFile, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "o") {
            meshes.emplace_back();
            iss >> meshes.back().name;
        }
        else if (token == "v") {
            glm::vec3 v;
            iss >> v.x >> v.y >> v.z;
            globalPos.push_back(v);
        }
        else if (token == "vt") {
            glm::vec2 vt;
            iss >> vt.x >> vt.y;
            globalTex.push_back(vt);
        }
        else if (token == "vn") {
            glm::vec3 vn;
            iss >> vn.x >> vn.y >> vn.z;
            globalNorm.push_back(vn);
        }
        else if (token == "f") {
            // Parse face (supports v, v/vt, v//vn, v/vt/vn)
            std::vector<FaceVertex> poly;
            std::string vertStr;
            while (iss >> vertStr) {
                FaceVertex fv = {0,0,0};
                size_t firstSlash = vertStr.find('/');
                size_t secondSlash = vertStr.find('/', firstSlash+1);
                fv.p = std::stoi(vertStr.substr(0, firstSlash));
                if (firstSlash != std::string::npos) {
                    if (secondSlash == firstSlash+1) { // v//vn
                        fv.n = std::stoi(vertStr.substr(secondSlash+1));
                    } else {
                        std::string texPart = vertStr.substr(firstSlash+1, secondSlash - (firstSlash+1));
                        if (!texPart.empty()) fv.t = std::stoi(texPart);
                        if (secondSlash != std::string::npos) {
                            fv.n = std::stoi(vertStr.substr(secondSlash+1));
                        }
                    }
                }
                poly.push_back(fv);
            }
            // Triangulate (simple fan)
            for (size_t i = 1; i+1 < poly.size(); ++i) {
                meshes.back().faces.push_back(poly[0]);
                meshes.back().faces.push_back(poly[i]);
                meshes.back().faces.push_back(poly[i+1]);
            }
        }
    }

    // Build output model (unindexed vertices, one per face vertex)
    sModelData* model = new sModelData;
    for (const auto& tmpMesh : meshes) {
        if (tmpMesh.faces.empty()) continue; // skip empty meshes
        model->mesh.emplace_back();
        auto& outMesh = model->mesh.back();
        outMesh.name = tmpMesh.name;
        outMesh.index.resize(tmpMesh.faces.size()); // sequential index
        outMesh.vertex.resize(tmpMesh.faces.size());

        for (size_t i = 0; i < tmpMesh.faces.size(); ++i) {
            const auto& fv = tmpMesh.faces[i];
            outMesh.index[i] = i;
            // OBJ indices are 1‑based; convert to 0‑based
            if (fv.p > 0) outMesh.vertex[i].position = globalPos[fv.p-1];
            if (fv.t > 0) outMesh.vertex[i].texCoord = globalTex[fv.t-1];
            if (fv.n > 0) outMesh.vertex[i].normal   = globalNorm[fv.n-1];
        }
    }

    objFile.close();
    return model;
}

void gSaveOBJ(sModelData *&_model, const std::string &_fileName)
{
    // if no data, early exit
    if (_model == nullptr)
        return;

    // Open the file
    std::ofstream objFile;
    objFile.open (_fileName);
    objFile << "# Grume obj loader" << std::endl;

    // For each mesh
    for (std::uint32_t m = 0; m < _model->mesh.size(); ++m)
    {
        objFile << "o " << _model->mesh[m].name << std::endl;

        // For each vertex position
        for (std::uint32_t v = 0; v < _model->mesh[m].vertex.size(); ++v)
        {
            objFile << "v " << _model->mesh[m].vertex[v].position.x << " "
                            << _model->mesh[m].vertex[v].position.y << " "
                            << _model->mesh[m].vertex[v].position.z << std::endl;
        }

        // For each vertex texture coord
        for (std::uint32_t vt = 0; vt < _model->mesh[m].vertex.size(); ++vt)
        {
            objFile << "vt " << _model->mesh[m].vertex[vt].texCoord.x << " "
                             << _model->mesh[m].vertex[vt].texCoord.y << std::endl;
        }

        // For each vertex normal
        for (std::uint32_t vn = 0; vn < _model->mesh[m].vertex.size(); ++vn)
        {
            objFile << "vn " << _model->mesh[m].vertex[vn].normal.x << " "
                             << _model->mesh[m].vertex[vn].normal.y << " "
                             << _model->mesh[m].vertex[vn].normal.z << std::endl;
        }

        // For each triangle face
        for (std::uint32_t f = 0; f < _model->mesh[m].index.size(); f += 3)
        {
            objFile << "f " << _model->mesh[m].index[f + 0] + 1 << "/"
                            << _model->mesh[m].index[f + 0] + 1 << "/"
                            << _model->mesh[m].index[f + 0] + 1 << " "

                            << _model->mesh[m].index[f + 1] + 1 << "/"
                            << _model->mesh[m].index[f + 1] + 1 << "/"
                            << _model->mesh[m].index[f + 1] + 1 << " "

                            << _model->mesh[m].index[f + 2] + 1 << "/"
                            << _model->mesh[m].index[f + 2] + 1 << "/"
                            << _model->mesh[m].index[f + 2] + 1 << std::endl;
        }
    }

    // Close the file
    objFile.close();
}

void gFreeOBJ(sModelData *&_model)
{
    if (_model != nullptr)
    {
        delete _model;
        _model = nullptr;
    }
}
