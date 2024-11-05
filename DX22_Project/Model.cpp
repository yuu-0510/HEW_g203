#include "Model.h"
#include "DirectXTex/TextureLoad.h"
#include <algorithm>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#ifdef _DEBUG
#include "Geometory.h"
#endif

#if _MSC_VER >= 1930
#ifdef _DEBUG
#pragma comment(lib, "assimp-vc143-mtd.lib")
#else
#pragma comment(lib, "assimp-vc143-mt.lib")
#endif
#elif _MSC_VER >= 1920
#ifdef _DEBUG
#pragma comment(lib, "assimp-vc142-mtd.lib")
#else
#pragma comment(lib, "assimp-vc142-mt.lib")
#endif
#elif _MSC_VER >= 1910
#ifdef _DEBUG
#pragma comment(lib, "assimp-vc141-mtd.lib")
#else
#pragma comment(lib, "assimp-vc141-mt.lib")
#endif
#endif

// static�����o�ϐ���`
VertexShader*	Model::m_pDefVS		= nullptr;
PixelShader*	Model::m_pDefPS		= nullptr;
unsigned int	Model::m_shaderRef	= 0;
#ifdef _DEBUG
std::string		Model::m_errorStr	= "";
#endif

/*
* @brief assimp���̍s���XMMATRIX�^�ɕϊ�
* @param[in] M assimp�̍s��
* @return �ϊ���̍s��
*/
DirectX::XMMATRIX GetMatrixFromAssimpMatrix(aiMatrix4x4 M)
{
	return DirectX::XMMatrixSet(
		M.a1, M.b1, M.c1, M.d1,
		M.a2, M.b2, M.c2, M.d2,
		M.a3, M.b3, M.c3, M.d3,
		M.a4, M.b4, M.c4, M.d4
	);
}

/*
* @brief �f�t�H���g�̃V�F�[�_�[���쐬
* @param[out] vs ���_�V�F�[�_�[�i�[��
* @param[out] ps �s�N�Z���V�F�[�_�[�i�[��
*/
void MakeModelDefaultShader(VertexShader** vs, PixelShader** ps)
{
	const char* ModelVS = R"EOT(
struct VS_IN {
	float3 pos : POSITION0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};
struct VS_OUT {
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};
VS_OUT main(VS_IN vin) {
	VS_OUT vout;
	vout.pos = float4(vin.pos, 1.0f);
	vout.pos.z += 0.5f;
	vout.pos.y -= 0.8f;
	vout.normal = vin.normal;
	vout.uv = vin.uv;
	return vout;
})EOT";
	const char* ModelPS = R"EOT(
struct PS_IN {
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};
Texture2D tex : register(t0);
SamplerState samp : register(s0);
float4 main(PS_IN pin) : SV_TARGET
{
	return tex.Sample(samp, pin.uv);
})EOT";
	*vs = new VertexShader();
	(*vs)->Compile(ModelVS);
	*ps = new PixelShader();
	(*ps)->Compile(ModelPS);
}



/*
* @brief �R���X�g���N�^
*/
Model::Model()
	: m_loadScale(1.0f)
	, m_loadFlip(None)
	, m_playNo(ANIME_NONE)
	, m_blendNo(ANIME_NONE)
	, m_parametric{ANIME_NONE, ANIME_NONE}
	, m_blendTime(0.0f)
	, m_blendTotalTime(0.0f)
	, m_parametricBlend(0.0f)
{
	// �f�t�H���g�V�F�[�_�[�̓K�p
	if (m_shaderRef == 0)
	{
		MakeModelDefaultShader(&m_pDefVS, &m_pDefPS);
	}
	m_pVS = m_pDefVS;
	m_pPS = m_pDefPS;
	++m_shaderRef;
}

/*
* @brief �f�X�g���N�^
*/
Model::~Model()
{
	Reset();
	--m_shaderRef;
	if (m_shaderRef <= 0)
	{
		delete m_pDefPS;
		delete m_pDefVS;
	}
}

/*
* @brief �����f�[�^�폜
*/
void Model::Reset()
{
	auto meshIt = m_meshes.begin();
	while (meshIt != m_meshes.end())
	{
		if (meshIt->pMesh) delete meshIt->pMesh;
		++meshIt;
	}

	auto matIt = m_materials.begin();
	while (matIt != m_materials.end())
	{
		if (matIt->pTexture) delete matIt->pTexture;
		++matIt;
	}
}

/*
* @brief ���_�V�F�[�_�[�ݒ�
*/
void Model::SetVertexShader(VertexShader* vs)
{
	m_pVS = vs;
}

/*
* @brief �s�N�Z���V�F�[�_�[�ݒ�
*/
void Model::SetPixelShader(PixelShader* ps)
{
	m_pPS = ps;
}

