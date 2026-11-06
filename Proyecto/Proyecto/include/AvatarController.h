#ifndef AVATAR_CONTROLLER_H
#define AVATAR_CONTROLLER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <iostream>
#include "shader_m.h"
#include "model.h"

enum AvatarAnimState {
    AVATAR_IDLE = 0,
    AVATAR_WALK = 1,
    AVATAR_WATER = 2
};

struct VertexWeightInfo {
    unsigned int boneIndex;
    float weight;
};

struct MeshSkinningData {
    std::string meshName;
    std::string nodeName;
    std::vector<Vertex> baseVertices;
    std::vector<std::vector<VertexWeightInfo>> vertexWeights;
    bool hasSkinning;
};

class AvatarController {
public:
    AvatarAnimState currentState;
    float animTime;
    bool isWatering;
    float waterActionTimer;

    Assimp::Importer importer;
    const aiScene* charScene;

    const aiAnimation* animWalk;
    const aiAnimation* animWater;
    const aiAnimation* animIdle;

    struct BoneData {
        std::string name;
        aiMatrix4x4 offsetMatrix;
    };
    std::vector<BoneData> bones;
    std::map<std::string, unsigned int> boneMapping;

    std::vector<MeshSkinningData> meshDataList;
    std::map<std::string, glm::mat4> globalNodeMats;

    glm::mat4 mGlobalInverse;

    AvatarController()
        : currentState(AVATAR_IDLE), animTime(0.0f), isWatering(false),
        waterActionTimer(0.0f), charScene(nullptr),
        animWalk(nullptr), animWater(nullptr), animIdle(nullptr),
        mGlobalInverse(1.0f) {
    }

    static inline glm::mat4 aiToGlm(const aiMatrix4x4& from)
    {
        glm::mat4 to;
        to[0][0] = (float)from.a1; to[0][1] = (float)from.b1; to[0][2] = (float)from.c1; to[0][3] = (float)from.d1;
        to[1][0] = (float)from.a2; to[1][1] = (float)from.b2; to[1][2] = (float)from.c2; to[1][3] = (float)from.d2;
        to[2][0] = (float)from.a3; to[2][1] = (float)from.b3; to[2][2] = (float)from.c3; to[2][3] = (float)from.d3;
        to[3][0] = (float)from.a4; to[3][1] = (float)from.b4; to[3][2] = (float)from.c4; to[3][3] = (float)from.d4;
        return to;
    }

    bool Init(const std::string& charPath)
    {
        std::string path = charPath;
        FILE* fCheck = fopen(path.c_str(), "rb");
        if (fCheck) { fclose(fCheck); }
        else { path = "../" + path; }

        charScene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
        if (!charScene || !charScene->mRootNode) {
            std::cout << "[AvatarController ERROR] No se pudo cargar: " << path << std::endl;
            return false;
        }

        aiMatrix4x4 globalInverse = charScene->mRootNode->mTransformation;
        globalInverse.Inverse();
        mGlobalInverse = aiToGlm(globalInverse);

        for (unsigned int a = 0; a < charScene->mNumAnimations; ++a) {
            std::string aName = charScene->mAnimations[a]->mName.C_Str();
            if (aName == "Esqueleto|Caminar" || aName.find("Caminar") != std::string::npos) animWalk = charScene->mAnimations[a];
            else if (aName == "Esqueleto|Regar" || aName.find("Regar") != std::string::npos) animWater = charScene->mAnimations[a];
            else if (aName == "Esqueleto|Reposo" || aName.find("Reposo") != std::string::npos) animIdle = charScene->mAnimations[a];
        }

        if (!animWalk && charScene->mNumAnimations > 0) animWalk = charScene->mAnimations[0];
        if (!animWater && charScene->mNumAnimations > 1) animWater = charScene->mAnimations[1];
        if (!animIdle && charScene->mNumAnimations > 2) animIdle = charScene->mAnimations[2];

        meshDataList.clear();
        bones.clear();
        boneMapping.clear();

        for (unsigned int m = 0; m < charScene->mNumMeshes; ++m) {
            aiMesh* mesh = charScene->mMeshes[m];
            MeshSkinningData msd;
            msd.meshName = mesh->mName.C_Str();
            msd.hasSkinning = (mesh->mNumBones > 0);

            msd.baseVertices.resize(mesh->mNumVertices);
            for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
                Vertex vert;
                vert.Position = glm::vec3(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
                vert.Normal = mesh->HasNormals() ? glm::vec3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z) : glm::vec3(0, 1, 0);
                vert.TexCoords = mesh->mTextureCoords[0] ? glm::vec2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y) : glm::vec2(0, 0);
                msd.baseVertices[v] = vert;
            }

