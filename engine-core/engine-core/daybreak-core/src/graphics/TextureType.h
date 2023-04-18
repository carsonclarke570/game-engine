#pragma once

namespace gfx {
    
    enum class DAYBREAK_API TextureType
    {
        ALBEDO,
        DIFFUSE = ALBEDO,       // Treat Diffuse and Albedo textures the same.
        HEIGHT_MAP,
        DEPTH = HEIGHT_MAP,      // Treat height and depth textures the same.
        NORMAL_MAP,
        RENDER_TARGET,           // Texture is used as a render target.
    };
}


