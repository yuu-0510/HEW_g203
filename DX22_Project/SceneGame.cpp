#include "SceneGame.h"
#include "Geometory.h"
#include "Defines.h"


#define MAX_BOX (5)

SceneGame::SceneGame()
{

	m_pModel = new Model();
	if (!m_pModel->Load("LowPolyNature/Branch_01.fbx",0.01f,Model::None)) { //  
		MessageBox(NULL, "LowPolyNature/Branch_01.fbx", "Error", MB_OK);    //  
	}

	RenderTarget* pRTV = GetDefaultRTV(); // RenderTargetView 
	DepthStencil* pDSV = GetDefaultDSV(); // DepthStencilView 
	SetRenderTargets(1, &pRTV, pDSV);
}

SceneGame::~SceneGame()
{
	if (m_pModel) {
		delete m_pModel;
		m_pModel = nullptr;
	}
}

void SceneGame::Update()
{
}

void SceneGame::Draw()
{
	
	
	//float BoxArray[MAX_BOX][9] = { {METER(0.0f), METER(0.0f), METER(50.0f), METER(50.0f), METER(1.0f), METER(100.0f), 0.0f, 0.0f, 0.0f},//Tx,Ty,Tz,Sx,Sy,Sz,Rx,Ry,Rz
	//							   {METER(-10.0f), METER(0.0f), METER(150.0f), METER(50.0f), METER(1.0f), METER(100.0f), 0.0f, 0.0f, 0.0f},
	//							   {METER(-40.0f),METER(0.0f), METER(250.0f), METER(100.0f), METER(1.0f), METER(100.0f), 0.0f, 0.0f, 0.0f},
	//							   {METER(-125.0f),METER(0.0f), METER(260.0f), METER(100.0f), METER(1.0f), METER(100.0f), 0.0f, 0.0f, 0.0f},
	//};

	//DirectX::XMMATRIX T;
	//DirectX::XMMATRIX S;
	//DirectX::XMMATRIX Rx;
	//DirectX::XMMATRIX Ry;
	//DirectX::XMMATRIX Rz;
	//DirectX::XMMATRIX mat;

	//

	//for (int i = 0; i < MAX_BOX; i++)
	//{
	//	T = DirectX::XMMatrixTranslation(BoxArray[i][0], BoxArray[i][1], BoxArray[i][2]);
	//	S = DirectX::XMMatrixScaling(BoxArray[i][3], BoxArray[i][4], BoxArray[i][5]);
	//	Rx = DirectX::XMMatrixRotationX(BoxArray[i][6]);
	//	Ry = DirectX::XMMatrixRotationY(BoxArray[i][7]);
	//	Rz = DirectX::XMMatrixRotationZ(BoxArray[i][8]);
	//	mat = S * Rx * Ry * Rz * T;
	//	mat = DirectX::XMMatrixTranspose(mat);
	//	DirectX::XMFLOAT4X4 fMat;
	//	DirectX::XMStoreFloat4x4(&fMat, mat);
	//	Geometory::SetWorld(fMat);
	//	Geometory::DrawBox();
	//}
	
	//Geometory::DrawBox();


	//static float rad = 0.0f;
	//DirectX::XMMATRIX Rx = DirectX::XMMatrixRotationX(rad);
	//DirectX::XMMATRIX Ry = DirectX::XMMatrixRotationY(rad);
	//DirectX::XMMATRIX Rz = DirectX::XMMatrixRotationZ(rad);
	//DirectX::XMMATRIX mat = Rx * Ry * Rz; // ‚»‚ê‚¼‚ê‚Ìs—ñ‚ðŠ|‚¯‡‚í‚¹‚ÄŠi”[ 
	//mat = DirectX::XMMatrixTranspose(mat);
	//DirectX::XMFLOAT4X4 fMat; // s—ñ‚ÌŠi”[æ 
	//DirectX::XMStoreFloat4x4(&fMat, mat);
	//Geometory::SetWorld(fMat); // ƒ{ƒbƒNƒX‚É•ÏŠ·s—ñ‚ðÝ’è 
	//rad += 0.01f;//‰ñ“]Šp‚ÌXV(‘¬“x‚Í‚¨”C‚¹);

	Geometory::DrawCylinder();
	

	

	/*if (m_pModel) {
		m_pModel->Draw();
	}*/

}
