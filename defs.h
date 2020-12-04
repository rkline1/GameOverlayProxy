#define VECTOR_H
#define degToRad( x ) x * 0.01745329251994329577f
#define radToDeg( x ) x * 57.2957795130823208768f


#pragma once
#include "includes.h"
#define _USE_MATH_DEFINES // for C

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEGREESTORADIANS(x) ((float)(x) * (float)(M_PI / 180.f))
#define RADPI 57.295779513082f

using dword = unsigned int;
using ulong = unsigned long;
using ulong64 = unsigned long long;
using uchar = unsigned char;
using ushort = unsigned short;
using uint = unsigned int;

#define MAX_PLAYERS 64

enum
{
	PITCH = 0,	// up / down
	YAW,		// left / right
	ROLL		// fall over
};

typedef float vec_t;

inline bool IsFinite(const float& f)
{
	return ((*reinterpret_cast<unsigned __int32*>((char*)(&f)) & 0x7F800000) != 0x7F800000);
}

class CSaveRestoreData;

struct CGlobalVarsBase {
public:
	// Absolute time (per frame still - Use Plat_FloatTime() for a high precision real time 
	//  perf clock, but not that it doesn't obey host_timescale/host_framerate)
	float realtime;

	// Absolute frame counter - continues to increase even if game is paused
	int	framecount;

	// Non-paused frametime
	float absoluteframetime;
	float absoluteframestarttimestddev;

	// Current time 
	//
	// On the client, this (along with tickcount) takes a different meaning based on what
	// piece of code you're in:
	// 
	//   - While receiving network packets (like in PreDataUpdate/PostDataUpdate and proxies),
	//     this is set to the SERVER TICKCOUNT for that packet. There is no interval between
	//     the server ticks.
	//     [server_current_Tick * tick_interval]
	//
	//   - While rendering, this is the exact client clock 
	//     [client_current_tick * tick_interval + interpolation_amount]
	//
	//   - During prediction, this is based on the client's current tick:
	//     [client_current_tick * tick_interval]
	float curtime;
	
	// Time spent on last server or client frame (has nothing to do with think intervals)
	float frametime;

	// current maxplayers setting
	int	maxClients;

	// Simulation ticks - does not increase when game is paused
	int	tickcount;

	// Simulation tick interval
	float interval_per_tick;

	// interpolation amount ( client-only ) based on fraction of next tick which has elapsed
	float interpolation_amount;
	int	  simTicksThisFrame;

	int network_protocol;

	// current saverestore data
	CSaveRestoreData *pSaveData;

	// Set to true in client code.
	bool m_bClient;
	bool m_bRemoteClient;
	
	// 100 (i.e., tickcount is rounded down to this base and then the "delta" from this base is networked
	int	nTimestampNetworkingBase;

	// 32 (entindex() % nTimestampRandomizeWindow ) is subtracted from gpGlobals->tickcount to set the networking basis, prevents
	//  all of the entities from forcing a new PackedEntity on the same tick (i.e., prevents them from getting lockstepped on this)
	int	nTimestampRandomizeWindow;  
};

class Vector
{
public:
	// members
	vec_t x, y, z;

	Vector() { x = y = z = 0.0f; }
	Vector(vec_t X, vec_t Y, vec_t Z) { x = X; y = Y; z = Z; }
	Vector(const Vector &v) { x = v.x; y = v.y; z = v.z; }



	float length() const { return sqrtf(x*x + y*y + z*z); }
	void substract(Vector sub) { x -= sub.x; y -= sub.y; z -= sub.z; }
	float dotproduct(Vector dot) const { return (x*dot.x + y*dot.y + z*dot.z); }

	// void normalize() { 
    //     vec_t l = 1.f / length(); x *= l; y *= l; z *= l; 
    // }

    FORCEINLINE float normalize() {
        const auto len = length();
        if( len <= 0.f )
            return 0.f;

        *this /= len;
        
        return len;
    }

