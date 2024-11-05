/*
* @brief 
* @ �����A�j���[�V�����Ƃ�
*/
#ifndef __MODEL_H__
#define __MODEL_H__

#include <DirectXMath.h>
#include <vector>
#include "Shader.h"
#include "MeshBuffer.h"
#include <functional>

class Model
{
public:
	// ���f�����]�ݒ�
	enum Flip
	{
		None,			// DirectX����(���]����
		XFlip,			// Maya����
		ZFlip,			// DirectX����(Maya����180��]�������
		ZFlipUseAnime,	// DirecX����(�A�j���[�V����������ꍇ
	};

private:
	// �A�j���[�V�����v�Z�̈�
	enum AnimeTransform
	{
		MAIN,			// �ʏ�Đ�
		BLEND,			// �u�����h�Đ�
		PARAMETRIC0,	// ����A
		PARAMETRIC1,	// ����B
		MAX_TRANSFORM
	};

public:
	// �^��`
	using NodeIndex	= int;	// �{�[��(�K�w)�ԍ�
	using AnimeNo	= int;	// �A�j���[�V�����ԍ�

	// �萔��`
	static const NodeIndex	INDEX_NONE = -1;		// �Y���m�[�h�Ȃ�
	static const AnimeNo	ANIME_NONE = -1;		// �Y���A�j���[�V�����Ȃ�
	static const AnimeNo	PARAMETRIC_ANIME = -2;	// �����A�j���[�V����

private:
	// �����^��`
	using Children	= std::vector<NodeIndex>;	// �m�[�h�K�w���

	// �����萔��`
	static const UINT		MAX_BONE			=	200;	// �P���b�V���̍ő�{�[����(������ύX����ꍇ.hlsl���̒�`���ύX����

	// �A�j���[�V�����̕ϊ����
	struct Transform
	{
		DirectX::XMFLOAT3	translate;
		DirectX::XMFLOAT4	quaternion;
		DirectX::XMFLOAT3	scale;
	};
	using Key			= std::pair<float, Transform>;
	using Timeline		= std::map<float, Transform>;
	using Transforms	= std::vector<Transform>;

	// �A�j���[�V�����ƃ{�[���̊֘A�t�����
	struct Channel
	{
		NodeIndex	index;
		Timeline	timeline;
	};
	using Channels = std::vector<Channel>;

	// �{�[�����
	struct Node
	{
		std::string			name;		// �{�[����
		NodeIndex			parent;		// �e�{�[��
		Children			children;	// �q�{�[��
		DirectX::XMMATRIX	mat;		// �ϊ��s��
	};
	using Nodes = std::vector<Node>;

	
public:
	// ���_���
	struct Vertex
	{
		DirectX::XMFLOAT3	pos;
		DirectX::XMFLOAT3	normal;
		DirectX::XMFLOAT2	uv;
		DirectX::XMFLOAT4	color;
		float				weight[4];
		unsigned int		index[4];
	};
	using Vertices	= std::vector<Vertex>;
	using Indices	= std::vector<unsigned long>;

	// ���_�̍��ό`���
	struct Bone
	{
		NodeIndex index;
		DirectX::XMMATRIX invOffset;
	};
	using Bones = std::vector<Bone>;

	// ���b�V��
	struct Mesh
	{
		Vertices		vertices;
		Indices			indices;
		unsigned int	materialID;
		Bones			bones;
		MeshBuffer*		pMesh;
	};
	using Meshes = std::vector<Mesh>;

	// �}�e���A�����
	struct Material
	{
		DirectX::XMFLOAT4	diffuse;	// �g�U��(���C���J���[
		DirectX::XMFLOAT4	ambient;	// ����(�A�̕����̃J���[
		DirectX::XMFLOAT4	specular;	// ���ʔ��ˌ�(�������镔���̃J���[
		Texture* pTexture;	// �e�N�X�`��
	};
	using Materials = std::vector<Material>;

