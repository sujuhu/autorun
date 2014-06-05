

BOOL EnumScrnSave( AUTORUN_CALLBACK pfnCallback, LPVOID lpParam )
{
	CHKey hKey;
	DWORD dwResult = RegOpenKeyEx( HKEY_CURRENT_USER,  "Control Panel\\Desktop",
									0, KEY_QUERY_VALUE, (PHKEY)&hKey );
	if(ERROR_SUCCESS != dwResult ) {
		SetLastError( dwResult );
		return FALSE;
	}

	AUTORUN_ITEM Item = {0};

	DWORD cbBuffer = sizeof( Item.ImagePath );
	dwResult = RegQueryValueEx( hKey.GetHandle(), "SCRNSAVE.EXE", NULL, NULL, (UCHAR*)Item.ImagePath, &cbBuffer);
	if( ERROR_SUCCESS != dwResult ) {
		SetLastError( dwResult );
		return FALSE;
	}

	if( !pfnCallback( AUTORUN_SCREEN_SAVE, &Item, lpParam )) {
		SetLastError( ERROR_CANCELLED );
		return FALSE;
	}

	return TRUE;
}