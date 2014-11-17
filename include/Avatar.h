#pragma once

#include <memory>

#include "cinder/Filesystem.h"

#include "AssimpLoader.h"

typedef std::shared_ptr< class Avatar > AvatarRef;

class Avatar
{
 public:
	static AvatarRef create( const ci::fs::path &modelPath )
	{ return AvatarRef( new Avatar( modelPath ) ); }

	void update();
	void draw();

 protected:
	Avatar( const ci::fs::path &modelPath );

	mndl::assimp::AssimpLoaderRef mAssimpLoader;
};
