#include "includes.h"

DWORD clientBase, engineBase;

CGlobalVarsBase *g_globalvars;

HWND csWind;
LocalPlayer localPlayer;
std::vector< Player > g_cur_players;

HINSTANCE mHinst = 0, mHinstDLL = 0;
UINT_PTR mProcs[14] = { 0 };

// arduino serial port
SerialPort g_arduino( "\\\\.\\COM3" );

// arduino triggerbot string
std::string g_arduino_shoot_code = "A";

// aimbot settings
int smoothMin = 9;
int smoothMax = 13;
int bone = 8;
float fovv = 3.0;
float m_yaw = 0.022;       // your ingame m_yaw 
float in_game_sens = 4.5; // your ingame sens

// triggerbot settings
float trigger_fov   = 0.6f;
float trigger_delay = 34.0f;

// initialize radar
bool radar;

Vector pos;

float randMToN(float M, float N);

// arduino MouseMove for aimbot
static void sendCoordinates( float x, float y ) {
    std::string send_str = std::to_string( x ) + "/" + std::to_string( y ) + ";";

	g_arduino.writeSerialPort( (char *)send_str.c_str(), send_str.size() );
}

Vector GetBonePosition( Player *player, int bone ) {
	const auto dwBoneMatrices = player->dwBoneMatrices;

    // get ptr to matrix
	auto boneMatrix = *(matrix3x4_t *)( dwBoneMatrices + ( sizeof( matrix3x4_t ) * bone ) );

    // return matrix origin position
	return boneMatrix.Origin();
}

// arduino mousemove with x y
__forceinline void setAngleWithMouse(Vector ang, Vector localViewAngles) {
	float Pixels = m_yaw * in_game_sens;
    Vector delta = localViewAngles - ang;

    Clamp(delta);

    delta.x /= ( -Pixels );
    delta.y /= Pixels;

    sendCoordinates(delta.y, delta.x);
}

static bool Aim( Vector &finalAng ) {
	Vector finalPos, aimAngle;

	if( !localPlayer.addr )
		return false;

    if( localPlayer.health < 1 )
        return false;

    // get localplayer data
    const auto eye_pos = localPlayer.EyePos();

    // reset local vars
    Player *best_player = nullptr;

	// find valid aimbot player
    for( size_t i = 0; i < g_cur_players.size(); ++i ) {
        // get player at index
        const auto cur_player = &g_cur_players[ i ];
        if( !cur_player )
            continue;

        // skip bad players
        if( cur_player->dormant )
            continue;

        if( cur_player->team == localPlayer.team )
            continue;

        if( cur_player->health < 1 )
            continue;

        const auto entityPos = GetBonePosition( cur_player, bone );
        if( !entityPos )
        	continue;
        
        aimAngle = ( entityPos - eye_pos ).toAngle();
        Clamp( aimAngle );
        
        float fovX;
        Vector nonRCSAngles = localPlayer.viewAngles + localPlayer.punchAngles * RandomFloat( 1.9, 2.1 );
        float fov = GetFov( nonRCSAngles, aimAngle, GetDistance( eye_pos, entityPos ), 0.002f, &fovX );
        
        // check fov
        if( fov < fovv ) {
        
        	finalPos    = entityPos;
            best_player = cur_player;
        	
        	break;
        }

		//if (players[i].addr)
		//{
		//	if (players[i].index == localPlayer.index)
		//		continue;
        //
		//	if (!players[i].dormant && players[i].health > 0 && players[i].team != localPlayer.team && (players[i].team == 2 || players[i].team == 3))
		//	{
		//		Vector entityPos = GetBonePosition(players[i], bone);
		//		if (entityPos == Vector(0, 0, 0))
		//			continue;
        //
		//		aimAngle = (entityPos - localPlayer.EyePos()).toAngle();
		//		Clamp(aimAngle);
        //
		//		float fovX;
		//		Vector nonRCSAngles = localPlayer.viewAngles + localPlayer.punchAngles * RandomFloat(1.9, 2.1);
		//		float fov = GetFov(nonRCSAngles, aimAngle, GetDistance(localPlayer.EyePos(), entityPos), 0.002f, &fovX);
        //
		//		if (fov < fovv)
		//		{
        //
		//			finalPos = entityPos;
		//			acquiredTarget = players[i].addr;
		//			targetAcquired = true;
		//			break;
		//		}
        //
        //
		//	}
		//}
	}

    if (best_player) //weapon checks
	{
		bone = 8;
		
		int handle = *(int*)(localPlayer.addr + dwActiveWeapon);
		
		if (!handle)
			return false;
		
		int pointer = *(int*)(clientBase + dwEntityList + ((handle & 0xFFF) - 1) * 0x10);

		if (!pointer)
			return false;
		
		int weapon = *(int*)(pointer + dwItemDefinitionIndex);

		if (!weapon)
			return false;
		
		if (weapon == 9 || weapon == 11 || weapon == 38) {
			bone = 8;
		}
	
		if (weapon == 1 || weapon == 4 || weapon == 32 || weapon == 61 || weapon == 64 || weapon == 40) {
			bone = 8;
		}

		if (weapon == 3 || weapon == 63 || weapon == 36 || weapon == 30) {
			bone = 8;
		}

		if (weapon == 43 || weapon == 44 || weapon == 45 || weapon == 46 || weapon == 47 || weapon == 48 || weapon == 49)
            return false;

		Vector targetAngles = aimAngle;
		if (localPlayer.shotsFired > 1)
			targetAngles = targetAngles - localPlayer.punchAngles * RandomFloat(1.9, 2.1);

		Vector delta(targetAngles - localPlayer.viewAngles);
		delta.x += delta.length() / RandomFloat(1, 3);
		Clamp(delta);
		Vector move = delta / RandomFloat(smoothMin, smoothMax);
		if (move.length() > 1)
			return false;

		finalAng = (localPlayer.viewAngles + move);
		Clamp(finalAng);
	}
	else {
		return false;
	}

	return true;
}

