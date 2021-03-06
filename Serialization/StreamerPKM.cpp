/*
	This file is part of the Rendering library.
	Copyright (C) 2007-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2007-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2007-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "StreamerPKM.h"
#include "../Texture/Texture.h"
#include "../GLHeader.h"
#include <algorithm>
#include <cstdint>

namespace Rendering {

const char * const StreamerPKM::fileExtension = "pkm";

inline static uint16_t convertBigEndianTwoBytes(const uint8_t bytes[2]) {
	uint16_t number = 0;
	number |= bytes[0];
	number <<= 8;
	number |= bytes[1];
	return number;
}

Texture * StreamerPKM::loadTexture(std::istream & input) {
	struct PKMHeader {
		uint8_t magic[4];
		uint8_t version[2];
		uint8_t textureType[2];
		uint8_t width[2];
		uint8_t height[2];
		uint8_t activeWidth[2];
		uint8_t activeHeight[2];
	};

	PKMHeader header;
	input.read(reinterpret_cast<char *>(&header), sizeof(PKMHeader));

	if(!std::equal(header.magic, header.magic + 4, "PKM ")) {
		WARN("Invalid magic found in PKM header.");
		return nullptr;
	}
	if(!std::equal(header.version, header.version + 2, "10")) {
		WARN("Invalid version found in PKM header.");
		return nullptr;
	}
	if(header.textureType[0] != 0 || header.textureType[1] != 0) {
		WARN("Unsupported texture type found in PKM header.");
		return nullptr;
	}
	// Numbers are stored big-endian.
	uint16_t width = convertBigEndianTwoBytes(header.width);
	uint16_t height = convertBigEndianTwoBytes(header.height);
	uint16_t activeWidth = convertBigEndianTwoBytes(header.activeWidth);
	uint16_t activeHeight = convertBigEndianTwoBytes(header.activeHeight);


	Texture::Format format;
	format.width = activeWidth;
	format.height = activeHeight;
	format.border = 0;
	format.glTextureType = GL_TEXTURE_2D;
#ifdef LIB_GLESv2
	format.glInternalFormat = GL_ETC1_RGB8_OES;
#endif /* LIB_GLESv2 */
	format.compressed = true;
	format.imageSize = 8 * ((width + 3) >> 2) * ((height + 3) >> 2);

	Util::Reference<Texture> texture = new Texture(format);
	texture->allocateLocalData();
	input.read(reinterpret_cast<char *>(texture->getLocalData()), format.imageSize);

	return texture.detachAndDecrease();
}

uint8_t StreamerPKM::queryCapabilities(const std::string & extension) {
	if(extension == fileExtension) {
		return CAP_LOAD_TEXTURE;
	} else {
		return 0;
	}
}

}
