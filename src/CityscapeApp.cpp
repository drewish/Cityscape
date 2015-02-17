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

//#include "StraightSkeleton.h"
#include "Road.h"
#include "Building.h"
#include "Lot.h"
#include "Block.h"

using namespace ci;
using namespace ci::app;
using namespace std;


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

//	PolyLine2f		mContour;
//	vector<PolyLine2f>	mSkeleton;

	float mArea, mLongestSide;

	params::InterfaceGlRef  mParams;
};

void CityscapeApp::setup()
{
//	mContour.push_back( Vec2f( -1, -1 ) );
//	mContour.push_back( Vec2f( 0, -12 ) );
//	mContour.push_back( Vec2f( 1, -1 ) );
//	mContour.push_back( Vec2f( 12, 0 ) );
//	mContour.push_back( Vec2f( 1, 1 ) );
//	mContour.push_back( Vec2f( 0, 12 ) );
//	mContour.push_back( Vec2f( -1, 1 ) );
//	mContour.push_back( Vec2f( -12, 0 ) );
//	mContour.setClosed();
//
//	mSkeleton = skeleton(mContour);

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

//	gl::pushMatrices();
//
//	gl::scale(8.0, 8.0);
//	gl::translate(Vec2f(20,20));
//
//	gl::color(1,1,0);
//	for( auto it = mSkeleton.begin(); it != mSkeleton.end(); ++it ) {
//		gl::draw(*it);
//	}
//	gl::color(1,0,0);
//	gl::draw(mContour);
//	gl::popMatrices();
//	return;


	
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
