#pragma once

#include <memory>
#include <string>

#include "cinder/Filesystem.h"
#include "cinder/Vector.h"

#include "AssimpLoader.h"

typedef std::shared_ptr< class Avatar > AvatarRef;

class Avatar
{
 public:
	static AvatarRef create( const ci::fs::path &modelPath )
	{ return AvatarRef( new Avatar( modelPath ) ); }

	void update();
	void draw();

	void setPosition( size_t frameId, size_t jointId, const ci::Vec3f &position );
	void setOrientation( size_t frameId, size_t jointId, const ci::Vec3f &eulerDegrees );

 protected:
	Avatar( const ci::fs::path &modelPath );

	void collectJoints();

	mndl::assimp::AssimpLoaderRef mAssimpLoader;

	enum Joints
	{
		HIP = 0,
		LOWER_SPINE,
		MIDDLE_SPINE,
		CHEST,
		NECK,
		HEAD,
		HEAD_END,
		LCLAVICLE,
		LSHOULDER,
		LFOREARM,
		LHAND,
		LFINGER1,
		LFINGER11,
		LFINGER12,
		LFINGER12_END,
		LFINGER2,
		LFINGER21,
		LFINGER22,
		LFINGER22_END,
		LFINGER3,
		LFINGER31,
		LFINGER32,
		LFINGER32_END,
		LFINGER4,
		LFINGER41,
		LFINGER42,
		LFINGER42_END,
		LFINGER0,
		LFINGER01,
		LFINGER02,
		LFINGER02_END,
		RCLAVICLE,
		RSHOULDER,
		RFOREARM,
		RHAND,
		RFINGER1,
		RFINGER11,
		RFINGER12,
		RFINGER12_END,
		RFINGER2,
		RFINGER21,
		RFINGER22,
		RFINGER22_END,
		RFINGER3,
		RFINGER31,
		RFINGER32,
		RFINGER32_END,
		RFINGER4,
		RFINGER41,
		RFINGER42,
		RFINGER42_END,
		RFINGER0,
		RFINGER01,
		RFINGER02,
		RFINGER02_END,
		RTHIGH,
		RSHIN,
		RFOOT,
		RTOE,
		RTOE_END,
		LTHIGH,
		LSHIN,
		LFOOT,
		LTOE,
		LTOE_END,
		TOTAL_JOINTS
	};

	mndl::assimp::AssimpNodeRef mJoints[ Joints::TOTAL_JOINTS ];

	static std::string sJointNames[ Joints::TOTAL_JOINTS ];
};
