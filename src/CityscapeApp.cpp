/*
 Next steps:
 - find longest segment in a polyline
 - find line for dividing polyline in half along longest segment
 */

#include "cinder/app/AppNative.h"
#include "cinder/Camera.h"
#include "cinder/ConvexHull.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

#include "CinderCGAL.h"
#include "Road.h"
#include "Building.h"
#include "Lot.h"
#include "Block.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class CityscapeApp : public AppNative {
  public:
    void prepareSettings( Settings *settings );
	void setup();
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void update();
	void draw();

	void addPoint(Vec2f);
	void layout();

    CameraPersp     mCamera;

	vector<Vec2f>   mPoints;
	PolyLine2f      mConvexHull;
	PolyLine2f      mDivider;
	vector<Road>    mRoads;
	vector<Block>   mBlocks;
    bool            mClipCityLimit = false;
    ci::gl::VboMesh mMesh;

	params::InterfaceGlRef  mParams;
};

void CityscapeApp::prepareSettings( Settings *settings )
{
    settings->enableHighDensityDisplay();
}

void CityscapeApp::setup()
{
	mParams = params::InterfaceGl::create( "App parameters", Vec2i( 180, 100 ) );
	mParams->addParam( "Limit", &mClipCityLimit, "key=l" );
    mParams->addButton( "Test 1", [&] {
        mPoints.clear();
        mPoints.push_back(Vec2f(133,41));
        mPoints.push_back(Vec2f(143,451));
        mPoints.push_back(Vec2f(143,451));
        mPoints.push_back(Vec2f(495,424));
        mPoints.push_back(Vec2f(491,421));
        mPoints.push_back(Vec2f(370,254));
        mPoints.push_back(Vec2f(377,262));
        mPoints.push_back(Vec2f(529,131));
        layout();
    }, "key=1" );
    mParams->addButton( "Test 2", [&] {
        mPoints.clear();
        mPoints.push_back(Vec2f(133,41));
        mPoints.push_back(Vec2f(143,451));
        mPoints.push_back(Vec2f(143,451));
        mPoints.push_back(Vec2f(495,424));
        mPoints.push_back(Vec2f(491,421));
        mPoints.push_back(Vec2f(370,254));
        mPoints.push_back(Vec2f(377,262));
        mPoints.push_back(Vec2f(529,131));
        mPoints.push_back(Vec2f(131,47));
        mPoints.push_back(Vec2f(523,132));
        layout();
    }, "key=2" );
    mParams->addButton( "Test 3", [&] {
        mPoints.clear();
        mPoints.push_back(Vec2f(119.284,17.3257));
        mPoints.push_back(Vec2f(301.294,1226.4));
        mPoints.push_back(Vec2f(301.294,1226.4));
        mPoints.push_back(Vec2f(546.399,74.1908));
        mPoints.push_back(Vec2f(544.513,79.3862));
        mPoints.push_back(Vec2f(118.603,19.5102));
        layout();
    }, "key=3" );
    mParams->addButton( "Test 4", [&] {
        mPoints.clear();
        mPoints.push_back(Vec2f(119.284,17.3257));
        mPoints.push_back(Vec2f(301.294,1226.4));
        mPoints.push_back(Vec2f(301.294,1226.4));
        mPoints.push_back(Vec2f(546.399,74.1908));
        mPoints.push_back(Vec2f(544.513,79.3862));
        mPoints.push_back(Vec2f(118.603,19.5102));
        mPoints.push_back(Vec2f(163.104,60.2898));
        mPoints.push_back(Vec2f(306.353,918.302));
        mPoints.push_back(Vec2f(306.353,918.302));
        mPoints.push_back(Vec2f(490.026,113.687));
        mPoints.push_back(Vec2f(490.026,113.687));
        mPoints.push_back(Vec2f(163.104,60.2898));
        mPoints.push_back(Vec2f(216.101,115.129));
        mPoints.push_back(Vec2f(310.713,599.953));
        mPoints.push_back(Vec2f(310.713,599.953));
        mPoints.push_back(Vec2f(421.241,167.717));
        mPoints.push_back(Vec2f(421.241,167.717));
        mPoints.push_back(Vec2f(220.751,122.422));
        layout();
    }, "key=4" );
    mParams->addButton( "Clear Points", [&] { mPoints.clear(); layout(); }, "key=0" );


	layout();

    mCamera.setPerspective( 40.0f, getWindowAspectRatio(), 300.0f, 2000.0f );
    Vec2i center = getWindowCenter();
    mCamera.lookAt( Vec3f( center.x, center.y - 600, 400.0f ), Vec3f(center,0.0), Vec3f::yAxis() );


    // For some fucking reason i have to build and render a mesh in this class
    // to be able to have the buildings render without dying....
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();

    int quadCount = 1;
    int vertCount = quadCount * 4;
    mMesh = gl::VboMesh(vertCount, quadCount * 4, layout, GL_QUADS);

    vector<uint32_t> indices;
    for (int i=0; i < vertCount; i++) {
        indices.push_back(i);
    }
    mMesh.bufferIndices(indices);

    vector<Vec3f> positions;
    positions.push_back( Vec3f( -0.5,  0.5, -0.5 ) );
    positions.push_back( Vec3f(  0.5,  0.5, -0.5 ) );
    positions.push_back( Vec3f(  0.5, -0.5, -0.5 ) );
    positions.push_back( Vec3f( -0.5, -0.5, -0.5 ) );
    mMesh.bufferPositions(positions);
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
}