/*
* @brief ���f���f�[�^�ǂݍ���
* @param[in] file �ǂݍ��ރ��f���t�@�C���ւ̃p�X
* @param[in] scale ���f���̃T�C�Y�ύX
* @param[in] flip ���]�ݒ�
* @return �ǂݍ��݌���
*/
bool Model::Load(const char* file, float scale, Flip flip)
{
#ifdef _DEBUG
	m_errorStr = "";
#endif
	Reset();

	// assimp�̐ݒ�
	Assimp::Importer importer;
	int flag = 0;
	flag |= aiProcess_Triangulate;
	flag |= aiProcess_FlipUVs;
	if (flip == Flip::XFlip)  flag |= aiProcess_MakeLeftHanded;

	// assimp�œǂݍ���
	const aiScene* pScene = importer.ReadFile(file, flag);
	if (!pScene) {
#ifdef _DEBUG
		m_errorStr = importer.GetErrorString();
#endif
		return false;
	}

	// �ǂݍ��ݎ��̐ݒ��ۑ�
	m_loadScale = scale;
	m_loadFlip = flip;

	// �f�B���N�g���̓ǂݎ��
	std::string directory = file;
	auto strIt = directory.begin();
	while (strIt != directory.end()) {
		if (*strIt == '/')
			*strIt = '\\';
		++strIt;
	}
	directory = directory.substr(0, directory.find_last_of('\\') + 1);

	// �m�[�h�̍쐬
	MakeBoneNodes(pScene);
	// ���b�V���쐬
	MakeMesh(pScene, scale, flip);
	// �}�e���A���̍쐬
	MakeMaterial(pScene, directory);

	return true;
}

/*
* @brief �`��
* @param[in] order �`�揇��
* @param[in] func ���b�V���`��R�[���o�b�N
*/
void Model::Draw(const std::vector<UINT>* order, std::function<void(int)> func)
{
	// �V�F�[�_�[�ݒ�
	m_pVS->Bind();
	m_pPS->Bind();

	// �`�搔�ݒ�
	size_t drawNum = m_meshes.size();
	if (order)
	{
		drawNum = order->size();
	}

	// �`��
	for (UINT i = 0; i < drawNum; ++i)
	{
		// ���b�V���ԍ��ݒ�
		UINT meshNo = i;
		if (order)
		{
			meshNo = (*order)[i];
		}

		// �`��R�[���o�b�N
		if (func)
		{
			func(meshNo);
		}
		else
		{
			m_pPS->SetTexture(0, m_materials[m_meshes[meshNo].materialID].pTexture);
		}

		// �`��
		m_meshes[meshNo].pMesh->Draw();
	}
}

/*
* @brief ���b�V�����擾
* @param[in] index ���b�V���ԍ�
* @return �Y�����b�V�����
*/
const Model::Mesh* Model::GetMesh(unsigned int index)
{
	if (index < GetMeshNum())
	{
		return &m_meshes[index];
	}
	return nullptr;
}

/*
* @brief ���b�V�����擾
*/
uint32_t Model::GetMeshNum()
{
	return static_cast<uint32_t>(m_meshes.size());
}

/*
* @brief �}�e���A�����擾
* @param[in] index �}�e���A���ԍ�
* @return �Y���}�e���A�����
*/
const Model::Material* Model::GetMaterial(unsigned int index)
{
	if (index < GetMaterialNum())
	{
		return &m_materials[index];
	}
	return nullptr;
}

/*
* @brief �}�e���A�����擾
*/
uint32_t Model::GetMaterialNum()
{
	return static_cast<uint32_t>(m_materials.size());
}

/*
* @brief �A�j���[�V������̕ϊ��s��擾
* @param[in] index �{�[���ԍ�
* @return �Y���{�[���̕ϊ��s��
*/
DirectX::XMMATRIX Model::GetBone(NodeIndex index)
{
	if (index < m_nodes.size())
	{
		return m_nodes[index].mat;
	}
	return DirectX::XMMatrixIdentity();
}

/*
* @brief �A�j���[�V�������擾
* @param[in] no �A�j���ԍ�
* @return �Y���A�j���[�V�������
*/
const Model::Animation* Model::GetAnimation(AnimeNo no)
{
	if (AnimeNoCheck(no))
	{
		return &m_animes[no];
	}
	return nullptr;
}


