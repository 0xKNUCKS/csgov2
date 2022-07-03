#include "VARS.h"

void SDK::Variables::UpdateMatrix(interfaces_t interfff)
{
	uintptr_t ModBase = (uintptr_t)GetModuleHandleA("client.dll");
	interfff.init();
	memcpy(&viewMatrix, (BYTE*)(ModBase + hazedumper::signatures::dwViewMatrix), sizeof(viewMatrix));
}

bool SDK::GAME::WorldToScreen(math::Vector pos, math::Vector& Screen, interfaces_t interfff)
{
	//float Matrix[16];
	//memcpy(&Matrix, (BYTE*)((uintptr_t)GetModuleHandle("client.dll") + hazedumper::signatures::dwViewMatrix), sizeof(Matrix));
	//
	//math::vec4 ClipCoords;
	//ClipCoords.x = pos.x * Matrix[0]  +  pos.y  *  Matrix[1]   + pos.z   * Matrix[2]  + Matrix[3];
	//ClipCoords.y = pos.x * Matrix[4]  +  pos.y  *  Matrix[5]   + pos.z   * Matrix[6]  + Matrix[7];
	//ClipCoords.z = pos.x * Matrix[8]  +  pos.y  *  Matrix[9]   + pos.z   * Matrix[10] + Matrix[11];
	//ClipCoords.w = pos.x * Matrix[12] +  pos.y  *  Matrix[13]  + pos.z   * Matrix[14] + Matrix[15];
	//
	//if (ClipCoords.w < 0.1f)
	//	return 0;
	//
	//math::Vector NDC;
	//NDC.x = ClipCoords.x / ClipCoords.w;
	//NDC.y = ClipCoords.y / ClipCoords.w;
	//
	//Screen.x = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
	//Screen.y = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);
	return true;
}
