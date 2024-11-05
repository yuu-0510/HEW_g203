#include "Main.h"
#include <memory>
#include "DirectX.h"
#include "Geometory.h"
#include "Sprite.h"
#include "Input.h"
#include "SceneGame.h"
#include "Defines.h"
#include "ShaderList.h"

//--- �O���[�o���ϐ�
SceneGame* g_pGame;

HRESULT Init(HWND hWnd, UINT width, UINT height)
{
	HRESULT hr;
	// DirectX������
	hr = InitDirectX(hWnd, width, height, false);
	if (FAILED(hr)) { return hr; }

	// ���@�\������
	Geometory::Init();
	Sprite::Init();
	InitInput();
	ShaderList::Init();

	// �V�[���쐬
	g_pGame = new SceneGame();

	return hr;
}

void Uninit()
{
	delete g_pGame;
	ShaderList::Uninit();
	UninitInput();
	Sprite::Uninit();
	Geometory::Uninit();
	UninitDirectX();
}

void Update()
{
	UpdateInput();
	g_pGame->Update();
}

void Draw()
{
	BeginDrawDirectX();

	// �����̕\��
#ifdef _DEBUG
	// �O���b�h
	DirectX::XMFLOAT4 lineColor(0.5f, 0.5f, 0.5f, 1.0f);
	float size = DEBUG_GRID_NUM * DEBUG_GRID_MARGIN;
	for (int i = 1; i <= DEBUG_GRID_NUM; ++i)
	{
		float grid = i * DEBUG_GRID_MARGIN;
		DirectX::XMFLOAT3 pos[2] = {
			DirectX::XMFLOAT3(grid, 0.0f, size),
			DirectX::XMFLOAT3(grid, 0.0f,-size),
		};
		Geometory::AddLine(pos[0], pos[1], lineColor);
		pos[0].x = pos[1].x = -grid;
		Geometory::AddLine(pos[0], pos[1], lineColor);
		pos[0].x = size;
		pos[1].x = -size;
		pos[0].z = pos[1].z = grid;
		Geometory::AddLine(pos[0], pos[1], lineColor);
		pos[0].z = pos[1].z = -grid;
		Geometory::AddLine(pos[0], pos[1], lineColor);
	}
	// ��
	Geometory::AddLine(DirectX::XMFLOAT3(0,0,0), DirectX::XMFLOAT3(size,0,0), DirectX::XMFLOAT4(1,0,0,1));
	Geometory::AddLine(DirectX::XMFLOAT3(0,0,0), DirectX::XMFLOAT3(0,size,0), DirectX::XMFLOAT4(0,1,0,1));
	Geometory::AddLine(DirectX::XMFLOAT3(0,0,0), DirectX::XMFLOAT3(0,0,size), DirectX::XMFLOAT4(0,0,1,1));
	Geometory::AddLine(DirectX::XMFLOAT3(0,0,0), DirectX::XMFLOAT3(-size,0,0),  DirectX::XMFLOAT4(0,0,0,1));
	Geometory::AddLine(DirectX::XMFLOAT3(0,0,0), DirectX::XMFLOAT3(0,0,-size),  DirectX::XMFLOAT4(0,0,0,1));

	Geometory::DrawLines();

	// �J�����̒l
	static bool camPosSwitch = false;
	if (IsKeyTrigger(VK_RETURN)) {
		camPosSwitch ^= true;
	}

	DirectX::XMVECTOR camPos;
	if (camPosSwitch) {
		camPos = DirectX::XMVectorSet(2.5f, 300.5f, -400.0f, 0.0f);
	}
	else {
		camPos = DirectX::XMVectorSet(2.5f, 3.5f, -4.0f, 0.0f);
	}

	// �W�I���g���p�J����������
	DirectX::XMFLOAT4X4 mat[2];
	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixTranspose(
		DirectX::XMMatrixLookAtLH(
			camPos,
			DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
			DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
		)));
	DirectX::XMStoreFloat4x4(&mat[1], DirectX::XMMatrixTranspose(
		DirectX::XMMatrixPerspectiveFovLH(
			DirectX::XMConvertToRadians(60.0f), (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 1000.0f)
	));
	Geometory::SetView(mat[0]);
	Geometory::SetProjection(mat[1]);
#endif

	g_pGame->Draw();
	EndDrawDirectX();
}

// EOF