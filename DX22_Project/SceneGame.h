#ifndef __SCENE_GAME_H__
#define __SCENE_GAME_H__
#include "Model.h"

class SceneGame
{
public:
	SceneGame();
	~SceneGame();
	void Update();
	void Draw();

private:
	Model* m_pModel;
};

#endif // __SCENE_GAME_H__