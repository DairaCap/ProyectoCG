#ifndef MODEL_H
#define MODEL_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <shader_m.h>
#include <mesh.h>
#include <modelstructs.h>
#include "AABB.h"

using namespace std;

class Model
{
public:
    vector<Texture> textures_loaded;
    vector<Mesh> meshes;
    string directory;
    bool gammaCorrection;
    string filename;
    unsigned int IDforPicking;
    bool rotateZUpToYUp;

    vector<AABB> cajasColision;

    struct ContactPoint {
        string name;
        glm::vec3 position;
        glm::vec3 normal;
        AABB box;
    };
    vector<ContactPoint> contactPoints;

    struct InteractiveDoor {
        string name;
        string handleName;
        glm::vec3 pivot;
        float currentAngle = 0.0f;
        float targetAngle = 0.0f;
        bool isOpen = false;
        float openDir = 1.0f;
        AABB closedCollider;

        AABB GetInteractionBox() const {
            glm::vec3 cMin = glm::min(closedCollider.minPoint, pivot) - glm::vec3(0.45f, 0.2f, 0.45f);
            glm::vec3 cMax = glm::max(closedCollider.maxPoint, pivot) + glm::vec3(0.45f, 0.2f, 0.45f);
            return AABB(cMin, cMax);
        }
    };

    struct InteractiveWindow {
        string name;
        glm::vec3 trackDir;
        float currentSlide = 0.0f;
        float targetSlide = 0.0f;
        bool isOpen = false;
        AABB closedCollider;

        AABB GetInteractionBox() const {
            return AABB(closedCollider.minPoint - glm::vec3(0.40f, 0.2f, 0.40f),
                closedCollider.maxPoint + glm::vec3(0.40f, 0.2f, 0.40f));
        }
    };

    vector<InteractiveDoor> doors;
    vector<InteractiveWindow> windows;

    void initInteractiveElements() {
        doors.clear();
        windows.clear();

        InteractiveDoor d0;
        d0.name = "Puerta.027";
        d0.handleName = "Manija.003";
        d0.pivot = glm::vec3(-3.0587f, 1.502f, 3.6825f);
        d0.openDir = 1.0f;
        d0.closedCollider = AABB(glm::vec3(-3.20f, 0.0f, 3.50f), glm::vec3(-1.85f, 2.45f, 3.85f));
        doors.push_back(d0);

        InteractiveDoor d1;
        d1.name = "Puerta.028";
        d1.handleName = "Manija.002";
        d1.pivot = glm::vec3(-0.6698f, 1.502f, 3.6825f);
        d1.openDir = -1.0f;
        d1.closedCollider = AABB(glm::vec3(-1.85f, 0.0f, 3.50f), glm::vec3(-0.50f, 2.45f, 3.85f));
        doors.push_back(d1);

        InteractiveDoor d2;
        d2.name = "Puerta.029";
        d2.handleName = "Manija.005";
        d2.pivot = glm::vec3(1.7213f, 1.344f, 1.3055f);
        d2.openDir = 1.0f;
        d2.closedCollider = AABB(glm::vec3(1.55f, 0.0f, 1.15f), glm::vec3(2.80f, 2.45f, 1.45f));
        doors.push_back(d2);

        InteractiveDoor d3;
        d3.name = "Puerta.030";
        d3.handleName = "Manija.001";
        d3.pivot = glm::vec3(3.0146f, 1.344f, 0.9984f);
        d3.openDir = 1.0f;
        d3.closedCollider = AABB(glm::vec3(2.85f, 0.0f, -0.05f), glm::vec3(3.18f, 2.45f, 1.15f));
        doors.push_back(d3);

        InteractiveDoor d4;
        d4.name = "Puerta.031";
        d4.handleName = "Manija.004";
        d4.pivot = glm::vec3(1.7213f, 1.344f, -1.4759f);
        d4.openDir = 1.0f;
        d4.closedCollider = AABB(glm::vec3(1.55f, 0.0f, -1.62f), glm::vec3(2.80f, 2.45f, -1.30f));
        doors.push_back(d4);

        InteractiveWindow w0;
        w0.name = "Ventana_Deslizable.011";
        w0.trackDir = glm::vec3(0.0f, 0.0f, 1.0f);
        w0.closedCollider = AABB(glm::vec3(-5.35f, 0.0f, -3.45f), glm::vec3(-4.95f, 2.80f, -1.80f));
        windows.push_back(w0);

        InteractiveWindow w1;
        w1.name = "Ventana_Deslizable.018";
        w1.trackDir = glm::vec3(1.0f, 0.0f, 0.0f);
        w1.closedCollider = AABB(glm::vec3(-1.30f, 0.0f, -4.85f), glm::vec3(0.45f, 2.80f, -4.50f));
        windows.push_back(w1);
    }