/*
* @brief �A�j���[�V�����ǂݍ���
* @param[in] file �ǂݍ��ރA�j���[�V�����t�@�C���ւ̃p�X
* @return �����Ŋ��蓖�Ă�ꂽ�A�j���[�V�����ԍ�
*/
Model::AnimeNo Model::AddAnimation(const char* file)
{
#ifdef _DEBUG
	m_errorStr = "";
#endif

	// assimp�̐ݒ�
	Assimp::Importer importer;
	int flag = 0;
	flag |= aiProcess_Triangulate;
	flag |= aiProcess_FlipUVs;
	if (m_loadFlip == Flip::XFlip)  flag |= aiProcess_MakeLeftHanded;

	// assimp�œǂݍ���
	const aiScene* pScene = importer.ReadFile(file, flag);
	if (!pScene)
	{
#ifdef _DEBUG
		m_errorStr += importer.GetErrorString();
#endif
		return ANIME_NONE;
	}

	// �A�j���[�V�����`�F�b�N
	if (!pScene->HasAnimations())
	{
#ifdef _DEBUG
		m_errorStr += "no animation.";
#endif
		return ANIME_NONE;
	}

	// �A�j���[�V�����f�[�^�m��
	aiAnimation* assimpAnime = pScene->mAnimations[0];
	m_animes.push_back(Animation());
	Animation& anime = m_animes.back();

	// �A�j���[�V�����ݒ�
	float animeFrame = static_cast<float>(assimpAnime->mTicksPerSecond);
	anime.totalTime = static_cast<float>(assimpAnime->mDuration )/ animeFrame;
	anime.channels.resize(assimpAnime->mNumChannels);
	Channels::iterator channelIt = anime.channels.begin();
	while (channelIt != anime.channels.end())
	{
		// �Ή�����`�����l��(�{�[��)��T��
		uint32_t channelIdx = static_cast<uint32_t>(channelIt - anime.channels.begin());
		aiNodeAnim* assimpChannel = assimpAnime->mChannels[channelIdx];
		Model::Nodes::iterator nodeIt = std::find_if(m_nodes.begin(), m_nodes.end(),
			[assimpChannel](Node& node) {
				return node.name == assimpChannel->mNodeName.data;
			});
		if (nodeIt == m_nodes.end())
		{
			channelIt->index = INDEX_NONE;
			++ channelIt;
			continue;
		}

		// �e�L�[�̒l��ݒ�
		channelIt->index = static_cast<NodeIndex>(nodeIt - m_nodes.begin());
		Timeline& timeline = channelIt->timeline;

		// ��xXMVECTOR�^�Ŋi�[
		using XMVectorKey = std::pair<float, DirectX::XMVECTOR>;
		using XMVectorKeys = std::map<float, DirectX::XMVECTOR>;
		XMVectorKeys keys[3];
		// �ʒu
		for (UINT i = 0; i < assimpChannel->mNumPositionKeys; ++i)
		{
			aiVectorKey& key = assimpChannel->mPositionKeys[i];
			keys[0].insert(XMVectorKey(static_cast<float>(key.mTime) / animeFrame,
					DirectX::XMVectorSet(key.mValue.x, key.mValue.y, key.mValue.z, 0.0f)
			));
		}
		// ��]
		for (UINT i = 0; i < assimpChannel->mNumRotationKeys; ++i)
		{
			aiQuatKey& key = assimpChannel->mRotationKeys[i];
			keys[1].insert(XMVectorKey(static_cast<float>(key.mTime) / animeFrame,
				DirectX::XMVectorSet(key.mValue.x, key.mValue.y, key.mValue.z, key.mValue.w)));
		}
		// �g�k
		for (UINT i = 0; i < assimpChannel->mNumScalingKeys; ++i)
		{
			aiVectorKey& key = assimpChannel->mScalingKeys[i];
			keys[2].insert(XMVectorKey(static_cast<float>(key.mTime) / animeFrame,
				DirectX::XMVectorSet(key.mValue.x, key.mValue.y, key.mValue.z, 0.0f)));
		}

		// �e�^�C�����C���̐擪�̎Q�Ƃ�ݒ�
		XMVectorKeys::iterator it[] = {keys[0].begin(), keys[1].begin(), keys[2].begin()};
		for (int i = 0; i < 3; ++i)
		{
			// �L�[��������Ȃ��ꍇ�́A�Q�ƏI��
			if (keys[i].size() == 1)
				++ it[i];
		}

		// �e�v�f���Ƃ̃^�C�����C���ł͂Ȃ��A���ׂĂ̕ϊ����܂߂��^�C�����C���̍쐬
		while (it[0] != keys[0].end() && it[1] != keys[1].end() && it[2] != keys[2].end())
		{
			// ����̎Q�ƈʒu�ň�ԏ��������Ԃ��擾
			float time = anime.totalTime;
			for (int i = 0; i < 3; ++i)
			{
				if (it[i] != keys[i].end())
				{
					time = std::min(it[i]->first, time);
				}
			}

			// ���ԂɊ�Â��ĕ�Ԓl���v�Z
			DirectX::XMVECTOR result[3];
			for (int i = 0; i < 3; ++i)
			{
				// �擪�̃L�[��菬�������Ԃł���΁A�擪�̒l��ݒ�
				if (time < keys[i].begin()->first)
				{
					result[i] = keys[i].begin()->second;
				}
				// �ŏI�L�[���傫�����Ԃł���΁A�ŏI�̒l��ݒ�
				else if (keys[i].rbegin()->first <= time)
				{
					result[i] = keys[i].rbegin()->second;
					it[i] = keys[i].end();
				}
				// �L�[���m�ɋ��܂ꂽ���Ԃł���΁A��Ԓl���v�Z
				else
				{
					// �Q�Ƃ��Ă��鎞�ԂƓ����ł���΁A���̎Q�ƂփL�[��i�߂�
					if (it[i]->first <= time)
					{
						++it[i];
					}

					// ��Ԓl�̌v�Z
					XMVectorKeys::iterator prev = it[i];
					--prev;
					float rate = (time - prev->first) / (it[i]->first - prev->first);
					result[i] = DirectX::XMVectorLerp(prev->second, it[i]->second, rate);
				}
			}

			// �w�莞�ԂɊ�Â����L�[��ǉ�
			Transform transform;
			DirectX::XMStoreFloat3(&transform.translate, result[0]);
			DirectX::XMStoreFloat4(&transform.quaternion, result[1]);
			DirectX::XMStoreFloat3(&transform.scale, result[2]);
			timeline.insert(Key(time, transform));
		}

		++ channelIt;
	}

	// �A�j���ԍ���Ԃ�
	return static_cast<AnimeNo>(m_animes.size() - 1);
}

