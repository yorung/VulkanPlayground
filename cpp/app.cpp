#include "stdafx.h"

App app;

App::App()
{
}

void App::Draw()
{
	sky.Draw();
	triangle.Draw();
}

void App::Create()
{
	GoMyDir();

	triangle.Create();
	sky.Create();
}

void App::Destroy()
{
//	deviceMan.Flush();
	triangle.Destroy();
	sky.Destroy();
}

void App::Update()
{
}
