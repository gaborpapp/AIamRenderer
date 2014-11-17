#include <vector>

#include "cinder/Camera.h"
#include "cinder/Cinder.h"
#include "cinder/MayaCamUI.h"
#include "cinder/TriMesh.h"
#include "cinder/app/App.h"
#include "cinder/app/AppNative.h"
#include "cinder/gl/DisplayList.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

#include "Avatar.h"
#include "Config.h"
#include "OscServer.h"
#include "ParamsUtils.h"

using namespace ci;
using namespace ci::app;

class AIamRendererApp : public AppNative
{
 public:
	void prepareSettings( Settings *settings );
	void setup();
	void shutdown();

	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void resize();
	void keyDown( KeyEvent event );

	void update();
	void draw();

 private:
	params::InterfaceGlRef mParams;

	void setupParams();

	float mFps;
	bool mVerticalSyncEnabled = false;
	bool mDebugDrawOrigin = false;

	TriMesh createSquare( const Vec2i &resolution );
	TriMesh mTriMeshPlane;
	void createGrid();
	gl::DisplayList mGrid;

	static const int PLANE_SIZE = 1024;

	bool mDrawPlane;
	bool mDrawGrid;
	int mGridSize;

	CameraPersp mCamera;
	MayaCamUI mMayaCam;

	float mCameraFov;
	Vec3f mCameraEyePoint;
	Vec3f mCameraCenterOfInterestPoint;
	Quatf mCameraOrientation;

	mndl::ConfigRef mConfig;

	void setupOsc();

	mndl::osc::Server mListener;
	std::mutex mOscMutex;

	bool orientationReceived( const mndl::osc::Message &message );
	bool translationReceived( const mndl::osc::Message &message );

	AvatarRef mAvatar;
};

void AIamRendererApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1152, 648 );
}

void AIamRendererApp::setup()
{
	mConfig = mndl::Config::create();

	disableFrameRate();

	setupParams();

	mAvatar = Avatar::create( getAssetPath( "model/avatar_90.dae" ) );

	setupOsc();

	createGrid();
    mTriMeshPlane = createSquare( Vec2i( 64, 64 ) );

	mndl::params::addParamsLayoutVars( mConfig );

	fs::path configPath = app::getAssetPath( "" ) / "config.xml";
	if ( fs::exists( configPath ) )
	{
		mConfig->read( loadFile( configPath ) );
		mndl::params::readParamsLayout();
	}

	gl::enableVerticalSync( mVerticalSyncEnabled );
	mCamera.setPerspective( mCameraFov, getWindowAspectRatio(), 0.1f, 10000.0f );
	mCamera.setEyePoint( mCameraEyePoint );
	mCamera.setCenterOfInterestPoint( mCameraCenterOfInterestPoint );
	mCamera.setOrientation( mCameraOrientation );
}

void AIamRendererApp::setupParams()
{
	mParams = params::InterfaceGl::create( "Parameters", Vec2i( 250, 400 ) );
	mParams->addParam( "Fps", &mFps, true );
	mParams->addParam( "Vertical sync", &mVerticalSyncEnabled ).updateFn(
			[ & ]() { gl::enableVerticalSync( mVerticalSyncEnabled ); } );
	mParams->addSeparator();

	mConfig->addVar( "Options/VSync", &mVerticalSyncEnabled, true );

	mParams->addText( "Camera" );
	mParams->addParam( "Fov", &mCameraFov ).min( 20.0f ).max( 179.0f ).step( 0.1f ).updateFn(
		[ & ]()
		{
			mCamera.setPerspective( mCameraFov, getWindowAspectRatio(), 0.1f, 10000.0f );
		} );
	mParams->addParam( "Eye", &mCameraEyePoint, true );
	mParams->addParam( "Center of Interest", &mCameraCenterOfInterestPoint, true );
	mParams->addParam( "Orientationt", &mCameraOrientation, true );

	mConfig->addVar( "Camera/Fov", &mCameraFov, 45.0f );
	mConfig->addVar( "Camera/EyePoint", &mCameraEyePoint, Vec3f( 0.0f, 0.0f, 500.0f ) );
	mConfig->addVar( "Camera/CenterOfInterestPoint", &mCameraCenterOfInterestPoint, Vec3f::zero() );
	mConfig->addVar( "Camera/Orientation", &mCameraOrientation, Quatf( -1.0f, 0.0f, 0.0f, 0.0f ) );

	mParams->addButton( "Reset camera",
			[ & ]()
			{
				mCameraCenterOfInterestPoint = Vec3f::zero();
				mCameraFov = 45.f;
				mCameraEyePoint = Vec3f( 0.0f, 0.0f, 500.0f );
				mCameraOrientation = Quatf( -1.0f, 0.0f, 0.0f, 0.0f );

				mCamera.setPerspective( mCameraFov, getWindowAspectRatio(), 0.1f, 10000.0f );
				mCamera.setEyePoint( mCameraEyePoint );
				mCamera.setCenterOfInterestPoint( mCameraCenterOfInterestPoint );
				mCamera.setOrientation( mCameraOrientation );
			} );
	mParams->addSeparator();

	mParams->addText( "Debug" );
	mParams->addParam( "Draw origin", &mDebugDrawOrigin );
	mParams->addParam( "Draw plane", &mDrawPlane );
	mParams->addParam( "Draw grid", &mDrawGrid );
	mParams->addParam( "Grid size", &mGridSize ).min( 1 ).max( 512 ).updateFn(
			std::bind( &AIamRendererApp::createGrid, this ) );

	mConfig->addVar( "Debug/DrawPlane", &mDrawPlane, true );
	mConfig->addVar( "Debug/DrawGrid", &mDrawGrid, true );
	mConfig->addVar( "Debug/GridSize", &mGridSize, 50 );

	mParams->addSeparator();
}