// call rand
float randMToN(float M, float N)
{
	return M + (rand() / (RAND_MAX / (N - M)));
}

// our own GetModuleHandle
dword getModuleBase(const std::string &str)
{
	PEB 				 *peb = 0;
	LIST_ENTRY			 *start = 0, *next = 0;
	LDR_DATA_TABLE_ENTRY *cur = 0;
	uint				 len = 0;
	char				 name[MAX_PATH]; // initialize?
	IMAGE_DOS_HEADER	 *dos = 0;
	dword 				 out = 0;

	if (str.empty())
		return 0;
	// read PEB from the TEB ( FS:[0x30] )
	peb = (PEB *)__readfsdword(0x30);
	if (!peb)
		return 0;
	// head of linked list.
	start = &peb->Ldr->InMemoryOrderModuleList;
	if (!start)
		return 0;
	next = start;
	// iterate, if the current list entry isn't the start then we haven't reached the end.
	do {
		
		// current list entry.
		cur = CONTAINING_RECORD(next, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

		if (!cur)
			continue;
		// get ascii length with RSH.
		len = cur->BaseDllName.Length >> 1;
		// iterate string ( we might lose some data depending on the string, but it shouldn't matter. )
		// todo; use bitwise operators instead, this is dumb.
		for (uint i = 0; i < len; i++) {
			// check the high bytes first ( > 0x0 )
			if (HIBYTE(cur->BaseDllName.Buffer[i]))
				continue;
			
			// we only want the lower bytes of this ( 0-255 ).
			name[i] = LOBYTE(cur->BaseDllName.Buffer[i]);
		}
		// null terminate.
		name[len] = '\0';
		if (name == str) {
			// valid base?
			if (!cur->DllBase)
				continue;

			// set for later.
			out = cur->DllBase;
			break;
		}

	} while ((next = next->Flink) != start);

	return out;
}

// update local players
bool updateLocalPlayer() {
	DWORD clientstate = *(DWORD*)(engineBase + dwClientState);
	localPlayer.addr = *(DWORD*)(clientBase + dwLocalPlayer);
	if (!localPlayer.addr)
		return false;

    localPlayer.team = *(int*)(localPlayer.addr + dwTeam);
    if( !localPlayer.team )
        return false;

	localPlayer.health = *(int *)(localPlayer.addr + dwHealth); 
	localPlayer.cid = *(int*)(localPlayer.addr + dwCrosshairId); 
	localPlayer.index = *(int*)(localPlayer.addr + dwIndex); 
	localPlayer.shotsFired = *(int*)(localPlayer.addr + dwShotsFired);
	localPlayer.viewAngles = *(Vector*)(clientstate + dwViewAnglesSet);
	localPlayer.punchAngles = *(Vector*)(localPlayer.addr + dwVecPunch);
	localPlayer.origin = *(Vector*)(localPlayer.addr + dwOrigin); 
	localPlayer.viewOffset = *(Vector*)(localPlayer.addr + dwVecViewOffset);
	localPlayer.velocity = *(Vector*)(localPlayer.addr + dwVelocity); 

	return true;
}

// entity loop and radar
bool updateEnts() {
    Player insert;

    // clear old player batch
    g_cur_players.clear();

    // update local player first
    if( !updateLocalPlayer() )
        return false;

    // no other players, do nothing
    if( g_globalvars->maxClients < 1 )
        return false;

    // loop valid player indexes
    // engine reserves up to servers "maxClients" for players
    // starting after world (0)
    for( int i = 1; i <= g_globalvars->maxClients; ++i ) {
        // check entlist entry, don't deref a nullptr...
        // this is probably not needed, but whatever
        const auto cur_player_offset = ( ( clientBase + dwEntityList ) + ( i * 0x10 ) );
        if( !cur_player_offset )
            continue;
    
        // get player base address
        const auto cur_player_base = *(DWORD *)( cur_player_offset );
        if( !cur_player_base )
            continue;

        // get index, skip localplayer index
        const auto cur_player_idx = *(int *)( cur_player_base + dwIndex );
        if( cur_player_idx == localPlayer.index )
            continue;

        insert.addr           = cur_player_base;
        insert.index          = cur_player_idx;
        insert.health         = *(int *)( cur_player_base + dwHealth );
        insert.team           = *(int *)( cur_player_base + dwTeam );
        insert.index          = *(int *)( cur_player_base + dwIndex );
        insert.origin         = *(Vector *)( cur_player_base + dwOrigin );
        insert.velocity       = *(Vector *)( cur_player_base + dwVelocity );
        insert.dormant        = *(bool *)( cur_player_base + dwDormant );
        insert.dwBoneMatrices = *(DWORD *)( cur_player_base + dwBoneMatrix );

        // radar
        if ( GetKeyState( 'E' ) & 0x108 )
            *(bool *)( cur_player_base + dwSpotted ) = true;

        // add to vector
        g_cur_players.push_back( insert );
    }

    return true;
}

//static float get_FOV( const Vector &va, const Vector &start, const Vector &end, float dist_mod = 1.f ) {
//    // calculate normalized delta between the start and end position
//    auto delta = end - start;
//    
//    // normalize and save distance
//    const auto dist = delta.normalize();
//    if( dist <= 0.f )
//        return 0.f;
//
//    // get the viewangle's forward directional vector
//    const auto va_fwd = va.Forward();
//
//    // get angle (in radians) between va_fwd vector and the delta vector
//    const auto angle = std::acos( va_fwd.dotproduct( delta ) );
//
//    // get radius to target
//    const auto radius = std::sin( angle ) * dist;
//
//    // calculate full FOV angle
//    const auto fov_angle = std::atan2( radius, dist ) * 2.f;
//
//    // return FOV in degrees
//    return std::max( 0.f, radToDeg( fov_angle ) );
//}

static void triggerbot() {
    // static auto hitbox_bone_list = std::array< int, 20 >{
    //     // 8,
    //     // 7,
    //     // 6,
    //     // 5,
    //     // 4,
    //     // 3,
    //     // 0
    // };

    // check local player
    if( !localPlayer.addr )
        return;

    if( localPlayer.health < 1 )
        return;

    // get localplayer data
    const auto eye_pos = localPlayer.EyePos();

    // reset local vars
    int best_player_idx = -1;

    // find valid triggerbot position
    for( size_t i = 0; i < g_cur_players.size(); ++i ) {
        // get player at index
        const auto cur_player = &g_cur_players[ i ];
        if( !cur_player )
            continue;

        // skip bad players
        if( cur_player->dormant )
            continue;

        if( cur_player->team == localPlayer.team )
            continue;

        if( cur_player->health < 1 )
            continue;
        
        // loop 'all' bones
        for( int b = 0; b < 256; ++b ) {
            const auto bone_pos = GetBonePosition( cur_player, b );
            if( !bone_pos )
                continue;
        
            auto aimAngle = ( bone_pos - eye_pos ).toAngle();
            Clamp( aimAngle );
            
            float fovX;
            Vector nonRCSAngles = localPlayer.viewAngles + localPlayer.punchAngles * RandomFloat( 1.9, 2.1 );
            float fov = GetFov( nonRCSAngles, aimAngle, GetDistance( eye_pos, bone_pos ), 0.002f, &fovX );
        
            // do we have a position within fov? break out now
            if( fov < trigger_fov ) {
                best_player_idx = i;
        
                break;
            }
        }
    }

    // no valid player found
    if( best_player_idx == -1 )
        return;
    
    // delay the trigger
    Sleep( trigger_delay );
    
    // write to the arduino
    g_arduino.writeSerialPort( (char *)g_arduino_shoot_code.c_str(), g_arduino_shoot_code.size() );
}

DWORD WINAPI Thread( LPVOID lpParams ) {
    Vector anglesToSet;

    while( !clientBase ) {
    
    	clientBase = getModuleBase("client_panorama.dll");
    	engineBase = getModuleBase("engine.dll");
    	Sleep( 2000 );
    }

    Sleep( 5000 );

    // misc
    csWind       = GetForegroundWindow();
    g_globalvars = (CGlobalVarsBase *)( engineBase + dwGlobalVars );

    while( true ) {
        // update players first
		if( !updateEnts() )
            continue;

		anglesToSet = localPlayer.viewAngles;
        
		if ( GetKeyState(VK_LBUTTON) & 0x108 )
        {
			if (Aim(anglesToSet)) {
				Clamp(anglesToSet);
				//*(Vector*)(clientstate + dwViewAnglesSet) = anglesToSet;
				setAngleWithMouse(anglesToSet, localPlayer.viewAngles);
			}
		}

        // run triggerbot
        if( GetKeyState( VK_XBUTTON1 ) & 0x108 ) 
            triggerbot();

		Sleep( 2 );
	}

	return 0;
}


LPCSTR mImportNames[] = { "BOverlayNeedsPresent", "IsOverlayEnabled", "NotifyVRCleanup", "NotifyVRInit", "OverlayHookD3D3", "SetNotificationInset", "SetNotificationPosition", "SteamOverlayIsUsingGamepad", "SteamOverlayIsUsingKeyboard", "SteamOverlayIsUsingMouse", "ValveHookScreenshots", "VirtualFreeWrapper", "VulkanSteamOverlayPresent", "VulkanSteamOverlayProcessCapturedFrame" };
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	;
	mHinst = hinstDLL;
	if (fdwReason == DLL_PROCESS_ATTACH) {
		mHinstDLL = LoadLibrary("C:\\Program Files (x86)\\Steam\\GameOverlayRenderer64.dll");
		if (!mHinstDLL)
			return (FALSE);
		for (int i = 0; i < 14; i++)
			mProcs[i] = (UINT_PTR)GetProcAddress(mHinstDLL, mImportNames[i]);
		CreateThread(0, 0, Thread, 0, 0, 0);
	}
	else if (fdwReason == DLL_PROCESS_DETACH) {
		FreeLibrary(mHinstDLL);
	}
	return (TRUE);
}

