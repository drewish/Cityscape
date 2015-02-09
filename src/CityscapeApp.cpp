/*
 Next steps:
 - find longest segment in a polyline
 - find line for dividing polyline in half along longest segment
 */

#include "cinder/app/AppNative.h"
#include "cinder/ConvexHull.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class Road {
  public:
	Road( const Vec2f a, const Vec2f b, uint width = 10 )
	: pointA(a), pointB(b), width(width)
	{
		Vec2f normal = Vec2f(b.y - a.y, -(b.x - a.x)).normalized();
		Vec2f offset = normal * width / 2;
		outline.push_back(a + offset);
		outline.push_back(a - offset);
		outline.push_back(b - offset);
		outline.push_back(b + offset);
		outline.setClosed();
	};
	const Vec2f pointA, pointB;
	PolyLine2f outline;
	const unsigned int width;
};

class Building {
  public:
	// Default to a 10x10 square
	Building() {
		Building( PolyLine2f( { Vec2f(-5, -5), Vec2f(5, -5), Vec2f(5, 5), Vec2f(5, 5) } ) );
	};
	Building( const PolyLine2f outline ) : outline(outline) { };
	Building( const Building &src ) : outline(src.outline) { };
	PolyLine2f outline;
};

class Lot {
  public:
	Lot( const Lot &src ) : outline(src.outline), building(src.building) { };
	Lot( const PolyLine2f outline ) : outline(outline) { };

	void place( const Building b ) {
		building = b;
	}

	PolyLine2f buildingPos() {
		return outline.c
	}

	PolyLine2f outline;
	Building building;
};

class Block {
  public:
	// Outline's coords should be centered around the origin so we can transform
	// it to fit on the lot.
	Block( const PolyLine2f outline ) : outline(outline) { };

	void subdivide()
	{
		lots.clear();
		Lot l = Lot(outline);
		lots.push_back(l);
	}

	PolyLine2f outline;
	vector<Lot> lots;
};



class CityscapeApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void update();
	void draw();

	void addPoint(Vec2f);
	void layout();

	vector<Vec2f>   mPoints;
	PolyLine2f      mConvexHull;
	PolyLine2f      mDivider;
	vector<Road>    mRoads;
	vector<Block>   mBlocks;

	float mArea, mLongestSide;

	params::InterfaceGlRef  mParams;
};

void CityscapeApp::setup()
{
	mParams = params::InterfaceGl::create( "App parameters", Vec2i( 180, 100 ) );
	mParams->addParam( "Area", &mArea );
	mParams->addButton( "Clear Points", [&] { mPoints.clear(); layout(); } );

	mPoints.push_back(Vec2f(151,172));
	mPoints.push_back(Vec2f(450,108));
	mPoints.push_back(Vec2f(425,82));
	mPoints.push_back(Vec2f(530,398));
	mPoints.push_back(Vec2f(544,380));
	mPoints.push_back(Vec2f(183,433));
	mPoints.push_back(Vec2f(191,450));
	mPoints.push_back(Vec2f(154,156));
	layout();
}

// Assumes line does not self intersecting
// Treats the line as closed (you wanted area right?)
// http://www.mathsisfun.com/geometry/area-irregular-polygons.html
float area( PolyLine2f line ) {
	float sum = 0.0;
	if ( line.size() > 1 ) {
		PolyLine2f::iterator it;
		Vec2f prev, curr;
		for ( it = line.begin(), prev = *it++; it != line.end() ; it++ ) {
			curr = *it;
			sum += ( prev.y + curr.y ) / 2.0 * ( curr.x - prev.x );
			prev = curr;
		}
	}
	return sum;
}

void CityscapeApp::update()
{
	//  mPolyResults = mConvexHull;

	//  mDivider.push_back(mPoints[1]);
	//  mDivider.push_back(mPoints[3]);

	// Only works on closed shapes.
	mArea = area(mConvexHull);

	//  mLongestSide
}

void CityscapeApp::addPoint(Vec2f pos)
{
	mPoints.push_back( pos );
	//  console() << pos << endl;
	layout();
}

void CityscapeApp::layout()
{
	float width = 20.0;
	vector<PolyLine2f> allRoads;

	mRoads.clear();
	for ( uint i = 1, size = mPoints.size(); i < size; i += 2 ) {
		Road road(mPoints[i-1], mPoints[i], width);
		mRoads.push_back(road);
		allRoads = PolyLine2f::calcUnion({ road.outline }, allRoads);
	}

	mConvexHull = calcConvexHull( mPoints );

	mBlocks.clear();
	vector<PolyLine2f> pieces = PolyLine2f::calcDifference({ mConvexHull }, allRoads);
	for( auto it = pieces.begin(); it != pieces.end(); ++it ) {
		Block b(*it);
		b.subdivide();
		for (auto lotIt = b.lots.begin(); lotIt != b.lots.end(); ++lotIt) {
			lotIt->place(Building());
		}
		mBlocks.push_back(b);
	}
}

void CityscapeApp::mouseDown( MouseEvent event )
{
	addPoint( event.getPos() );
}

void CityscapeApp::mouseDrag( MouseEvent event )
{
	addPoint( event.getPos() );
}

void CityscapeApp::draw()
{
	gl::clear( Color( 0.8, 0.8, 0.8 ) );
	gl::enableAlphaBlending();

	// draw solid convex hull
	//  gl::color( ColorA( 0.8f, 0, 1.0f, 0.1f ) );
	//  gl::drawSolid( mConvexHull );
	gl::color( ColorA( 0.8f, 0, 1.0f, 0.8f ) );
	gl::draw( mConvexHull );

	gl::color( ColorA( 1.0f, 0, 0.8f, 0.8f ) );
	gl::draw( mDivider );

	gl::color( ColorA( 0.3f, 0.3f, 0.3f, 0.4f ) );
	for( auto it = mRoads.begin(); it != mRoads.end(); ++it ) {
		gl::drawSolid( it->outline );
	}

	for( auto it = mBlocks.begin(); it != mBlocks.end(); ++it ) {
		gl::color( ColorA( 0.0f, 0.8f, 0.2f, 0.5f ) );
		gl::drawSolid( it->outline );

		gl::color( ColorA( 0.8f, 0.8f, 0.8f, 0.8f ) );
		for( auto itL = it->lots.begin(); itL != it->lots.end(); ++itL ) {
			//      gl::draw( itL->outline );

			gl::draw( itL->building.outline );
		}
	}

	// draw convex hull points
	gl::color( Color( 0.0f, 0, 1.0f ) );
	for( auto it = mConvexHull.begin(); it != mConvexHull.end(); ++it ) {
		gl::drawSolidCircle( *it, 4 );
	}

	gl::color( ColorA( 1.0f, 0.8f, 0, 0.6f ) );
	for( auto it = mPoints.begin(); it != mPoints.end(); ++it ) {
		gl::drawSolidCircle( *it, 3 );
	}

	mParams->draw();
}

CINDER_APP_NATIVE( CityscapeApp, RendererGl )