/*
* @brief �A�j���[�V�����̍X�V����
* @param[in] tick �A�j���[�V�����o�ߎ���
*/
void Model::Step(float tick)
{
	// �A�j���[�V�����̍Đ��m�F
	if (m_playNo == ANIME_NONE) { return; }

	//--- �A�j���[�V�����s��̍X�V
	// �p�����g���b�N
	if (m_playNo == PARAMETRIC_ANIME || m_blendNo == PARAMETRIC_ANIME)
	{
		CalcAnime(PARAMETRIC0, m_parametric[0]);
		CalcAnime(PARAMETRIC1, m_parametric[1]);
	}
	// ���C���A�j��
	if (m_playNo != ANIME_NONE && m_playNo != PARAMETRIC_ANIME)
	{
		CalcAnime(MAIN, m_playNo);
	}
	// �u�����h�A�j��
	if (m_blendNo != ANIME_NONE && m_blendNo != PARAMETRIC_ANIME)
	{
		CalcAnime(BLEND, m_blendNo);
	}

	// �A�j���[�V�����s��Ɋ�Â��č��s����X�V
	CalcBones(0, DirectX::XMMatrixScaling(m_loadScale, m_loadScale, m_loadScale));

	//--- �A�j���[�V�����̎��ԍX�V
	// ���C���A�j��
	UpdateAnime(m_playNo, tick);
	// �u�����h�A�j��
	if (m_blendNo != ANIME_NONE)
	{
		UpdateAnime(m_blendNo, tick);
		m_blendTime += tick;
		if (m_blendTime <= m_blendTime)
		{
			// �u�����h�A�j���̎����I��
			m_blendTime = 0.0f;
			m_blendTotalTime = 0.0f;
			m_playNo = m_blendNo;
			m_blendNo = ANIME_NONE;
		}
	}
	// �p�����g���b�N
	if (m_playNo == PARAMETRIC_ANIME || m_blendNo == PARAMETRIC_ANIME)
	{
		UpdateAnime(m_parametric[0], tick);
		UpdateAnime(m_parametric[1], tick);
	}
}

/*
* @brief �A�j���[�V�����Đ�
* @param[in] no �Đ�����A�j���[�V�����ԍ�
* @param[in] loop ���[�v�Đ��t���O
* @param[in] speed �Đ����x
*/
void Model::Play(AnimeNo no, bool loop, float speed)
{
	// �Đ��`�F�b�N
	if (!AnimeNoCheck(no)) { return; }
	if (m_playNo == no) { return; }

	// �����A�j���[�V�������`�F�b�N
	if (no != PARAMETRIC_ANIME)
	{
		// �ʏ�̏�����
		InitAnime(no);
		m_animes[no].isLoop = loop;
		m_animes[no].speed = speed;
	}
	else
	{
		// �����A�j���[�V�����̌��ɂȂ��Ă���A�j���[�V������������
		InitAnime(m_parametric[0]);
		InitAnime(m_parametric[1]);
		m_animes[m_parametric[0]].isLoop = loop;
		m_animes[m_parametric[1]].isLoop = loop;
		SetParametricBlend(0.0f);
	}

	// �Đ��A�j���[�V�����̐ݒ�
	m_playNo = no;
}