extern "C" __declspec(naked) void __stdcall BOverlayNeedsPresent_wrapper() { __asm {jmp mProcs[0 * 4]} }
extern "C" __declspec(naked) void __stdcall IsOverlayEnabled_wrapper() { __asm {jmp mProcs[1 * 4]} }
extern "C" __declspec(naked) void __stdcall NotifyVRCleanup_wrapper() { __asm {jmp mProcs[2 * 4]} }
extern "C" __declspec(naked) void __stdcall NotifyVRInit_wrapper() { __asm {jmp mProcs[3 * 4]} }
extern "C" __declspec(naked) void __stdcall OverlayHookD3D3_wrapper() { __asm {jmp mProcs[4 * 4]} }
extern "C" __declspec(naked) void __stdcall SetNotificationInset_wrapper() { __asm {jmp mProcs[5 * 4]} }
extern "C" __declspec(naked) void __stdcall SetNotificationPosition_wrapper() { __asm {jmp mProcs[6 * 4]} }
extern "C" __declspec(naked) void __stdcall SteamOverlayIsUsingGamepad_wrapper() { __asm {jmp mProcs[7 * 4]} }
extern "C" __declspec(naked) void __stdcall SteamOverlayIsUsingKeyboard_wrapper() { __asm {jmp mProcs[8 * 4]} }
extern "C" __declspec(naked) void __stdcall SteamOverlayIsUsingMouse_wrapper() { __asm {jmp mProcs[9 * 4]} }
extern "C" __declspec(naked) void __stdcall ValveHookScreenshots_wrapper() { __asm {jmp mProcs[10 * 4]} }
extern "C" __declspec(naked) void __stdcall VirtualFreeWrapper_wrapper() { __asm {jmp mProcs[11 * 4]} }
extern "C" __declspec(naked) void __stdcall VulkanSteamOverlayPresent_wrapper() { __asm {jmp mProcs[12 * 4]} }
extern "C" __declspec(naked) void __stdcall VulkanSteamOverlayProcessCapturedFrame_wrapper() { __asm {jmp mProcs[13 * 4]} }