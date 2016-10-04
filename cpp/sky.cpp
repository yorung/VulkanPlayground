#include "stdafx.h"

Sky sky;

void Sky::Draw()
{
	renderStates.Apply();
	Mat matV, matP;
	matrixMan.Get(MatrixMan::VIEW, matV);
	matrixMan.Get(MatrixMan::PROJ, matP);
	matV._41 = matV._42 = matV._43 = 0;
	Mat invVP = inv(matV * matP);
	afBindBuffer(sizeof(invVP), &invVP, 0);
	afBindTexture(texture, 1);
	afDraw(4);
}

void Sky::Create()
{
	TexDesc desc;
	renderStates.Create("sky_photosphere", 0, nullptr, AFRS_DEPTH_CLOSEREQUAL_READONLY);
	//texture = afLoadTexture("yangjae.dds", desc);
	//texture = afLoadTexture("yangjae_mip.dds", desc);
	texture = afLoadTexture("hakodate.jpg", desc);

	//renderStates.Create("sky_cubemap", 0, nullptr, AFRS_DEPTH_CLOSEREQUAL_READONLY);
	//texture = afLoadTexture("cube.dds", desc);
}

void Sky::Destroy()
{
	afSafeDeleteTexture(texture);
	VkDevice device = deviceMan.GetDevice();
	renderStates.Destroy();
}
