#include "Asset.h"

std::string GetAssetPath(std::string path)
{
	std::string assetPath;

	if (const char* rootDir = getenv("SG_ASSET_DIRECTORY"))
	{
		assetPath.append(rootDir);
	}

    assetPath.append(path);

	return assetPath;
}
