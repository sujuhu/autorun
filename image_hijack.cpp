
BOOL EnumImageHijack( AUTORUN_CALLBACK pfnCallback, LPVOID lpParam )
{
	CHKey hKey;
	DWORD dwResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
						"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options",
						0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, (PHKEY)&hKey );
	if(ERROR_SUCCESS != dwResult ) {
		SetLastError( dwResult );
		return FALSE;
	}

	//查询子键数量
	DWORD cSubKey = 0;
	dwResult = RegQueryInfoKey( hKey.GetHandle(), NULL, NULL, 0, &cSubKey, NULL,NULL, NULL, NULL, NULL, NULL, NULL );
	if( ERROR_SUCCESS != dwResult )	{
		SetLastError( dwResult );
		return FALSE;
	}

	LPCTSTR lpszSystemDir = getenv( "SystemRoot" );
	if( lpszSystemDir == NULL ) 
		return FALSE;

	for( DWORD i=0; i < cSubKey; i++ ) {

		AUTORUN_ITEM Item = {0};
		dwResult = RegEnumKey( hKey.GetHandle(), i, Item.Name, sizeof( Item.Name ) );
		if( ERROR_SUCCESS != dwResult ) {
			SetLastError( dwResult );
			continue;
		}
		
		//打开子键
		CHKey hSubKey ;
		dwResult = RegOpenKeyEx( hKey.GetHandle(), Item.Name, 0, KEY_QUERY_VALUE, (PHKEY)&hSubKey );
		if( ERROR_SUCCESS != dwResult ){
			SetLastError( dwResult );
			continue;
		}

		char Buffer[512] = {0};
		DWORD cbSize = sizeof( Buffer );
		dwResult = RegQueryValueEx( hSubKey.GetHandle(), "Debugger", NULL, NULL, (PUCHAR)Buffer, &cbSize );
		if( dwResult != ERROR_SUCCESS ) {
			SetLastError( dwResult );
			continue;
		}

		strncpy( Item.ImagePath, lpszSystemDir, sizeof( Item.ImagePath ) - 1 );
		PathAddBackslash( Item.ImagePath );
		strncat( Item.ImagePath, "System32\\", sizeof( Item.ImagePath ) - strlen( Item.ImagePath ) - 1 );
		strncat( Item.ImagePath, Buffer, sizeof( Item.ImagePath ) - strlen( Item.ImagePath ) - 1 );

		//去掉删除
		PathRemoveArgs( Item.ImagePath );
		PathAddExtension( Item.ImagePath, ".exe" );

		if( !pfnCallback( AUTORUN_IMAGE_HIJACK, &Item, lpParam ) ) {
			SetLastError( ERROR_CANCELLED );
			return FALSE;
		}	
	}

	return TRUE;
}