    void ToggleDoor(size_t index) {
        if (index < doors.size()) {
            doors[index].isOpen = !doors[index].isOpen;
            doors[index].targetAngle = doors[index].isOpen ? 85.0f : 0.0f;
            cout << "[Interaccion] Puerta [" << doors[index].name << "]: "
                << (doors[index].isOpen ? "ABIERTA" : "CERRADA") << endl;
        }
    }

    void ToggleWindow(size_t index) {
        if (index < windows.size()) {
            windows[index].isOpen = !windows[index].isOpen;
            windows[index].targetSlide = windows[index].isOpen ? 0.75f : 0.0f;
            cout << "[Interaccion] Ventana [" << windows[index].name << "]: "
                << (windows[index].isOpen ? "ABIERTA" : "CERRADA") << endl;
        }
    }

    void ToggleAllDoors() {
        bool anyOpen = false;
        for (const auto& d : doors) if (d.isOpen) anyOpen = true;
        for (size_t i = 0; i < doors.size(); ++i) {
            doors[i].isOpen = !anyOpen;
            doors[i].targetAngle = doors[i].isOpen ? 85.0f : 0.0f;
        }
    }

    void ToggleAllWindows() {
        bool anyOpen = false;
        for (const auto& w : windows) if (w.isOpen) anyOpen = true;
        for (size_t i = 0; i < windows.size(); ++i) {
            windows[i].isOpen = !anyOpen;
            windows[i].targetSlide = windows[i].isOpen ? 0.75f : 0.0f;
        }
    }

    void UpdateAnimations(float deltaTime) {
        float speed = 4.5f;
        for (auto& d : doors) {
            d.currentAngle += (d.targetAngle - d.currentAngle) * std::min(deltaTime * speed, 1.0f);
        }
        for (auto& w : windows) {
            w.currentSlide += (w.targetSlide - w.currentSlide) * std::min(deltaTime * speed, 1.0f);
        }
    }

    std::vector<AABB> GetActiveColliders() const {
        std::vector<AABB> colliders = cajasColision;
        for (const auto& d : doors) {
            if (!d.isOpen || d.currentAngle < 35.0f) {
                colliders.push_back(d.closedCollider);
            }
        }
        for (const auto& w : windows) {
            if (!w.isOpen || w.currentSlide < 0.35f) {
                colliders.push_back(w.closedCollider);
            }
        }
        return colliders;
    }

    vector<Bone> bones;
    Assimp::Importer importer;
    const aiScene* scene;
    map<string, unsigned int> m_BoneMapping;
    unsigned int m_NumBones;
    vector<BoneInfo> m_BoneInfo;
    glm::mat4 m_GlobalInverseTransform;

    Model(string const& path, bool rotateZUp = true, unsigned int pickingID = 0, bool gamma = false)
        : rotateZUpToYUp(rotateZUp), gammaCorrection(gamma), IDforPicking(pickingID), scene(nullptr), m_NumBones(0)
    {
        loadModel(path);
    }