/*
* @brief �u�����h�Đ�
* @param[in] no �A�j���[�V�����ԍ�
* @param[in] blendTime �u�����h�Ɋ|���鎞��
* @param[in] loop ���[�v�t���O
* @param[in] speed �Đ����x
*/
void Model::PlayBlend(AnimeNo no, float blendTime, bool loop, float speed)
{
	// �Đ��`�F�b�N
	if (!AnimeNoCheck(no)) { return; }

	// �����A�j���[�V�������`�F�b�N
	if (no != PARAMETRIC_ANIME)
	{
		InitAnime(no);
		m_animes[no].isLoop = loop;
		m_animes[no].speed = speed;
	}
	else
	{
		// �����A�j���[�V�����̌��ɂȂ��Ă���A�j���[�V������������
		InitAnime(m_parametric[0]);
		InitAnime(m_parametric[1]);
		m_animes[m_parametric[0]].isLoop = loop;
		m_animes[m_parametric[1]].isLoop = loop;
		SetParametricBlend(0.0f);
	}

	// �u�����h�̐ݒ�
	m_blendTime = 0.0f;
	m_blendTotalTime = blendTime;
	m_blendNo = no;
}

/*
* @brief �������A�j���[�V�����̐ݒ�
* @param[in] no1 �������A�j��1
* @param[in] no2 �������A�j��2
*/
void Model::SetParametric(AnimeNo no1, AnimeNo no2)
{
	// �A�j���[�V�����`�F�b�N
	if (!AnimeNoCheck(no1)) { return; }
	if (!AnimeNoCheck(no2)) { return; }

	// �����ݒ�
	m_parametric[0] = no1;
	m_parametric[1] = no2;
	SetParametricBlend(0.0f);
}

/*
* @brief �A�j���[�V�����̍��������ݒ�
* @param[in] blendRate ��������
*/
void Model::SetParametricBlend(float blendRate)
{
	// �������A�j�����ݒ肳��Ă��邩�m�F
	if (m_parametric[0] == ANIME_NONE || m_parametric[1] == ANIME_NONE) return;

	// ���������ݒ�
	m_parametricBlend = blendRate;

	// �����Ɋ�Â��ăA�j���[�V�����̍Đ����x��ݒ�
	Animation& anime1 = m_animes[m_parametric[0]];
	Animation& anime2 = m_animes[m_parametric[1]];
	float blendTotalTime = anime1.totalTime * (1.0f - m_parametricBlend) + anime2.totalTime * m_parametricBlend;
	anime1.speed = anime1.totalTime / blendTotalTime;
	anime2.speed = anime2.totalTime / blendTotalTime;
}

/*
* @brief �A�j���[�V�����̍Đ����Ԃ�ύX
* @param[in] no �ύX����A�j��
* @param[in] time �V�����Đ�����
*/
void Model::SetAnimationTime(AnimeNo no, float time)
{
	// �A�j���[�V�����`�F�b�N
	if (!AnimeNoCheck(no)) { return; }

	// �Đ����ԕύX
	Animation& anime = m_animes[no];
	anime.nowTime = time;
	while (anime.nowTime >= anime.totalTime)
	{
		anime.nowTime -= anime.totalTime;
	}
}

/*
* @brief �Đ��t���O�̎擾
* @param[in] no ���ׂ�A�j���ԍ�
* @return ���ݍĐ����Ȃ�true
*/
bool Model::IsPlay(AnimeNo no)
{
	// �A�j���[�V�����`�F�b�N
	if (!AnimeNoCheck(no)) { return false; }

	// �p�����g���b�N�͍������̃A�j������ɔ��f
	if (no == PARAMETRIC_ANIME) { no = m_parametric[0]; }

	// �Đ����Ԃ̔���
	if (m_animes[no].totalTime < m_animes[no].nowTime) { return false; }

	// ���ꂼ��̍Đ��ԍ��ɐݒ肳��Ă��邩�m�F
	if (m_playNo == no) { return true; }
	if (m_blendNo == no) { return true; }
	if (m_playNo == PARAMETRIC_ANIME || m_blendNo == PARAMETRIC_ANIME)
	{
		if (m_parametric[0] == no) { return true; }
		if (m_parametric[1] == no) { return true; }
	}

	// �Đ����łȂ�
	return false;
}

/*
* @brief �Đ����̔ԍ��̎擾
* @return �A�j���ԍ�
*/
Model::AnimeNo Model::GetPlayNo()
{
	return m_playNo;
}

