#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


void Model::MakeMesh(const void* ptr, float scale, Flip flip)
{
	// ���O����
	aiVector3D zero3(0.0f, 0.0f, 0.0f);
	aiColor4D one4(1.0f, 1.0f, 1.0f, 1.0f);
	const aiScene* pScene = reinterpret_cast<const aiScene*>(ptr);
	float xFlip = flip == Flip::XFlip ? -1.0f : 1.0f;
	float zFlip = (flip == Flip::ZFlip || flip == Flip::ZFlipUseAnime) ? -1.0f : 1.0f;
	int idx1 = (flip == Flip::XFlip || flip == Flip::ZFlip) ? 2 : 1;
	int idx2 = (flip == Flip::XFlip || flip == Flip::ZFlip) ? 1 : 2;

	// ���b�V���̍쐬
	m_meshes.resize(pScene->mNumMeshes);
	for (unsigned int i = 0; i < m_meshes.size(); ++i)
	{
		// ���_�������ݐ�̗̈��p��
		m_meshes[i].vertices.resize(pScene->mMeshes[i]->mNumVertices);

		// ���_�f�[�^�̏�������
		for (unsigned int j = 0; j < m_meshes[i].vertices.size(); ++j) {
			// �����f���f�[�^����l�̎擾
			aiVector3D pos		= pScene->mMeshes[i]->mVertices[j];   //  �ǂ̃��f�������W�͎���
			aiVector3D normal	= pScene->mMeshes[i]->HasNormals() ?   //  �f�[�^�̂Ȃ����f�������邽�ߔ���
									pScene->mMeshes[i]->mNormals[j] : zero3;
			aiVector3D uv		= pScene->mMeshes[i]->HasTextureCoords(0) ? //  �f�[�^�̂Ȃ����f�������邽�ߔ���
									pScene->mMeshes[i]->mTextureCoords[0][j] : zero3;
			aiColor4D color		= pScene->mMeshes[i]->HasVertexColors(0) ? //  �f�[�^�̂Ȃ����f�������邽�ߔ���
									pScene->mMeshes[i]->mColors[0][j] : one4;
			//  �l��ݒ�
			m_meshes[i].vertices[j] = {
			 DirectX::XMFLOAT3(pos.x * scale * xFlip, pos.y * scale, pos.z * scale * zFlip),
			 DirectX::XMFLOAT3(normal.x, normal.y, normal.z),
			 DirectX::XMFLOAT2(uv.x, uv.y),
			 DirectX::XMFLOAT4(color.r, color.g, color.b, color.a)
			};
		}

		// �{�[������
		MakeWeight(pScene, i);

		// �C���f�b�N�X�̏������ݐ�̗p��
		// mNumFaces�̓|���S���̐���\��(�P�|���S����3�C���f�b�N�X
		m_meshes[i].indices.resize(pScene->mMeshes[i]->mNumFaces * 3);

		// �C���f�b�N�X�̏�������
		for (unsigned int j = 0; j < pScene->mMeshes[i]->mNumFaces; ++j) {
			// �����f���f�[�^����l�̎擾
			aiFace face = pScene->mMeshes[i]->mFaces[j];  // face�̓|���S���f�[�^���h��
			// ���l��ݒ�
			int idx = j * 3;  //�|���S���͎O�p�`�ō\������邽��1�|���S����3�C���f�N�b�X�܂܂��
			m_meshes[i].indices[idx + 0] = face.mIndices[0];
			m_meshes[i].indices[idx + 1] = face.mIndices[idx1];
			m_meshes[i].indices[idx + 2] = face.mIndices[idx2];
		}

		// �}�e���A���̊��蓖��
		m_meshes[i].materialID = pScene->mMeshes[i]->mMaterialIndex;

		// �����_�o�b�t�@�ɕK�v�ȃf�[�^��ݒ�
		MeshBuffer::Description desc = {};
		desc.pVtx		= m_meshes[i].vertices.data(); //  �������ݍς݂̒��_�f�[�^
		desc.vtxSize	= sizeof(Vertex);    // 1���_������̃f�[�^�T�C�Y
		desc.vtxCount	= m_meshes[i].vertices.size(); //  ���b�V���Ŏg�p���Ă��钸�_�̐�
		desc.pIdx		= m_meshes[i].indices.data(); //  �������ݍς݂̃C���f�b�N�X�f�[�^
		desc.idxSize	= sizeof(unsigned long);   // 1�C���f�b�N�X������̃f�[�^�T�C�Y 
		desc.idxCount	= m_meshes[i].indices.size(); //  ���b�V���Ŏg�p���Ă���C���f�b�N�X
		desc.topology	= D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		// �����_�o�b�t�@�쐬
		m_meshes[i].pMesh = new MeshBuffer();
		m_meshes[i].pMesh->Create(desc);
	}
}
void Model::MakeMaterial(const void* ptr, std::string directory)
{
	// ���O����
	aiColor3D color(0.0f, 0.0f, 0.0f);
	float shininess;
	const aiScene* pScene = reinterpret_cast<const aiScene*>(ptr);

	// �}�e���A���̍쐬
	m_materials.resize(pScene->mNumMaterials);
	for (unsigned int i = 0; i < m_materials.size(); ++i)
	{
		//--- �e��}�e���A���p�����[�^�[�̓ǂݎ��
		// ���g�U���̓ǂݎ��
		if (pScene->mMaterials[i]->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
			m_materials[i].diffuse = DirectX::XMFLOAT4(color.r, color.g, color.b, 1.0f);
		else
			m_materials[i].diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		// �������̓ǂݎ��
		if (pScene->mMaterials[i]->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS)
			m_materials[i].ambient = DirectX::XMFLOAT4(color.r, color.g, color.b, 1.0f);
		else
			m_materials[i].ambient = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
		// �����ˌ��̓ǂݎ��
		if (pScene->mMaterials[i]->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
			m_materials[i].specular = DirectX::XMFLOAT4(color.r, color.g, color.b, 0.0f);
		else
			m_materials[i].specular = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		// �����ˌ��̋�����ǂݎ��
		if (pScene->mMaterials[i]->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
			m_materials[i].specular.w = shininess;
		// �e�N�X�`���ǂݍ��ݏ���
		HRESULT hr;
		aiString path;

		// �e�N�X�`���̃p�X����ǂݍ���
		m_materials[i].pTexture = nullptr;
		if (pScene->mMaterials[i]->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path) != AI_SUCCESS) {
			continue;
		}

		// �e�N�X�`���̈�m��
		m_materials[i].pTexture = new Texture;

		// ���̂܂ܓǂݍ���
		hr = m_materials[i].pTexture->Create(path.C_Str());
		if (SUCCEEDED(hr)) { continue; }

		// �f�B���N�g���ƘA�����ĒT��
		hr = m_materials[i].pTexture->Create((directory + path.C_Str()).c_str());
		if (SUCCEEDED(hr)) { continue; }

		// ���f���Ɠ����K�w��T��
		// �p�X����t�@�C�����̂ݎ擾
		std::string fullPath = path.C_Str();
		std::string::iterator strIt = fullPath.begin();
		while (strIt != fullPath.end()) {
			if (*strIt == '/')
				*strIt = '\\';
			++strIt;
		}
		size_t find = fullPath.find_last_of("\\");
		std::string fileName = fullPath;
		if (find != std::string::npos)
			fileName = fileName.substr(find + 1);
		// �e�N�X�`���̓Ǎ�
		hr = m_materials[i].pTexture->Create((directory + fileName).c_str());
		if (SUCCEEDED(hr)) { continue; }

		// �e�N�X�`����������Ȃ�����
		delete m_materials[i].pTexture;
		m_materials[i].pTexture = nullptr;
#ifdef _DEBUG
		m_errorStr += path.C_Str();
#endif
	}
}