	float vectodegree(Vector to) const { return (float)(180.0f / M_PI) * (asinf(dotproduct(to))); }

	Vector RotateX(Vector in, float angle) const
	{
		float a, c, s;
		Vector out;
		a = (float)DEGREESTORADIANS(angle);
		c = (float)cos(a);
		s = (float)sin(a);
		out.x = in.x;
		out.y = c*in.y - s*in.z;
		out.z = s*in.y + c*in.z;
		return out;
	}

	Vector RotateY(Vector in, float angle) const
	{
		float a, c, s;
		Vector out;
		a = (float)DEGREESTORADIANS(angle);
		c = (float)cos(a);
		s = (float)sin(a);
		out.x = c*in.x + s*in.z;
		out.y = in.y;
		out.z = -s*in.x + c*in.z;
		return out;
	}

	Vector RotateZ(Vector in, float angle)
	{
		float a, c, s;
		Vector out;
		a = (float)DEGREESTORADIANS(angle);
		c = (float)cos(a);
		s = (float)sin(a);
		out.x = c*in.x - s*in.y;
		out.y = s*in.x + c*in.y;
		out.z = in.z;
		return out;
	}


	Vector Right(Vector angles)
	{
		Vector out;

		float	sr, sp, sy, cr, cp, cy;

		sy = sinf(DEGREESTORADIANS(angles[YAW]));
		cy = cosf(DEGREESTORADIANS(angles[YAW]));

		sp = sinf(DEGREESTORADIANS(angles[PITCH]));
		cp = cosf(DEGREESTORADIANS(angles[PITCH]));

		sr = sinf(DEGREESTORADIANS(angles[ROLL]));
		cr = cosf(DEGREESTORADIANS(angles[ROLL]));

		out.x = (-1 * (sr*sp*cy) + -1 * (cr*-sy));
		out.y = (-1 * (sr*sp*sy) + -1 * (cr*cy));
		out.z = (-1 * sr*cp);

		return out;
	}

	Vector Up(Vector angles)
	{
		Vector out;

		float	sr, sp, sy, cr, cp, cy;

		sy = sinf(DEGREESTORADIANS(angles[YAW]));
		cy = cosf(DEGREESTORADIANS(angles[YAW]));

		sp = sinf(DEGREESTORADIANS(angles[PITCH]));
		cp = cosf(DEGREESTORADIANS(angles[PITCH]));

		sr = sinf(DEGREESTORADIANS(angles[ROLL]));
		cr = cosf(DEGREESTORADIANS(angles[ROLL]));

		out.x = (cr*sp*cy + -sr*-sy);
		out.y = (cr*sp*sy + -sr*cy);
		out.z = cr*cp;

		return out;
	}

	float Distance(Vector in)
	{
		vec_t deltax = in.x - x;
		vec_t deltay = in.y - y;
		vec_t deltaz = in.z - z;
		return sqrtf((deltax*deltax) + (deltay*deltay) + (deltaz*deltaz));
	}

	Vector Forward() const
	{
		Vector out;

		float	sp, sy, cp, cy;

		sy = sinf(DEGREESTORADIANS(YAW));
		cy = cosf(DEGREESTORADIANS(YAW));

		sp = sinf(DEGREESTORADIANS(PITCH));
		cp = cosf(DEGREESTORADIANS(PITCH));

		out.x = cp*cy;
		out.y = cp*sy;
		out.z = -sp;

		return out;
	}

	inline bool IsValid() const { return IsFinite(x) && IsFinite(y) && IsFinite(z); }

	void nullvec() { x = y = z = 0; }

	bool isNull() { return ((x == 0) && (y == 0) && (z == 0)); }

	void operator=(const vec_t *farray) { x = farray[0]; y = farray[1]; z = farray[2]; }
	void operator=(const vec_t &val) { x = y = z = val; }

