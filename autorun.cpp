#include <wtypes.h>
#include <stdio.h>
#include <shlwapi.h>
#include <ShObjIdl.h>
#include <ShlGuid.h>
#include "autorun.h"
#include "registry.h"
#include "utility.h"
#include "assert.h"
#include "string.h"
#include "cmdline.h"
#include "file.h"

typedef struct _autorun_t
{
	slist_t logon_list;
	slist_t url_search_hooks;
	slist_t bho_list;
	slist_t services;
	slist_t startup_list;
	slist_t shell_execute_hooks;
}autorun_t;

int autorun_query(AUTORUN_MASK mask)
{

}

autoitem_t* autorun_first(int fd);

autoitem_t* autorun_next(autoitem_t* iter);

int autorun_close(int fd);




BOOL SplitCmdLine( LPCTSTR lpszCmdline, AUTORUN_ITEM* pBootExecute )
{
	char* Pos1 = strchr( lpszCmdline, 0x20 );
	if( Pos1 == NULL ) return FALSE;
	strncpy( pBootExecute->Name, lpszCmdline, Pos1 - lpszCmdline );

	//获取系统目录
	GetSystemWindowsDirectory( pBootExecute->ImagePath, sizeof( pBootExecute->ImagePath ) - 1 );
	strncat( pBootExecute->ImagePath, "\\System32\\", sizeof( pBootExecute->ImagePath ) - strlen( pBootExecute->ImagePath) - 1 );
	char* Pos2 = strchr( Pos1 + 1 , 0x20 );
	if( Pos2 == NULL ) return FALSE;
	strncat( pBootExecute->ImagePath, Pos1 + 1 , Pos2 - Pos1 - 1 );
	PathAddExtension( pBootExecute->ImagePath, ".exe" );

	//获取命令行
	strncpy( pBootExecute->Paramter, Pos2 + 1, sizeof( pBootExecute->Paramter ) - 1 );

	return TRUE;
}

BOOL EnumBootExecute( AUTORUN_CALLBACK pfnCallback, LPVOID lpParam )
{
	CHKey hKey;
	DWORD dwResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,  "SYSTEM\\CurrentControlSet\\Control\\Session Manager",
															 0, KEY_QUERY_VALUE, (PHKEY)&hKey );
	if(ERROR_SUCCESS != dwResult ) {
		SetLastError( dwResult );
		return FALSE;
	}

	char Buffer[512] = {0};
	DWORD cbSize = sizeof( Buffer );
	dwResult = RegQueryValueEx( hKey.GetHandle(), "BootExecute", NULL, NULL, (PUCHAR)Buffer, &cbSize );
	if( dwResult != ERROR_SUCCESS ) {
		SetLastError( dwResult );
		return FALSE;
	}	

	//计算多少行
	DWORD cItem = 0;
	if( Buffer[0] != '\0' ) { 
		cItem++;
	}
	//枚举所有字符
	DWORD i = 0;
	while( !(Buffer[i] == '\0' && Buffer[i+1] == '\0' ) ) {
		if( Buffer[i] == '\0' ) {
			cItem ++;
		}
		i++;
	}
	
	DWORD j= 0;
	char *pos = Buffer;
	while( pos != NULL && *(pos + 1) != '\0' && j < cItem ) {
		AUTORUN_ITEM Item = {0};
		if( !SplitCmdLine( pos == Buffer ? pos : pos + 1, &Item )) {
			SetLastError( ERROR_BAD_FORMAT );
 			return FALSE;
		}

		if( !pfnCallback( AUTORUN_BOOT_EXECUTE, &Item, lpParam ) ) {
			SetLastError( ERROR_CANCELLED );
			return FALSE;
		}

		pos = strchr( pos + 1, '\0' );
		j++;
	}

	return TRUE;
}

BOOL EnumShell( AUTORUN_CALLBACK pfnCallback, LPVOID lpParam )
{
	CHKey hKey;
	DWORD dwResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
						"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
						0, KEY_QUERY_VALUE, (PHKEY)&hKey );
	if(ERROR_SUCCESS != dwResult ) {
		SetLastError( dwResult );
		return FALSE;
	}

	AUTORUN_ITEM Item = {0};

	DWORD cbBuffer = sizeof( Item.Name );
	dwResult = RegQueryValueEx( hKey.GetHandle(), "Shell", NULL, NULL, (UCHAR*)Item.Name, &cbBuffer);
	if( ERROR_SUCCESS != dwResult ) {
		SetLastError( dwResult );
		return FALSE;
	}

	//获取系统的路径
	GetSystemWindowsDirectory( Item.ImagePath, sizeof( Item.ImagePath ) - 1 );
	PathAddBackslash( Item.ImagePath );
	strncat( Item.ImagePath, Item.Name, sizeof( Item.ImagePath ) - strlen( Item.ImagePath ) - 1 );

	if( !pfnCallback( AUTORUN_SHELL, &Item, lpParam ) ) {
		SetLastError( ERROR_CANCELLED );
		return FALSE;
	}
	
	return TRUE;
}


