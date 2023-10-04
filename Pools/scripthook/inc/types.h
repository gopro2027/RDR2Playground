/*
	THIS FILE IS A PART OF RDR 2 SCRIPT HOOK SDK
				http://dev-c.com
			(C) Alexander Blade 2019
*/

#pragma once

#include <windows.h>
#include <cstdint>
#include <unordered_map>
#include <type_traits>

typedef DWORD Void;
typedef DWORD Any;
typedef DWORD uint;
typedef DWORD Hash;
typedef int Entity;
typedef int Player;
typedef int FireId;
typedef int Ped;
typedef int Vehicle;
typedef int Cam;
typedef int CarGenerator;
typedef int Group;
typedef int Train;
typedef int Pickup;
typedef int Object;
typedef int Weapon;
typedef int Interior;
typedef int Blip;
typedef int Texture;
typedef int TextureDict;
typedef int CoverPoint;
typedef int Camera;
typedef int TaskSequence;
typedef int ColourIndex;
typedef int Sphere;
typedef int ScrHandle;

#define ALIGN8 __declspec(align(8))

struct Vector3
{
	ALIGN8 float x;
	ALIGN8 float y;
	ALIGN8 float z;

	float magnitude() {
		return sqrtf(x * x + y * y + z * z);
	}
	Vector3 normalize() {
		float mag = magnitude();
		Vector3 ret = { x / mag, y / mag, z / mag };
		return ret;
	}
	Vector3 add(Vector3 adder) {
		Vector3 ret = { x + adder.x, y + adder.y, z + adder.z };
		return ret;
	}
	Vector3 sub(Vector3 subtractor) {
		Vector3 ret = { x - subtractor.x, y - subtractor.y, z - subtractor.z };
		return ret;
	}
	Vector3 scale(float scalar) {
		Vector3 ret = { x * scalar, y * scalar, z * scalar };
		return ret;
	}
};

static_assert(sizeof(Vector3) == 24, "");