void AIamRendererApp::setupOsc()
{
	mListener = mndl::osc::Server( 10000 );
	mListener.registerOscReceived( &AIamRendererApp::translationReceived, this, "/translation", "iifff" );
	mListener.registerOscReceived( &AIamRendererApp::orientationReceived, this, "/orientation", "iifff" );
}

void AIamRendererApp::update()
{
	mFps = getAverageFps();

	mAvatar->update();
}

void AIamRendererApp::draw()
{
	gl::setViewport( getWindowBounds() );
	gl::setMatrices( mCamera );
	gl::clear();

	gl::enableDepthRead();
	gl::enableDepthWrite();

	if ( mDebugDrawOrigin )
	{
		gl::drawCoordinateFrame( 20.0f );
	}

	glPolygonOffset( 1.0f, 1.0f );
	gl::enable( GL_POLYGON_OFFSET_FILL );

	mAvatar->draw();

	if ( mDrawPlane )
	{
		gl::pushModelView();
		gl::color( Color::gray( 0.1f ) );
		gl::scale( Vec3f( PLANE_SIZE, 1.0f, PLANE_SIZE ) );
		gl::draw( mTriMeshPlane );
		gl::popModelView();
	}
	gl::disable( GL_POLYGON_OFFSET_FILL );

	if ( mDrawGrid && mGrid )
	{
		gl::pushModelView();
		gl::color( Color::white() );
		mGrid.draw();
		gl::popModelView();
	}

	mParams->draw();
}

bool AIamRendererApp::orientationReceived( const mndl::osc::Message &message )
{
	//app::console() << message << std::endl;
	int frameId = message.getArg< int >( 0 );
	int jointId = message.getArg< int >( 1 );

	Vec3f eulerAngles;
	eulerAngles.x = message.getArg< float >( 2 );
	eulerAngles.y = message.getArg< float >( 3 );
	eulerAngles.z = message.getArg< float >( 4 );

	mAvatar->setOrientation( frameId, jointId, eulerAngles );
	return false;
}

bool AIamRendererApp::translationReceived( const mndl::osc::Message &message )
{
	//app::console() << message << std::endl;
	int frameId = message.getArg< int >( 0 );
	int jointId = message.getArg< int >( 1 );

	Vec3f p;
	p.x = message.getArg< float >( 2 );
	p.y = message.getArg< float >( 3 );
	p.z = message.getArg< float >( 4 );

	mAvatar->setPosition( frameId, jointId, p );

	return false;
}

