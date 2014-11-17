#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "cinder/Color.h"
#include "cinder/Quaternion.h"
#include "cinder/Xml.h"

namespace mndl
{

typedef std::shared_ptr< class Config > ConfigRef;

class Config
{
 public:
	static ConfigRef create() { return ConfigRef( new Config() ); }

	template< typename T, typename TVAL >
	void addVar( const std::string &name, T *var, const TVAL &defVal )
	{
		*var = (T)defVal;
		mConfigReadCallbacks.push_back( [ = ] ( ci::XmlTree &xml )
				{ Config::readVar( var, defVal, name, xml ); } );
		mConfigWriteCallbacks.push_back( [ = ] ( ci::XmlTree &xml )
				{ Config::writeVar( var, name, xml ); } );
	}

	void read( const ci::DataSourceRef &source );
	void write( const ci::DataTargetRef &target );

 protected:
	Config() {}

	template< typename T, typename TVAL >
	void readVar( T *var, TVAL defVal, const std::string &name, ci::XmlTree &xml )
	{
		*var = xml.hasChild( name ) ?
			   xml.getChild( name ).getAttributeValue( "value", defVal ) :
			   (T)defVal;
	}

	template< typename T >
	void writeVar( T *var, const std::string &name, ci::XmlTree &xml )
	{
		addChild( name, xml );
		ci::XmlTree &node = xml.getChild( name );
		node.setAttribute( "value", *var );
	}

	void readVar( ci::ColorA *var, const ci::ColorA &defVal,
				  const std::string &name, ci::XmlTree &xml );
	void writeVar( ci::ColorA *var, const std::string &name, ci::XmlTree &xml );

	void readVar( ci::Color *var, const ci::Color &defVal,
				  const std::string &name, ci::XmlTree &xml );
	void writeVar( ci::Color *var, const std::string &name, ci::XmlTree &xml );

	template< typename T >
	void readVar( ci::Vec2< T > *var, const ci::Vec2< T > &defVal,
					const std::string &name, ci::XmlTree &xml )
	{
		if ( xml.hasChild( name ) )
		{
			var->x = xml.getChild( name ).getAttributeValue( "x", defVal.x );
			var->y = xml.getChild( name ).getAttributeValue( "y", defVal.y );
		}
		else
		{
			*var = defVal;
		}
	}

	template< typename T >
	void writeVar( ci::Vec2< T > *var, const std::string &name, ci::XmlTree &xml )
	{
		addChild( name, xml );
		ci::XmlTree &node = xml.getChild( name );
		node.setAttribute( "x", var->x );
		node.setAttribute( "y", var->y );
	}

	template< typename T >
	void readVar( ci::Vec3< T > *var, const ci::Vec3< T > &defVal,
					const std::string &name, ci::XmlTree &xml )
	{
		if ( xml.hasChild( name ) )
		{
			var->x = xml.getChild( name ).getAttributeValue( "x", defVal.x );
			var->y = xml.getChild( name ).getAttributeValue( "y", defVal.y );
			var->z = xml.getChild( name ).getAttributeValue( "z", defVal.z );
		}
		else
		{
			*var = defVal;
		}
	}

	template< typename T >
	void writeVar( ci::Vec3< T > *var, const std::string &name, ci::XmlTree &xml )
	{
		addChild( name, xml );
		ci::XmlTree &node = xml.getChild( name );
		node.setAttribute( "x", var->x );
		node.setAttribute( "y", var->y );
		node.setAttribute( "z", var->z );
	}

	template< typename T >
	void readVar( ci::Quaternion< T > *var, const ci::Quaternion< T > &defVal,
					const std::string &name, ci::XmlTree &xml )
	{
		if ( xml.hasChild( name ) )
		{
			var->v.x = xml.getChild( name ).getAttributeValue( "x", defVal.v.x );
			var->v.y = xml.getChild( name ).getAttributeValue( "y", defVal.v.y );
			var->v.z = xml.getChild( name ).getAttributeValue( "z", defVal.v.z );
			var->w = xml.getChild( name ).getAttributeValue( "w", defVal.w );
		}
		else
		{
			*var = defVal;
		}
	}

	template< typename T >
	void writeVar( ci::Quaternion< T > *var, const std::string &name, ci::XmlTree &xml )
	{
		addChild( name, xml );
		ci::XmlTree &node = xml.getChild( name );
		node.setAttribute( "x", var->v.x );
		node.setAttribute( "y", var->v.y );
		node.setAttribute( "z", var->v.z );
		node.setAttribute( "w", var->w );
	}

	std::string colorToHex( const ci::ColorA &color );
	ci::ColorA hexToColor( const std::string &hexStr );

	void addChild( const std::string &name, ci::XmlTree &xml );

	std::vector< std::function< void ( ci::XmlTree & ) > > mConfigReadCallbacks;
	std::vector< std::function< void ( ci::XmlTree & ) > > mConfigWriteCallbacks;
};

};

