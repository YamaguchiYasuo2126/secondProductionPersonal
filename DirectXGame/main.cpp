#include "GameScene.h"
#include "GameTime.h"
#include "KamataEngine.h"

#include <Windows.h>

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	using namespace KamataEngine;

	KamataEngine::Initialize(L"斜め見下ろしアクション試作");
	GameTime::Initialize();
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();

	{
		GameScene gameScene;
		gameScene.Initialize();
		while (true)
		{
			if (KamataEngine::Update())
			{
				break;
			}
			GameTime::BeginFrame();
			imguiManager->Begin();
			gameScene.Update();
			imguiManager->End();

			dxCommon->PreDraw();
			gameScene.Draw();
			AxisIndicator::GetInstance()->Draw();
			imguiManager->Draw();
			dxCommon->PostDraw();
		}
	}

	KamataEngine::Finalize();
	return 0;
}
