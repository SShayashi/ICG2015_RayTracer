#include <cmath>
#include <float.h>
#include "trimesh.h"

using namespace std;

Trimesh::~Trimesh()
{
	for( Materials::iterator i = materials.begin(); i != materials.end(); ++i )
		delete *i;
}

// must add vertices, normals, and materials IN ORDER
void Trimesh::addVertex( const Vec3d &v )
{
    vertices.push_back( v );
}

void Trimesh::addMaterial( Material *m )
{
    materials.push_back( m );
}

void Trimesh::addNormal( const Vec3d &n )
{
    normals.push_back( n );
}

// Returns false if the vertices a,b,c don't all exist
bool Trimesh::addFace( int a, int b, int c )
{
    int vcnt = vertices.size();

    if( a >= vcnt || b >= vcnt || c >= vcnt )
        return false;

    TrimeshFace *newFace = new TrimeshFace( scene, new Material(*this->material), this, a, b, c );
    newFace->setTransform(this->transform);
    faces.push_back( newFace );
    scene->add(newFace);
    return true;
}

char *
Trimesh::doubleCheck()
// Check to make sure that if we have per-vertex materials or normals
// they are the right number.
{
    if( !materials.empty() && materials.size() != vertices.size() )
        return "Bad Trimesh: Wrong number of materials.";
    if( !normals.empty() && normals.size() != vertices.size() )
        return "Bad Trimesh: Wrong number of normals.";

    return 0;
}

// Calculates and returns the normal of the triangle too.
bool TrimeshFace::intersectLocal( const ray& r, isect& i ) const {
   Vec3d p = r.getPosition();	// Rayの位置ベクトルpを取得
   Vec3d d = r.getDirection();	// Rayの方向ベクトルdを取得
   Vec3d& alpha = parent->vertices[ids[0]];	// 三角形の頂点alphaを取得
   Vec3d& beta = parent->vertices[ids[1]]; 	// 三角形の頂点betaを取得
   Vec3d& gamma = parent->vertices[ids[2]]; 	// 三角形の頂点gammaを取得
   Vec3d normal = ((beta-alpha)^(gamma-alpha)); // 平面の法線

   normal.normalize();                                          // 正規化

   double t;		// Rayの媒介変数t
   /* tを求める計算を自分で書こう */
	t = (normal*alpha - normal*p)/(normal*d);

   if (t < RAY_EPSILON)
      return false;	// tがRAY_EPSILONより小さい場合は平面と交差しない

	Vec3d Q = r.at(t);	// 交点の位置ベクトル
	/* 交点が三角形の内部かを判定する部分を自分で書こう */
	auto cross_alpha = (beta - alpha)^(Q - alpha);
	auto cross_beta  =(gamma - beta)^(Q - beta);
	auto cross_gamma = (alpha - gamma)^(Q - gamma);

	auto inner_a =((cross_alpha*cross_beta)*(cross_alpha*cross_gamma));
	auto inner_b =((cross_beta*cross_gamma)*(cross_beta*cross_alpha));
	auto inner_g =((cross_gamma*cross_alpha)*(cross_gamma*cross_beta));


	/*向きが全部同じなら、内積は正になるはず*/
	if( inner_a < 0  || inner_b < 0 || inner_g < 0 ){
		return false;
	}

	i.obj = this;
	i.t = t;
	i.N = normal;

	return true;
}


void
Trimesh::generateNormals()
// Once you've loaded all the verts and faces, we can generate per
// vertex normals by averaging the normals of the neighboring faces.
{
    int cnt = vertices.size();
    normals.resize( cnt );
    int *numFaces = new int[ cnt ]; // the number of faces assoc. with each vertex
    memset( numFaces, 0, sizeof(int)*cnt );
    
    for( Faces::iterator fi = faces.begin(); fi != faces.end(); ++fi )
    {
        Vec3d a = vertices[(**fi)[0]];
        Vec3d b = vertices[(**fi)[1]];
        Vec3d c = vertices[(**fi)[2]];
        
        Vec3d faceNormal = ((b-a) ^ (c-a));
		faceNormal.normalize();
        
        for( int i = 0; i < 3; ++i )
        {
            normals[(**fi)[i]] += faceNormal;
            ++numFaces[(**fi)[i]];
        }
    }

    for( int i = 0; i < cnt; ++i )
    {
        if( numFaces[i] )
            normals[i]  /= numFaces[i];
    }

    delete [] numFaces;
}