    FORCEINLINE bool is_zero( float tolerance = 0.01f ) const {
        return x < -tolerance && x > tolerance && 
               y < -tolerance && y > tolerance && 
               z < -tolerance && z > tolerance;
    }

    // valid checks
    FORCEINLINE operator bool() const {
        return is_zero() != false;
    }
    
    FORCEINLINE bool operator !() const {
        return is_zero() == true;
    }

	// array access...
	float operator[](int i) const;
	float& operator[](int i);

	Vector operator+(const Vector& add) const { return Vector(x + add.x, y + add.y, z + add.z); }
	void operator+=(const Vector& add) { x += add.x; y += add.y; z += add.z; }

	Vector operator-(const Vector& sub) const { return Vector(x - sub.x, y - sub.y, z - sub.z); }
	void operator-=(const Vector& sub) { x -= sub.x; y -= sub.y; z -= sub.z; }

	Vector operator*(const float mul)	const { return Vector(x*mul, y*mul, z*mul); }
	void operator*=(const float mul) { x *= mul; y *= mul; z *= mul; }

	Vector operator/(const float div) { return Vector(x / div, y / div, z / div); }
	Vector operator/(const float div)	const { return Vector(x / div, y / div, z / div); }
	void operator/=(const float div) { x /= div; y /= div; z /= div; }

	bool operator==(const Vector& eqal) { return ((x == eqal.x) && (y == eqal.y) && (z == eqal.z)); }
	bool operator!=(const Vector& eqal) { return ((x != eqal.x) && (y != eqal.y) && (z != eqal.z)); }

	bool operator>(const Vector& eqal) { return ((x > eqal.x) && (y > eqal.y) && (z > eqal.z)); }
	bool operator<(const Vector& eqal) { return ((x < eqal.x) && (y < eqal.y) && (z < eqal.z)); }

	bool operator>=(const Vector& eqal) { return ((x >= eqal.x) && (y >= eqal.y) && (z >= eqal.z)); }
	bool operator<=(const Vector& eqal) { return ((x <= eqal.x) && (y <= eqal.y) && (z <= eqal.z)); }

	inline Vector toAngle() {
		return Vector(
			radToDeg(atan2f(-z, sqrtf(x * x + y * y))),
			radToDeg(atan2f(y, x)),
			0
		);
	}

	// inline float angleBetween(Vector &dest) {
	// 	return radToDeg(acosf(dotproduct(dest)));
	// }

	// // credits to ph0ne.
	// inline float getFOV(Vector &src, Vector &dest) {
	// 	Vector dir = Vector();
	// 	Vector fw = Vector();
    // 
	// 	// get direction and make sure we're working with a unit vector.
	// 	dir = dest - src;
	// 	dir.normalize();
    // 
	// 	// forward this angle.
	// 	fw = Forward();
    // 
	// 	// check our forwarded angles.
	// 	return max(angleBetween(dest), 0);
	// }


};

// Array access ---------------------------------------------------------------
inline vec_t& Vector::operator[](int i)
{
	return ((vec_t*)this)[i];
}
inline vec_t Vector::operator[](int i) const
{
	return ((vec_t*)this)[i];
}
//-----------------------------------------------------------------------------



class Player
{
public:
	//Player() : viewOffset(Vector(0, 0, 64.3f)) {}
	Player() {}
	DWORD addr;
	DWORD dwBoneMatrices;
	unsigned int flags;

	int team;
	int health;
	int glowIndex;
	int index;

	bool dormant;

	Vector origin;
	Vector eyeAngles;
	Vector viewOffset;
	Vector velocity;
	Vector aimAngle;

	Vector EyePos() { return (origin + viewOffset); }
};

class LocalPlayer : public Player
{
public:
	LocalPlayer() {}
	Vector punchAngles;
	
	Vector viewAngles;
	int cid;
	int shotsFired;
};