    void Draw(Shader shader, const glm::mat4& globalModel = glm::mat4(1.0f))
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
        {
            if (meshes[i].isCollision || IsCollisionMaterial(meshes[i].materialName) || IsContactMaterial(meshes[i].materialName, meshes[i].nodeName))
                continue;

            glm::mat4 modelTransform = meshes[i].nodeTransform;

            for (size_t d = 0; d < doors.size(); ++d) {
                if (meshes[i].nodeName == doors[d].name || (!doors[d].handleName.empty() && meshes[i].nodeName == doors[d].handleName)) {
                    float ang = doors[d].currentAngle * doors[d].openDir;
                    glm::vec3 piv = doors[d].pivot;
                    glm::mat4 anim = glm::translate(glm::mat4(1.0f), piv)
                        * glm::rotate(glm::mat4(1.0f), glm::radians(ang), glm::vec3(0.0f, 1.0f, 0.0f))
                        * glm::translate(glm::mat4(1.0f), -piv);
                    modelTransform = anim * modelTransform;
                    break;
                }
            }

            for (size_t w = 0; w < windows.size(); ++w) {
                if (meshes[i].nodeName == windows[w].name) {
                    glm::vec3 offset = windows[w].trackDir * windows[w].currentSlide;
                    modelTransform = glm::translate(glm::mat4(1.0f), offset) * modelTransform;
                    break;
                }
            }

            glm::mat4 model = globalModel * modelTransform;
            shader.setMat4("model", model);
            meshes[i].Draw(shader);
        }
    }

    void SetPose(float time, glm::mat4* gBones) {
        if (!scene || !scene->mRootNode) return;
        glm::mat4 n_matrix(1.0f);
        ReadNodeHierarchy(time, scene->mRootNode, n_matrix);

        for (unsigned int i = 0; i < bones.size(); i++) {
            if (i < 100) {
                gBones[i] = bones[i].transformation;
            }
        }
    }

    double getNumFrames() {
        if (!scene || !scene->mAnimations || scene->mNumAnimations == 0) return -1.0;
        const aiAnimation* pAnimation = scene->mAnimations[0];
        if (pAnimation == nullptr) return -1.0;
        return pAnimation->mDuration;
    }

    double getFramerate() {
        if (!scene || !scene->mAnimations || scene->mNumAnimations == 0) return -1.0;
        const aiAnimation* pAnimation = scene->mAnimations[0];
        if (pAnimation == nullptr) return -1.0;
        return pAnimation->mTicksPerSecond;
    }

    void ResolveContactNormals() {
        for (auto& cp : contactPoints) {
            glm::vec3 testDirs[4] = {
                glm::vec3(1.0f,  0.0f,  0.0f),
                glm::vec3(-1.0f,  0.0f,  0.0f),
                glm::vec3(0.0f,  0.0f,  1.0f),
                glm::vec3(0.0f,  0.0f, -1.0f)
            };
            float minDist = 9999.0f;
            glm::vec3 bestNormal(0.0f, 0.0f, -1.0f);

            for (int d = 0; d < 4; ++d) {
                for (const auto& col : cajasColision) {
                    if (std::abs(testDirs[d].x) > 0.5f) {
                        if (cp.position.y >= col.minPoint.y - 0.4f && cp.position.y <= col.maxPoint.y + 0.4f &&
                            cp.position.z >= col.minPoint.z - 0.4f && cp.position.z <= col.maxPoint.z + 0.4f) {
                            float dist = (testDirs[d].x > 0.0f) ? (col.minPoint.x - cp.position.x) : (cp.position.x - col.maxPoint.x);
                            if (dist >= -0.25f && dist < minDist) {
                                minDist = std::max(dist, 0.0f);
                                bestNormal = -testDirs[d];
                            }
                        }
                    }
                    else {
                        if (cp.position.y >= col.minPoint.y - 0.4f && cp.position.y <= col.maxPoint.y + 0.4f &&
                            cp.position.x >= col.minPoint.x - 0.4f && cp.position.x <= col.maxPoint.x + 0.4f) {
                            float dist = (testDirs[d].z > 0.0f) ? (col.minPoint.z - cp.position.z) : (cp.position.z - col.maxPoint.z);
                            if (dist >= -0.25f && dist < minDist) {
                                minDist = std::max(dist, 0.0f);
                                bestNormal = -testDirs[d];
                            }
                        }
                    }
                }
            }
            cp.normal = bestNormal;
        }
    }

    void ImportContactsFromFBX(const std::string& extraPath) {
        FILE* fCheck = fopen(extraPath.c_str(), "rb");
        if (!fCheck) return;
        fclose(fCheck);

        Assimp::Importer extraImporter;
        const aiScene* extraScene = extraImporter.ReadFile(extraPath, aiProcess_Triangulate | aiProcess_FlipUVs);
        if (!extraScene || !extraScene->mRootNode) return;

        std::function<void(aiNode*, const glm::mat4&)> parseExtraNode = [&](aiNode* node, const glm::mat4& parentTransform) {
            float scaleFactor = glm::length(glm::vec3(node->mTransformation.a1, node->mTransformation.b1, node->mTransformation.c1));
            float div = (scaleFactor > 50.0f) ? 100.0f : 1.0f;

            glm::mat4 localTransform(1.0f);
            localTransform[0] = glm::vec4(node->mTransformation.a1 / div, node->mTransformation.b1 / div, node->mTransformation.c1 / div, 0.0f);
            localTransform[1] = glm::vec4(node->mTransformation.a2 / div, node->mTransformation.b2 / div, node->mTransformation.c2 / div, 0.0f);
            localTransform[2] = glm::vec4(node->mTransformation.a3 / div, node->mTransformation.b3 / div, node->mTransformation.c3 / div, 0.0f);
            localTransform[3] = glm::vec4(node->mTransformation.a4 / div, node->mTransformation.b4 / div, node->mTransformation.c4 / div, 1.0f);

            glm::mat4 globalTransform = parentTransform * localTransform;
            glm::mat4 rotZtoY = rotateZUpToYUp
                ? glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f))
                : glm::mat4(1.0f);
            glm::mat4 nodeRenderTransform = rotZtoY * globalTransform;

            for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
                aiMesh* mesh = extraScene->mMeshes[node->mMeshes[i]];
                aiMaterial* material = extraScene->mMaterials[mesh->mMaterialIndex];
                aiString matName;
                material->Get(AI_MATKEY_NAME, matName);
                string nombreMaterial = matName.C_Str();

                if (IsContactMaterial(nombreMaterial, node->mName.C_Str())) {
                    if (mesh->mNumVertices > 0) {
                        glm::vec4 p0(mesh->mVertices[0].x, mesh->mVertices[0].y, mesh->mVertices[0].z, 1.0f);
                        glm::vec3 tp0 = glm::vec3(nodeRenderTransform * p0);
                        glm::vec3 minP = tp0;
                        glm::vec3 maxP = tp0;

                        for (unsigned int v = 1; v < mesh->mNumVertices; ++v) {
                            glm::vec4 p(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z, 1.0f);
                            glm::vec3 tp = glm::vec3(nodeRenderTransform * p);
                            minP.x = std::min(minP.x, tp.x);
                            minP.y = std::min(minP.y, tp.y);
                            minP.z = std::min(minP.z, tp.z);
                            maxP.x = std::max(maxP.x, tp.x);
                            maxP.y = std::max(maxP.y, tp.y);
                            maxP.z = std::max(maxP.z, tp.z);
                        }

                        ContactPoint cp;
                        cp.name = node->mName.C_Str();
                        cp.box = AABB(minP, maxP);
                        cp.position = (minP + maxP) * 0.5f;
                        cp.normal = glm::vec3(0.0f, 0.0f, -1.0f);
                        contactPoints.push_back(cp);
                        cout << "  [Contacto Importado de " << extraPath << "] Nodo: \"" << cp.name
                            << "\" en (" << cp.position.x << ", " << cp.position.y << ", " << cp.position.z << ")" << endl;
                    }
                }
            }

            for (unsigned int i = 0; i < node->mNumChildren; ++i) {
                parseExtraNode(node->mChildren[i], globalTransform);
            }
            };

        parseExtraNode(extraScene->mRootNode, glm::mat4(1.0f));
    }

