#include "daybreak.h"

#include "RenderTarget.h"

namespace dx12 {

	RenderTarget::RenderTarget() :
		m_textures(AttachmentPoint::NUM_ATTACHMENT_POINTS) {}

	void RenderTarget::AttachTexture(AttachmentPoint attachmentPoint, const Texture& texture) {
		m_textures[attachmentPoint] = texture;
	}

	const Texture& RenderTarget::GetTexture(AttachmentPoint attachmentPoint) const {
		return m_textures[attachmentPoint];
	}

	void RenderTarget::Resize(uint32_t width, uint32_t height) {
		for (auto& texture : m_textures) {
			texture.Resize(width, height);
		}
	}

	void RenderTarget::Release() {
		for (int i = AttachmentPoint::COLOR_0; i <= AttachmentPoint::COLOR_7; i++) {
			AttachTexture(AttachmentPoint(i), Texture());
		}
	}

	const std::vector<Texture>& RenderTarget::GetTextures() const {
		return m_textures;
	}

	D3D12_RT_FORMAT_ARRAY RenderTarget::GetRenderTargetFormats() const {
		D3D12_RT_FORMAT_ARRAY rtvFormats = {};
		for (int i = AttachmentPoint::COLOR_0; i <= AttachmentPoint::COLOR_7; i++) {
			const Texture& texture = m_textures[i];
			if (texture.IsValid()) {
				rtvFormats.RTFormats[rtvFormats.NumRenderTargets++] = texture.ResourceDesc().Format;
			}
		}

		return rtvFormats;
	}

	DXGI_FORMAT RenderTarget::GetDepthStencilFormat() const {
		DXGI_FORMAT dsvFormat = DXGI_FORMAT_UNKNOWN;
		const Texture& depthStencilTexture = m_textures[AttachmentPoint::DEPTH_STENCIL];
		if (depthStencilTexture.IsValid()) {
			dsvFormat = depthStencilTexture.ResourceDesc().Format;
		}

		return dsvFormat;
	}
}