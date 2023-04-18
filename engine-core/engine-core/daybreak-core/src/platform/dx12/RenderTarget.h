#pragma once

#include "Texture.h"

namespace dx12 {

	enum DAYBREAK_API AttachmentPoint {
		COLOR_0,
		COLOR_1,
		COLOR_2,
		COLOR_3,
		COLOR_4,
		COLOR_5,
		COLOR_6,
		COLOR_7,
		DEPTH_STENCIL,
		NUM_ATTACHMENT_POINTS
	};

	class DAYBREAK_API RenderTarget {
	    public:
            RenderTarget();
            RenderTarget(const RenderTarget& copy) = default;
            RenderTarget(RenderTarget&& copy) = default;

            RenderTarget& operator=(const RenderTarget& other) = default;
            RenderTarget& operator=(RenderTarget&& other) = default;

            void AttachTexture(AttachmentPoint attachmentPoint, const Texture& texture);
            const Texture& GetTexture(AttachmentPoint attachmentPoint) const;

            void Resize(uint32_t width, uint32_t height);
            void Release();

            // Get a list of the textures attached to the render target.
            // This method is primarily used by the CommandList when binding the
            // render target to the output merger stage of the rendering pipeline.
            const std::vector<Texture>& GetTextures() const;

            // Get the render target formats of the textures currently 
            // attached to this render target object.
            // This is needed to configure the Pipeline state object.
            D3D12_RT_FORMAT_ARRAY GetRenderTargetFormats() const;

            // Get the format of the attached depth/stencil buffer.
            DXGI_FORMAT GetDepthStencilFormat() const;


	    private:
		    std::vector<Texture> m_textures;
	};
}