// winternl stuff ( taken from my copy of win10 ).
struct PEB_LDR_DATA {
	uint		Length;
	uchar		Initialized;
	dword		SsHandle;
	LIST_ENTRY	InLoadOrderModuleList;
	LIST_ENTRY	InMemoryOrderModuleList;
	LIST_ENTRY	InInitializationOrderModuleList;
	dword		EntryInProgress;
	uchar		ShutdownInProgress;
	dword		ShutdownThreadId;
};

struct UNICODE_STRING {
	ushort	Length;
	ushort	MaximumLength;
	wchar_t *Buffer;
};

struct STRING {
	ushort	Length;
	ushort	MaximumLength;
	char	*Buffer;
};

struct CURDIR {
	UNICODE_STRING	DosPath;
	dword		Handle;
};

struct RTL_DRIVE_LETTER_CURDIR {
	ushort	Flags;
	ushort	Length;
	uint	TimeStamp;
	STRING	DosPath;
};

struct RTL_USER_PROCESS_PARAMETERS {
	uint						MaximumLength;
	uint						Length;
	uint						Flags;
	uint						DebugFlags;
	dword						ConsoleHandle;
	uint						ConsoleFlags;
	dword						StandardInput;
	dword						StandardOutput;
	dword						StandardError;
	CURDIR						CurrentDirectory;
	UNICODE_STRING					DllPath;
	UNICODE_STRING					ImagePathName;
	UNICODE_STRING					CommandLine;
	dword						Environment;
	uint						StartingX;
	uint						StartingY;
	uint						CountX;
	uint						CountY;
	uint						CountCharsX;
	uint						CountCharsY;
	uint						FillAttribute;
	uint						WindowFlags;
	uint						ShowWindowFlags;
	UNICODE_STRING					WindowTitle;
	UNICODE_STRING					DesktopInfo;
	UNICODE_STRING					ShellInfo;
	UNICODE_STRING					RuntimeData;
	RTL_DRIVE_LETTER_CURDIR				CurrentDirectores[32];
	dword						EnvironmentSize;
	dword						EnvironmentVersion;
	dword						PackageDependencyData;
	uint						ProcessGroupId;
	uint						LoaderThreads;
};

struct RTL_BALANCED_NODE {
	RTL_BALANCED_NODE	*Children[2];
	RTL_BALANCED_NODE	*Left;
	RTL_BALANCED_NODE	*Right;
	dword			ParentValue;
};

