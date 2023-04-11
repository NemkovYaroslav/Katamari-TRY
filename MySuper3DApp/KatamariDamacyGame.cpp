#include "KatamariDamacyGame.h"
#include "GameObject.h"
#include "CameraComponent.h"
#include "CameraControllerComponent.h"
#include "KatamariControllerComponent.h"
#include "DirectionalLightComponent.h"
#include "ModelComponent.h"

KatamariDamacyGame::KatamariDamacyGame(LPCWSTR name, int clientWidth, int clientHeight) : Game(name, clientWidth, clientHeight)
{
	Game::CreateInstance(name, clientWidth, clientHeight);
	Initialize();
}

void KatamariDamacyGame::Initialize()
{
	GameObject* ground = new GameObject();
	ground->CreatePlane(25.0f, "../Textures/ice.png");
	ground->transformComponent->SetRotation(Quaternion::CreateFromAxisAngle(DirectX::SimpleMath::Vector3::Right, -DirectX::XM_PIDIV2));

	GameObject* katamari = new GameObject();
	katamari->CreateMesh(50.0f, "../Textures/katamari.png", "../Models/katamari.obj");
	katamari->transformComponent->SetPosition(Vector3(0, 1, 0));
	KatamariControllerComponent* katamariController = new KatamariControllerComponent();
	katamariController->katamariSpeed = 5.0f;
	katamari->AddComponent(katamariController);
	katamari->transformComponent->SetPosition(Vector3(0.0f, 1.0f, 15.0f));
	katamari->modelComponent->material.ambient = { 0.2f, 0.5f, 0.1f };
	katamari->modelComponent->material.diffuse = { 0.2f, 0.5f, 0.1f };
	katamari->modelComponent->material.specular = { 0.2f, 0.5f, 0.1f };

	GameObject* camera = new GameObject();
	CameraComponent* cameraComponent = new CameraComponent();
	CameraArmControllerComponent* armCameraController = new CameraArmControllerComponent();
	camera->AddComponent(cameraComponent);
	armCameraController->aim = katamari->transformComponent;
	camera->AddComponent(armCameraController);
	Game::GetInstance()->currentCamera = cameraComponent;
	katamariController->cameraTransform = camera->transformComponent;

	GameObject* removeLight = new GameObject();
	DirectionalLightComponent* directionalLightComponent = new DirectionalLightComponent(1024, 40.0f, 40.0f, 0.1f, 200.0f);
	removeLight->AddComponent(directionalLightComponent);
	Game::GetInstance()->currentLight = directionalLightComponent;

	Game::GetInstance()->AddGameObject(ground);      // 0
	Game::GetInstance()->AddGameObject(camera);      // 1
	Game::GetInstance()->AddGameObject(katamari);    // 2
	Game::GetInstance()->AddGameObject(removeLight); // 3

	GameObject* statue = new GameObject();
	statue->CreateMesh(0.02f, "../Textures/bull.jpg", "../Models/bull.obj");
	statue->transformComponent->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
	statue->transformComponent->SetRotation(
		Quaternion::CreateFromAxisAngle(DirectX::SimpleMath::Vector3::Right, DirectX::XM_PIDIV2)
		* 
		Quaternion::CreateFromAxisAngle(DirectX::SimpleMath::Vector3::Up, DirectX::XM_PI)
	);
	Game::GetInstance()->AddGameObject(statue);

	GameObject* spider0 = new GameObject();
	spider0->CreateMesh(15.0f, "../Textures/infrared.jpg", "../Models/spider.fbx");
	spider0->transformComponent->SetPosition(Vector3(10.0f, 5.0f, 0.0f));
	spider0->transformComponent->SetRotation(
		Quaternion::CreateFromAxisAngle(DirectX::SimpleMath::Vector3::Right, DirectX::XM_PIDIV2)
		*
		Quaternion::CreateFromAxisAngle(DirectX::SimpleMath::Vector3::Up, DirectX::XM_PI)
	);
	Game::GetInstance()->AddGameObject(spider0);

	GameObject* spider1 = new GameObject();
	spider1->CreateMesh(15.0f, "../Textures/infrared.jpg", "../Models/spider.fbx");
	spider1->transformComponent->SetPosition(Vector3(-10.0f, 5.0f, 0.0f));
	spider1->transformComponent->SetRotation(
		Quaternion::CreateFromAxisAngle(DirectX::SimpleMath::Vector3::Right, DirectX::XM_PIDIV2)
		*
		Quaternion::CreateFromAxisAngle(DirectX::SimpleMath::Vector3::Up, DirectX::XM_PI)
	);
	Game::GetInstance()->AddGameObject(spider1);

	GameObject* spider2 = new GameObject();
	spider2->CreateMesh(15.0f, "../Textures/infrared.jpg", "../Models/spider.fbx");
	spider2->transformComponent->SetPosition(Vector3(0.0f, 5.0f, 10.0f));
	spider2->transformComponent->SetRotation(
		Quaternion::CreateFromAxisAngle(DirectX::SimpleMath::Vector3::Right, DirectX::XM_PIDIV2)
		*
		Quaternion::CreateFromAxisAngle(DirectX::SimpleMath::Vector3::Up, DirectX::XM_PI)
	);
	Game::GetInstance()->AddGameObject(spider2);

	GameObject* spider3 = new GameObject();
	spider3->CreateMesh(15.0f, "../Textures/infrared.jpg", "../Models/spider.fbx");
	spider3->transformComponent->SetPosition(Vector3(0.0f, 5.0f, -10.0f));
	spider3->transformComponent->SetRotation(
		Quaternion::CreateFromAxisAngle(DirectX::SimpleMath::Vector3::Right, DirectX::XM_PIDIV2)
		*
		Quaternion::CreateFromAxisAngle(DirectX::SimpleMath::Vector3::Up, DirectX::XM_PI)
	);
	Game::GetInstance()->AddGameObject(spider3);
}

void KatamariDamacyGame::Run()
{
	Game::GetInstance()->Run();
}