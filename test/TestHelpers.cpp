#include "TestHelpers.h"

std::string GetAssetPath(const char* path)
{
	std::string assetPath;

	if (const char* rootDir = getenv("SG_ASSET_DIRECTORY"))
	{
		assetPath.append(rootDir);
	}

    assetPath.append(path);

	return assetPath;
}