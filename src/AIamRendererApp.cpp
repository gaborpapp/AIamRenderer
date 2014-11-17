#include "cinder/Camera.h"
#include "cinder/Cinder.h"
#include "cinder/MayaCamUI.h"
#include "cinder/app/App.h"
#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

#include "Avatar.h"
#include "Config.h"
#include "OscServer.h"
#include "ParamsUtils.h"

using namespace ci;
using namespace ci::app;

class AIamRendereApp : public AppNative
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

void AIamRendereApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1152, 648 );
}

void AIamRendereApp::setup()
{
	mConfig = mndl::Config::create();

	disableFrameRate();

	setupParams();
	setupOsc();

	mAvatar = Avatar::create( getAssetPath( "model/avatar.dae" ) );

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

void AIamRendereApp::setupParams()
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
}

void AIamRendereApp::setupOsc()
{
	mListener = mndl::osc::Server( 10000 );
	mListener.registerOscReceived( &AIamRendereApp::translationReceived, this, "/translation", "iifff" );
	mListener.registerOscReceived( &AIamRendereApp::orientationReceived, this, "/orientation", "iifff" );
}

void AIamRendereApp::update()
{
	mFps = getAverageFps();

	mAvatar->update();
}

void AIamRendereApp::draw()
{
	gl::setViewport( getWindowBounds() );
	gl::setMatrices( mCamera );
	gl::clear();

	gl::enableDepthRead();
	gl::enableDepthWrite();

	mAvatar->draw();

	mParams->draw();
}

bool AIamRendereApp::orientationReceived( const mndl::osc::Message &message )
{
	app::console() << message << std::endl;
	return false;
}

bool AIamRendereApp::translationReceived( const mndl::osc::Message &message )
{
	app::console() << message << std::endl;
	return false;
}

void AIamRendereApp::mouseDown( MouseEvent event )
{
	mMayaCam.setCurrentCam( mCamera );
	mMayaCam.mouseDown( event.getPos() );

	mCamera = mMayaCam.getCamera();
	mCameraEyePoint = mCamera.getEyePoint();
	mCameraCenterOfInterestPoint = mCamera.getCenterOfInterestPoint();
	mCameraOrientation = mCamera.getOrientation();
}

void AIamRendereApp::mouseDrag( MouseEvent event )
{
	mMayaCam.setCurrentCam( mCamera );
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );

	mCamera = mMayaCam.getCamera();
	mCameraEyePoint = mCamera.getEyePoint();
	mCameraCenterOfInterestPoint = mCamera.getCenterOfInterestPoint();
	mCameraOrientation = mCamera.getOrientation();
}

void AIamRendereApp::resize()
{
	mCamera.setAspectRatio( getWindowAspectRatio() );
	mMayaCam.setCurrentCam( mCamera );
}

void AIamRendereApp::keyDown( KeyEvent event )
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

void AIamRendereApp::shutdown()
{
	fs::path configPath = app::getAssetPath( "" ) / "config.xml";
	mndl::params::writeParamsLayout();
	mConfig->write( writeFile( configPath ) );
}

CINDER_APP_BASIC( AIamRendereApp, RendererGl )

