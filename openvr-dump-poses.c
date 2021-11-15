#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#undef EXTERN_C
#include "openvr_capi.h"

intptr_t VR_InitInternal( EVRInitError *peError, EVRApplicationType eType );
void VR_ShutdownInternal();
bool VR_IsHmdPresent();
intptr_t VR_GetGenericInterface( const char *pchInterfaceVersion, EVRInitError *peError );
bool VR_IsRuntimeInstalled();
const char * VR_GetVRInitErrorAsSymbol( EVRInitError error );
const char * VR_GetVRInitErrorAsEnglishDescription( EVRInitError error );

struct VR_IVRSystem_FnTable * oSystem;

VROverlayHandle_t ulHandle;

void * CNOVRGetOpenVRFunctionTable( const char * interfacename )
{
	EVRInitError e;
	char fnTableName[128];
	int result1 = snprintf( fnTableName, 128, "FnTable:%s", interfacename );
	void * ret = (void *)VR_GetGenericInterface(fnTableName, &e );
	printf( "Getting System FnTable: %s = %p (%d)\n", fnTableName, ret, e );
	if( !ret )
	{
		exit( 1 );
	}
	return ret;
}

int main()
{
	{
        EVRInitError e;
        VR_InitInternal( &e, EVRApplicationType_VRApplication_Utility );
        if( e != EVRInitError_VRInitError_None )
        {
            printf( "Error initializing OpenVR %d\n", e );
            return -5;
        }
    }

    printf( "OpenVR Started Successfully\n" );

    oSystem = (struct VR_IVRSystem_FnTable *)CNOVRGetOpenVRFunctionTable( IVRSystem_Version );

    if( !oSystem )
    {
        printf( "Error getting function tables from OpenVR\n" );
        return -9;
    }

	FILE * f = fopen( "poselog.csv", "w" );

	while(1)
	{
		struct TrackedDevicePose_t poses[16];
		oSystem->GetDeviceToAbsoluteTrackingPose( 
				ETrackingUniverseOrigin_TrackingUniverseStanding, 
				.022, poses+0, 8);
		oSystem->GetDeviceToAbsoluteTrackingPose( 
				ETrackingUniverseOrigin_TrackingUniverseStanding, 
				-.022, poses+8, 8);
				
		int i;
		int got = 0;
		for( i = 0 ; i < sizeof(poses)/sizeof(poses[0]); i++ )
		{
			struct TrackedDevicePose_t * p = poses+i;
			if( p->bPoseIsValid )
			{
				struct HmdMatrix34_t m = p->mDeviceToAbsoluteTracking;
				fprintf( f, "%d\t%f\t%f\t%f\t", i, m.m[0][3], m.m[1][3], m.m[2][3] );
				got++;
			}
		}
		Sleep( 1 );
		fprintf( f, "\n" );
		printf( "Got %d devices\n", got );
	}
}