void CityscapeApp::addPoint(Vec2f pos)
{
	mPoints.push_back( pos );
    console() << "mPoints.push_back(Vec2f(" << pos.x << "," << pos.y << "));\n";
	layout();
}

void CityscapeApp::layout()
{
	float width = 20.0;
//	vector<PolyLine2f> allRoads;

	mRoads.clear();
    mBlocks.clear();

	for ( uint i = 1, size = mPoints.size(); i < size; i += 2 ) {
		Road road(mPoints[i-1], mPoints[i], width);
		mRoads.push_back(road);
//		allRoads = PolyLine2f::calcUnion({ road.outline }, allRoads);
	}


    Vec2i windowSize = getWindowSize();
    Polygon_2 window;
    window.push_back( K::Point_2( 0, 0 ) );
    window.push_back( K::Point_2( windowSize.x, 0 ) );
    window.push_back( K::Point_2( windowSize.x, windowSize.y ) );
    window.push_back( K::Point_2( 0, windowSize.y ) );

    Polygon_set_2 unPaved;
    for ( auto it = mRoads.begin(); it != mRoads.end(); ++it ) {
        console() << "road: " << it->pointA << "\t-> " << it->pointB << endl;
        unPaved.join( polygonFrom( it->outline ) );
    }
    unPaved.complement();
    unPaved.intersection(window); // Intersect with the clipping rectangle.



    std::list<Polygon_with_holes_2> res;
    unPaved.polygons_with_holes (std::back_inserter (res));
    for (auto it = res.begin(); it != res.end(); ++it) {
        unsigned int k = 1;
        Polygon_with_holes_2 pwh = *it;


        Block b( polyLineFrom( pwh.outer_boundary() ) );
        b.subdivide();
        // b.placeBuildings();
        mBlocks.push_back(b);

        console() << "--> "  << pwh.number_of_holes() << " holes:" << std::endl;
        for (auto hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit, ++k) {
            console() << " Hole #" << k << " = ";

            Polygon_2 P = *hit;

            if (P.size()) {
                Block b( polyLineFrom( P ) );
                b.subdivide();
//                b.placeBuildings();
                mBlocks.push_back(b);
            }

            console() << "[ " << P.size() << " vertices:";
            for (auto vit = P.vertices_begin(); vit != P.vertices_end(); ++vit)
                console() << " (" << *vit << ")";
            console() << " ]" << std::endl;

        }
    }
    
/*
	mConvexHull = calcConvexHull( mPoints );

	mBlocks.clear();
	vector<PolyLine2f> pieces = PolyLine2f::calcDifference({ mConvexHull }, allRoads);
    
	for( auto it = pieces.begin(); it != pieces.end(); ++it ) {
        if (it->size()) {
            Block b(*it);
            b.subdivide();
            for (auto lotIt = b.lots.begin(); lotIt != b.lots.end(); ++lotIt) {
                lotIt->place(Building());
            }
            mBlocks.push_back(b);
        }
	}
*/
    for( auto it = mRoads.begin(); it != mRoads.end(); ++it ) {
        it->setup();
    }
    for( auto it = mBlocks.begin(); it != mBlocks.end(); ++it ) {
        it->setup();
    }
}

void CityscapeApp::mouseDown( MouseEvent event )
{
    float u = ((float) event.getX()) / getWindowWidth();
    float v = ((float) (getWindowHeight() - event.getY())) / getWindowHeight();
    Ray r = mCamera.generateRay(u, v, mCamera.getAspectRatio());
    float result = 0.0f;
    Vec3f point;
    if (r.calcPlaneIntersection(Vec3f::zero(), Vec3f::zAxis(), &result)) {
        point = r.calcPosition(result);
        addPoint( point.xy() );
    }
}

void CityscapeApp::mouseDrag( MouseEvent event )
{
//	addPoint( event.getPos() );
}

void CityscapeApp::draw()
{
	gl::clear( Color( 0.9, 0.9, 0.9 ) );
	gl::enableAlphaBlending();

    gl::setMatrices( mCamera );

    gl::draw(mMesh);

	for( auto it = mRoads.begin(); it != mRoads.end(); ++it ) {
        it->draw();
	}

    gl::lineWidth(4);
	for( auto it = mBlocks.begin(); it != mBlocks.end(); ++it ) {
        it->draw();
    }

/*
	// draw convex hull points
	gl::color( Color( 0.0f, 0, 1.0f ) );
	for( auto it = mConvexHull.begin(); it != mConvexHull.end(); ++it ) {
		gl::drawSolidCircle( *it, 4 );
	}

	gl::color( ColorA( 1.0f, 0.8f, 0, 0.6f ) );
	for( auto it = mPoints.begin(); it != mPoints.end(); ++it ) {
		gl::drawSolidCircle( *it, 3 );
	}
*/

	mParams->draw();
}

CINDER_APP_NATIVE( CityscapeApp, RendererGl )