// based on Cinder-MeshHelper by Ban the Rewind
// https://github.com/BanTheRewind/Cinder-MeshHelper/
TriMesh AIamRendererApp::createSquare( const Vec2i &resolution )
{
	std::vector< uint32_t > indices;
	std::vector< Vec3f > normals;
	std::vector< Vec3f > positions;
	std::vector< Vec2f > texCoords;

	Vec3f norm0( 0.0f, 1.0f, 0.0f );

	Vec2f scale( 1.0f / math< float >::max( (float)resolution.x, 1.0f ),
				 1.0f / math<float>::max( (float)resolution.y, 1.0f ) );
	uint32_t index = 0;
	for ( int32_t y = 0; y < resolution.y; ++y )
	{
		for ( int32_t x = 0; x < resolution.x; ++x, ++index )
		{
			float x1 = (float)x * scale.x;
			float y1 = (float)y * scale.y;
			float x2 = (float)( x + 1 ) * scale.x;
			float y2 = (float)( y + 1 ) * scale.y;

			Vec3f pos0( x1 - 0.5f, 0.0f, y1 - 0.5f );
			Vec3f pos1( x2 - 0.5f, 0.0f, y1 - 0.5f );
			Vec3f pos2( x1 - 0.5f, 0.0f, y2 - 0.5f );
			Vec3f pos3( x2 - 0.5f, 0.0f, y2 - 0.5f );

			Vec2f texCoord0( x1, y1 );
			Vec2f texCoord1( x2, y1 );
			Vec2f texCoord2( x1, y2 );
			Vec2f texCoord3( x2, y2 );

			positions.push_back( pos2 );
			positions.push_back( pos1 );
			positions.push_back( pos0 );
			positions.push_back( pos1 );
			positions.push_back( pos2 );
			positions.push_back( pos3 );

			texCoords.push_back( texCoord2 );
			texCoords.push_back( texCoord1 );
			texCoords.push_back( texCoord0 );
			texCoords.push_back( texCoord1 );
			texCoords.push_back( texCoord2 );
			texCoords.push_back( texCoord3 );

			for ( uint32_t i = 0; i < 6; ++i )
			{
				indices.push_back( index * 6 + i );
				normals.push_back( norm0 );
			}
		}
	}

	TriMesh mesh;

	mesh.appendIndices( &indices[ 0 ], indices.size() );
	for ( const auto & normal : normals )
	{
		mesh.appendNormal( normal );
	}

	mesh.appendVertices( &positions[ 0 ], positions.size() );

	for ( const Vec2f texCoord : texCoords )
	{
		mesh.appendTexCoord( texCoord );
	}

	return mesh;
}

void AIamRendererApp::createGrid()
{
	mGrid = gl::DisplayList( GL_COMPILE );
	mGrid.newList();
	gl::color( Color::black() );
	int n = PLANE_SIZE / mGridSize;
	Vec3f step( mGridSize, 0, 0 );
	Vec3f p( 0, 0, -PLANE_SIZE * .5f );
	p -= step * n / 2;
	for ( int i = 0; i < n; i++ )
	{
		gl::drawLine( p, p + Vec3f( 0, 0, PLANE_SIZE ) );
		p += step;
	}
	step = Vec3f( 0, 0, mGridSize );
	p = Vec3f( -PLANE_SIZE * .5f, 0, 0 );
	p -= step * n / 2;
	for ( int i = 0; i < n; i++ )
	{
		gl::drawLine( p, p + Vec3f( PLANE_SIZE, 0, 0 ) );
		p += step;
	}
	mGrid.endList();
}


void AIamRendererApp::mouseDown( MouseEvent event )
{
	mMayaCam.setCurrentCam( mCamera );
	mMayaCam.mouseDown( event.getPos() );

	mCamera = mMayaCam.getCamera();
	mCameraEyePoint = mCamera.getEyePoint();
	mCameraCenterOfInterestPoint = mCamera.getCenterOfInterestPoint();
	mCameraOrientation = mCamera.getOrientation();
}

void AIamRendererApp::mouseDrag( MouseEvent event )
{
	mMayaCam.setCurrentCam( mCamera );
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );

	mCamera = mMayaCam.getCamera();
	mCameraEyePoint = mCamera.getEyePoint();
	mCameraCenterOfInterestPoint = mCamera.getCenterOfInterestPoint();
	mCameraOrientation = mCamera.getOrientation();
}

void AIamRendererApp::resize()
{
	mCamera.setAspectRatio( getWindowAspectRatio() );
	mMayaCam.setCurrentCam( mCamera );
}

void AIamRendererApp::keyDown( KeyEvent event )
{
	switch ( event.getCode() )
	{
		case KeyEvent::KEY_f:
		{
			setFullScreen( ! isFullScreen() );
			if ( isFullScreen() )
			{
				if ( mParams->isVisible() )
				{
					showCursor();
				}
				else
				{
					hideCursor();
				}
			}
			else
			{
				showCursor();
			}
			break;
		}

		case KeyEvent::KEY_s:
			mndl::params::showAllParams( ! mParams->isVisible() );
			if ( isFullScreen() )
			{
				if ( mParams->isVisible() )
				{
					showCursor();
				}
				else
				{
					hideCursor();
				}
			}
			break;

		case KeyEvent::KEY_ESCAPE:
			quit();
			break;

		default:
			break;
	}
}

void AIamRendererApp::shutdown()
{
	fs::path configPath = app::getAssetPath( "" ) / "config.xml";
	mndl::params::writeParamsLayout();
	mConfig->write( writeFile( configPath ) );
}

CINDER_APP_BASIC( AIamRendererApp, RendererGl )