BOOL EnumAutorun( AUTORUN_MASK Mask, AUTORUN_CALLBACK pfnCallback, LPVOID lpParam )
{
	if( Mask & AUTORUN_BHO ) {
		if( !EnumBHO( pfnCallback, lpParam ) && GetLastError() == ERROR_CANCELLED )
			//用户取消的话，返回， 否则继续
			return FALSE;
	}

	if( Mask & AUTORUN_SHELL_EXECUTE_HOOK ) {
		if( !EnumShellExecuteHook( pfnCallback, lpParam ) && GetLastError() == ERROR_CANCELLED ) 
			return FALSE;
	}

	if( Mask & AUTORUN_URL_SEARCH_HOOK ) {
		if( !EnumUrlSearchHook( pfnCallback, lpParam ) && GetLastError() == ERROR_CANCELLED ) 
			return FALSE;
	}

	if( Mask & AUTORUN_USERINIT ) {
		if( !EnumUserinit( pfnCallback, lpParam ) && GetLastError() == ERROR_CANCELLED ) 
			return FALSE;
	}

	if( Mask & AUTORUN_LOGON ) {
		if( !EnumLogon( pfnCallback, lpParam ) && GetLastError() == ERROR_CANCELLED ) 
			return FALSE;
	}

	if( Mask & AUTORUN_NETWORK_PROVIDER ) {
		if( !EnumNetworkProvider( pfnCallback, lpParam ) && GetLastError() == ERROR_CANCELLED  ) 
			return FALSE;
	}

	if( Mask & AUTORUN_GINA ) {
		if( !EnumGina( pfnCallback, lpParam ) && GetLastError() == ERROR_CANCELLED ) 
			return FALSE;
	}

	if( Mask & AUTORUN_SCREEN_SAVE ) {
		if( !EnumScrnSave( pfnCallback, lpParam ) && GetLastError() == ERROR_CANCELLED ) 
			return FALSE;
	}

	if( Mask & AUTORUN_KNOWNDLL ) {
		if( !EnumKnownDlls( pfnCallback, lpParam )  && GetLastError() == ERROR_CANCELLED ) 
			return FALSE;
	}

	if( Mask & AUTORUN_IMAGE_HIJACK ) {
		if( !EnumImageHijack( pfnCallback, lpParam ) && GetLastError() == ERROR_CANCELLED ) 
			return FALSE;
	}

	if( Mask & AUTORUN_BOOT_EXECUTE ) {
		if( !EnumBootExecute( pfnCallback, lpParam ) && GetLastError() == ERROR_CANCELLED ) 
			return FALSE;
	}

	if( Mask & AUTORUN_SHELL ) {
		if( !EnumShell( pfnCallback, lpParam ) && GetLastError() == ERROR_CANCELLED ) 
			return FALSE;
	}

	if( Mask & AUTORUN_STARTUP ) {
		if( !EnumStartup( pfnCallback, lpParam ) && GetLastError() == ERROR_CANCELLED ) 
			return FALSE;
	}
	
	if( Mask & AUTORUN_SERVICE ) {
		if( !EnumService( pfnCallback, lpParam, TRUE ) && GetLastError() == ERROR_CANCELLED ) 
			return FALSE;
	}

	if( Mask & AUTORUN_DRIVER ) {
		if( !EnumService( pfnCallback, lpParam, FALSE ) ) 
			return FALSE;
	}

	return TRUE;
}


LPCTSTR GetAutorunName( AUTORUN_MASK Mask )
{
	switch( Mask )
	{
	case AUTORUN_BHO:
		return "BHO";
		break;
	case AUTORUN_SHELL_EXECUTE_HOOK:
		return "Shell Execute Hook";
		break;
	case AUTORUN_URL_SEARCH_HOOK:
		return "Url Search Hook";
	    break;
	case AUTORUN_USERINIT:
		return "Userinit";
	    break;
	case AUTORUN_LOGON:
		return "Logon";
		break;
	case AUTORUN_NETWORK_PROVIDER:
		return "Network Provider";
		break;
	case AUTORUN_APPINIT:
		return "Appinit";
	    break;
	case AUTORUN_GINA:
		return "Gina";
	    break;
	case AUTORUN_SCREEN_SAVE:
		return "ScreenSave";
		break;
	case AUTORUN_KNOWNDLL:
		return "KnownDll";
		break;
	case AUTORUN_IMAGE_HIJACK:
		return "ImageHijack";
		break;
	case AUTORUN_BOOT_EXECUTE:
		return "Boot Execute";
		break;
	case AUTORUN_SHELL:
		return "Shell";
		break;
	case AUTORUN_STARTUP:
		return "Startup";
		break;
	case AUTORUN_SERVICE:
		return "Services";
		break;
	case AUTORUN_DRIVER:
		return "Drivers";
		break;
	default:
		return NULL;
	    break;
	}
}

