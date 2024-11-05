#include "Geometory.h"

#define PI (3.141592)

#define TORAD(deg) (((deg) / 180.0f) * PI)		//ラジアンを求める計算
#define TODEF(rad) (((red) / PI) * 180.0f)		//ラジアンから度数表記に戻す計算

void Geometory::MakeBox()
{
	//--- 頂点の作成

	Vertex vtx[] = {
		// -Z面 
		{{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f}},//0
		{{ 0.5f, 0.5f, -0.5f}, {1.0f, 0.0f}},//1
		{{-0.5f,-0.5f, -0.5f}, {0.0f, 1.0f}},//2
		{{ 0.5f,-0.5f, -0.5f}, {1.0f, 1.0f}},//3
		 //Z面

		{{ 0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}},//4
		{{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},//5
		{{ 0.5f,-0.5f, 0.5f}, {0.0f, 1.0f}},//6
		{{-0.5f,-0.5f, 0.5f}, {1.0f, 1.0f}},//7
		
		//-X面
		{{-0.5f, 0.5f,  0.5f}, {0.0f, 0.0f}},//8
		{{-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f}},//9
		{{-0.5f,-0.5f,  0.5f}, {0.0f, 1.0f}},//10
		{{-0.5f,-0.5f, -0.5f}, {1.0f, 1.0f}},//11
	
		// X面
		{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f}},//12
		{{0.5f, 0.5f,  0.5f}, {1.0f, 0.0f}},//13
		{{0.5f,-0.5f, -0.5f}, {0.0f, 1.0f}},//14
		{{0.5f,-0.5f,  0.5f}, {1.0f, 1.0f}},//15
		
		//-Y面
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},//16
		{{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},//17
		{{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}},//18
		{{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}},//19

		// Y面
		{{-0.5f, 0.5f,  0.5f}, {0.0f, 0.0f}},//20
		{{ 0.5f, 0.5f,  0.5f}, {1.0f, 0.0f}},//21
		{{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},//22
		{{ 0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},//23
		
	
	};
	

	//--- インデックスの作成

	int idx[] = {
	 0, 1, 2,  1, 3, 2, //-Z面 
	 4, 5, 6,  5, 7, 6,	// Z面
	 8, 9,10,  9,11,10,	//-X面
	12,13,14, 13,15,14,	// X面 
	16,17,18, 17,19,18,	// Y面
	20,21,22, 21,23,22	//-Y面
	};

	// バッファの作成
	MeshBuffer::Description desc = {};
	desc.pVtx = vtx;
	desc.vtxCount = sizeof(vtx) / sizeof(vtx[0]);
	desc.vtxSize = sizeof(Vertex);
	desc.pIdx = idx;
	desc.idxCount = sizeof(idx) / sizeof(idx[0]);
	desc.idxSize = sizeof(int);
		desc.topology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pBox = new MeshBuffer();
	m_pBox->Create(desc);
}

void Geometory::MakeCylinder()
{
	//float rad = 1.0f;
	//--- 頂点の作成

	// 天面、底面
	Vertex vtx2[] = {
		//Y
		{{(float)cos(TORAD( 0.0f))  * 0.5f,1.0f,(float)sin(TORAD( 0.0f))  * 0.5f},{  0.0f,0.0f}},//0(1.0f / 8.0f / 2.0f) * 0.0f
		//第一象限
		{{(float)cos(TORAD(22.5f))  * 0.5f,1.0f,(float)sin(TORAD(22.5f))  * 0.5f},{0.125f,0.25f}},//1
		{{(float)cos(TORAD(45.0f))  * 0.5f,1.0f,(float)sin(TORAD(45.0f))  * 0.5f},{ 0.25f,0.5f}},//2
		{{(float)cos(TORAD(67.5f))  * 0.5f,1.0f,(float)sin(TORAD(67.5f))  * 0.5f},{0.375f,0.75f}},//3
		{{(float)cos(TORAD(90.0f))  * 0.5f,1.0f,(float)sin(TORAD(90.0f))  * 0.5f},{  0.5f,1.0f}},//4
		//第二象限
		{{(float)-cos(TORAD(67.5f)) * 0.5f,1.0f,(float)sin(TORAD(67.5f))  * 0.5f},{0.625f,0.75f}},//5
		{{(float)-cos(TORAD(45.0f)) * 0.5f,1.0f,(float)sin(TORAD(45.0f))  * 0.5f},{ 0.75f,0.5f}},//6
		{{(float)-cos(TORAD(22.5f)) * 0.5f,1.0f,(float)sin(TORAD(22.5f))  * 0.5f},{0.875f,0.25f}},//7
		{{(float)-cos(TORAD( 0.0f)) * 0.5f,1.0f,(float)sin(TORAD( 0.0f))  * 0.5f},{  1.0f,0.0f}},//8
		//第三象限
		{{(float)-cos(TORAD(22.5f)) * 0.5f,1.0f,(float)-sin(TORAD(22.5f)) * 0.5f},{0.875f,0.25f}},//9
		{{(float)-cos(TORAD(45.0f)) * 0.5f,1.0f,(float)-sin(TORAD(45.0f)) * 0.5f},{0.625f,0.5f}},//10
		{{(float)-cos(TORAD(67.5f)) * 0.5f,1.0f,(float)-sin(TORAD(67.5f)) * 0.5f},{ 0.75f,0.75f}},//11
		{{(float)-cos(TORAD(90.0f)) * 0.5f,1.0f,(float)-sin(TORAD(90.0f)) * 0.5f},{  0.5f,1.0f}},//12
		//第四象限
		{{(float)cos(TORAD(67.5f))  * 0.5f,1.0f,(float)-sin(TORAD(67.5f)) * 0.5f},{0.375f,0.75f}},//13
		{{(float)cos(TORAD(45.0f))  * 0.5f,1.0f,(float)-sin(TORAD(45.0f)) * 0.5f},{ 0.25f,0.5f}},//14
		{{(float)cos(TORAD(22.5f))  * 0.5f,1.0f,(float)-sin(TORAD(22.5f)) * 0.5f},{0.125f,0.25f}},//15


		

		//-Y
		//{{cos(TORAD( 0.0f)) *  0.5f,-1.0f,sin(TORAD( 0.0f) *  0.5f)},{ 0.0f,0.0f}},//8
		//{{cos(TORAD(45.0f)) *  0.5f,-1.0f,sin(TORAD(45.0f) * -0.5f)},{0.25f,0.5f}},//9
		//{{cos(TORAD(90.0f)) *  0.5f,-1.0f,sin(TORAD(90.0f) * -0.5f)},{ 0.5f,1.0f}},//10
		//{{cos(TORAD(45.0f)) * -0.5f,-1.0f,sin(TORAD(45.0f) * -0.5f)},{0.75f,0.5f}},//11
		//{{cos(TORAD( 0.0f)) * -0.5f,-1.0f,sin(TORAD( 0.0f) * -0.5f)},{ 1.0f,0.0f}},//12
		//{{cos(TORAD(45.0f)) * -0.5f,-1.0f,sin(TORAD(45.0f) *  0.5f)},{0.75f,0.5f}},//13
		//{{cos(TORAD(90.0f)) * -0.5f,-1.0f,sin(TORAD(90.0f) *  0.5f)},{ 0.5f,1.0f}},//14
		//{{cos(TORAD(45.0f)) *  0.5f,-1.0f,sin(TORAD(45.0f) *  0.5f)},{0.25f,0.5f}},//15


		// 側面

		//aaabbb
		//hi

	};
	


	//--- インデックスの作成
	
	int idx2[] = {
	//天面
		0,2,1,
		0,3,2,
		0,4,3,
		0,5,4,
		0,6,5,
		0,7,6,
		0,8,7,
		0,9,8,
		0,10,9,
		0,11,10,
		0,12,11,
		0,13,12,
		0,14,13,
		0,15,14,
	//底面
		/*8,10,9,
		8,11,10,
		8,12,11,
		8,13,12,
		8,14,13,
		8,15,14,*/
	};
	
	
	//側面

	// バッファの作成
	MeshBuffer::Description desc2 = {};
	desc2.pVtx = vtx2;
	desc2.vtxCount = sizeof(vtx2) / sizeof(vtx2[0]);
	desc2.vtxSize = sizeof(Vertex);
	desc2.pIdx = idx2;
	desc2.idxCount = sizeof(idx2) / sizeof(idx2[0]);
	desc2.idxSize = sizeof(int);
	desc2.topology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pCylinder = new MeshBuffer();
	m_pCylinder->Create(desc2);
}

void Geometory::MakeSphere()
{
	//--- 頂点の作成

	//--- インデックスの作成

	// バッファの作成
}