/*
* @brief �Đ����̃u�����h�A�j���̎擾
* @return �A�j���ԍ�
*/
Model::AnimeNo Model::GetBlendNo()
{
	return m_blendNo;
}


#ifdef _DEBUG

/*
* @brief �G���[���b�Z�[�W�擾
* @returnn �G���[���b�Z�[�W
*/
std::string Model::GetError()
{
	return m_errorStr;
}

/*
* @brief �{�[���f�o�b�O�`��
*/
void Model::DrawBone()
{
	// �ċA����
	std::function<void(int, DirectX::XMFLOAT3)> FuncDrawBone =
		[&FuncDrawBone, this](int idx, DirectX::XMFLOAT3 parent)
	{
		// �e�m�[�h���猻�݈ʒu�܂ŕ`��
		DirectX::XMFLOAT3 pos;
		DirectX::XMStoreFloat3(&pos, DirectX::XMVector3TransformCoord(DirectX::XMVectorZero(), m_nodes[idx].mat));
		Geometory::AddLine(parent, pos, DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));

		// �q�m�[�h�̕`��
		auto it = m_nodes[idx].children.begin();
		while (it != m_nodes[idx].children.end())
		{
			FuncDrawBone(*it, pos);
			++it;
		}
	};

	// �`����s
	FuncDrawBone(0, DirectX::XMFLOAT3());
	Geometory::DrawLines();
}

#endif


void Model::MakeBoneNodes(const void* ptr)
{
	// �ċA������Assimp�̃m�[�h����ǂݎ��
	std::function<NodeIndex(aiNode*, NodeIndex, DirectX::XMMATRIX mat)> FuncAssimpNodeConvert =
		[&FuncAssimpNodeConvert, this](aiNode* assimpNode, NodeIndex parent, DirectX::XMMATRIX mat)
	{
		DirectX::XMMATRIX transform = GetMatrixFromAssimpMatrix(assimpNode->mTransformation);
		std::string name = assimpNode->mName.data;
		if (name.find("$AssimpFbx") != std::string::npos)
		{
			mat = transform * mat;
			return FuncAssimpNodeConvert(assimpNode->mChildren[0], parent, mat);
		}
		else
		{
			// Assimp�̃m�[�h�������f���N���X�֊i�[
			Node node;
			node.name = assimpNode->mName.data;
			node.parent = parent;
			node.children.resize(assimpNode->mNumChildren);
			node.mat = mat;

			// �m�[�h���X�g�ɒǉ�
			m_nodes.push_back(node);
			NodeIndex nodeIndex = static_cast<NodeIndex>(m_nodes.size() - 1);

			// �q�v�f�����l�ɕϊ�
			for (UINT i = 0; i < assimpNode->mNumChildren; ++i)
			{
				m_nodes[nodeIndex].children[i] = FuncAssimpNodeConvert(
					assimpNode->mChildren[i], nodeIndex, DirectX::XMMatrixIdentity());
			}
			return nodeIndex;
		}
	};

	// �m�[�h�쐬
	m_nodes.clear();
	FuncAssimpNodeConvert(reinterpret_cast<const aiScene*>(ptr)->mRootNode, INDEX_NONE, DirectX::XMMatrixIdentity());

	// �A�j���[�V�����v�Z�̈�ɁA�m�[�h�����̏����f�[�^���쐬
	Transform init = {
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f)
	};
	for (int i = 0; i < MAX_TRANSFORM; ++i)
	{
		m_nodeTransform[i].resize(m_nodes.size(), init);
	}
}

