#include "cinder/Utilities.h"

#include "Config.h"

namespace mndl
{

void Config::read( const ci::DataSourceRef &source )
{
	ci::XmlTree doc = ci::XmlTree( source );
	for ( auto f : mConfigReadCallbacks )
	{
		f( doc );
	}
}

void Config::write( const ci::DataTargetRef &target )
{
	ci::XmlTree doc = ci::XmlTree::createDoc();
	for ( auto f : mConfigWriteCallbacks )
	{
		f( doc );
	}
	doc.write( target );
}

void Config::addChild( const std::string &name, ci::XmlTree &xml )
{
	std::vector< std::string > tokens = ci::split( name, "/" );
	std::string parentId = "";
	for ( const std::string &token : tokens )
	{
		if ( ! xml.hasChild( parentId + "/" + token ) )
		{
			if ( parentId == "" )
				xml.push_back( ci::XmlTree( token, "" ) );
			else
				xml.getChild( parentId ).push_back( ci::XmlTree( token, "" ) );
		}
		parentId += "/" + token;
	}
}

std::string Config::colorToHex( const ci::ColorA &color )
{
	uint32_t a = ( static_cast< uint32_t >( color.a * 255 ) & 0xff ) << 24;
	uint32_t r = ( static_cast< uint32_t >( color.r * 255 ) & 0xff ) << 16;
	uint32_t g = ( static_cast< uint32_t >( color.g * 255 ) & 0xff ) << 8;
	uint32_t b = ( static_cast< uint32_t >( color.b * 255 ) & 0xff );

	uint32_t value = a + r + g + b;

	std::stringstream clr;
	clr << std::hex << value;

	return clr.str();
}

ci::ColorA Config::hexToColor( const std::string &hexStr )
{
	std::stringstream converter( hexStr );
	uint32_t value;
	converter >> std::hex >> value;

	float a = ( ( value >> 24 ) & 0xff ) / 255.0f;
	float r = ( ( value >> 16 ) & 0xff ) / 255.0f;
	float g = ( ( value >> 8 ) & 0xff ) / 255.0f;
	float b = ( value & 0xff ) / 255.0f;

	return ci::ColorA( r, g, b, a );
}

void Config::readVar( ci::ColorA *var, const ci::ColorA &defVal,
					  const std::string &name, ci::XmlTree &xml )
{
	std::string colorStr = xml.hasChild( name ) ?
							xml.getChild( name ).getAttributeValue( "value", colorToHex( defVal ) ) :
							colorToHex( defVal );
	*var = hexToColor( colorStr );
}

void Config::writeVar( ci::ColorA *var, const std::string &name, ci::XmlTree &xml )
{
	addChild( name, xml );
	ci::XmlTree &node = xml.getChild( name );
	node.setAttribute( "value", colorToHex( *var ) );
	xml.push_back( node );
}

void Config::readVar( ci::Color *var, const ci::Color &defVal,
					  const std::string &name, ci::XmlTree &xml )
{
	std::string colorStr = xml.hasChild( name ) ?
							xml.getChild( name ).getAttributeValue( "value", colorToHex( defVal ) ) :
							colorToHex( defVal );
	*var = ci::Color( hexToColor( colorStr ) );
}

void Config::writeVar( ci::Color *var, const std::string &name, ci::XmlTree &xml )
{
	addChild( name, xml );
	ci::XmlTree &node = xml.getChild( name );
	node.setAttribute( "value", colorToHex( *var ) );
}

}
