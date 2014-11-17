#include "cinder/app/App.h"

#include "Avatar.h"

using namespace ci;

Avatar::Avatar( const fs::path &modelPath )
{
	mAssimpLoader = mndl::assimp::AssimpLoader::create( modelPath );
	mAssimpLoader->enableSkinning();

	collectJoints();
}

void Avatar::collectJoints()
{
	for ( size_t i = 0; i < Joints::TOTAL_JOINTS; i++ )
	{
		const std::string &jointName = sJointNames[ i ];
		auto node = mAssimpLoader->getAssimpNode( jointName );
		mJoints[ i ] = node;
		if ( ! node )
		{
			app::console() << "Warning: joint not found for name " << jointName << std::endl;
		}
	}
}

void Avatar::update()
{
	mAssimpLoader->update();
}

void Avatar::draw()
{
	mAssimpLoader->draw();
}

void Avatar::setPosition( size_t frameId, size_t jointId, const Vec3f &position )
{
	if ( jointId >= Joints::TOTAL_JOINTS )
	{
		return;
	}

	auto node = mJoints[ jointId ];
	if ( node )
	{
		node->setPosition( position );
	}
}

void Avatar::setOrientation( size_t frameId, size_t jointId, const ci::Vec3f &eulerDegrees )
{
	if ( jointId >= Joints::TOTAL_JOINTS )
	{
		return;
	}
	auto node = mJoints[ jointId ];
	if ( ! node )
	{
		return;
	}

	// BVH rotation order is ZXY
	Matrix33f rotation = Matrix33f::createRotation( Vec3f::zAxis(), toRadians( eulerDegrees.z ) );
	rotation.rotate( Vec3f::xAxis(), toRadians( eulerDegrees.x ) );
	rotation.rotate( Vec3f::yAxis(), toRadians( eulerDegrees.y ) );

	Quatf quat( rotation );
	node->setOrientation( quat );
}

std::string Avatar::sJointNames[ Joints::TOTAL_JOINTS ] =
{
	"Hip", "LowerSpine", "MiddleSpine", "Chest", "Neck", "Head", "HeadEnd",
	"LClavicle", "LShoulder", "LForearm", "LHand", "LFinger1", "LFinger11",
	"LFinger12", "LFinger12End", "LFinger2", "LFinger21", "LFinger22",
	"LFinger22End", "LFinger3", "LFinger31", "LFinger32", "LFinger32End",
	"LFinger4", "LFinger41", "LFinger42", "LFinger42End", "LFinger0",
	"LFinger01", "LFinger02", "LFinger02End", "RClavicle", "RShoulder",
	"RForearm", "RHand", "RFinger1", "RFinger11", "RFinger12", "RFinger12End",
	"RFinger2", "RFinger21", "RFinger22", "RFinger22End", "RFinger3",
	"RFinger31", "RFinger32", "RFinger32End", "RFinger4", "RFinger41",
	"RFinger42", "RFinger42End", "RFinger0", "RFinger01", "RFinger02",
	"RFinger02End", "RThigh", "RShin", "RFoot", "RToe", "RToeEnd", "LThigh",
	"LShin", "LFoot", "LToe", "LToeEnd"
};
