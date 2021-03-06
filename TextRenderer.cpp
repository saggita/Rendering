/*
	This file is part of the Rendering library.
	Copyright (C) 2013 Benjamin Eikel <benjamin@eikel.org>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "TextRenderer.h"
#include "Mesh/Mesh.h"
#include "Mesh/VertexDescription.h"
#include "MeshUtils/MeshBuilder.h"
#include "RenderingContext/ParameterStructs.h"
#include "RenderingContext/RenderingContext.h"
#include "Shader/Shader.h"
#include "Shader/Uniform.h"
#include "Texture/Texture.h"
#include "Texture/TextureUtils.h"
#include "Draw.h"
#include <Geometry/Rect.h>
#include <Geometry/Vec2.h>
#include <Util/Graphics/Color.h>
#include <Util/Graphics/FontRenderer.h>
#include <Util/References.h>
#include <cstdint>
#include <string>

namespace Rendering {

static const std::string vertexProgram(R"***(#version 130

uniform mat4 sg_modelViewProjectionMatrix;

in vec2 sg_Position;
in vec2 sg_TexCoord0;
out vec2 glyphPos;

void main(void) {
	glyphPos = sg_TexCoord0;
	gl_Position = (sg_modelViewProjectionMatrix * vec4(sg_Position, 0.0, 1.0));
}
)***");

static const std::string fragmentProgram(R"***(#version 130

uniform sampler2D sg_texture0;
uniform vec4 textColor;

in vec2 glyphPos;
out vec4 fragColor;

void main(void) {
	fragColor = vec4(1.0, 1.0, 1.0, texelFetch(sg_texture0, ivec2(glyphPos), 0).r) * textColor;
}
)***");

struct TextRenderer::Implementation {
	Util::Reference<Shader> shader;
	Util::Reference<Texture> texture;
	Util::FontInfo fontInfo;
};

TextRenderer::TextRenderer(const Util::Bitmap & glyphBitmap, 
						   const Util::FontInfo & fontInfo) :
		impl(new Implementation) {
	impl->texture = TextureUtils::createTextureFromBitmap(glyphBitmap, false, false);
	impl->fontInfo = fontInfo;
}

TextRenderer::~TextRenderer() = default;

TextRenderer::TextRenderer(const TextRenderer & other) :
		impl(new Implementation(*other.impl.get())) {
}

TextRenderer::TextRenderer(TextRenderer &&) = default;

void TextRenderer::draw(RenderingContext & context,
						const std::u32string & text,
						const Geometry::Vec2i & textPosition,
						const Util::Color4f & textColor) const {
	if(impl->shader.isNull()) {
		impl->shader = Shader::createShader(vertexProgram, fragmentProgram, Shader::USE_UNIFORMS);
	}
	context.pushAndSetBlending(BlendingParameters(BlendingParameters::SRC_ALPHA, BlendingParameters::ONE_MINUS_SRC_ALPHA));
	context.pushAndSetDepthBuffer(DepthBufferParameters(false, false, Comparison::LESS));
	context.pushAndSetShader(impl->shader.get());
	context.pushAndSetTexture(0, impl->texture.get());

	enable2DMode(context);

	impl->shader->setUniform(context, Uniform("textColor", textColor));

	VertexDescription vertexDescription;
	vertexDescription.appendPosition2D();
	vertexDescription.appendTexCoord();
	MeshUtils::MeshBuilder builder(vertexDescription);

	const auto textureHeight = static_cast<int32_t>(impl->texture->getHeight());

	int cursorX = textPosition.getX();
	for(const auto & character : text) {
		const auto it = impl->fontInfo.glyphMap.find(character);
		if(it == impl->fontInfo.glyphMap.cend()) {
			// Skip missing characters
			continue;
		}
		const auto & glyphInfo = it->second;

		const auto glyphWidth = glyphInfo.size.first;
		const auto glyphHeight = glyphInfo.size.second;

		const Geometry::Vec2i topLeftPos(cursorX + glyphInfo.offset.first,
										 textPosition.getY() + impl->fontInfo.ascender - glyphInfo.offset.second);
		const Geometry::Vec2i topLeftTexCoord(glyphInfo.position.first,
											  textureHeight - glyphInfo.position.second);

		// Top left
		builder.position(topLeftPos);
		builder.texCoord0(topLeftTexCoord);
		const auto indexA = builder.addVertex();
		// Bottom left
		builder.position(topLeftPos + Geometry::Vec2i(0, glyphHeight));
		builder.texCoord0(topLeftTexCoord + Geometry::Vec2i(0, -glyphHeight));
		const auto indexB = builder.addVertex();
		// Bottom right
		builder.position(topLeftPos + Geometry::Vec2i(glyphWidth, glyphHeight));
		builder.texCoord0(topLeftTexCoord + Geometry::Vec2i(glyphWidth, -glyphHeight));
		const auto indexC = builder.addVertex();
		// Top right
		builder.position(topLeftPos + Geometry::Vec2i(glyphWidth, 0));
		builder.texCoord0(topLeftTexCoord + Geometry::Vec2i(glyphWidth, 0));
		const auto indexD = builder.addVertex();

		builder.addQuad(indexA, indexB, indexC, indexD);

		cursorX += glyphInfo.xAdvance;
	}
	Util::Reference<Mesh> mesh = builder.buildMesh();

	context.displayMesh(mesh.get());

	disable2DMode(context);

	context.popTexture(0);
	context.popShader();
	context.popDepthBuffer();
	context.popBlending();
}

Geometry::Rect_i TextRenderer::getTextSize(const std::u32string & text) const {
	Geometry::Rect_i textRect;
	textRect.invalidate();

	int cursorX = 0;
	for(const auto & character : text) {
		const auto it = impl->fontInfo.glyphMap.find(character);
		if(it == impl->fontInfo.glyphMap.cend()) {
			// Skip missing characters
			continue;
		}
		const auto & glyphInfo = it->second;

		const auto glyphWidth = glyphInfo.size.first;
		const auto glyphHeight = glyphInfo.size.second;
		
		const Geometry::Vec2i topLeftPos(cursorX + glyphInfo.offset.first,
										 impl->fontInfo.ascender - glyphInfo.offset.second);

		// Top left
		textRect.include(topLeftPos);
		// Bottom right
		textRect.include(topLeftPos + Geometry::Vec2i(glyphWidth, glyphHeight));

		cursorX += glyphInfo.xAdvance;
	}

	return textRect;
}

int TextRenderer::getHeightOfX() const {
	const auto it = impl->fontInfo.glyphMap.find('x');
	if(it == impl->fontInfo.glyphMap.cend()) {
		return 0;
	}
	const auto & glyphInfo = it->second;
	return glyphInfo.size.second;
}

int TextRenderer::getWidthOfM() const {
	const auto it = impl->fontInfo.glyphMap.find('M');
	if(it == impl->fontInfo.glyphMap.cend()) {
		return 0;
	}
	const auto & glyphInfo = it->second;
	return glyphInfo.size.first;
}

}