void Model::MakeWeight(const void* ptr, int meshIdx)
{
	const aiScene* pScene = reinterpret_cast<const aiScene*>(ptr);

	// ���b�V���ɑΉ�����{�[�������邩
	aiMesh* assimpMesh = pScene->mMeshes[meshIdx];
	Mesh& mesh = m_meshes[meshIdx];
	if (assimpMesh->HasBones())
	{
		// ���b�V�����̒��_�̈�쐬
		struct WeightPair
		{
			unsigned int idx;
			float weight;
		};
		std::vector<std::vector<WeightPair>> weights;
		weights.resize(mesh.vertices.size());


		// ���b�V���Ɋ��蓖�Ă��Ă���{�[���̈�m��
		mesh.bones.resize(assimpMesh->mNumBones);
		for (auto boneIt = mesh.bones.begin(); boneIt != mesh.bones.end(); ++boneIt)
		{
			UINT boneIdx = static_cast<UINT>(boneIt - mesh.bones.begin());
			aiBone* assimpBone = assimpMesh->mBones[boneIdx];
			// �\�z�ς݂̃{�[���m�[�h����Y���m�[�h���擾
			std::string boneName = assimpBone->mName.data;
			auto nodeIt = std::find_if(m_nodes.begin(), m_nodes.end(),
				[boneName](const Node& val) {
				return val.name == boneName;
			});
			// ���b�V���Ɋ��蓖�Ă��Ă���{�[�����A�m�[�h�ɑ��݂��Ȃ�
			if (nodeIt == m_nodes.end())
			{
				boneIt->index = INDEX_NONE;
				continue;
			}

			// ���b�V���̃{�[���ƃm�[�h�̕R�Â�
			boneIt->index = static_cast<NodeIndex>(nodeIt - m_nodes.begin());
			boneIt->invOffset = GetMatrixFromAssimpMatrix(assimpBone->mOffsetMatrix);
			boneIt->invOffset.r[3].m128_f32[0] *= m_loadScale;
			boneIt->invOffset.r[3].m128_f32[1] *= m_loadScale;
			boneIt->invOffset.r[3].m128_f32[2] *= m_loadScale;
			boneIt->invOffset =
				DirectX::XMMatrixScaling(m_loadFlip == ZFlipUseAnime ? -1.0f : 1.0f, 1.0f, 1.0f) *
				boneIt->invOffset * 
				DirectX::XMMatrixScaling(1.f / m_loadScale, 1.f / m_loadScale, 1.f / m_loadScale);

			// �E�F�C�g�̐ݒ�
			UINT weightNum = assimpBone->mNumWeights;
			for (UINT i = 0; i < weightNum; ++i)
			{
				aiVertexWeight weight = assimpBone->mWeights[i];
				weights[weight.mVertexId].push_back({boneIdx, weight.mWeight});
			}
		}

		// �擾���Ă������_�E�F�C�g��ݒ�
		for (int i = 0; i < weights.size(); ++i)
		{
			if (weights[i].size() >= 4)
			{
				std::sort(weights[i].begin(), weights[i].end(), [](WeightPair& a, WeightPair& b) {
					return a.weight > b.weight;
				});
				// �E�F�C�g��4�ɍ��킹�Đ��K��
				float total = 0.0f;
				for (int j = 0; j < 4; ++j)
					total += weights[i][j].weight;
				for (int j = 0; j < 4; ++j)
					weights[i][j].weight /= total;
			}
			for (int j = 0; j < weights[i].size() && j < 4; ++j)
			{
				mesh.vertices[i].index[j] = weights[i][j].idx;
				mesh.vertices[i].weight[j] = weights[i][j].weight;
			}
		}
	}
	else
	{
		// ���b�V���̐e�m�[�h���g�����X�t�H�[�����Ƃ��Čv�Z
		std::string nodeName = assimpMesh->mName.data;
		auto nodeIt = std::find_if(m_nodes.begin(), m_nodes.end(),
			[nodeName](const Node& val) {
				return val.name == nodeName;
			});
		if (nodeIt == m_nodes.end())
		{
			return;	// �{�[���f�[�^�Ȃ�
		}

		// ���b�V���łȂ��e�m�[�h���ċA�T��
		std::function<int(int)> FuncFindNode =
			[&FuncFindNode, this, pScene](NodeIndex parent)
		{
			std::string name = m_nodes[parent].name;
			for (UINT i = 0; i < pScene->mNumMeshes; ++i)
			{
				if (name == pScene->mMeshes[i]->mName.data)
				{
					return FuncFindNode(m_nodes[parent].parent);
				}
			}
			return parent;
		};

		Bone bone;
		bone.index = FuncFindNode(nodeIt->parent);
		bone.invOffset = DirectX::XMMatrixInverse(nullptr, m_nodes[bone.index].mat);
		for (auto vtxIt = mesh.vertices.begin(); vtxIt != mesh.vertices.end(); ++vtxIt)
		{
			vtxIt->weight[0] = 1.0f;
		}

		mesh.bones.resize(1);
		mesh.bones[0] = bone;
	}
}