private:
    bool IsCollisionMaterial(const std::string& name) const {
        std::string lower = name;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        return (lower.find("colision") != std::string::npos || lower.find("collision") != std::string::npos);
    }

    bool IsContactMaterial(const std::string& matName, const std::string& nodeName) const {
        std::string lowerMat = matName;
        std::string lowerNode = nodeName;
        std::transform(lowerMat.begin(), lowerMat.end(), lowerMat.begin(), ::tolower);
        std::transform(lowerNode.begin(), lowerNode.end(), lowerNode.begin(), ::tolower);
        return (lowerMat.find("contacto") != std::string::npos || lowerNode.find("contacto") != std::string::npos);
    }

    inline glm::mat4 aiMatrix4x4ToGlm(aiMatrix4x4 from)
    {
        glm::mat4 to;
        to[0][0] = (GLfloat)from.a1; to[0][1] = (GLfloat)from.b1;  to[0][2] = (GLfloat)from.c1; to[0][3] = (GLfloat)from.d1;
        to[1][0] = (GLfloat)from.a2; to[1][1] = (GLfloat)from.b2;  to[1][2] = (GLfloat)from.c2; to[1][3] = (GLfloat)from.d2;
        to[2][0] = (GLfloat)from.a3; to[2][1] = (float)from.b3;  to[2][2] = (GLfloat)from.c3; to[2][3] = (GLfloat)from.d3;
        to[3][0] = (GLfloat)from.a4; to[3][1] = (GLfloat)from.b4;  to[3][2] = (GLfloat)from.c4; to[3][3] = (GLfloat)from.d4;
        return to;
    }

    void loadModel(string const& path)
    {
        scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        filename = path;
        size_t lastSep = path.find_last_of("/\\");
        directory = (lastSep != string::npos) ? path.substr(0, lastSep) : ".";

        aiMatrix4x4 inverseTransform = scene->mRootNode->mTransformation;
        inverseTransform.Inverse();
        m_GlobalInverseTransform = aiMatrix4x4ToGlm(inverseTransform);

        processNode(scene->mRootNode, scene, glm::mat4(1.0f));

        initInteractiveElements();
        ResolveContactNormals();

        cout << "[Model Loaded] " << path << " -> Mallas procesadas: " << meshes.size()
            << " | Cajas de colision: " << cajasColision.size()
            << " | Contactos: " << contactPoints.size() << endl;
    }

    void processNode(aiNode* node, const aiScene* scene, const glm::mat4& parentTransform)
    {
        float scaleFactor = glm::length(glm::vec3(node->mTransformation.a1, node->mTransformation.b1, node->mTransformation.c1));
        float div = (scaleFactor > 50.0f) ? 100.0f : 1.0f;

        glm::mat4 localTransform(1.0f);
        localTransform[0] = glm::vec4(node->mTransformation.a1 / div, node->mTransformation.b1 / div, node->mTransformation.c1 / div, 0.0f);
        localTransform[1] = glm::vec4(node->mTransformation.a2 / div, node->mTransformation.b2 / div, node->mTransformation.c2 / div, 0.0f);
        localTransform[2] = glm::vec4(node->mTransformation.a3 / div, node->mTransformation.b3 / div, node->mTransformation.c3 / div, 0.0f);
        localTransform[3] = glm::vec4(node->mTransformation.a4 / div, node->mTransformation.b4 / div, node->mTransformation.c4 / div, 1.0f);

        glm::mat4 globalTransform = parentTransform * localTransform;

        glm::mat4 rotZtoY = rotateZUpToYUp
            ? glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f))
            : glm::mat4(1.0f);

        glm::mat4 nodeRenderTransform = rotZtoY * globalTransform;

        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            aiString matName;
            material->Get(AI_MATKEY_NAME, matName);
            string nombreMaterial = matName.C_Str();

            if (nombreMaterial != "Mat_colision" && nombreMaterial != "Contactos" && nombreMaterial != "contacto") {
                cout << "  [Mesh Visible] Nodo: \"" << node->mName.C_Str() << "\" | Material: \"" << nombreMaterial << "\"" << endl;
            }

            if (IsCollisionMaterial(nombreMaterial))
            {
                if (mesh->mNumVertices > 0)
                {
                    glm::vec4 p0(mesh->mVertices[0].x, mesh->mVertices[0].y, mesh->mVertices[0].z, 1.0f);
                    glm::vec3 tp0 = glm::vec3(nodeRenderTransform * p0);
                    glm::vec3 minP = tp0;
                    glm::vec3 maxP = tp0;

                    for (unsigned int v = 1; v < mesh->mNumVertices; ++v)
                    {
                        glm::vec4 p(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z, 1.0f);
                        glm::vec3 tp = glm::vec3(nodeRenderTransform * p);

                        minP.x = std::min(minP.x, tp.x);
                        minP.y = std::min(minP.y, tp.y);
                        minP.z = std::min(minP.z, tp.z);

                        maxP.x = std::max(maxP.x, tp.x);
                        maxP.y = std::max(maxP.y, tp.y);
                        maxP.z = std::max(maxP.z, tp.z);
                    }

                    cajasColision.push_back(AABB(minP, maxP));
                }
            }
            else if (IsContactMaterial(nombreMaterial, node->mName.C_Str()))
            {
                if (mesh->mNumVertices > 0)
                {
                    glm::vec4 p0(mesh->mVertices[0].x, mesh->mVertices[0].y, mesh->mVertices[0].z, 1.0f);
                    glm::vec3 tp0 = glm::vec3(nodeRenderTransform * p0);
                    glm::vec3 minP = tp0;
                    glm::vec3 maxP = tp0;

                    for (unsigned int v = 1; v < mesh->mNumVertices; ++v)
                    {
                        glm::vec4 p(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z, 1.0f);
                        glm::vec3 tp = glm::vec3(nodeRenderTransform * p);

                        minP.x = std::min(minP.x, tp.x);
                        minP.y = std::min(minP.y, tp.y);
                        minP.z = std::min(minP.z, tp.z);

                        maxP.x = std::max(maxP.x, tp.x);
                        maxP.y = std::max(maxP.y, tp.y);
                        maxP.z = std::max(maxP.z, tp.z);
                    }

                    ContactPoint cp;
                    cp.name = node->mName.C_Str();
                    cp.box = AABB(minP, maxP);
                    cp.position = (minP + maxP) * 0.5f;
                    cp.normal = glm::vec3(0.0f, 0.0f, -1.0f);
                    contactPoints.push_back(cp);
                    cout << "  [Punto Contacto Detectado] Nodo: \"" << node->mName.C_Str() << "\" en ("
                        << cp.position.x << ", " << cp.position.y << ", " << cp.position.z << ")" << endl;
                }
            }
            else
            {
                meshes.push_back(processMesh(mesh, scene, node->mName.C_Str(), nombreMaterial, nodeRenderTransform));
            }
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene, globalTransform);
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene, const string& nodeName,
        const string& materialName, const glm::mat4& nodeTransform)
    {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

            if (mesh->HasNormals()) {
                vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            }
            else {
                vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
            }

            if (mesh->mTextureCoords[0])
            {
                vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            }
            else
            {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            if (mesh->HasTangentsAndBitangents()) {
                vertex.Tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
                vertex.Bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
            }

            for (unsigned int pb = 0; pb < MAX_NUM_BONES; pb++) {
                vertex.IDs1[pb] = 0.0f;
                vertex.IDs2[pb] = 0.0f;
                vertex.IDs3[pb] = 0.0f;
                vertex.Weights1[pb] = 0.0f;
                vertex.Weights2[pb] = 0.0f;
                vertex.Weights3[pb] = 0.0f;
            }

            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

        if (diffuseMaps.empty())
        {
            string autoTex = "";
            if (materialName == "Pasto" || nodeName.find("Pasto") != string::npos) {
                autoTex = "textures/pasto.png";
            }
            else if (materialName == "Rocas" || nodeName.find("Rocas") != string::npos) {
                autoTex = "textures/rocas.png";
            }
            else if (nodeName == "Techo" || materialName.find("Techo") != string::npos) {
                autoTex = "textures/techo.png";
            }
            else if (materialName == "Piso_Mad" || nodeName.find("Piso") != string::npos) {
                autoTex = "textures/piso_madera.jpg";
            }
            else if (nodeName.find("Puerta") != string::npos || materialName == "Mat_Madera") {
                autoTex = "textures/madera_oscura.jpg";
            }
            else if (nodeName.find("Muros") != string::npos) {
                autoTex = "textures/pared_estuco.jpg";
            }

            if (!autoTex.empty()) {
                Texture tex;
                tex.id = TextureFromFile(autoTex.c_str(), this->directory);
                tex.type = "texture_diffuse";
                tex.path = autoTex;
                textures.push_back(tex);
                diffuseMaps.push_back(tex);
            }
        }

        Material meshMat;
        aiColor4D dColor(0.8f, 0.8f, 0.8f, 1.0f);
        aiColor4D aColor(0.3f, 0.3f, 0.3f, 1.0f);
        aiColor4D sColor(0.2f, 0.2f, 0.2f, 1.0f);
        float opacity = 1.0f;

        if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &dColor)) {
            meshMat.diffuse = glm::vec4(dColor.r, dColor.g, dColor.b, dColor.a);
        }
        if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &aColor)) {
            meshMat.ambient = glm::vec4(aColor.r, aColor.g, aColor.b, aColor.a);
        }
        if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &sColor)) {
            meshMat.specular = glm::vec4(sColor.r, sColor.g, sColor.b, sColor.a);
        }
        if (AI_SUCCESS == aiGetMaterialFloat(material, AI_MATKEY_OPACITY, &opacity)) {
            meshMat.transparency = opacity;
        }

        // Materiales de los muebles de Romero: se reconocen por el nombre del material (aunque lleven textura)
        // para que el shader los trate como metal, vidrio/agua o plastico/ceramica y no como opacos.
        string nombreMin = materialName;
        std::transform(nombreMin.begin(), nombreMin.end(), nombreMin.begin(), ::tolower);
        bool conTextura = diffuseMaps.size() > 0;
        if (nombreMin.find("acero") != string::npos || nombreMin.find("laton") != string::npos || nombreMin.find("espejo") != string::npos) {
            // Metalico: brillo muy nitido + reflejo del cubemap (espejo y manijas mas pulidos que el acero)
            bool pulido = (nombreMin.find("manija") != string::npos || nombreMin.find("laton") != string::npos || nombreMin.find("espejo") != string::npos);
            meshMat.diffuse = conTextura ? glm::vec4(1.0f) : glm::vec4(glm::vec3(meshMat.diffuse) * 0.6f, 1.0f);
            meshMat.ambient = glm::vec4(0.15f, 0.15f, 0.16f, 1.0f);
            meshMat.specular = glm::vec4(0.90f, 0.90f, 0.92f, 1.0f);
            meshMat.transparency = 1.0f;
            meshMat.type = MAT_METALICO;
            meshMat.shininess = pulido ? 128.0f : 64.0f;
        }
        else if (nombreMin.find("agua") != string::npos || nombreMin.find("vidrio") != string::npos || nombreMin.find("cristal") != string::npos) {
            // Traslucido: transparencia + Fresnel con el entorno
            meshMat.diffuse = glm::vec4(0.70f, 0.85f, 0.95f, 0.35f);
            meshMat.ambient = glm::vec4(0.30f, 0.40f, 0.50f, 0.35f);
            meshMat.specular = glm::vec4(0.95f, 0.95f, 0.95f, 1.0f);
            meshMat.transparency = 0.35f;
            meshMat.type = MAT_TRASLUCIDO;
            meshMat.shininess = 128.0f;
        }
        else if (nombreMin.find("ceramica") != string::npos || nombreMin.find("cer\xC3\xA1mica") != string::npos ||
                 nombreMin.find("fruta") != string::npos || nombreMin.find("plastico") != string::npos) {
            // Plastico / ceramica vidriada: color propio y brillo blanco concentrado
            meshMat.diffuse = conTextura ? glm::vec4(1.0f) : meshMat.diffuse;
            meshMat.ambient = glm::vec4(0.35f, 0.35f, 0.35f, 1.0f);
            meshMat.specular = glm::vec4(0.90f, 0.90f, 0.90f, 1.0f);
            meshMat.transparency = 1.0f;
            meshMat.type = MAT_PLASTICO;
            meshMat.shininess = 80.0f;
        }
        else if (diffuseMaps.size() > 0) {
            meshMat.diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            meshMat.ambient = glm::vec4(0.35f, 0.35f, 0.35f, 1.0f);
            meshMat.specular = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
            meshMat.transparency = 1.0f;
            meshMat.type = MAT_OPACO;
            meshMat.shininess = 24.0f;
        }
        else if (nodeName.find("Muros") != string::npos) {
            meshMat.diffuse = glm::vec4(0.86f, 0.84f, 0.80f, 1.0f);
            meshMat.ambient = glm::vec4(0.35f, 0.33f, 0.30f, 1.0f);
            meshMat.specular = glm::vec4(0.12f, 0.12f, 0.12f, 1.0f);
            meshMat.transparency = 1.0f;
            meshMat.type = MAT_OPACO;
            meshMat.shininess = 16.0f;
        }
        else if (nodeName.find("Plano.00") != string::npos) {
            meshMat.diffuse = glm::vec4(0.92f, 0.92f, 0.90f, 1.0f);
            meshMat.ambient = glm::vec4(0.35f, 0.35f, 0.35f, 1.0f);
            meshMat.specular = glm::vec4(0.10f, 0.10f, 0.10f, 1.0f);
            meshMat.transparency = 1.0f;
            meshMat.type = MAT_OPACO;
            meshMat.shininess = 16.0f;
        }
        else if (nodeName.find("Ventana") != string::npos || materialName == "Mat_Vidrio" || materialName == "Vidrio") {
            meshMat.diffuse = glm::vec4(0.70f, 0.85f, 0.95f, 0.35f);
            meshMat.ambient = glm::vec4(0.30f, 0.40f, 0.50f, 0.35f);
            meshMat.specular = glm::vec4(0.95f, 0.95f, 0.95f, 1.0f);
            meshMat.transparency = 0.35f;
            meshMat.type = MAT_TRASLUCIDO;
            meshMat.shininess = 128.0f;
        }
        else if (materialName == "Metal_negro" || nodeName.find("Manija") != string::npos) {
            meshMat.diffuse = glm::vec4(0.20f, 0.20f, 0.22f, 1.0f);
            meshMat.ambient = glm::vec4(0.15f, 0.15f, 0.16f, 1.0f);
            meshMat.specular = glm::vec4(0.90f, 0.90f, 0.92f, 1.0f);
            meshMat.transparency = 1.0f;
            meshMat.type = MAT_METALICO;
            meshMat.shininess = 128.0f;
        }
        else if (materialName == "Mat_Marco") {
            meshMat.diffuse = glm::vec4(0.28f, 0.28f, 0.30f, 1.0f);
            meshMat.ambient = glm::vec4(0.18f, 0.18f, 0.20f, 1.0f);
            meshMat.specular = glm::vec4(0.50f, 0.50f, 0.50f, 1.0f);
            meshMat.transparency = 1.0f;
            meshMat.type = MAT_METALICO;
            meshMat.shininess = 64.0f;
        }
        else if (materialName == "Regadera_Verde" || nodeName.find("Regadera") != string::npos) {
            meshMat.diffuse = glm::vec4(0.20f, 0.70f, 0.30f, 1.0f);
            meshMat.ambient = glm::vec4(0.15f, 0.35f, 0.18f, 1.0f);
            meshMat.specular = glm::vec4(0.85f, 0.85f, 0.85f, 1.0f);
            meshMat.transparency = 1.0f;
            meshMat.type = MAT_PLASTICO;
            meshMat.shininess = 96.0f;
        }
        else {
            meshMat.diffuse = glm::vec4(0.85f, 0.82f, 0.78f, 1.0f);
            meshMat.ambient = glm::vec4(0.35f, 0.33f, 0.30f, 1.0f);
            meshMat.specular = glm::vec4(0.20f, 0.20f, 0.20f, 1.0f);
            meshMat.transparency = 1.0f;
            meshMat.type = MAT_OPACO;
            meshMat.shininess = 32.0f;
        }

        return Mesh(vertices, indices, textures, meshMat, materialName,
            mesh->mName.C_Str(), nodeName, nodeTransform, false);
    }

    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip)
            {
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }
        }
        return textures;
    }

    unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false)
    {
        string p = string(path);
        for (char& c : p) { if (c == '\\') c = '/'; }

        size_t lastSlash = p.find_last_of('/');
        string baseName = (lastSlash != string::npos) ? p.substr(lastSlash + 1) : p;

        vector<string> candidates;
        candidates.push_back(p);
        candidates.push_back(baseName);
        if (!directory.empty()) {
            candidates.push_back(directory + "/" + p);
            candidates.push_back(directory + "/" + baseName);
        }
        candidates.push_back("Casa_coliciones2.fbm/" + baseName);
        candidates.push_back("../Casa_coliciones2.fbm/" + baseName);
        candidates.push_back("textures/" + baseName);
        candidates.push_back("../textures/" + baseName);

        unsigned char* data = nullptr;
        int width = 0, height = 0, nrComponents = 0;
        string loadedPath = "";

        for (const auto& candidate : candidates) {
            data = stbi_load(candidate.c_str(), &width, &height, &nrComponents, 0);
            if (data) {
                loadedPath = candidate;
                break;
            }
        }

        unsigned int textureID;
        glGenTextures(1, &textureID);

        if (data)
        {
            cout << "  [Textura Cargada] " << loadedPath << " (" << width << "x" << height << ", " << nrComponents << " canales)" << endl;
            GLenum format = GL_RGB;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, textureID);
            unsigned char white[4] = { 255, 255, 255, 255 };
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        return textureID;
    }

    void ReadNodeHierarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform)
    {
        if (pNode == nullptr || scene == nullptr || scene->mNumAnimations == 0) return;
    }
};

#endif