struct PEB {
	uchar						InheritedAddressSpace;
	uchar						ReadImageFileExecOptions;
	uchar						BeingDebugged;
	uchar						BitField;
	//uchar						Padding0[ 4 ];
	dword						Mutant;
	dword						ImageBaseAddress;
	PEB_LDR_DATA					*Ldr;
	RTL_USER_PROCESS_PARAMETERS 			*ProcessParameters;
	dword						SubSystemData;
	dword						ProcessHeap;
	RTL_CRITICAL_SECTION				*FastPebLock;
	dword						AtlThunkSListPtr;
	dword						IFEOKey;
	uint						CrossProcessFlags;
	uchar						Padding1[4];
	dword						KernelCallbackTable;
	dword						UserSharedInfoPtr;
	uint						SystemReserved[1];
	uint						AtlThunkSListPtr32;
	dword						ApiSetMap;
	uint						TlsExpansionCounter;
	uchar						Padding2[4];
	dword						TlsBitmap;
	uint						TlsBitmapBits[2];
	dword						ReadOnlySharedMemoryBase;
	dword						SparePvoid0;
	dword						ReadOnlyStaticServerData; // or just dword?
	dword						AnsiCodePageData;
	dword						OemCodePageData;
	dword						UnicodeCaseTableData;
	uint						NumberOfProcessors;
	uint						NtGlobalFlag;
	LARGE_INTEGER					CriticalSectionTimeout;
	dword						HeapSegmentReserve;
	dword						HeapSegmentCommit;
	dword						HeapDeCommitTotalFreeThreshold;
	dword						HeapDeCommitFreeBlockThreshold;
	uint						NumberOfHeaps;
	uint						MaximumNumberOfHeaps;
	dword						ProcessHeaps;
	dword						GdiSharedHandleTable;
	dword						ProcessStarterHelper;
	uint						GdiDCAttributeList;
	uchar						Padding3[4];
	RTL_CRITICAL_SECTION				*LoaderLock;
	uint						OSMajorVersion;
	uint						OSMinorVersion;
	ushort						OSBuildNumber;
	ushort						OSCSDVersion;
	uint						OSPlatformId;
	uint						ImageSubsystem;
	uint						ImageSubsystemMajorVersion;
	uint						ImageSubsystemMinorVersion;
	uchar						Padding4[4];
	dword						ActiveProcessAffinityMask;
#ifdef _WIN64
	uint						GdiHandleBuffer[60];
#else
	uint						GdiHandleBuffer[34];
#endif
	dword						PostProcessInitRoutine;
	dword						TlsExpansionBitmap;
	uint						TlsExpansionBitmapBits[32];
	uint						SessionId;
	uchar						Padding5[4];
	ULARGE_INTEGER					AppCompatFlags;
	ULARGE_INTEGER					AppCompatFlagsUser;
	dword						pShimData;
	dword						AppCompatInfo;
	UNICODE_STRING					CSDVersion;
	dword						ActivationContextData;
	dword						ProcessAssemblyStorageMap;
	dword						SystemDefaultActivationContextData;
	dword						SystemAssemblyStorageMap;
	dword						MinimumStackCommit;
	dword						FlsCallback;
	LIST_ENTRY					FlsListHead;
	dword						FlsBitmap;
	uint						FlsBitmapBits[4];
	uint						FlsHighIndex;
	dword						WerRegistrationData;
	dword						WerShipAssertPtr;
	dword						pUnused;
	dword						pImageHeaderHash;
	uint						TracingFlags;
	uchar						Padding6[4];
	ulong64						CsrServerReadOnlySharedMemoryBase;
	dword						TppWorkerpListLock;
	LIST_ENTRY					TppWorkerpList;
	dword						WaitOnAddressHashTable[128];
};

struct LDR_DATA_TABLE_ENTRY {
	LIST_ENTRY			InLoadOrderLinks;
	LIST_ENTRY			InMemoryOrderLinks;
	LIST_ENTRY			InInitializationOrderLinks;
	dword				DllBase;
	dword				EntryPoint;
	uint				SizeOfImage;
	UNICODE_STRING			FullDllName;
	UNICODE_STRING			BaseDllName;
	uchar				FlagGroup[4];
	uint				Flags;
	ushort				ObsoleteLoadCount;
	ushort				TlsIndex;
	LIST_ENTRY			HashLinks;
	uint				TimeDateStamp;
	dword				EntryPointActivationContext;
	dword				Lock;
	dword				DdagNode;
	LIST_ENTRY			NodeModuleLink;
	dword				LoadContext;
	dword				ParentDllBase;
	dword				SwitchBackContext;
	RTL_BALANCED_NODE		BaseAddressIndexNode;
	RTL_BALANCED_NODE		MappingInfoIndexNode;
	dword				OriginalBase;
	LARGE_INTEGER			LoadTime;
	uint				BaseNameHashValue;
	uint				LoadReason;
	uint				ImplicitPathOptions;
	uint				ReferenceCount;
};
struct matrix3x4_t
{
	matrix3x4_t() {}
	matrix3x4_t(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23)
	{
		m_flMatVal[0][0] = m00;	m_flMatVal[0][1] = m01; m_flMatVal[0][2] = m02; m_flMatVal[0][3] = m03;
		m_flMatVal[1][0] = m10;	m_flMatVal[1][1] = m11; m_flMatVal[1][2] = m12; m_flMatVal[1][3] = m13;
		m_flMatVal[2][0] = m20;	m_flMatVal[2][1] = m21; m_flMatVal[2][2] = m22; m_flMatVal[2][3] = m23;
	}