            msd.vertexWeights.resize(mesh->mNumVertices);
            for (unsigned int b = 0; b < mesh->mNumBones; ++b) {
                aiBone* bone = mesh->mBones[b];
                std::string bName = bone->mName.C_Str();
                unsigned int bIdx = 0;
                if (boneMapping.find(bName) == boneMapping.end()) {
                    bIdx = (unsigned int)bones.size();
                    boneMapping[bName] = bIdx;
                    BoneData bd;
                    bd.name = bName;
                    bd.offsetMatrix = bone->mOffsetMatrix;
                    bones.push_back(bd);
                }
                else {
                    bIdx = boneMapping[bName];
                }

                for (unsigned int w = 0; w < bone->mNumWeights; ++w) {
                    unsigned int vId = bone->mWeights[w].mVertexId;
                    float weight = bone->mWeights[w].mWeight;
                    if (vId < msd.vertexWeights.size()) {
                        msd.vertexWeights[vId].push_back({ bIdx, weight });
                    }
                }
            }
            meshDataList.push_back(msd);
        }

        std::cout << "[AvatarController] Modelo cargado con " << meshDataList.size()
            << " mallas y " << bones.size() << " huesos en esqueleto." << std::endl;
        return true;
    }

    void TriggerWatering() {
        isWatering = true;
        waterActionTimer = 0.0f;
        currentState = AVATAR_WATER;
    }

    static const aiNodeAnim* FindNodeAnim(const aiAnimation* anim, const std::string& nodeName)
    {
        if (!anim) return nullptr;
        for (unsigned int c = 0; c < anim->mNumChannels; ++c) {
            if (anim->mChannels[c]->mNodeName.C_Str() == nodeName)
                return anim->mChannels[c];
        }
        return nullptr;
    }

    // CORRECCIÓN MAGISTRAL: Recibir la transformación base original del hueso para no perder su longitud ni escala.
    static aiMatrix4x4 InterpolateNodeTransform(const aiNodeAnim* na, float time, const aiMatrix4x4& baseTrans)
    {
        aiVector3D pos;
        aiQuaternion rot;
        aiVector3D scl;

        // ¡Extraer valores por defecto de la pose original del modelo!
        baseTrans.Decompose(scl, rot, pos);

        if (na->mNumPositionKeys > 0) {
            pos = na->mPositionKeys[0].mValue;
            for (unsigned int k = 0; k < na->mNumPositionKeys - 1; ++k) {
                if (time < (float)na->mPositionKeys[k + 1].mTime) {
                    float factor = (time - (float)na->mPositionKeys[k].mTime) / (float)(na->mPositionKeys[k + 1].mTime - na->mPositionKeys[k].mTime);
                    pos = na->mPositionKeys[k].mValue + factor * (na->mPositionKeys[k + 1].mValue - na->mPositionKeys[k].mValue);
                    break;
                }
            }
        }

        if (na->mNumRotationKeys > 0) {
            rot = na->mRotationKeys[0].mValue;
            for (unsigned int k = 0; k < na->mNumRotationKeys - 1; ++k) {
                if (time < (float)na->mRotationKeys[k + 1].mTime) {
                    float factor = (time - (float)na->mRotationKeys[k].mTime) / (float)(na->mRotationKeys[k + 1].mTime - na->mRotationKeys[k].mTime);
                    aiQuaternion::Interpolate(rot, na->mRotationKeys[k].mValue, na->mRotationKeys[k + 1].mValue, factor);
                    rot = rot.Normalize();
                    break;
                }
            }
        }

        if (na->mNumScalingKeys > 0) {
            scl = na->mScalingKeys[0].mValue;
            for (unsigned int k = 0; k < na->mNumScalingKeys - 1; ++k) {
                if (time < (float)na->mScalingKeys[k + 1].mTime) {
                    float factor = (time - (float)na->mScalingKeys[k].mTime) / (float)(na->mScalingKeys[k + 1].mTime - na->mScalingKeys[k].mTime);
                    scl = na->mScalingKeys[k].mValue + factor * (na->mScalingKeys[k + 1].mValue - na->mScalingKeys[k].mValue);
                    break;
                }
            }
        }

        aiMatrix4x4 mRot(rot.GetMatrix());
        aiMatrix4x4 mPos;
        aiMatrix4x4::Translation(pos, mPos);
        aiMatrix4x4 mScl;
        aiMatrix4x4::Scaling(scl, mScl);

        return mPos * mRot * mScl;
    }

    void EvaluateHierarchy(aiNode* node, const aiMatrix4x4& parentTransform, const aiAnimation* anim, float time, std::map<std::string, aiMatrix4x4>& outMats)
    {
        std::string nName = node->mName.C_Str();
        aiMatrix4x4 localTransform = node->mTransformation;
        const aiNodeAnim* na = FindNodeAnim(anim, nName);
        if (na) {
            // CORRECCIÓN: Pasar localTransform para no perder la longitud de los huesos que no tienen anim_keys de posicion
            localTransform = InterpolateNodeTransform(na, time, localTransform);
        }
        aiMatrix4x4 globalTransform = parentTransform * localTransform;
        outMats[nName] = globalTransform;

        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            EvaluateHierarchy(node->mChildren[i], globalTransform, anim, time, outMats);
        }
    }

    void Update(float deltaTime, bool isMoving, Model* characterModel)
    {
        if (!charScene || !characterModel) return;

        const aiAnimation* activeAnim = animIdle;

        // Evitar división por 0. Assimp guarda mTicksPerSecond (usualmente 24 o 30)
        float ticksPerSecond = 24.0f;

        if (isWatering) {
            currentState = AVATAR_WATER;
            activeAnim = animWater ? animWater : animIdle;
            if (activeAnim && activeAnim->mTicksPerSecond > 0) ticksPerSecond = (float)activeAnim->mTicksPerSecond;

            waterActionTimer += deltaTime;
            float durationInSecs = activeAnim ? (activeAnim->mDuration / ticksPerSecond) : 2.2f;

            if (waterActionTimer >= durationInSecs) {
                isWatering = false;
                waterActionTimer = 0.0f;
                currentState = isMoving ? AVATAR_WALK : AVATAR_IDLE;
            }
        }
        else if (isMoving) {
            currentState = AVATAR_WALK;
            activeAnim = animWalk ? animWalk : animIdle;
        }
        else {
            currentState = AVATAR_IDLE;
            activeAnim = animIdle ? animIdle : animWalk;
        }

        if (!activeAnim) return;

        if (activeAnim->mTicksPerSecond > 0) ticksPerSecond = (float)activeAnim->mTicksPerSecond;

        animTime += deltaTime * ticksPerSecond;
        if (activeAnim->mDuration > 0.0) {
            animTime = std::fmod(animTime, (float)activeAnim->mDuration);
        }

        std::map<std::string, aiMatrix4x4> globalAiMats;
        aiMatrix4x4 ident;
        EvaluateHierarchy(charScene->mRootNode, ident, activeAnim, animTime, globalAiMats);

        for (const auto& pair : globalAiMats) {
            globalNodeMats[pair.first] = aiToGlm(pair.second);
        }

        std::vector<glm::mat4> boneFinalMats(bones.size());
        for (size_t b = 0; b < bones.size(); ++b) {
            const std::string& bName = bones[b].name;
            if (globalAiMats.find(bName) != globalAiMats.end()) {
                boneFinalMats[b] = mGlobalInverse * aiToGlm(globalAiMats[bName]) * aiToGlm(bones[b].offsetMatrix);
            }
            else {
                boneFinalMats[b] = aiToGlm(bones[b].offsetMatrix);
            }
        }

        std::vector<glm::mat4> glmBoneMats(bones.size());
        std::vector<glm::mat3> glmBoneNormMats(bones.size());
        for (size_t b = 0; b < bones.size(); ++b) {
            glmBoneMats[b] = boneFinalMats[b];
            glmBoneNormMats[b] = glm::mat3(glmBoneMats[b]);
        }

        for (size_t m = 0; m < characterModel->meshes.size(); ++m) {
            auto& mesh = characterModel->meshes[m];

            // CORRECCIÓN: Anclaje de objetos rígidos (Gorra y Regadera) usando la matriz Skinning correcta
            if (mesh.nodeName == "Gorra" || mesh.materialName == "Gorra_Azul") {
                if (boneMapping.find("Cabeza") != boneMapping.end()) {
                    mesh.nodeTransform = glmBoneMats[boneMapping["Cabeza"]];
                }
                continue;
            }
            if (mesh.nodeName == "Regadera" || mesh.materialName == "Regadera_Verde") {
                if (boneMapping.find("Mano.R") != boneMapping.end()) {
                    mesh.nodeTransform = glmBoneMats[boneMapping["Mano.R"]];
                }
                continue;
            }

            if (m < meshDataList.size() && meshDataList[m].hasSkinning) {
                const auto& msd = meshDataList[m];
                std::vector<Vertex> deformedVertices = msd.baseVertices;

                for (size_t v = 0; v < deformedVertices.size(); ++v) {
                    const auto& weights = msd.vertexWeights[v];
                    if (weights.empty()) continue;

                    glm::vec4 rawP(msd.baseVertices[v].Position, 1.0f);
                    glm::vec3 rawN = msd.baseVertices[v].Normal;
                    glm::vec3 animP(0.0f);
                    glm::vec3 animN(0.0f);

                    for (const auto& w : weights) {
                        if (w.boneIndex < glmBoneMats.size()) {
                            animP += w.weight * glm::vec3(glmBoneMats[w.boneIndex] * rawP);
                            animN += w.weight * (glmBoneNormMats[w.boneIndex] * rawN);
                        }
                    }

                    deformedVertices[v].Position = animP;
                    float lenN = glm::length(animN);
                    deformedVertices[v].Normal = (lenN > 0.0001f) ? (animN / lenN) : rawN;
                }

                mesh.UpdateSkinningVBO(deformedVertices);
                mesh.nodeTransform = glm::mat4(1.0f);
            }
        }
    }

    void Draw(Shader& shader, Model* characterModel, const glm::vec3& feetPosition, float yaw, bool showRegadera = false)
    {
        if (!characterModel) return;

        glm::mat4 baseMat = glm::translate(glm::mat4(1.0f), feetPosition);
        baseMat = glm::rotate(baseMat, glm::radians(-yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        baseMat = glm::scale(baseMat, glm::vec3(1.0f));

        for (size_t i = 0; i < characterModel->meshes.size(); ++i) {
            auto& m = characterModel->meshes[i];

            if ((m.nodeName == "Regadera" || m.materialName == "Regadera_Verde") && !showRegadera) {
                continue;
            }

            glm::mat4 finalModel = baseMat * m.nodeTransform;
            shader.setMat4("model", finalModel);
            m.Draw(shader);
        }
    }
};

#endif