

BOOL EnumGina( AUTORUN_CALLBACK pfnCallback, LPVOID lpParam )
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
	dwResult = RegQueryValueEx( hKey.GetHandle(), "GinaDll", NULL, NULL, (UCHAR*)Item.Name, &cbBuffer);
	if( ERROR_SUCCESS != dwResult ) {
		SetLastError( dwResult );
		return FALSE;
	}

	//获取系统的路径
	GetSystemWindowsDirectory( Item.ImagePath, sizeof( Item.ImagePath ) - 1 );
	PathAddBackslash( Item.ImagePath );
	strncat( Item.ImagePath, "System32\\", sizeof( Item.ImagePath ) - strlen( Item.ImagePath ) - 1 );
	strncat( Item.ImagePath, Item.Name, sizeof( Item.ImagePath ) - strlen( Item.ImagePath ) - 1 );

	if( !pfnCallback( AUTORUN_GINA, &Item, lpParam ) ) {
		SetLastError( ERROR_CANCELLED );
		return FALSE;
	}
	
	return TRUE;
}