	//void dump(const char* name)
	//{
	//	printf("%s: \n", name);
	//	printf("\t%f\t%f\t%f\t%f\n", m_flMatVal[0][0], m_flMatVal[0][1], m_flMatVal[0][2], m_flMatVal[0][3]);
	//	printf("\t%f\t%f\t%f\t%f\n", m_flMatVal[1][0], m_flMatVal[1][1], m_flMatVal[1][2], m_flMatVal[1][3]);
	//	printf("\t%f\t%f\t%f\t%f\n", m_flMatVal[2][0], m_flMatVal[2][1], m_flMatVal[2][2], m_flMatVal[2][3]);
	//}

	//-----------------------------------------------------------------------------
	// Creates a matrix where the X axis = forward
	// the Y axis = left, and the Z axis = up
	//-----------------------------------------------------------------------------
	void Init(const Vector& xAxis, const Vector& yAxis, const Vector& zAxis, const Vector &vecOrigin)
	{
		m_flMatVal[0][0] = xAxis.x; m_flMatVal[0][1] = yAxis.x; m_flMatVal[0][2] = zAxis.x; m_flMatVal[0][3] = vecOrigin.x;
		m_flMatVal[1][0] = xAxis.y; m_flMatVal[1][1] = yAxis.y; m_flMatVal[1][2] = zAxis.y; m_flMatVal[1][3] = vecOrigin.y;
		m_flMatVal[2][0] = xAxis.z; m_flMatVal[2][1] = yAxis.z; m_flMatVal[2][2] = zAxis.z; m_flMatVal[2][3] = vecOrigin.z;
	}

	//-----------------------------------------------------------------------------
	// Creates a matrix where the X axis = forward
	// the Y axis = left, and the Z axis = up
	//-----------------------------------------------------------------------------
	matrix3x4_t(const Vector& xAxis, const Vector& yAxis, const Vector& zAxis, const Vector &vecOrigin)
	{
		Init(xAxis, yAxis, zAxis, vecOrigin);
	}

	inline void SetOrigin(Vector const & p)
	{
		m_flMatVal[0][3] = p.x;
		m_flMatVal[1][3] = p.y;
		m_flMatVal[2][3] = p.z;
	}

	Vector Forward() { return Vector(m_flMatVal[0][0], m_flMatVal[1][0], m_flMatVal[2][0]); }
	Vector Right() { return Vector(m_flMatVal[0][1], m_flMatVal[1][1], m_flMatVal[2][1]); }
	Vector Up() { return Vector(m_flMatVal[0][2], m_flMatVal[1][2], m_flMatVal[2][2]); }
	Vector Origin() { return Vector(m_flMatVal[0][3], m_flMatVal[1][3], m_flMatVal[2][3]); }

	float *operator[](int i) { if ((i >= 0) && (i < 3)) return m_flMatVal[i]; }
	const float *operator[](int i) const { if ((i >= 0) && (i < 3)) return m_flMatVal[i]; }
	float *Base() { return &m_flMatVal[0][0]; }
	const float *Base() const { return &m_flMatVal[0][0]; }

	float m_flMatVal[3][4];
};
void CalcAngles(Vector src, Vector dst, Vector& out)
{
	Vector delta = src - dst;
	float hyp = sqrt(delta[0] * delta[0] + delta[1] * delta[1]);
	out.y = atanf(delta[1] / delta[0]) * RADPI;
	out.x = atan2f(delta[2], hyp) * RADPI;
	out.z = 0.0f;
	if (delta[0] >= 0.0f)
		out.y += 180.0f;
}
Vector VectorSubtract(Vector vec1, Vector vec2)
{
	return Vector(vec1.x - vec2.x, vec1.y - vec2.y, vec1.z - vec2.z);
};
void CalcAngle(Vector src, Vector dst, Vector &angles)
{
	Vector forward = VectorSubtract(dst, src);
	float yaw, tmp, pitch;

	tmp = sqrt(forward.x*forward.x + forward.y*forward.y);
	yaw = (atan2(forward.y, forward.x) * 180 / M_PI);
	pitch = (atan2(-forward.z, tmp) * 180 / M_PI);

	angles.x = pitch;
	angles.y = yaw;
	angles.z = 0;
}
static NOINLINE float normalize( float axis ) {
    // invalid number
    if( !std::isfinite( axis ) )
        return 0.f;
    
    // nothing to do, angle is in bounds
    if( axis >= -180.f && axis <= 180.f )
        return axis;

    return std::remainder( axis, 360.f );
}

