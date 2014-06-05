

BOOL EnumShellExecuteHook( AUTORUN_CALLBACK pfnCallback, LPVOID lpParam )
{
	CHKey hKey;
	DWORD dwResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
						"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellExecuteHooks",
						0, KEY_QUERY_VALUE, (PHKEY)&hKey );
	if(ERROR_SUCCESS !=  dwResult ) {
		SetLastError( dwResult );
		return FALSE;
	}

	DWORD cValue = 0;
	dwResult = RegQueryInfoKey( hKey.GetHandle(), NULL, NULL, 0, NULL, NULL,NULL, &cValue, NULL, NULL, NULL, NULL );
	if( ERROR_SUCCESS != dwResult) {
		SetLastError( dwResult );
		return FALSE;
	}

	for( DWORD i = 0; i < cValue; i++ ) {
		AUTORUN_ITEM Item = {0};

		char	ClassId[40] = {0};
		DWORD cbBufSize = sizeof( ClassId );
		dwResult = RegEnumValue( hKey.GetHandle(), i, ClassId, &cbBufSize, NULL, NULL, NULL, NULL );
		if( ERROR_SUCCESS != dwResult) {
			SetLastError( dwResult );
			return FALSE;
		}

		CHKey	hSubKey;
		char szSubKey[128] ={0};
		strncpy( szSubKey, "CLSID\\", sizeof( szSubKey ) - 1 );
		strncat( szSubKey, ClassId, sizeof( szSubKey ) - strlen( szSubKey ) - 1 );
		strncat( szSubKey, "\\InprocServer32", sizeof( szSubKey ) - strlen( szSubKey ) - 1 );

		dwResult = RegOpenKeyEx( HKEY_CLASSES_ROOT, szSubKey, 0, KEY_QUERY_VALUE, (PHKEY)&hSubKey );
		if( ERROR_SUCCESS != dwResult) {
			SetLastError( dwResult );
			return FALSE;
		}
		
		char Buffer[512] = {0};
		cbBufSize = sizeof( Buffer );
		dwResult = RegQueryValueEx( hSubKey.GetHandle(), "", 0, NULL ,(UCHAR*)Buffer, &cbBufSize );
		if( ERROR_SUCCESS != dwResult ) {
			SetLastError( dwResult );
			return FALSE;
		}

		char szImagePath[512]  = {0};
		ParseCmdLine( Buffer, szImagePath, sizeof( szImagePath ) );
		GetLongPathName( szImagePath, Item.ImagePath, sizeof( Item.ImagePath ) );

		if( !pfnCallback( AUTORUN_SHELL_EXECUTE_HOOK, &Item, lpParam ) ) {
			SetLastError( ERROR_CANCELLED );
			return FALSE;
		}
	}
	
	return TRUE;
}