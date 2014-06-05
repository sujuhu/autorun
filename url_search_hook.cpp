
BOOL EnumUrlSearchHook( AUTORUN_CALLBACK pfnCallback, LPVOID lpParam )
{
	CHKey hKey;
	DWORD dwResult = RegOpenKeyEx( HKEY_CURRENT_USER, 
						"Software\\Microsoft\\Internet Explorer\\URLSearchHooks",
						0, KEY_QUERY_VALUE, (PHKEY)&hKey );
	if( ERROR_SUCCESS != dwResult ) {
		SetLastError( dwResult );
		return FALSE;
	}
	
	DWORD cValue = 0;
	dwResult = RegQueryInfoKey( hKey.GetHandle(), NULL, NULL, 0, NULL, NULL,NULL, &cValue, NULL, NULL, NULL, NULL );
	if( ERROR_SUCCESS != dwResult ) {
		SetLastError( dwResult );
		return FALSE;
	}

	for( DWORD i=0; i < cValue; i++ ) {

		AUTORUN_ITEM Item = {0};
		DWORD cbBuffer = sizeof( Item.Name );
		dwResult =  RegEnumValue( hKey.GetHandle(), i, Item.Name, &cbBuffer, NULL, NULL, NULL, NULL );
		if( ERROR_SUCCESS != dwResult ) {
			SetLastError( dwResult );
			return FALSE;
		}

		CHKey	hSubKey;
		char szSubKey[128] ={0};
		strncpy( szSubKey, "CLSID\\", sizeof( szSubKey ) - 1 );
		strncat( szSubKey, Item.Name, sizeof( szSubKey ) - strlen( szSubKey ) - 1 );
		strncat( szSubKey, "\\InprocServer32", sizeof( szSubKey ) - strlen( szSubKey ) - 1 );
		
		DWORD dwResult = RegOpenKeyEx( HKEY_CLASSES_ROOT, szSubKey, 0, KEY_QUERY_VALUE, (PHKEY)&hSubKey );
		if( ERROR_SUCCESS != dwResult ) 	{
			SetLastError( dwResult );
			return FALSE;
		}
		
		cbBuffer = sizeof( Item.ImagePath );
		dwResult = RegQueryValueEx( hSubKey.GetHandle(), "", 0, NULL ,(UCHAR*)Item.ImagePath, &cbBuffer );
		if( ERROR_SUCCESS != dwResult )  {
			SetLastError( dwResult );
			return FALSE;
		}

		//替换Systemroot
		char Buffer[512] = {0};
		ExpandEnvironmentStrings( Item.ImagePath, Buffer, sizeof( Buffer) - 1 );
		strncpy( Item.ImagePath, Buffer, sizeof( Item.ImagePath ) - 1 );

		if( !pfnCallback( AUTORUN_URL_SEARCH_HOOK,  &Item, lpParam ) ) {
			SetLastError( ERROR_CANCELLED );
			return FALSE;
		}
	}

	return TRUE;
}