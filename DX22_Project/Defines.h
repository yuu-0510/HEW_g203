#ifndef __DEFINES_H__
#define __DEFINES_H__

#include <assert.h>
#include <Windows.h>
#include <stdarg.h>
#include <stdio.h>

static const int FPS = 60;
static const float fFPS = static_cast<float>(FPS);
// ���\�[�X�p�X
#define ASSET(path)	"Assets/"path

// 3D��Ԓ�`
#define CMETER(value) (value * 0.01f)
#define METER(value) (value * 1.0f)
#define MSEC(value) (value / fFPS)
#define CMSEC(value) MSEC(CMETER(value))
static const float GRAVITY = 0.98f;

// �^�C�g��
static const char* APP_TITLE = "DX22_Golf";

// ��ʃT�C�Y
static const int SCREEN_WIDTH	= 1280;
static const int SCREEN_HEIGHT	= 720;

// �O���b�h�T�C�Y(�f�o�b�O�p
static const int	DEBUG_GRID_NUM		= 10;			// �O���b�h���S����[�܂ł̐��̖{��
static const float	DEBUG_GRID_MARGIN	= METER(1.0f);	// �O���b�h�z�u��


#endif // __DEFINES_H__