bool Model::AnimeNoCheck(AnimeNo no)
{
	// �p�����g���b�N�A�j���[�V�����m�F
	if (no == PARAMETRIC_ANIME)
	{
		// �p�����g���b�N�̃A�j���[�V�����������������ݒ肳��Ă��邩
		return
			m_parametric[0] != ANIME_NONE &&
			m_parametric[1] != ANIME_NONE;
	}
	else
	{
		// ���Ȃ��A�j���[�V�����ԍ����ǂ���
		return 0 <= no && no < m_animes.size();
	}
}
void Model::InitAnime(AnimeNo no)
{
	// �A�j���̐ݒ�Ȃ��A�p�����g���b�N�Őݒ肳��Ă���Ȃ珉�������Ȃ�
	if (no == ANIME_NONE || no == PARAMETRIC_ANIME) { return; }

	Animation& anime = m_animes[no];
	anime.nowTime = 0.0f;
	anime.speed = 1.0f;
	anime.isLoop = false;
}
void Model::CalcAnime(AnimeTransform kind, AnimeNo no)
{
	Animation& anime = m_animes[no];
	Channels::iterator channelIt = anime.channels.begin();
	while (channelIt != anime.channels.end())
	{
		// ��v����{�[�����Ȃ���΃X�L�b�v
		Timeline& timeline = channelIt->timeline;
		if (channelIt->index == INDEX_NONE || timeline.empty())
		{
			++channelIt;
			continue;
		}

		//--- �Y���m�[�h�̎p�����A�j���[�V�����ōX�V
		Transform& transform = m_nodeTransform[kind][channelIt->index];
		if (timeline.size() <= 1)
		{
			// �L�[��������Ȃ��̂Œl�����̂܂܎g�p
			transform = channelIt->timeline[0];
		}
		else
		{
			Timeline::iterator startIt = timeline.begin();
			if (anime.nowTime <= startIt->first)
			{
				// �擪�L�[�����O�̎��ԂȂ�A�擪�̒l���g�p
				transform = startIt->second;
			}
			else if (timeline.rbegin()->first <= anime.nowTime)
			{
				// �ŏI�L�[������̎��ԂȂ�A�Ō�̒l���g�p
				transform = timeline.rbegin()->second;
			}
			else
			{
				// �w�肳�ꂽ���Ԃ�����2�̃L�[����A��Ԃ��ꂽ�l���v�Z
				Timeline::iterator nextIt = timeline.upper_bound(anime.nowTime);
				startIt = nextIt;
				--startIt;
				float rate = (anime.nowTime - startIt->first) / (nextIt->first - startIt->first);
				LerpTransform(&transform, startIt->second, nextIt->second, rate);
			}
		}

		++channelIt;
	}
}
void Model::UpdateAnime(AnimeNo no, float tick)
{
	if (no == PARAMETRIC_ANIME) { return; }

	Animation& anime = m_animes[no];
	anime.nowTime += anime.speed * tick;
	if (anime.isLoop)
	{
		while (anime.nowTime >= anime.totalTime)
		{
			anime.nowTime -= anime.totalTime;
		}
	}
}
void Model::CalcBones(NodeIndex index, const DirectX::XMMATRIX parent)
{
	//--- �A�j���[�V�������Ƃ̃p�����[�^������
	Transform transform;
	// �p�����g���b�N
	if (m_playNo == PARAMETRIC_ANIME || m_blendNo == PARAMETRIC_ANIME)
	{
		LerpTransform(&transform, m_nodeTransform[PARAMETRIC0][index], m_nodeTransform[PARAMETRIC1][index], m_parametricBlend);
		if (m_playNo == PARAMETRIC_ANIME)
		{
			m_nodeTransform[MAIN][index] = transform;
		}
		if (m_blendNo == PARAMETRIC_ANIME)
		{
			m_nodeTransform[BLEND][index] = transform;
		}
	}
	// �u�����h�A�j��
	if (m_blendNo != ANIME_NONE)
	{
		LerpTransform(&transform, m_nodeTransform[MAIN][index], m_nodeTransform[BLEND][index], m_blendTime / m_blendTotalTime);
	}
	else
	{
		// ���C���A�j���̂�
		transform = m_nodeTransform[MAIN][index];
	}

	// �Y���m�[�h�̎p���s����v�Z
	Node& node = m_nodes[index];
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&transform.translate));
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&transform.quaternion));
	DirectX::XMMATRIX S = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&transform.scale));
	node.mat = (S * R * T) * parent;

	// �q�v�f�̎p�����X�V
	Children::iterator it = node.children.begin();
	while (it != node.children.end())
	{
		CalcBones(*it, node.mat);
		++it;
	}
}

void Model::LerpTransform(Transform* pOut, const Transform& a, const Transform& b, float rate)
{
	DirectX::XMVECTOR vec[][2] = {
		{ DirectX::XMLoadFloat3(&a.translate),	DirectX::XMLoadFloat3(&b.translate) },
		{ DirectX::XMLoadFloat4(&a.quaternion),	DirectX::XMLoadFloat4(&b.quaternion) },
		{ DirectX::XMLoadFloat3(&a.scale),		DirectX::XMLoadFloat3(&b.scale) },
	};
	for (int i = 0; i < 3; ++i)
	{
		vec[i][0] = DirectX::XMVectorLerp(vec[i][0], vec[i][1], rate);
	}
	DirectX::XMStoreFloat3(&pOut->translate, vec[0][0]);
	DirectX::XMStoreFloat4(&pOut->quaternion, vec[1][0]);
	DirectX::XMStoreFloat3(&pOut->scale, vec[2][0]);
}

