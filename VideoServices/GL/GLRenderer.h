#pragma once
#include <vector>
#include <array>

#include "Camera.h"
#include "SokolState.h"
#include "../Sprite.h"
#include "SDL3/SDL.h"
#include "../../ObjFacTools/ResBitmap.h"
#include "../../GameSDL/VideoPalette.h"
struct Vertex
{
    glm::i32vec3 position;
    glm::vec2 texcoord;
};

struct VertexWithTextureId
{
    Vertex vertex;
    int textureIdx;
};

struct WallVertex
{
    VertexWithTextureId vertex;
    int rotationSpeed;
    int rotationLength;
    int segment;
};

template <typename T>
struct VerticesData
{
    std::vector<T> vertices;
    std::vector<uint16_t> indices;
};

inline Vertex makeVertex(int32_t x, int32_t y, int32_t z, float u, float v)
{
    return Vertex{
        .position = glm::i32vec3(x, y, z),
        .texcoord = glm::vec2(u, v),
    };
}

inline VertexWithTextureId makeVertexWithTextureId(int32_t x, int32_t y, int32_t z, float u, float v,
                                                   int textureIdx)
{
    return VertexWithTextureId{
        .vertex = makeVertex(x, y, z, u, v),
        .textureIdx = textureIdx
    };
}

inline WallVertex makeWallVertex(int32_t x, int32_t y, int32_t z, float u, float v,
                                                   uint32_t textureIdx, int rotationSpeed = 0, int rotationLength = 0, int segment = 0)
{
    return WallVertex {
        .vertex = makeVertexWithTextureId(x, y, z, u, v, textureIdx),
        .rotationSpeed = rotationSpeed,
        .rotationLength = rotationLength,
        .segment = segment
    };
}

inline glm::i32vec3 SwapYZ(glm::i32vec3 vec)
{
    // -z means ahead, which was +y in the old coordinate system
    return glm::i32vec3(vec.x, vec.z, -vec.y);
}

inline Vertex SwapYZ(Vertex vertex)
{
    vertex.position = SwapYZ(vertex.position);
    return vertex;
}

inline VertexWithTextureId SwapYZ(VertexWithTextureId vertex)
{
    vertex.vertex = SwapYZ(vertex.vertex);
    return vertex;
}

inline WallVertex SwapYZ(WallVertex vertex)
{
    vertex.vertex = SwapYZ(vertex.vertex);
    return vertex;
}

struct AtlasCoords
{
    float u1, v1; // Top-left
    float u2, v2; // Bottom-right
};

struct MipmapLevel
{
    int width;
    int height;
    uint32_t* pixels;
};

template<size_t NumLevels>
struct TextureData
{
    MR_UInt32 id;   // high bit for 'b' animation frame
    AtlasCoords atlas_coords;
    std::array<MipmapLevel, NumLevels> levels;
};


struct FreeElementVertex
{
    VertexWithTextureId vertex;
    int sequence;
    int frame;
    int is_variant_texture;
};

struct FreeElementInstance
{
    glm::i32vec3 position;
    int type;
    int orientation;
    int sequence;
    int frame;
    int variant;

    bool operator==(const FreeElementInstance& other) const = default;
};

class GLRenderer
{
    static constexpr int ATLAS_PADDING = 1;  // Base padding around each texture (scaled by mipmap level)

    std::vector<TextureData<6>> floor_textures;
    std::vector<TextureData<1>> wall_textures;
    std::vector<TextureData<1>> free_element_textures;
    std::vector<TextureData<1>> sprites;
    std::unordered_map<int, std::vector<FreeElementInstance>> freeElementInstances;
    VideoPalette* videoPalette;
    uint32_t* ConvertTextureToRGBA8(const MR_ResBitmap* bitmap, uint8_t alpha = 0xFF, int mipmapLevel = 0);
    uint32_t* ConvertBackgroundToRGBA8(const MR_UInt8* backImage);
    uint32_t* ConvertSpriteToRGBA8(const MR_Sprite* sprite);
    template<size_t NumLevels>
    unsigned long LoadTextureInternal(std::vector<TextureData<NumLevels>>& collection, MR_UInt32 id, const MR_ResBitmap* bitmap, uint8_t alpha = 0xFF);
    template<size_t NumLevels>
    void PadMipmapLevelsWithUpscaling(TextureData<NumLevels>& textureData, int sourceLevel, int destStartLevel);
    void CopyTextureToAtlasWithPadding(uint32_t* atlas_pixels, int atlas_width,
                                        const MipmapLevel& level, int rect_x, int rect_y, int mipLevel, int padding);
    template<size_t N, size_t M>
    std::tuple<sg_image, std::array<glm::vec4, N>> BindTexturesInternal(std::vector<TextureData<M>>& collection);
    float CalculateFontScale(int height);


public:
    GLRenderer(SDL_Window* glWindow, SDL_GLContext glContext, VideoPalette* pVideoPalette);
    ~GLRenderer();
    Sokol_State state{};
    SDL_Window* glWindow;
    SDL_GLContext glContext;

    void BindFloorVertices(const VerticesData<VertexWithTextureId>& vertices);
    void BindWaterVertices(const VerticesData<VertexWithTextureId>& vertices);
    void BindWallVertices(const VerticesData<WallVertex>& vertices);
    void BindFreeElementVertices(const std::unordered_map<int, VerticesData<FreeElementVertex>>& freeElements);
    void BindFreeElementInstances(const std::unordered_map<int, std::vector<FreeElementInstance>>& updatedFreeElementInstances);
    unsigned long LoadFloorTexture(MR_UInt32 id, const MR_ResBitmap* bitmap, uint8_t alpha = 0xFF);
    unsigned long LoadWallTexture(MR_UInt32 id, const MR_ResBitmap* bitmap);
    unsigned long LoadFreeElementTexture(MR_UInt32 id, const MR_ResBitmap* bitmap);
    unsigned long LoadSprite(MR_UInt32 id, const MR_Sprite* sprite);
    unsigned long GetSpriteAtlasIndex(MR_UInt32 id) const;
    void BindFloorTextures();
    void BindWallTextures();
    void BindFreeElementTextures();
    void BindSpriteTextures();
    void BindBackgroundVertices(const VerticesData<Vertex>& vertices);
    void BindBackgroundTexture(const MR_UInt8* backImage);
    void MakeGLContextCurrent() const;
    void BeginRender() const;
    void EndRender() const;
    void RenderMiniMap(glm::ivec4 size);
    void BeginImguiFrame() const;
    void EndImguiFrame() const;
    void ChangeResolution(int width, int height);
};
