#include "Avatar.h"

using namespace ci;

Avatar::Avatar( const fs::path &modelPath )
{
	mAssimpLoader = mndl::assimp::AssimpLoader::create( modelPath );
}