	// �A�j���[�V�������
	struct Animation
	{
		float		nowTime;	// ���݂̍Đ�����
		float		totalTime;	// �ő�Đ�����
		float		speed;		// �Đ����x
		bool		isLoop;		// ���[�v�w��
		Channels	channels;	// �ϊ����
	};
	using Animations = std::vector<Animation>;

public:
	Model();
	~Model();
	void Reset();
	void SetVertexShader(VertexShader* vs);
	void SetPixelShader(PixelShader* ps);
	bool Load(const char* file, float scale = 1.0f, Flip flip = Flip::None);
	void Draw(const std::vector<UINT>* order = nullptr, std::function<void(int)> func = nullptr);

	//--- �e����擾
	const Mesh* GetMesh(unsigned int index);
	uint32_t GetMeshNum();
	const Material* GetMaterial(unsigned int index);
	uint32_t GetMaterialNum();
	DirectX::XMMATRIX GetBone(NodeIndex index);
	const Animation* GetAnimation(AnimeNo no);

	//--- �A�j���[�V����
	// �A�j���[�V�����̓ǂݍ���
	AnimeNo AddAnimation(const char* file);
	// �A�j���[�V�����̍X�V
	void Step(float tick);

	// �A�j���[�V�����̍Đ�
	void Play(AnimeNo no, bool loop, float speed = 1.0f);
	// �A�j���[�V�����̃u�����h�Đ�
	void PlayBlend(AnimeNo no, float blendTime, bool loop, float speed = 1.0f);
	// �A�j���[�V�����̍����ݒ�
	void SetParametric(AnimeNo no1, AnimeNo no2);
	// �A�j���[�V�����̍��������ݒ�
	void SetParametricBlend(float blendRate);
	// �A�j���[�V�����̌��ݍĐ����Ԃ�ύX
	void SetAnimationTime(AnimeNo no, float time);

	// �Đ��t���O
	bool IsPlay(AnimeNo no);
	// ���ݍĐ����̃A�j���ԍ�
	AnimeNo GetPlayNo();
	// �u�����h���̃A�j���ԍ�
	AnimeNo GetBlendNo();

#ifdef _DEBUG
	static std::string GetError();
	void DrawBone();
#endif


private:
	// �e�퐶��
	void MakeMesh(const void* ptr, float scale, Flip flip);
	void MakeMaterial(const void* ptr, std::string directory);
	void MakeBoneNodes(const void* ptr);
	void MakeWeight(const void* ptr, int meshIdx);

	// �����v�Z
	bool AnimeNoCheck(AnimeNo no);
	void InitAnime(AnimeNo no);
	void CalcAnime(AnimeTransform kind, AnimeNo no);
	void UpdateAnime(AnimeNo no, float tick);
	void CalcBones(NodeIndex node, const DirectX::XMMATRIX parent);
	void LerpTransform(Transform* pOut, const Transform& a, const Transform& b, float rate);

private:
	static VertexShader*	m_pDefVS;		// �f�t�H���g���_�V�F�[�_�[
	static PixelShader*		m_pDefPS;		// �f�t�H���g�s�N�Z���V�F�[�_�[
	static unsigned int		m_shaderRef;	// �V�F�[�_�[�Q�Ɛ�
#ifdef _DEBUG
	static std::string m_errorStr;	
#endif

private:
	float			m_loadScale;	// 
	Flip			m_loadFlip;		// 

	Meshes			m_meshes;		// ���b�V���z��
	Materials		m_materials;	// �}�e���A���z��
	Nodes			m_nodes;		// �K�w���
	Animations		m_animes;		// �A�j���z��
	VertexShader*	m_pVS;			// �ݒ蒆�̒��_�V�F�[�_
	PixelShader*	m_pPS;			// �ݒ蒆�̃s�N�Z���V�F�[�_
	
	AnimeNo			m_playNo;			// ���ݍĐ����̃A�j���ԍ�
	AnimeNo			m_blendNo;			// �u�����h�Đ����s���A�j���ԍ�
	AnimeNo			m_parametric[2];	// �����Đ����s���A�j���ԍ�
	float			m_blendTime;		// ���݂̑J�ڌo�ߎ���
	float			m_blendTotalTime;	// �A�j���J�ڂɂ����鍇�v����
	float			m_parametricBlend;	// �p�����g���b�N�̍Đ�����

	Transforms		m_nodeTransform[MAX_TRANSFORM];	// �A�j���[�V�����ʕό`���
};


#endif // __MODEL_H__