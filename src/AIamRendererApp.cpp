#include "cinder/Camera.h"
#include "cinder/Cinder.h"
#include "cinder/MayaCamUI.h"
#include "cinder/app/App.h"
#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

#include "Config.h"
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
	mConfig->addVar( "Camera/EyePoint", &mCameraEyePoint, Vec3f( 0.0f, -55.0f, 0.0f ) );
	mConfig->addVar( "Camera/CenterOfInterestPoint", &mCameraCenterOfInterestPoint, Vec3f( 0.0f, 0.0f, -3.0f ) );
	mConfig->addVar( "Camera/Orientation", &mCameraOrientation, Quatf::identity() );

	mParams->addButton( "Reset camera",
			[ & ]()
			{
				mCameraCenterOfInterestPoint = Vec3f( 0.0f, 0.0f, -3.0f );
				mCameraFov = 45.f;
				mCameraEyePoint = Vec3f( 0.0f, -55.0f, 0.0f );
				mCameraOrientation = Quatf::identity();

				mCamera.setPerspective( mCameraFov, getWindowAspectRatio(), 0.1f, 10000.0f );
				mCamera.setEyePoint( mCameraEyePoint );
				mCamera.setCenterOfInterestPoint( mCameraCenterOfInterestPoint );
				mCamera.setOrientation( mCameraOrientation );
			} );
	mParams->addSeparator();
}

void AIamRendereApp::update()
{
	mFps = getAverageFps();
}

void AIamRendereApp::draw()
{
	gl::setViewport( getWindowBounds() );
	gl::setMatricesWindow( getWindowSize() );
	gl::clear();

	gl::enableDepthRead();
	gl::enableDepthWrite();

	gl::color( Color::white() );

	mParams->draw();
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