static void Clamp(Vector &angles)
{
    angles.x = std::clamp( angles.x, -89.f, 89.f );
    angles.y = normalize( angles.y );
    angles.z = 0.f;
}

/*static void Clamp(Vector &angles)
{
	if (angles.x > 89.f) angles.x = 89.f;
	if (angles.x < -89.f) angles.x = -89.f;

	if (angles.y > 180.f) angles.y = 180.f;
	if (angles.y < -180.f) angles.y = -180.f;

	angles.z = 0;
}*/
static float GetFovX(Vector& vView, Vector& vCalc, float dist, float dist_tradeoff)
{
	float sx = 0;
	//Calc the YAW difference between the current view and the angle to aim at
	if (vView[YAW] > vCalc[YAW])
	{
		sx = vView[YAW] - vCalc[YAW];
		if (sx > 180.0f)
			sx = 360.0f - sx;
	}
	else if (vView[YAW] < vCalc[YAW])
	{
		sx = vCalc[YAW] - vView[YAW];
		if (sx > 180.0f)
			sx = 360.0f - sx;
	}

	//And scale this by the distance to the enemy to generate an effect so the fov gets smaller with the distance of the enemy getting bigger
	float fov = sx;
	float mult = dist - 180;
	if (mult < 0.0f)
		mult = 0.0f;

	fov = fov + ((mult * dist_tradeoff) * fov);

	return fov;
}

static float GetFov(Vector& vView, Vector& vCalc, float dist, float dist_tradeoff, float* fovX = nullptr, float* fovY = nullptr)
{
	float sx = 0;
	float sy = 0;

	//Basically we calc the PITCH difference between the current view and the angle to aim at
	sy = abs(vView[PITCH] - vCalc[PITCH]);

	//Calc the YAW difference between the current view and the angle to aim at
	if (vView[YAW] > vCalc[YAW])
	{
		sx = vView[YAW] - vCalc[YAW];
		if (sx > 180.0f)
			sx = 360.0f - sx;
	}
	else if (vView[YAW] < vCalc[YAW])
	{
		sx = vCalc[YAW] - vView[YAW];
		if (sx > 180.0f)
			sx = 360.0f - sx;
	}

	float mult = dist - 180;
	if (mult < 0.0f)
		mult = 0.0f;

	if (fovX != nullptr)
	{
		float fovx = sx;
		fovx = fovx + ((mult * dist_tradeoff) * fovx);
		*fovX = fovx;
	}

	if (fovY != nullptr)
	{
		float fovy = sy;
		fovy = fovy + ((mult * dist_tradeoff) * fovy);
		*fovY = fovy;
	}

	//And scale this by the distance to the enemy to generate an effect so the fov gets smaller with the distance of the enemy getting bigger
	float fov = (sx + sy) * 0.5f;
	fov = fov + ((mult * dist_tradeoff) * fov);

	return fov;
}
static float GetDistance(Vector one, Vector two)
{
	float dx = one.x - two.x;
	float dy = one.y - two.y;
	float dz = one.z - two.z;
	return sqrtf(dx*dx + dy*dy + dz*dz);
}

float RandomFloat(float a, float b)
{
	return ((b - a)*((float)rand() / RAND_MAX)) + a;
}
