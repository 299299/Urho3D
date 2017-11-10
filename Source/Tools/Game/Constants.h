#pragma once
#include <Urho3D/Math/Vector3.h>

namespace Urho3D
{
static bool d_log = false;

static const int COLLISION_PHYSICS = 0;
static const int CTRL_ATTACK = (1 << 0);
static const int CTRL_JUMP = (1 << 1);
static const int CTRL_ALL = (1 << 16);

static const int FLAGS_ATTACK  = (1 << 0);
static const int FLAGS_COUNTER = (1 << 1);
static const int FLAGS_REDIRECTED = (1 << 2);
static const int FLAGS_NO_MOVE = (1 << 3);
static const int FLAGS_MOVING = (1 << 4);
static const int FLAGS_INVINCIBLE = (1 << 5);
static const int FLAGS_STUN = (1 << 6);

static const int COLLISION_LAYER_LANDSCAPE = (1 << 0);
static const int COLLISION_LAYER_CHARACTER = (1 << 1);
static const int COLLISION_LAYER_PROP      = (1 << 2);
static const int COLLISION_LAYER_RAGDOLL   = (1 << 3);
static const int COLLISION_LAYER_AI        = (1 << 4);

static const float CHARACTER_HEIGHT = 5.0f;

static const float COLLISION_RADIUS = 1.5f;
static const float COLLISION_SAFE_DIST = COLLISION_RADIUS * 1.85f;

static const Vector3 WORLD_HALF_SIZE(1000, 0, 1000);

static const char* TITLE = "AssetProcess";
static const char* TRANSLATE_BONE_NAME = "Bip01_$AssimpFbx$_Translation";
static const char* ROTATE_BONE_NAME = "Bip01_$AssimpFbx$_Rotation";
static const char* SCALE_BONE_NAME = "Bip01_$AssimpFbx$_Scaling";
static const char* HEAD = "Bip01_Head";
static const char* L_HAND = "Bip01_L_Hand";
static const char* R_HAND = "Bip01_R_Hand";
static const char* L_FOOT = "Bip01_L_Foot";
static const char* R_FOOT = "Bip01_R_Foot";
static const char* L_ARM = "Bip01_L_Forearm";
static const char* R_ARM = "Bip01_R_Forearm";
static const char* L_CALF = "Bip01_L_Calf";
static const char* R_CALF = "Bip01_R_Calf";

static const float FRAME_PER_SEC = 30.0f;
static const float SEC_PER_FRAME = 1.0f/FRAME_PER_SEC;
static const unsigned  PROCESS_TIME_PER_FRAME = 60; // ms
static const float BONE_SCALE = 100.0f;

}