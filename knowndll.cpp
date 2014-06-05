


BOOL EnumKnownDlls( AUTORUN_CALLBACK pfnCallback, LPVOID lpParam )
{
	CHKey hKey;
	DWORD dwResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
						"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\KnownDLLs",
						0, KEY_QUERY_VALUE, (PHKEY)&hKey );
	if(ERROR_SUCCESS != dwResult ) {
		SetLastError( dwResult );
		return FALSE;
	}

	char Buffer[512] = {0};
	DWORD cbSize = sizeof( Buffer );
	dwResult = RegQueryValueEx( hKey.GetHandle(), "DllDirectory", NULL, NULL, (UCHAR*)Buffer, &cbSize );
	if( ERROR_SUCCESS != dwResult ) {
		SetLastError( dwResult );
		return FALSE;
	}

	char szSystemDir[512] = {0};
	cbSize = sizeof( szSystemDir );
	ExpandEnvironmentStrings( Buffer, szSystemDir, sizeof( szSystemDir ) - 1 );
	PathAddBackslash( szSystemDir );

	//获取子键数量
	DWORD cValue = 0;
	dwResult = RegQueryInfoKey( hKey.GetHandle(), NULL, NULL, 0, NULL, NULL,NULL, &cValue, NULL, NULL, NULL, NULL );
	if( ERROR_SUCCESS != dwResult ) {
		SetLastError( dwResult );
		return FALSE;
	}

	DWORD j = 0;
	for( DWORD i = 0; i < cValue; i++ ) {
		AUTORUN_ITEM Item = {0};
		char Data[512] = {0};
		DWORD cbName = sizeof( Item.Name );
		DWORD cbData = sizeof( Data );
		dwResult = RegEnumValue( hKey.GetHandle(), i , Item.Name, &cbName, NULL, NULL, (PUCHAR)&Data, &cbData );
		if( dwResult != ERROR_SUCCESS ) {
			SetLastError( dwResult );
			return FALSE;
		}

		if( 0 == strnicmp( Item.Name, "DllDirectory", sizeof( "DllDirectory" ))) {
			continue;
		}
		
		strncpy( Item.ImagePath, szSystemDir, sizeof( Item.ImagePath ) - 1 );
		strncat( Item.ImagePath, Data, sizeof( Item.ImagePath ) - strlen( Item.ImagePath ) - 1 );

		if( !pfnCallback( AUTORUN_KNOWNDLL, &Item, lpParam )) {
			SetLastError( ERROR_CANCELLED );
			return FALSE;
		}

		j++;
	}

	return TRUE;
}
